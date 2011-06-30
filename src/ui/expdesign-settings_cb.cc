// ;-*-C++-*- *  Time-stamp: "2011-06-29 12:38:51 hmmr"
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

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;
using namespace aghui;





// ============== Measurements settings tab

// --- PSD & Scoring


using namespace aghui;

extern "C" {

	void
	tDesign_switch_page_cb( GtkNotebook     *notebook,
				gpointer	 unused,
				guint            page_num,
				gpointer         user_data)
	{
		using namespace agh;

	      // save parameters changing which should trigger tree rescan
		static size_t
			FFTPageSizeItem_saved;
		static agh::SFFTParamSet::TWinType
			FFTWindowType_saved,
			AfDampingWindowType_saved;
		static float
			FFTBinSize_saved;

		if ( page_num == 0 ) {  // switching back from settings tab

		      // collect values from widgets
			AghCC->fft_params.page_size =
				FFTPageSizeValues[ FFTPageSizeItem = gtk_combo_box_get_active( eFFTParamsPageSize)];
			DisplayPageSizeItem = 0;
			while ( DisplayPageSizeValues[DisplayPageSizeItem] != FFTPageSizeValues[FFTPageSizeItem] )
				assert ( ++DisplayPageSizeItem < DisplayPageSizeValues.size() );

			AghCC->fft_params.welch_window_type =
				(SFFTParamSet::TWinType)gtk_combo_box_get_active( eFFTParamsWindowType);
			AghCC->fft_params.bin_size =
				gtk_spin_button_get_value( eFFTParamsBinSize);
			AghCC->af_dampen_window_type =
				(SFFTParamSet::TWinType)gtk_combo_box_get_active( eArtifWindowType);

			for ( gushort i = 0; i < (size_t)SPage::TScore::_total; ++i )
				ExtScoreCodes[i] = gtk_entry_get_text( eScoreCode[i]);

			FreqBands[(size_t)TBand::delta][0] = gtk_spin_button_get_value( eBand[(size_t)TBand::delta][0]);
			FreqBands[(size_t)TBand::delta][1] = gtk_spin_button_get_value( eBand[(size_t)TBand::delta][1]);
			FreqBands[(size_t)TBand::theta][0] = gtk_spin_button_get_value( eBand[(size_t)TBand::theta][0]);
			FreqBands[(size_t)TBand::theta][1] = gtk_spin_button_get_value( eBand[(size_t)TBand::theta][1]);
			FreqBands[(size_t)TBand::alpha][0] = gtk_spin_button_get_value( eBand[(size_t)TBand::alpha][0]);
			FreqBands[(size_t)TBand::alpha][1] = gtk_spin_button_get_value( eBand[(size_t)TBand::alpha][1]);
			FreqBands[(size_t)TBand::beta ][0] = gtk_spin_button_get_value( eBand[(size_t)TBand::beta ][0]);
			FreqBands[(size_t)TBand::beta ][1] = gtk_spin_button_get_value( eBand[(size_t)TBand::beta ][1]);
			FreqBands[(size_t)TBand::gamma][0] = gtk_spin_button_get_value( eBand[(size_t)TBand::gamma][0]);
			FreqBands[(size_t)TBand::gamma][1] = gtk_spin_button_get_value( eBand[(size_t)TBand::gamma][1]);

			SFNeighPagePeek			= gtk_spin_button_get_value( eSFNeighPagePeekPercent) / 100.;

			WidgetSize_SFPageHeight		= gtk_spin_button_get_value( eDAPageHeight);
			WidgetSize_SFHypnogramHeight	= gtk_spin_button_get_value( eDAHypnogramHeight);
			WidgetSize_SFSpectrumWidth	= gtk_spin_button_get_value( eDASpectrumWidth);
			WidgetSize_SFEMGProfileHeight	= gtk_spin_button_get_value( eDAEMGHeight);

		      // scan as necessary
			if ( FFTPageSizeItem_saved != FFTPageSizeItem ||
			     FFTWindowType_saved != AghCC->fft_params.welch_window_type ||
			     AfDampingWindowType_saved != AghCC->af_dampen_window_type ||
			     FFTBinSize_saved != AghCC->fft_params.bin_size ) {
			      // rescan tree
				set_cursor_busy( TRUE, (GtkWidget*)wMainWindow);
				gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, FALSE);
				while ( gtk_events_pending() )
					gtk_main_iteration();
				AghCC->scan_tree( sb::progress_indicator);
				populate( 0); // no new objects expected, don't depopulate (don't rebuild gtk tree storaage)

				set_cursor_busy( false, (GtkWidget*)wMainWindow);
				gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, TRUE);
				gtk_statusbar_push( sbMainStatusBar, sb::sbContextIdGeneral,
						    "Scanning complete");
			}
		} else {
			FFTPageSizeItem_saved	  = FFTPageSizeItem;
			FFTWindowType_saved       = AghCC->fft_params.welch_window_type;
			AfDampingWindowType_saved = AghCC->af_dampen_window_type;
			FFTBinSize_saved          = AghCC->fft_params.bin_size;

		      // also assign values to widgets
			// -- maybe not? None of them are changeable by user outside settings tab
			// -- rather do: they are loaded at init
			// FFT parameters
			guint i = 0;
			while ( FFTPageSizeValues[i] != (guint)-1 && FFTPageSizeValues[i] < AghCC->fft_params.page_size )
				++i;
			gtk_combo_box_set_active( eFFTParamsPageSize, FFTPageSizeItem = i);

			gtk_combo_box_set_active( eFFTParamsWindowType, (int)AghCC->fft_params.welch_window_type);
			gtk_spin_button_set_value( eFFTParamsBinSize, AghCC->fft_params.bin_size);

			// artifacts
			gtk_combo_box_set_active( eArtifWindowType, (int)AghCC->af_dampen_window_type);

			// custom score codes
			for ( gushort i = 0; i < (size_t)SPage::TScore::_total; ++i )
				gtk_entry_set_text( eScoreCode[i], ExtScoreCodes[i].c_str());

			// misc
			gtk_spin_button_set_value( eBand[(size_t)TBand::delta][0], FreqBands[(size_t)TBand::delta][0]);
			gtk_spin_button_set_value( eBand[(size_t)TBand::delta][1], FreqBands[(size_t)TBand::delta][1]);
			gtk_spin_button_set_value( eBand[(size_t)TBand::theta][0], FreqBands[(size_t)TBand::theta][0]);
			gtk_spin_button_set_value( eBand[(size_t)TBand::theta][1], FreqBands[(size_t)TBand::theta][1]);
			gtk_spin_button_set_value( eBand[(size_t)TBand::alpha][0], FreqBands[(size_t)TBand::alpha][0]);
			gtk_spin_button_set_value( eBand[(size_t)TBand::alpha][1], FreqBands[(size_t)TBand::alpha][1]);
			gtk_spin_button_set_value( eBand[(size_t)TBand::beta ][0], FreqBands[(size_t)TBand::beta ][0]);
			gtk_spin_button_set_value( eBand[(size_t)TBand::beta ][1], FreqBands[(size_t)TBand::beta ][1]);
			gtk_spin_button_set_value( eBand[(size_t)TBand::gamma][0], FreqBands[(size_t)TBand::gamma][0]);
			gtk_spin_button_set_value( eBand[(size_t)TBand::gamma][1], FreqBands[(size_t)TBand::gamma][1]);

			gtk_spin_button_set_value( eSFNeighPagePeekPercent,	SFNeighPagePeek * 100.);

			gtk_spin_button_set_value( eDAPageHeight,	WidgetSize_SFPageHeight);
			gtk_spin_button_set_value( eDAHypnogramHeight,	WidgetSize_SFHypnogramHeight);
			gtk_spin_button_set_value( eDASpectrumWidth,	WidgetSize_SFSpectrumWidth);
			gtk_spin_button_set_value( eDAEMGHeight,	WidgetSize_SFEMGProfileHeight);

			// colours are served specially elsewhere
		}
	}



	void
	tTaskSelector_switch_page_cb( GtkNotebook     *notebook,
				      gpointer	       unused,
				      guint            page_num,
				      gpointer         user_data)
	{
		if ( page_num == 1 ) {
			simview::populate();
			snprintf_buf( "Session: <b>%s</b>", AghD());
			gtk_label_set_markup( lSimulationsSession, __buf__);
			snprintf_buf( "Channel: <b>%s</b>", AghT());
			gtk_label_set_markup( lSimulationsChannel, __buf__);
			gtk_widget_set_sensitive( (GtkWidget*)bExpChange, FALSE);
		} else if ( page_num == 0 ) {
			AghCC->remove_untried_modruns();
			msmt::populate();
			gtk_widget_set_sensitive( (GtkWidget*)bExpChange, TRUE);
		}
	}
} // extern "C"






