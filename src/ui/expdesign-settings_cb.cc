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


#include "misc.hh"
#include "ui.hh"
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
		// Profile tab
		ED.ED->af_dampen_window_type =
			(SFFTParamSet::TWinType)gtk_combo_box_get_active( ED.eArtifDampenWindowType);
		ED.ED->af_dampen_factor =
			gtk_spin_button_get_value( ED.eArtifDampenFactor);

		ED.ED->fft_params.pagesize =
			ED.FFTPageSizeValues[ ED.pagesize_item = gtk_combo_box_get_active( ED.eFFTParamsPageSize)];
		ED.ED->fft_params.binsize =
			ED.FFTBinSizeValues[ ED.binsize_item = gtk_combo_box_get_active( ED.eFFTParamsBinSize)];
		ED.ED->fft_params.welch_window_type =
			(SFFTParamSet::TWinType)gtk_combo_box_get_active( ED.eFFTParamsWindowType);
		try { ED.ED->fft_params.check(); }
		catch (invalid_argument ex) {
			pop_ok_message( ED.wMainWindow, "Invalid FFT parameters; resetting to defaults.");
			ED.ED->fft_params.reset();
		}

		ED.ED->mc_params.bandwidth		=         gtk_spin_button_get_value( ED.eMCParamBandWidth);
		ED.ED->mc_params.iir_backpolate		=         gtk_spin_button_get_value( ED.eMCParamIIRBackpolate);
		ED.ED->mc_params.mc_gain		=         gtk_spin_button_get_value( ED.eMCParamMCGain);
		try {
			ED.ED->mc_params.check( ED.ED->fft_params.pagesize);
		} catch (invalid_argument ex) {
			pop_ok_message( ED.wMainWindow, "Invalid MC parameters; resetting to defaults.");
			ED.ED->mc_params.reset( ED.ED->fft_params.pagesize);
		}

		switch ( ED.display_profile_type ) {
		case sigfile::Psd:
			gtk_adjustment_set_step_increment( ED.jMsmtOpFreqFrom,  ED.ED->fft_params.binsize);
			gtk_adjustment_set_step_increment( ED.jMsmtOpFreqWidth, ED.ED->fft_params.binsize);
		    break;
		case sigfile::Mc:
			gtk_adjustment_set_step_increment( ED.jMsmtOpFreqFrom,  ED.ED->mc_params.bandwidth);
			gtk_adjustment_set_step_increment( ED.jMsmtOpFreqWidth, ED.ED->mc_params.bandwidth);
		    break;
		}

		// General tab
		for ( gushort i = 0; i < (size_t)SPage::TScore::_total; ++i )
			ED.ext_score_codes[i] = gtk_entry_get_text( ED.eScoreCode[i]);

		ED.freq_bands[TBand::delta][0] = gtk_spin_button_get_value( ED.eBand[TBand::delta][0]);
		ED.freq_bands[TBand::delta][1] = gtk_spin_button_get_value( ED.eBand[TBand::delta][1]);
		ED.freq_bands[TBand::theta][0] = gtk_spin_button_get_value( ED.eBand[TBand::theta][0]);
		ED.freq_bands[TBand::theta][1] = gtk_spin_button_get_value( ED.eBand[TBand::theta][1]);
		ED.freq_bands[TBand::alpha][0] = gtk_spin_button_get_value( ED.eBand[TBand::alpha][0]);
		ED.freq_bands[TBand::alpha][1] = gtk_spin_button_get_value( ED.eBand[TBand::alpha][1]);
		ED.freq_bands[TBand::beta ][0] = gtk_spin_button_get_value( ED.eBand[TBand::beta ][0]);
		ED.freq_bands[TBand::beta ][1] = gtk_spin_button_get_value( ED.eBand[TBand::beta ][1]);
		ED.freq_bands[TBand::gamma][0] = gtk_spin_button_get_value( ED.eBand[TBand::gamma][0]);
		ED.freq_bands[TBand::gamma][1] = gtk_spin_button_get_value( ED.eBand[TBand::gamma][1]);

		SScoringFacility::IntersignalSpace	= gtk_spin_button_get_value( ED.eDAPageHeight);
		SScoringFacility::HypnogramHeight	= gtk_spin_button_get_value( ED.eDAHypnogramHeight);
		SScoringFacility::EMGProfileHeight	= gtk_spin_button_get_value( ED.eDAEMGHeight);

		ED.browse_command.assign( gtk_entry_get_text( ED.eBrowseCommand));

	      // scan as necessary
		if ( ED.pagesize_item_saved	  		!= ED.pagesize_item ||
		     ED.binsize_item_saved	  		!= ED.binsize_item ||
		     ED.fft_params_welch_window_type_saved  	!= ED.ED->fft_params.welch_window_type ||
		     ED.af_dampen_window_type_saved  		!= ED.ED->af_dampen_window_type ||
		     ED.af_dampen_factor_saved	  		!= ED.ED->af_dampen_factor ||
		     !(ED.ED->mc_params				== ED.mc_params_saved) ) {
		      // rescan tree
			ED.do_rescan_tree(); // with populate
		}
	} else {
		ED.pagesize_item_saved			= ED.pagesize_item;
		ED.binsize_item_saved			= ED.binsize_item;
		ED.fft_params_welch_window_type_saved	= ED.ED->fft_params.welch_window_type;
		ED.af_dampen_window_type_saved		= ED.ED->af_dampen_window_type;
		ED.af_dampen_factor_saved		= ED.ED->af_dampen_factor;
		ED.mc_params_saved			= ED.ED->mc_params;

	      // also assign values to widgets
		// -- maybe not? None of them are changeable by user outside settings tab
		// -- rather do: they are loaded at init
		// Profile tab
		guint i = 0;
		while ( SExpDesignUI::FFTPageSizeValues[i] < ED.ED->fft_params.pagesize )
			++i;
		gtk_combo_box_set_active( ED.eFFTParamsPageSize, ED.pagesize_item = i);
		i = 0;
		while ( SExpDesignUI::FFTBinSizeValues[i] < ED.ED->fft_params.binsize )
			++i;
		gtk_combo_box_set_active( ED.eFFTParamsBinSize, ED.binsize_item = i);
		gtk_combo_box_set_active( ED.eFFTParamsWindowType, (int)ED.ED->fft_params.welch_window_type);

		gtk_spin_button_set_value( ED.eMCParamIIRBackpolate,	ED.ED->mc_params.iir_backpolate);
		gtk_spin_button_set_value( ED.eMCParamMCGain,		ED.ED->mc_params.mc_gain);
		gtk_spin_button_set_value( ED.eMCParamBandWidth,	ED.ED->mc_params.bandwidth);

		// artifacts
		gtk_combo_box_set_active( ED.eArtifDampenWindowType, (int)ED.ED->af_dampen_window_type);
		gtk_spin_button_set_value( ED.eArtifDampenFactor,	ED.ED->af_dampen_factor);

		// custom score codes
		for ( gushort i = 0; i < (size_t)SPage::TScore::_total; ++i )
			gtk_entry_set_text( ED.eScoreCode[i], ED.ext_score_codes[i].c_str());

		// misc
		gtk_spin_button_set_value( ED.eBand[TBand::delta][0], ED.freq_bands[TBand::delta][0]);
		gtk_spin_button_set_value( ED.eBand[TBand::delta][1], ED.freq_bands[TBand::delta][1]);
		gtk_spin_button_set_value( ED.eBand[TBand::theta][0], ED.freq_bands[TBand::theta][0]);
		gtk_spin_button_set_value( ED.eBand[TBand::theta][1], ED.freq_bands[TBand::theta][1]);
		gtk_spin_button_set_value( ED.eBand[TBand::alpha][0], ED.freq_bands[TBand::alpha][0]);
		gtk_spin_button_set_value( ED.eBand[TBand::alpha][1], ED.freq_bands[TBand::alpha][1]);
		gtk_spin_button_set_value( ED.eBand[TBand::beta ][0], ED.freq_bands[TBand::beta ][0]);
		gtk_spin_button_set_value( ED.eBand[TBand::beta ][1], ED.freq_bands[TBand::beta ][1]);
		gtk_spin_button_set_value( ED.eBand[TBand::gamma][0], ED.freq_bands[TBand::gamma][0]);
		gtk_spin_button_set_value( ED.eBand[TBand::gamma][1], ED.freq_bands[TBand::gamma][1]);

		gtk_spin_button_set_value( ED.eDAPageHeight,		SScoringFacility::IntersignalSpace);
		gtk_spin_button_set_value( ED.eDAHypnogramHeight,	SScoringFacility::HypnogramHeight);
		gtk_spin_button_set_value( ED.eDAEMGHeight,		SScoringFacility::EMGProfileHeight);

		gtk_entry_set_text( ED.eBrowseCommand,		ED.browse_command.c_str());

		// colours are served specially elsewhere
	}
}







