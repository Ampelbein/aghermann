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

#if HAVE_CONFIG_H
#  include "config.h"
#endif


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
		ED.ED->fft_params.page_size =
			ED.FFTPageSizeValues[ ED.pagesize_item = gtk_combo_box_get_active( ED.eFFTParamsPageSize)];
		ED.ED->fft_params.bin_size =
			ED.FFTBinSizeValues[ ED.binsize_item = gtk_combo_box_get_active( ED.eFFTParamsBinSize)];
		if ( not ED.ED->fft_params.validate() )
			;

		ED.ED->fft_params.welch_window_type =
			(SFFTParamSet::TWinType)gtk_combo_box_get_active( ED.eFFTParamsWindowType);
		// ED.ED->fft_params.freq_trunc =
		// 	gtk_spin_button_get_value( ED.eFFTParamsFreqTrunc);
		ED.ED->af_dampen_window_type =
			(SFFTParamSet::TWinType)gtk_combo_box_get_active( ED.eArtifWindowType);

		for ( gushort i = 0; i < (size_t)SPage::TScore::_total; ++i )
			ED.ext_score_codes[i] = gtk_entry_get_text( ED.eScoreCode[i]);

		ED.freq_bands[(size_t)TBand::delta][0] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::delta][0]);
		ED.freq_bands[(size_t)TBand::delta][1] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::delta][1]);
		ED.freq_bands[(size_t)TBand::theta][0] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::theta][0]);
		ED.freq_bands[(size_t)TBand::theta][1] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::theta][1]);
		ED.freq_bands[(size_t)TBand::alpha][0] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::alpha][0]);
		ED.freq_bands[(size_t)TBand::alpha][1] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::alpha][1]);
		ED.freq_bands[(size_t)TBand::beta ][0] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::beta ][0]);
		ED.freq_bands[(size_t)TBand::beta ][1] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::beta ][1]);
		ED.freq_bands[(size_t)TBand::gamma][0] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::gamma][0]);
		ED.freq_bands[(size_t)TBand::gamma][1] = gtk_spin_button_get_value( ED.eBand[(size_t)TBand::gamma][1]);

		SScoringFacility::NeighPagePeek	= gtk_spin_button_get_value( ED.eSFNeighPagePeekPercent) / 100.;

		SScoringFacility::IntersignalSpace	= gtk_spin_button_get_value( ED.eDAPageHeight);
		SScoringFacility::HypnogramHeight	= gtk_spin_button_get_value( ED.eDAHypnogramHeight);
		SScoringFacility::SpectrumWidth		= gtk_spin_button_get_value( ED.eDASpectrumWidth);
		SScoringFacility::EMGProfileHeight	= gtk_spin_button_get_value( ED.eDAEMGHeight);

		ED.browse_command.assign( gtk_entry_get_text( ED.eBrowseCommand));

	      // scan as necessary
		if ( ED.pagesize_item_saved	  != ED.pagesize_item ||
		     ED.binsize_item_saved	  != ED.binsize_item ||
		     ED.FFTWindowType_saved	  != ED.ED->fft_params.welch_window_type ||
		     ED.AfDampingWindowType_saved != ED.ED->af_dampen_window_type ) {
		      // rescan tree
			ED.do_rescan_tree(); // with populate
		}
	} else {
		ED.pagesize_item_saved		= ED.pagesize_item;
		ED.binsize_item_saved		= ED.binsize_item;
		ED.FFTWindowType_saved		= ED.ED->fft_params.welch_window_type;
		ED.AfDampingWindowType_saved	= ED.ED->af_dampen_window_type;
		//ED.FFTFreqTrunc_saved		= ED.ED->fft_params.freq_trunc;

	      // also assign values to widgets
		// -- maybe not? None of them are changeable by user outside settings tab
		// -- rather do: they are loaded at init
		// FFT parameters
		guint i = 0;
		while ( SExpDesignUI::FFTPageSizeValues[i] < ED.ED->fft_params.page_size )
			++i;
		gtk_combo_box_set_active( ED.eFFTParamsPageSize, ED.pagesize_item = i);
		i = 0;
		while ( SExpDesignUI::FFTBinSizeValues[i] < ED.ED->fft_params.bin_size )
			++i;
		gtk_combo_box_set_active( ED.eFFTParamsBinSize, ED.binsize_item = i);

		gtk_combo_box_set_active( ED.eFFTParamsWindowType, (int)ED.ED->fft_params.welch_window_type);
		//gtk_spin_button_set_value( ED.eFFTParamsFreqTrunc, ED.ED->fft_params.freq_trunc);

		// artifacts
		gtk_combo_box_set_active( ED.eArtifWindowType, (int)ED.ED->af_dampen_window_type);

		// custom score codes
		for ( gushort i = 0; i < (size_t)SPage::TScore::_total; ++i )
			gtk_entry_set_text( ED.eScoreCode[i], ED.ext_score_codes[i].c_str());

		// misc
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::delta][0], ED.freq_bands[(size_t)TBand::delta][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::delta][1], ED.freq_bands[(size_t)TBand::delta][1]);
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::theta][0], ED.freq_bands[(size_t)TBand::theta][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::theta][1], ED.freq_bands[(size_t)TBand::theta][1]);
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::alpha][0], ED.freq_bands[(size_t)TBand::alpha][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::alpha][1], ED.freq_bands[(size_t)TBand::alpha][1]);
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::beta ][0], ED.freq_bands[(size_t)TBand::beta ][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::beta ][1], ED.freq_bands[(size_t)TBand::beta ][1]);
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::gamma][0], ED.freq_bands[(size_t)TBand::gamma][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)TBand::gamma][1], ED.freq_bands[(size_t)TBand::gamma][1]);

		gtk_spin_button_set_value( ED.eSFNeighPagePeekPercent,	SScoringFacility::NeighPagePeek * 100.);

		gtk_spin_button_set_value( ED.eDAPageHeight,		SScoringFacility::IntersignalSpace);
		gtk_spin_button_set_value( ED.eDAHypnogramHeight,	SScoringFacility::HypnogramHeight);
		gtk_spin_button_set_value( ED.eDASpectrumWidth,		SScoringFacility::SpectrumWidth);
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

}

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
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamScoreMVTAsWake,
					      ED.ED->ctl_params0.ScoreMVTAsWake);
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
		ED.ED->ctl_params0.ScoreMVTAsWake      = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eCtlParamScoreMVTAsWake);
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
	ED.ED->tunables0.assign_defaults();
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


// EOF