// ================== Simulations part



inline namespace {
	void
	__widgets_to_tunables()
	{
		using namespace agh;
		for ( TTunable_underlying_type t = 0; t < TTunable::_basic_tunables; ++t ) {
			AghCC->tunables0.value [t] = gtk_spin_button_get_value( eTunable[t][(size_t)TTIdx::val ]) / STunableSet::stock[t].display_scale_factor;
			AghCC->tunables0.lo    [t] = gtk_spin_button_get_value( eTunable[t][(size_t)TTIdx::min ]) / STunableSet::stock[t].display_scale_factor;
			AghCC->tunables0.hi    [t] = gtk_spin_button_get_value( eTunable[t][(size_t)TTIdx::max ]) / STunableSet::stock[t].display_scale_factor;
			AghCC->tunables0.step  [t] = gtk_spin_button_get_value( eTunable[t][(size_t)TTIdx::step]) / STunableSet::stock[t].display_scale_factor;
		}
	}


	void
	__tunables_to_widgets()
	{
		using namespace agh;
		for ( TTunable_underlying_type t = 0; t < TTunable::_basic_tunables; ++t ) {
			gtk_spin_button_set_value( eTunable[t][(size_t)TTIdx::val ],	STunableSet::stock[t].display_scale_factor * AghCC->tunables0.value[t]);
			gtk_spin_button_set_value( eTunable[t][(size_t)TTIdx::min ],	STunableSet::stock[t].display_scale_factor * AghCC->tunables0.lo   [t]);
			gtk_spin_button_set_value( eTunable[t][(size_t)TTIdx::max ],	STunableSet::stock[t].display_scale_factor * AghCC->tunables0.hi   [t]);
			gtk_spin_button_set_value( eTunable[t][(size_t)TTIdx::step],	STunableSet::stock[t].display_scale_factor * AghCC->tunables0.step [t]);
		}
	}

}

