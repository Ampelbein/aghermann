// ;-*-C++-*-
/*
 *       File name:  ui/expdesign-settings_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  SExpDesignUI settings tab callbacks
 *
 *         License:  GPL
 */


#include "../common/string.hh"
#include "misc.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"

using namespace std;
using namespace aghui;



extern "C"
void
tDesign_switch_page_cb( GtkNotebook     *notebook,
			gpointer	 unused,
			guint            page_num,
			gpointer         userdata)
{
	using namespace sigfile;
	auto& ED = *(SExpDesignUI*)userdata;

      // save parameters changing which should trigger tree rescan
	if ( page_num == 0 ) {  // switching back from settings tab

	      // collect values from widgets
		ED.W_V.down();

		// Profile tab

		ED.ED->fft_params.pagesize = ED.FFTPageSizeValues[ED.pagesize_item];
		ED.ED->fft_params.binsize =  ED.FFTBinSizeValues [ED.binsize_item];

		try { ED.ED->fft_params.check(); }
		catch (invalid_argument ex) {
			pop_ok_message( ED.wMainWindow,
					"Invalid FFT parameters", "Resetting to defaults.");
			ED.ED->fft_params.reset();
		}

		try {
			ED.ED->mc_params.check( ED.ED->fft_params.pagesize);
		} catch (invalid_argument ex) {
			pop_ok_message( ED.wMainWindow, "Invalid uC parameters", "Resetting to defaults.");
			ED.ED->mc_params.reset();
		}

		ED.__adjust_op_freq_spinbuttons();

	      // scan as necessary
		if ( ED.pagesize_item_saved	  		!= ED.pagesize_item ||
		     ED.binsize_item_saved	  		!= ED.binsize_item ||
		     ED.fft_params_welch_window_type_saved  	!= ED.ED->fft_params.welch_window_type ||
		     ED.af_dampen_window_type_saved  		!= ED.ED->af_dampen_window_type ||
		     ED.af_dampen_factor_saved	  		!= ED.ED->af_dampen_factor ||
		     !(ED.ED->mc_params				== ED.mc_params_saved) ) {
		      // rescan tree
			ED.do_rescan_tree(); // with populate
		} else if ( ED.timeline_height_saved			!= ED.timeline_height ||
	      // recalculte mesurements layout as necessary
			    ED.timeline_pph_saved			!= ED.timeline_pph )
			ED.populate_1();
	} else {
		ED.timeline_pph_saved			= ED.timeline_pph;
		ED.timeline_height_saved		= ED.timeline_height;
		ED.pagesize_item_saved			= ED.pagesize_item;
		ED.binsize_item_saved			= ED.binsize_item;
		ED.fft_params_welch_window_type_saved	= ED.ED->fft_params.welch_window_type;
		ED.af_dampen_window_type_saved		= ED.ED->af_dampen_window_type;
		ED.af_dampen_factor_saved		= ED.ED->af_dampen_factor;
		ED.mc_params_saved			= ED.ED->mc_params;

		ED.pagesize_item = ED.figure_pagesize_item();
		ED.binsize_item = ED.figure_binsize_item();

		ED.W_V.up();

		// colours are served specially elsewhere
	}
}







// ================== Simulations part



inline namespace {
void
__widgets_to_tunables( SExpDesignUI& ED)
{
	using namespace agh::ach;
	// don't mess with classed enums!
	for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t ) {
		ED.ED->tunables0 [t] = gtk_spin_button_get_value( ED.eTunable[t][0]) / stock[t].display_scale_factor;
		ED.ED->tlo       [t] = gtk_spin_button_get_value( ED.eTunable[t][1]) / stock[t].display_scale_factor;
		ED.ED->thi       [t] = gtk_spin_button_get_value( ED.eTunable[t][2]) / stock[t].display_scale_factor;
		ED.ED->tstep     [t] = gtk_spin_button_get_value( ED.eTunable[t][3]) / stock[t].display_scale_factor;
	}
}


