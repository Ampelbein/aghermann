// ;-*-C++-*- *  Time-stamp: "2011-07-07 12:49:08 hmmr"
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


// #include <cstring>

// #include <array>
// #include <initializer_list>

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
	auto& ED = *(SExpDesignUI*)userdata;

      // save parameters changing which should trigger tree rescan
	if ( page_num == 0 ) {  // switching back from settings tab

	      // collect values from widgets
		ED.ED->fft_params.page_size =
			ED.FFTPageSizeValues[ ED.pagesize_item = gtk_combo_box_get_active( ED.eFFTParamsPageSize)];

		ED.ED->fft_params.welch_window_type =
			(agh::SFFTParamSet::TWinType)gtk_combo_box_get_active( ED.eFFTParamsWindowType);
		ED.ED->fft_params.bin_size =
			gtk_spin_button_get_value( ED.eFFTParamsBinSize);
		ED.ED->af_dampen_window_type =
			(agh::SFFTParamSet::TWinType)gtk_combo_box_get_active( ED.eArtifWindowType);

		for ( gushort i = 0; i < (size_t)agh::SPage::TScore::_total; ++i )
			ED.ext_score_codes[i] = gtk_entry_get_text( ED.eScoreCode[i]);

		ED.freq_bands[(size_t)agh::TBand::delta][0] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::delta][0]);
		ED.freq_bands[(size_t)agh::TBand::delta][1] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::delta][1]);
		ED.freq_bands[(size_t)agh::TBand::theta][0] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::theta][0]);
		ED.freq_bands[(size_t)agh::TBand::theta][1] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::theta][1]);
		ED.freq_bands[(size_t)agh::TBand::alpha][0] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::alpha][0]);
		ED.freq_bands[(size_t)agh::TBand::alpha][1] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::alpha][1]);
		ED.freq_bands[(size_t)agh::TBand::beta ][0] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::beta ][0]);
		ED.freq_bands[(size_t)agh::TBand::beta ][1] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::beta ][1]);
		ED.freq_bands[(size_t)agh::TBand::gamma][0] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::gamma][0]);
		ED.freq_bands[(size_t)agh::TBand::gamma][1] = gtk_spin_button_get_value( ED.eBand[(size_t)agh::TBand::gamma][1]);

		SScoringFacility::NeighPagePeek	= gtk_spin_button_get_value( ED.eSFNeighPagePeekPercent) / 100.;

		SScoringFacility::IntersignalSpace	= gtk_spin_button_get_value( ED.eDAPageHeight);
		SScoringFacility::HypnogramHeight	= gtk_spin_button_get_value( ED.eDAHypnogramHeight);
		SScoringFacility::SpectrumWidth		= gtk_spin_button_get_value( ED.eDASpectrumWidth);
		SScoringFacility::EMGProfileHeight	= gtk_spin_button_get_value( ED.eDAEMGHeight);

	      // scan as necessary
		if ( ED.pagesize_item_saved != ED.pagesize_item ||
		     ED.FFTWindowType_saved != ED.ED->fft_params.welch_window_type ||
		     ED.AfDampingWindowType_saved != ED.ED->af_dampen_window_type ||
		     ED.FFTBinSize_saved != ED.ED->fft_params.bin_size ) {
		      // rescan tree
			ED.do_rescan_tree(); // with populate
		}
	} else {
		ED.pagesize_item_saved		= ED.pagesize_item;
		ED.FFTWindowType_saved		= ED.ED->fft_params.welch_window_type;
		ED.AfDampingWindowType_saved	= ED.ED->af_dampen_window_type;
		ED.FFTBinSize_saved		= ED.ED->fft_params.bin_size;

	      // also assign values to widgets
		// -- maybe not? None of them are changeable by user outside settings tab
		// -- rather do: they are loaded at init
		// FFT parameters
		guint i = 0;
		while ( SExpDesignUI::FFTPageSizeValues[i] != (guint)-1 && SExpDesignUI::FFTPageSizeValues[i] < ED.ED->fft_params.page_size )
			++i;
		gtk_combo_box_set_active( ED.eFFTParamsPageSize, ED.pagesize_item = i);

		gtk_combo_box_set_active( ED.eFFTParamsWindowType, (int)ED.ED->fft_params.welch_window_type);
		gtk_spin_button_set_value( ED.eFFTParamsBinSize, ED.ED->fft_params.bin_size);

		// artifacts
		gtk_combo_box_set_active( ED.eArtifWindowType, (int)ED.ED->af_dampen_window_type);

		// custom score codes
		for ( gushort i = 0; i < (size_t)agh::SPage::TScore::_total; ++i )
			gtk_entry_set_text( ED.eScoreCode[i], ED.ext_score_codes[i].c_str());

		// misc
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::delta][0], ED.freq_bands[(size_t)agh::TBand::delta][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::delta][1], ED.freq_bands[(size_t)agh::TBand::delta][1]);
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::theta][0], ED.freq_bands[(size_t)agh::TBand::theta][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::theta][1], ED.freq_bands[(size_t)agh::TBand::theta][1]);
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::alpha][0], ED.freq_bands[(size_t)agh::TBand::alpha][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::alpha][1], ED.freq_bands[(size_t)agh::TBand::alpha][1]);
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::beta ][0], ED.freq_bands[(size_t)agh::TBand::beta ][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::beta ][1], ED.freq_bands[(size_t)agh::TBand::beta ][1]);
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::gamma][0], ED.freq_bands[(size_t)agh::TBand::gamma][0]);
		gtk_spin_button_set_value( ED.eBand[(size_t)agh::TBand::gamma][1], ED.freq_bands[(size_t)agh::TBand::gamma][1]);

		gtk_spin_button_set_value( ED.eSFNeighPagePeekPercent, SScoringFacility::NeighPagePeek * 100.);

		gtk_spin_button_set_value( ED.eDAPageHeight,		SScoringFacility::IntersignalSpace);
		gtk_spin_button_set_value( ED.eDAHypnogramHeight,	SScoringFacility::HypnogramHeight);
		gtk_spin_button_set_value( ED.eDASpectrumWidth,		SScoringFacility::SpectrumWidth);
		gtk_spin_button_set_value( ED.eDAEMGHeight,		SScoringFacility::EMGProfileHeight);

		// colours are served specially elsewhere
	}
}