extern "C" {
	void
	tSimulations_switch_page_cb( GtkNotebook     *notebook,
				     gpointer	      page,
				     guint            page_num,
				     gpointer         user_data)
	{
		if ( page_num == 1 ) {  // switching to display parameters tab
		      // Controlling parameters frame
			gtk_spin_button_set_value( eCtlParamAnnlNTries,		AghCC->ctl_params0.siman_params.n_tries);
			gtk_spin_button_set_value( eCtlParamAnnlItersFixedT,	AghCC->ctl_params0.siman_params.iters_fixed_T);
			gtk_spin_button_set_value( eCtlParamAnnlStepSize,	AghCC->ctl_params0.siman_params.step_size);
			gtk_spin_button_set_value( eCtlParamAnnlBoltzmannk,	AghCC->ctl_params0.siman_params.k);
			gtk_spin_button_set_value( eCtlParamAnnlDampingMu,	AghCC->ctl_params0.siman_params.mu_t);
			float mantissa;
			int exponent;
			decompose_double( AghCC->ctl_params0.siman_params.t_min, &mantissa, &exponent);
			gtk_spin_button_set_value( eCtlParamAnnlTMinMantissa,	mantissa);
			gtk_spin_button_set_value( eCtlParamAnnlTMinExponent,	exponent);
			decompose_double( AghCC->ctl_params0.siman_params.t_initial, &mantissa, &exponent);
			gtk_spin_button_set_value( eCtlParamAnnlTInitialMantissa,	mantissa);
			gtk_spin_button_set_value( eCtlParamAnnlTInitialExponent,	exponent);

		      // Achermann parameters
			gtk_toggle_button_set_active( eCtlParamDBAmendment1, AghCC->ctl_params0.DBAmendment1);
			gtk_toggle_button_set_active( eCtlParamDBAmendment2, AghCC->ctl_params0.DBAmendment2);
			gtk_toggle_button_set_active( eCtlParamAZAmendment,  AghCC->ctl_params0.AZAmendment);
			gtk_spin_button_set_value( eCtlParamNSWAPpBeforeSimStart, AghCC->ctl_params0.swa_laden_pages_before_SWA_0);
			gtk_spin_button_set_value( eCtlParamReqScoredPercent, AghCC->ctl_params0.req_percent_scored);

		      // Unconventional scores frame
			gtk_toggle_button_set_active( eCtlParamScoreMVTAsWake,
						      AghCC->ctl_params0.ScoreMVTAsWake);
			gtk_toggle_button_set_active( eCtlParamScoreUnscoredAsWake,
						      AghCC->ctl_params0.ScoreUnscoredAsWake);

		      // Tunables tab
			__tunables_to_widgets();

		} else {
		      // Controlling parameters frame
			AghCC->ctl_params0.siman_params.n_tries       = gtk_spin_button_get_value( eCtlParamAnnlNTries);
			AghCC->ctl_params0.siman_params.iters_fixed_T = gtk_spin_button_get_value( eCtlParamAnnlItersFixedT);
			AghCC->ctl_params0.siman_params.step_size     = gtk_spin_button_get_value( eCtlParamAnnlStepSize);
			AghCC->ctl_params0.siman_params.k             = gtk_spin_button_get_value( eCtlParamAnnlBoltzmannk);
			AghCC->ctl_params0.siman_params.mu_t          = gtk_spin_button_get_value( eCtlParamAnnlDampingMu);
			AghCC->ctl_params0.siman_params.t_initial     = gtk_spin_button_get_value( eCtlParamAnnlTInitialMantissa)
				* pow(10, gtk_spin_button_get_value( eCtlParamAnnlTInitialExponent));
			AghCC->ctl_params0.siman_params.t_min	     = gtk_spin_button_get_value( eCtlParamAnnlTMinMantissa)
				* pow(10, gtk_spin_button_get_value( eCtlParamAnnlTMinExponent));
		      // Achermann parameters
			AghCC->ctl_params0.DBAmendment1 = gtk_toggle_button_get_active( eCtlParamDBAmendment1);
			AghCC->ctl_params0.DBAmendment2 = gtk_toggle_button_get_active( eCtlParamDBAmendment2);
			AghCC->ctl_params0.AZAmendment  = gtk_toggle_button_get_active( eCtlParamAZAmendment);
			AghCC->ctl_params0.swa_laden_pages_before_SWA_0	= gtk_spin_button_get_value( eCtlParamNSWAPpBeforeSimStart);
			AghCC->ctl_params0.req_percent_scored		= gtk_spin_button_get_value( eCtlParamReqScoredPercent);

		      // Unconventional scores frame
			AghCC->ctl_params0.ScoreMVTAsWake      = gtk_toggle_button_get_active( eCtlParamScoreMVTAsWake);
			AghCC->ctl_params0.ScoreUnscoredAsWake = gtk_toggle_button_get_active( eCtlParamScoreUnscoredAsWake);

		      // Tunables tab
			__widgets_to_tunables();

		      // for ctlparam chnges to take effect on virgin modruns
			AghCC->remove_untried_modruns();
			simview::populate();
		}
	}






	// possibly for some live validation; unused for now
	void eCtlParamAnnlNTries_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	void eCtlParamAnnlItersFixedT_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	void eCtlParamAnnlStepSize_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	void eCtlParamAnnlBoltzmannk_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	void eCtlParamAnnlTInitial_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	void eCtlParamAnnlDampingMu_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	void eCtlParamAnnlTMinMantissa_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	void eCtlParamAnnlTMinExponent_value_changed_cb( GtkSpinButton *e, gpointer u)	{ }
	void eCtlParamScoreMVTAs_toggled_cb( GtkToggleButton *e, gpointer u)		{ }
	void eCtlParamScoreUnscoredAs_toggled_cb( GtkToggleButton *e, gpointer u)	{ }




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


	void
	bSimParamRevertTunables_clicked_cb()
	{
		AghCC->tunables0.assign_defaults();
		__tunables_to_widgets();
	}

} // extern "C"


// EOF