// ================== Simulations part



inline namespace {
void
__widgets_to_tunables( SExpDesignUI& ED)
{
	using namespace agh;
	// don't mess with classed enums!
	for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t ) {
		ED.ED->tunables0.value [t] = gtk_spin_button_get_value( ED.eTunable[t][(size_t)TTIdx::val ]) / STunableSet::stock[t].display_scale_factor;
		ED.ED->tunables0.lo    [t] = gtk_spin_button_get_value( ED.eTunable[t][(size_t)TTIdx::min ]) / STunableSet::stock[t].display_scale_factor;
		ED.ED->tunables0.hi    [t] = gtk_spin_button_get_value( ED.eTunable[t][(size_t)TTIdx::max ]) / STunableSet::stock[t].display_scale_factor;
		ED.ED->tunables0.step  [t] = gtk_spin_button_get_value( ED.eTunable[t][(size_t)TTIdx::step]) / STunableSet::stock[t].display_scale_factor;
	}
}


void
__tunables_to_widgets( SExpDesignUI& ED)
{
	using namespace agh;
	for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t ) {
		// gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::val ],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.value[t]);
		// gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::min ],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.lo   [t]);
		// gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::max ],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.hi   [t]);
		// gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::step],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.step [t]);

		gtk_adjustment_configure( ED.jTunable[t][(size_t)TTIdx::val ],
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.value[t],
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.lo[t],
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.hi[t],
					  STunableSet::stock[t].adj_step, 10 * STunableSet::stock[t].adj_step,
					  0);
		gtk_adjustment_configure( ED.jTunable[t][(size_t)TTIdx::min ],
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.lo[t],
					  0,
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.hi[t],
					  STunableSet::stock[t].adj_step, 10 * STunableSet::stock[t].adj_step,
					  0);
		gtk_adjustment_configure( ED.jTunable[t][(size_t)TTIdx::max ],
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.hi[t],
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.lo[t],
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.hi[t] * 1.5,
					  STunableSet::stock[t].adj_step, 10 * STunableSet::stock[t].adj_step,
					  0);
		gtk_adjustment_configure( ED.jTunable[t][(size_t)TTIdx::step ],
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.step[t],
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.value[t] / 1000.,
					  STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.value[t],
					  STunableSet::stock[t].adj_step, 10 * STunableSet::stock[t].adj_step,
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
		decompose_double( ED.ED->ctl_params0.siman_params.t_min, &mantissa, &exponent);
		gtk_spin_button_set_value( ED.eCtlParamAnnlTMinMantissa,	mantissa);
		gtk_spin_button_set_value( ED.eCtlParamAnnlTMinExponent,	exponent);
		decompose_double( ED.ED->ctl_params0.siman_params.t_initial, &mantissa, &exponent);
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
	ED.ED->tunables0.reset();
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