void
__tunables_to_widgets( SExpDesignUI& ED)
{
	using namespace agh::ach;
	for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t ) {
		// gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::val ],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.value[t]);
		// gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::min ],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.lo   [t]);
		// gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::max ],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.hi   [t]);
		// gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::step],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.step [t]);

		gtk_adjustment_configure( ED.jTunable[t][0],
					  stock[t].display_scale_factor * ED.ED->tunables0[t],
					  stock[t].display_scale_factor * ED.ED->tlo[t],
					  stock[t].display_scale_factor * ED.ED->thi[t],
					  stock[t].adj_step, 10 * stock[t].adj_step,
					  0);
		gtk_adjustment_configure( ED.jTunable[t][1],
					  stock[t].display_scale_factor * ED.ED->tlo[t],
					  0,
					  stock[t].display_scale_factor * ED.ED->thi[t],
					  stock[t].adj_step, 10 * stock[t].adj_step,
					  0);
		gtk_adjustment_configure( ED.jTunable[t][2],
					  stock[t].display_scale_factor * ED.ED->thi[t],
					  stock[t].display_scale_factor * ED.ED->tlo[t],
					  stock[t].display_scale_factor * ED.ED->thi[t] * 1.5,
					  stock[t].adj_step, 10 * stock[t].adj_step,
					  0);
		gtk_adjustment_configure( ED.jTunable[t][3],
					  stock[t].display_scale_factor * ED.ED->tstep[t],
					  stock[t].display_scale_factor * ED.ED->tunables0[t] / 1000.,
					  stock[t].display_scale_factor * ED.ED->tunables0[t] / 2,
					  stock[t].adj_step, 10 * stock[t].adj_step,
					  0);
	}
}

} // inline namespace

extern "C"
void
tSimulations_switch_page_cb( GtkNotebook     *notebook,
			     gpointer	      page,
			     guint            page_num,
			     gpointer         userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	if ( page_num == 1 ) {  // switching to display parameters tab
	      // Controlling parameters frame
		gtk_spin_button_set_value( ED.eCtlParamAnnlNTries,	ED.ED->ctl_params0.siman_params.n_tries);
		gtk_spin_button_set_value( ED.eCtlParamAnnlItersFixedT,	ED.ED->ctl_params0.siman_params.iters_fixed_T);
		gtk_spin_button_set_value( ED.eCtlParamAnnlStepSize,	ED.ED->ctl_params0.siman_params.step_size);
		gtk_spin_button_set_value( ED.eCtlParamAnnlBoltzmannk,	ED.ED->ctl_params0.siman_params.k);
		gtk_spin_button_set_value( ED.eCtlParamAnnlDampingMu,	ED.ED->ctl_params0.siman_params.mu_t);
		float mantissa;
		int exponent;
		agh::str::decompose_double( ED.ED->ctl_params0.siman_params.t_min, &mantissa, &exponent);
		gtk_spin_button_set_value( ED.eCtlParamAnnlTMinMantissa,	mantissa);
		gtk_spin_button_set_value( ED.eCtlParamAnnlTMinExponent,	exponent);
		agh::str::decompose_double( ED.ED->ctl_params0.siman_params.t_initial, &mantissa, &exponent);
		gtk_spin_button_set_value( ED.eCtlParamAnnlTInitialMantissa,	mantissa);
		gtk_spin_button_set_value( ED.eCtlParamAnnlTInitialExponent,	exponent);

	      // Achermann parameters
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamDBAmendment1, !ED.ED->ctl_params0.DBAmendment1); // force emission of the toggle signal
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamDBAmendment2, !ED.ED->ctl_params0.DBAmendment2);
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamAZAmendment1, !ED.ED->ctl_params0.AZAmendment1);
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamAZAmendment2, !ED.ED->ctl_params0.AZAmendment2);

		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamDBAmendment1, ED.ED->ctl_params0.DBAmendment1);
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamDBAmendment2, ED.ED->ctl_params0.DBAmendment2);
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamAZAmendment1, ED.ED->ctl_params0.AZAmendment1);
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamAZAmendment2, ED.ED->ctl_params0.AZAmendment2);
		gtk_spin_button_set_value( ED.eCtlParamNSWAPpBeforeSimStart, ED.ED->ctl_params0.swa_laden_pages_before_SWA_0);
		gtk_spin_button_set_value( ED.eCtlParamReqScoredPercent, ED.ED->ctl_params0.req_percent_scored);

	      // Unconventional scores frame
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamScoreUnscoredAsWake,
					      ED.ED->ctl_params0.ScoreUnscoredAsWake);

	      // Tunables tab
		__tunables_to_widgets( ED);

	} else {
	      // Controlling parameters frame
		ED.ED->ctl_params0.siman_params.n_tries       = gtk_spin_button_get_value( ED.eCtlParamAnnlNTries);
		ED.ED->ctl_params0.siman_params.iters_fixed_T = gtk_spin_button_get_value( ED.eCtlParamAnnlItersFixedT);
		ED.ED->ctl_params0.siman_params.step_size     = gtk_spin_button_get_value( ED.eCtlParamAnnlStepSize);
		ED.ED->ctl_params0.siman_params.k             = gtk_spin_button_get_value( ED.eCtlParamAnnlBoltzmannk);
		ED.ED->ctl_params0.siman_params.mu_t          = gtk_spin_button_get_value( ED.eCtlParamAnnlDampingMu);
		ED.ED->ctl_params0.siman_params.t_initial     = gtk_spin_button_get_value( ED.eCtlParamAnnlTInitialMantissa)
			* pow(10, gtk_spin_button_get_value( ED.eCtlParamAnnlTInitialExponent));
		ED.ED->ctl_params0.siman_params.t_min	     = gtk_spin_button_get_value( ED.eCtlParamAnnlTMinMantissa)
			* pow(10, gtk_spin_button_get_value( ED.eCtlParamAnnlTMinExponent));
	      // Achermann parameters
		ED.ED->ctl_params0.DBAmendment1 = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eCtlParamDBAmendment1);
		ED.ED->ctl_params0.DBAmendment2 = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eCtlParamDBAmendment2);
		ED.ED->ctl_params0.AZAmendment1 = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eCtlParamAZAmendment1);
		ED.ED->ctl_params0.AZAmendment2 = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eCtlParamAZAmendment2);
		ED.ED->ctl_params0.swa_laden_pages_before_SWA_0	= gtk_spin_button_get_value( ED.eCtlParamNSWAPpBeforeSimStart);
		ED.ED->ctl_params0.req_percent_scored		= gtk_spin_button_get_value( ED.eCtlParamReqScoredPercent);

	      // Unconventional scores frame
		ED.ED->ctl_params0.ScoreUnscoredAsWake = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eCtlParamScoreUnscoredAsWake);

	      // Tunables tab
		__widgets_to_tunables( ED);

	      // for ctlparam changes to take effect on virgin modruns
		ED.populate_2();
	}
}






	// // possibly for some live validation; unused for now
	// void eCtlParamAnnlNTries_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	// void eCtlParamAnnlItersFixedT_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	// void eCtlParamAnnlStepSize_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	// void eCtlParamAnnlBoltzmannk_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	// void eCtlParamAnnlTInitial_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	// void eCtlParamAnnlDampingMu_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	// void eCtlParamAnnlTMinMantissa_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	// void eCtlParamAnnlTMinExponent_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	// void eCtlParamScoreMVTAs_toggled_cb( GtkToggleButton *e, gpointer u)		{ }
	// void eCtlParamScoreUnscoredAs_toggled_cb( GtkToggleButton *e, gpointer u)	{ }