// ================== Simulations part



inline namespace {
	void
	__widgets_to_tunables( SExpDesignUI& ED)
	{
		using namespace agh;
		for ( TTunable_underlying_type t = 0; t < TTunable::_basic_tunables; ++t ) {
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
		for ( TTunable_underlying_type t = 0; t < TTunable::_basic_tunables; ++t ) {
			gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::val ],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.value[t]);
			gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::min ],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.lo   [t]);
			gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::max ],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.hi   [t]);
			gtk_spin_button_set_value( ED.eTunable[t][(size_t)TTIdx::step],	STunableSet::stock[t].display_scale_factor * ED.ED->tunables0.step [t]);
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
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamDBAmendment1, ED.ED->ctl_params0.DBAmendment1);
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamDBAmendment2, ED.ED->ctl_params0.DBAmendment2);
		gtk_toggle_button_set_active( (GtkToggleButton*)ED.eCtlParamAZAmendment,  ED.ED->ctl_params0.AZAmendment);
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
		ED.ED->ctl_params0.AZAmendment  = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eCtlParamAZAmendment);
		ED.ED->ctl_params0.swa_laden_pages_before_SWA_0	= gtk_spin_button_get_value( ED.eCtlParamNSWAPpBeforeSimStart);
		ED.ED->ctl_params0.req_percent_scored		= gtk_spin_button_get_value( ED.eCtlParamReqScoredPercent);

	      // Unconventional scores frame
		ED.ED->ctl_params0.ScoreMVTAsWake      = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eCtlParamScoreMVTAsWake);
		ED.ED->ctl_params0.ScoreUnscoredAsWake = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eCtlParamScoreUnscoredAsWake);

	      // Tunables tab
		__widgets_to_tunables( ED);

	      // for ctlparam chnges to take effect on virgin modruns
		ED.ED->remove_untried_modruns();
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




/*
void eTunable_S0_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_S0_);  }
void eTunable_S0_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_S0_);  }
void eTunable_S0_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_S0_);  }
void eTunable_S0_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_S0_); }

void eTunable_SU_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_SU_);  }
void eTunable_SU_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_SU_);  }
void eTunable_SU_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_SU_);  }
void eTunable_SU_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_SU_); }

void eTunable_fcR_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_fcR_);  }
void eTunable_fcR_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_fcR_);  }
void eTunable_fcR_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_fcR_);  }
void eTunable_fcR_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_fcR_); }

void eTunable_fcW_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_fcW_);  }
void eTunable_fcW_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_fcW_);  }
void eTunable_fcW_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_fcW_);  }
void eTunable_fcW_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_fcW_); }

void eTunable_gc_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_gc_);  }
void eTunable_gc_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_gc_);  }
void eTunable_gc_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_gc_);  }
void eTunable_gc_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_gc_); }

void eTunable_rc_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_rc_);  }
void eTunable_rc_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_rc_);  }
void eTunable_rc_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_rc_);  }
void eTunable_rc_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_rc_); }

void eTunable_rs_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_rs_);  }
void eTunable_rs_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_rs_);  }
void eTunable_rs_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_rs_);  }
void eTunable_rs_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_rs_); }

void eTunable_ta_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_ta_);  }
void eTunable_ta_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_ta_);  }
void eTunable_ta_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_ta_);  }
void eTunable_ta_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_ta_); }

void eTunable_tp_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_tp_);  }
void eTunable_tp_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_tp_);  }
void eTunable_tp_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_tp_);  }
void eTunable_tp_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_tp_); }
*/


extern "C"
void
bSimParamRevertTunables_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.ED->tunables0.assign_defaults();
	__tunables_to_widgets( ED);
}



// EOF