extern "C"
void
bSimParamRevertTunables_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.ED->tunables0.set_defaults();
	__tunables_to_widgets( ED);
}


extern "C"
void
eCtlParamDBAmendment1_toggled_cb( GtkToggleButton *button, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_label_set_markup( ED.lCtlParamDBAmendment1,
			      gtk_toggle_button_get_active( button)
			      ? "<small>Let SWA be affected by <i>S</i> at all times</small>"
			      : "<small>Cancel <i>rc</i>-dependent SWA increase in Wake</small>");
}
extern "C"
void
eCtlParamDBAmendment2_toggled_cb( GtkToggleButton *button, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_label_set_markup( ED.lCtlParamDBAmendment2,
			      gtk_toggle_button_get_active( button)
			      ? "<small>Assume sleep homeostat is stable (<i>S</i><sub>24h</sub> = <i>S</i><sub>0</sub>)</small>"
			      : "<small>Don't assume <i>S</i><sub>24h</sub> = <i>S</i><sub>0</sub></small>");
}
extern "C"
void
eCtlParamAZAmendment1_toggled_cb( GtkToggleButton *button, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_label_set_markup( ED.lCtlParamAZAmendment1,
			      gtk_toggle_button_get_active( button)
			      ? "<small>Compute <i>gc</i> per-episode</small>"
			      : "<small>Assume <i>gc</i> is not variable across episodes</small>");
}
extern "C"
void
eCtlParamAZAmendment2_toggled_cb( GtkToggleButton *button, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_label_set_markup( ED.lCtlParamAZAmendment2,
			      gtk_toggle_button_get_active( button)
			      ? "<small>(has no effect yet)</small>"
			      : "<small>(reserved)</small>");
}


// eof
