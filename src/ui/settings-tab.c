// ;-*-C-*- *  Time-stamp: "2011-03-19 14:24:04 hmmr"
/*
 *       File name:  ui/settings-tab.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  settings tab construct and callbacks
 *
 *         License:  GPL
 */


#include <string.h>
#include <math.h>
#include <glade/glade.h>
#include "../libagh/iface.h"
#include "misc.h"
#include "ui.h"
#include "settings.h"


guint	AghAfDampingWindowType = AGH_WT_WELCH;

float	AghOperatingRangeFrom = 2.,
	AghOperatingRangeUpto = 3.;


guint	AghDisplayPageSizeItem = 4,  // the one used to obtain FFTs
	AghFFTPageSizeCurrent = 2;


const char *AghExtScoreCodes[AGH_SCORE__TOTAL];
const char *AghExtScoreCodes_defaults[AGH_SCORE__TOTAL] =
	{ " -0", "1", "2", "3", "4", "6Rr8", "Ww5", "mM"};
const char *AghScoreNames[AGH_SCORE__TOTAL] =
	{ "Unscored", "NREM1", "NREM2", "NREM3", "NREM4", "REM", "Wake", "MVT"};

gfloat
	AghFreqBands[AGH_BAND__TOTAL][2] = {
	{  1.5,  4.0 },
	{  4.0,  8.0 },
	{  8.0, 12.0 },
	{ 15.0, 30.0 },
	{ 30.0, 40.0 },
};




GtkWidget
	*eBand[AGH_BAND__TOTAL][2];


static GtkWidget
	*eCtlParamAnnlNTries,
	*eCtlParamAnnlItersFixedT,
	*eCtlParamAnnlStepSize,
	*eCtlParamAnnlBoltzmannk,
	*eCtlParamAnnlTInitialMantissa,
	*eCtlParamAnnlTInitialExponent,
	*eCtlParamAnnlDampingMu,
	*eCtlParamAnnlTMinMantissa,
	*eCtlParamAnnlTMinExponent,
	*eCtlParamDBAmendment1,
	*eCtlParamDBAmendment2,
	*eCtlParamAZAmendment,
	*eCtlParamScoreMVTAsWake,
	*eCtlParamScoreUnscoredAsWake,
//	*eCtlParamScoreMVTAsPrevScore,
//	*eCtlParamScoreUnscoredAsPrevScore,

	*eFFTParamsBinSize,
	*eFFTParamsWindowType,
	*eFFTParamsPageSize,
//	*eArtifSmoothOver,
	*eArtifWindowType,

	*eScoreCode[AGH_SCORE__TOTAL],

	*ePatternDZCDFSigmaDefault,
	*ePatternDZCDFStepDefault,
	*ePatternDZCDFSmoothDefault,
	*ePatternFilterCutoffDefault,
	*ePatternFilterOrderDefault,
	*ePatternEnvTightnessDefault,
	*eSignalAnalysisUseOnNonEEG,

	*eDAPageHeight,
	*eDAPowerHeight,
	*eDASpectrumWidth,
	*eDAEMGHeight,

	*eTunable[_agh_basic_tunables_][4];


// ------------------ construct

gint
agh_ui_construct_Settings( GladeXML *xml)
{
	GtkCellRenderer *renderer;

     // ------------- fFFTParams
	if ( !(eFFTParamsBinSize    = glade_xml_get_widget( xml, "eFFTParamsBinSize")) ||
	     !(eFFTParamsPageSize   = glade_xml_get_widget( xml, "eFFTParamsPageSize")) ||
	     !(eFFTParamsWindowType = glade_xml_get_widget( xml, "eFFTParamsWindowType")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eFFTParamsPageSize),
				 GTK_TREE_MODEL (agh_mFFTParamsPageSize));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT (eFFTParamsPageSize), renderer, FALSE);
	gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT (eFFTParamsPageSize), renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( GTK_COMBO_BOX (eFFTParamsWindowType),
				 GTK_TREE_MODEL (agh_mFFTParamsWindowType));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eFFTParamsWindowType), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eFFTParamsWindowType), renderer,
					"text", 0,
					NULL);

      // ------------- fArtifacts
	if ( !(eArtifWindowType	     = glade_xml_get_widget( xml, "eArtifWindowType")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eArtifWindowType),
				 GTK_TREE_MODEL (agh_mAfDampingWindowType));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eArtifWindowType), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eArtifWindowType), renderer,
					"text", 0,
					NULL);

      // ------- custom score codes
	if ( !(eScoreCode[AGH_SCORE_NONE]	= glade_xml_get_widget( xml, "eScoreCodeUnscored")) ||
	     !(eScoreCode[AGH_SCORE_NREM1]	= glade_xml_get_widget( xml, "eScoreCodeNREM1")) ||
	     !(eScoreCode[AGH_SCORE_NREM2]	= glade_xml_get_widget( xml, "eScoreCodeNREM2")) ||
	     !(eScoreCode[AGH_SCORE_NREM3]	= glade_xml_get_widget( xml, "eScoreCodeNREM3")) ||
	     !(eScoreCode[AGH_SCORE_NREM4]	= glade_xml_get_widget( xml, "eScoreCodeNREM4")) ||
	     !(eScoreCode[AGH_SCORE_REM]	= glade_xml_get_widget( xml, "eScoreCodeREM")) ||
	     !(eScoreCode[AGH_SCORE_WAKE]	= glade_xml_get_widget( xml, "eScoreCodeWake")) ||
	     !(eScoreCode[AGH_SCORE_MVT]	= glade_xml_get_widget( xml, "eScoreCodeMVT")) )
		return -1;

      // ----------- fSignalCriteria
	if ( !(ePatternDZCDFStepDefault		= glade_xml_get_widget( xml, "ePatternDZCDFStepDefault")) ||
	     !(ePatternDZCDFSigmaDefault	= glade_xml_get_widget( xml, "ePatternDZCDFSigmaDefault")) ||
	     !(ePatternDZCDFSmoothDefault	= glade_xml_get_widget( xml, "ePatternDZCDFSmoothDefault")) ||
	     !(ePatternFilterCutoffDefault	= glade_xml_get_widget( xml, "ePatternFilterCutoffDefault")) ||
	     !(ePatternFilterOrderDefault	= glade_xml_get_widget( xml, "ePatternFilterOrderDefault")) ||
	     !(ePatternEnvTightnessDefault	= glade_xml_get_widget( xml, "ePatternEnvTightnessDefault")) ||
	     !(eSignalAnalysisUseOnNonEEG	= glade_xml_get_widget( xml, "eSignalAnalysisUseOnNonEEG")) )
		return -1;

      // --------- Bands
	if ( !(eBand[AGH_BAND_DELTA][0]   = glade_xml_get_widget( xml, "eBandDeltaFrom")) ||
	     !(eBand[AGH_BAND_DELTA][1]   = glade_xml_get_widget( xml, "eBandDeltaUpto")) ||
	     !(eBand[AGH_BAND_THETA][0]   = glade_xml_get_widget( xml, "eBandThetaFrom")) ||
	     !(eBand[AGH_BAND_THETA][1]   = glade_xml_get_widget( xml, "eBandThetaUpto")) ||
	     !(eBand[AGH_BAND_ALPHA][0]   = glade_xml_get_widget( xml, "eBandAlphaFrom")) ||
	     !(eBand[AGH_BAND_ALPHA][1]   = glade_xml_get_widget( xml, "eBandAlphaUpto")) ||
	     !(eBand[AGH_BAND_BETA ][0]   = glade_xml_get_widget( xml, "eBandBetaFrom" )) ||
	     !(eBand[AGH_BAND_BETA ][1]   = glade_xml_get_widget( xml, "eBandBetaUpto" )) ||
	     !(eBand[AGH_BAND_GAMMA][0]   = glade_xml_get_widget( xml, "eBandGammaFrom")) ||
	     !(eBand[AGH_BAND_GAMMA][1]   = glade_xml_get_widget( xml, "eBandGammaUpto")) )
		return -1;

      // --------- Misc
	if ( !(eDAPageHeight	= glade_xml_get_widget( xml, "eDAPageHeight")) ||
	     !(eDAPowerHeight	= glade_xml_get_widget( xml, "eDAPowerHeight")) ||
	     !(eDASpectrumWidth	= glade_xml_get_widget( xml, "eDASpectrumWidth")) ||
	     !(eDAEMGHeight	= glade_xml_get_widget( xml, "eDAEMGHeight")) )
		return -1;

     // ------------- eCtrlParam*
	if ( !(eCtlParamAnnlNTries		= glade_xml_get_widget( xml, "eCtlParamAnnlNTries")) ||
	     !(eCtlParamAnnlItersFixedT		= glade_xml_get_widget( xml, "eCtlParamAnnlItersFixedT")) ||
	     !(eCtlParamAnnlStepSize		= glade_xml_get_widget( xml, "eCtlParamAnnlStepSize")) ||
	     !(eCtlParamAnnlBoltzmannk		= glade_xml_get_widget( xml, "eCtlParamAnnlBoltzmannk")) ||
	     !(eCtlParamAnnlDampingMu		= glade_xml_get_widget( xml, "eCtlParamAnnlDampingMu")) ||
	     !(eCtlParamAnnlTInitialMantissa	= glade_xml_get_widget( xml, "eCtlParamAnnlTInitialMantissa")) ||
	     !(eCtlParamAnnlTInitialExponent	= glade_xml_get_widget( xml, "eCtlParamAnnlTInitialExponent")) ||
	     !(eCtlParamAnnlTMinMantissa	= glade_xml_get_widget( xml, "eCtlParamAnnlTMinMantissa")) ||
	     !(eCtlParamAnnlTMinExponent	= glade_xml_get_widget( xml, "eCtlParamAnnlTMinExponent")) ||
	     !(eCtlParamDBAmendment1		= glade_xml_get_widget( xml, "eCtlParamDBAmendment1")) ||
	     !(eCtlParamDBAmendment2		= glade_xml_get_widget( xml, "eCtlParamDBAmendment2")) ||
	     !(eCtlParamAZAmendment		= glade_xml_get_widget( xml, "eCtlParamAZAmendment")) ||
	     !(eCtlParamScoreMVTAsWake		= glade_xml_get_widget( xml, "eCtlParamScoreMVTAsWake")) ||
	     !(eCtlParamScoreUnscoredAsWake	= glade_xml_get_widget( xml, "eCtlParamScoreUnscoredAsWake")) )
		return -1;

      // ------------- eTunable_*
	if ( !(eTunable[_rs_][_val_]		= glade_xml_get_widget( xml, "eTunable_rs")) ||
	     !(eTunable[_rs_][_min_]		= glade_xml_get_widget( xml, "eTunable_rs_min")) ||
	     !(eTunable[_rs_][_max_]		= glade_xml_get_widget( xml, "eTunable_rs_max")) ||
	     !(eTunable[_rs_][_step_]		= glade_xml_get_widget( xml, "eTunable_rs_step")) ||

	     !(eTunable[_rc_][_val_]		= glade_xml_get_widget( xml, "eTunable_rc")) ||
	     !(eTunable[_rc_][_min_]		= glade_xml_get_widget( xml, "eTunable_rc_min")) ||
	     !(eTunable[_rc_][_max_]		= glade_xml_get_widget( xml, "eTunable_rc_max")) ||
	     !(eTunable[_rc_][_step_]		= glade_xml_get_widget( xml, "eTunable_rc_step")) ||

	     !(eTunable[_fcR_][_val_]		= glade_xml_get_widget( xml, "eTunable_fcR")) ||
	     !(eTunable[_fcR_][_min_]		= glade_xml_get_widget( xml, "eTunable_fcR_min")) ||
	     !(eTunable[_fcR_][_max_]		= glade_xml_get_widget( xml, "eTunable_fcR_max")) ||
	     !(eTunable[_fcR_][_step_]		= glade_xml_get_widget( xml, "eTunable_fcR_step")) ||

	     !(eTunable[_fcW_][_val_]		= glade_xml_get_widget( xml, "eTunable_fcW")) ||
	     !(eTunable[_fcW_][_min_]		= glade_xml_get_widget( xml, "eTunable_fcW_min")) ||
	     !(eTunable[_fcW_][_max_]		= glade_xml_get_widget( xml, "eTunable_fcW_max")) ||
	     !(eTunable[_fcW_][_step_]		= glade_xml_get_widget( xml, "eTunable_fcW_step")) ||

	     !(eTunable[_S0_][_val_]		= glade_xml_get_widget( xml, "eTunable_S0")) ||
	     !(eTunable[_S0_][_min_]		= glade_xml_get_widget( xml, "eTunable_S0_min")) ||
	     !(eTunable[_S0_][_max_]		= glade_xml_get_widget( xml, "eTunable_S0_max")) ||
	     !(eTunable[_S0_][_step_]		= glade_xml_get_widget( xml, "eTunable_S0_step")) ||

	     !(eTunable[_SU_][_val_]		= glade_xml_get_widget( xml, "eTunable_SU")) ||
	     !(eTunable[_SU_][_min_]		= glade_xml_get_widget( xml, "eTunable_SU_min")) ||
	     !(eTunable[_SU_][_max_]		= glade_xml_get_widget( xml, "eTunable_SU_max")) ||
	     !(eTunable[_SU_][_step_]		= glade_xml_get_widget( xml, "eTunable_SU_step")) ||

	     !(eTunable[_ta_][_val_]		= glade_xml_get_widget( xml, "eTunable_ta")) ||
	     !(eTunable[_ta_][_min_]		= glade_xml_get_widget( xml, "eTunable_ta_min")) ||
	     !(eTunable[_ta_][_max_]		= glade_xml_get_widget( xml, "eTunable_ta_max")) ||
	     !(eTunable[_ta_][_step_]		= glade_xml_get_widget( xml, "eTunable_ta_step")) ||

	     !(eTunable[_tp_][_val_]		= glade_xml_get_widget( xml, "eTunable_tp")) ||
	     !(eTunable[_tp_][_min_]		= glade_xml_get_widget( xml, "eTunable_tp_min")) ||
	     !(eTunable[_tp_][_max_]		= glade_xml_get_widget( xml, "eTunable_tp_max")) ||
	     !(eTunable[_tp_][_step_]		= glade_xml_get_widget( xml, "eTunable_tp_step")) ||

	     !(eTunable[_gc_][_val_]		= glade_xml_get_widget( xml, "eTunable_gc")) ||
	     !(eTunable[_gc_][_min_]		= glade_xml_get_widget( xml, "eTunable_gc_min")) ||
	     !(eTunable[_gc_][_max_]		= glade_xml_get_widget( xml, "eTunable_gc_max")) ||
	     !(eTunable[_gc_][_step_]		= glade_xml_get_widget( xml, "eTunable_gc_step")) )
		return -1;


	return 0;
}


// ============== Measurements settings tab

// --- PSD & Scoring




void
tDesign_switch_page_cb( GtkNotebook     *notebook,
			GtkNotebookPage *page,
			guint            page_num,
			gpointer         user_data)
{
      // save parameters changing which should trigger tree rescan
	static size_t
		AghFFTPageSizeCurrent_saved,
		AghFFTWindowType_saved,
		AghAfDampingWindowType_saved;
	static float
		AghFFTBinSize_saved;

	if ( page_num == 0 ) {  // switching back from settings tab

	      // collect values from widgets
		agh_fft_set_pagesize( AghFFTPageSizeValues[ AghFFTPageSizeCurrent = gtk_combo_box_get_active( GTK_COMBO_BOX (eFFTParamsPageSize))] );
		AghDisplayPageSizeItem = 0;
		while ( AghDisplayPageSizeValues[AghDisplayPageSizeItem] != AghFFTPageSizeValues[AghFFTPageSizeCurrent] )
			if ( ++AghDisplayPageSizeItem > 10 )
				abort();

		agh_fft_set_window_type( gtk_combo_box_get_active( GTK_COMBO_BOX (eFFTParamsWindowType)));
		agh_fft_set_binsize( gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFFTParamsBinSize)));
		agh_af_set_window_type( gtk_combo_box_get_active( GTK_COMBO_BOX (eArtifWindowType)));

		for ( gushort i = 0; i < AGH_SCORE__TOTAL; ++i ) {
			free( (void*)AghExtScoreCodes[i]);
			AghExtScoreCodes[i] = strdup( gtk_entry_get_text( GTK_ENTRY (eScoreCode[i])));
		}

		AghEnvTightness	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternEnvTightnessDefault));
		AghBWFCutoff	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterCutoffDefault));
		AghBWFOrder	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterOrderDefault));
		AghDZCDFStep	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFStepDefault));
		AghDZCDFSigma	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSigmaDefault));
		AghDZCDFSmooth	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSmoothDefault));
		AghUseSigAnOnNonEEGChannels
				= gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eSignalAnalysisUseOnNonEEG));

		AghFreqBands[AGH_BAND_DELTA][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_DELTA][0]));
		AghFreqBands[AGH_BAND_DELTA][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_DELTA][1]));
		AghFreqBands[AGH_BAND_THETA][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_THETA][0]));
		AghFreqBands[AGH_BAND_THETA][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_THETA][1]));
		AghFreqBands[AGH_BAND_ALPHA][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_ALPHA][0]));
		AghFreqBands[AGH_BAND_ALPHA][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_ALPHA][1]));
		AghFreqBands[AGH_BAND_BETA ][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_BETA ][0]));
		AghFreqBands[AGH_BAND_BETA ][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_BETA ][1]));
		AghFreqBands[AGH_BAND_GAMMA][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_GAMMA][0]));
		AghFreqBands[AGH_BAND_GAMMA][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_GAMMA][1]));

		AghSFDAPageHeight		= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eDAPageHeight));
		AghSFDASpectrumWidth		= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eDASpectrumWidth));
		AghSFDAPowerProfileHeight	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eDAPowerHeight));
		AghSFDAEMGProfileHeight		= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eDAEMGHeight));

	      // scan as necessary
		if ( AghFFTPageSizeCurrent_saved != AghFFTPageSizeCurrent ||
		     AghFFTWindowType_saved != agh_fft_get_window_type() ||
		     AghAfDampingWindowType_saved != agh_af_get_window_type() ||
		     AghFFTBinSize_saved != agh_fft_get_binsize() ) {
		      // rescan tree
			set_cursor_busy( TRUE, wMainWindow);
			gtk_widget_set_sensitive( wMainWindow, FALSE);
			while ( gtk_events_pending() )
				gtk_main_iteration();
			agh_expdesign_scan_tree( progress_indicator);
			agh_ui_populate( 0); // no new objects expected, don't depopulate (don't rebuild gtk tree storaage)

			set_cursor_busy( FALSE, wMainWindow);
			gtk_widget_set_sensitive( wMainWindow, TRUE);
			gtk_statusbar_push( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General,
					    "Scanning complete");
		}
	} else {
		AghFFTPageSizeCurrent_saved = AghFFTPageSizeCurrent;
		AghFFTWindowType_saved = agh_fft_get_window_type();
		AghAfDampingWindowType_saved = agh_af_get_window_type();
		AghFFTBinSize_saved = agh_fft_get_binsize();

	      // also assign values to widgets
		// -- maybe not? None of them are changeable by user outside settings tab
		// -- rather do: they are loaded at init
		// FFT parameters
		guint i = 0;
		while ( AghFFTPageSizeValues[i] != (guint)-1 && AghFFTPageSizeValues[i] < agh_fft_get_pagesize() )
			++i;
		gtk_combo_box_set_active( GTK_COMBO_BOX (eFFTParamsPageSize), AghFFTPageSizeCurrent = i);

		gtk_combo_box_set_active( GTK_COMBO_BOX (eFFTParamsWindowType), agh_fft_get_window_type());
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eFFTParamsBinSize), agh_fft_get_binsize());

		// artifacts
//		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eArtifSmoothOver), agh_af_get_smoothover());
		gtk_combo_box_set_active( GTK_COMBO_BOX (eArtifWindowType), agh_af_get_window_type());

		// custom score codes
		for ( gushort i = 0; i < AGH_SCORE__TOTAL; ++i )
			gtk_entry_set_text( GTK_ENTRY (eScoreCode[i]), AghExtScoreCodes[i]);

		// signal criteria
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternEnvTightnessDefault),	AghEnvTightness);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternFilterCutoffDefault),	AghBWFCutoff);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternFilterOrderDefault),	AghBWFOrder);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFStepDefault),		AghDZCDFStep);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFSigmaDefault),		AghDZCDFSigma);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFSmoothDefault),	AghDZCDFSmooth);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eSignalAnalysisUseOnNonEEG),	AghUseSigAnOnNonEEGChannels);

		// misc
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_DELTA][0]), AghFreqBands[AGH_BAND_DELTA][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_DELTA][1]), AghFreqBands[AGH_BAND_DELTA][1]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_THETA][0]), AghFreqBands[AGH_BAND_THETA][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_THETA][1]), AghFreqBands[AGH_BAND_THETA][1]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_ALPHA][0]), AghFreqBands[AGH_BAND_ALPHA][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_ALPHA][1]), AghFreqBands[AGH_BAND_ALPHA][1]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_BETA ][0]), AghFreqBands[AGH_BAND_BETA ][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_BETA ][1]), AghFreqBands[AGH_BAND_BETA ][1]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_GAMMA][0]), AghFreqBands[AGH_BAND_GAMMA][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[AGH_BAND_GAMMA][1]), AghFreqBands[AGH_BAND_GAMMA][1]);

		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eDAPageHeight), AghSFDAPageHeight);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eDASpectrumWidth), AghSFDASpectrumWidth);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eDAPowerHeight), AghSFDAPowerProfileHeight);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eDAEMGHeight), AghSFDAEMGProfileHeight);

		// colours are served specially elsewhere
	}
}


void
ePatternDZCDFStepFineDefault_toggled_cb( GtkToggleButton *togglebutton, gpointer unused)
{
	gtk_widget_set_sensitive( ePatternDZCDFStepDefault,
				  !gtk_toggle_button_get_active( togglebutton));
}


void
tTaskSelector_switch_page_cb( GtkNotebook     *notebook,
			      GtkNotebookPage *page,
			      guint            page_num,
			      gpointer         user_data)
{
	if ( page_num == 1 ) {
		agh_populate_mSimulations( TRUE);
		gtk_widget_set_sensitive( bExpChange, FALSE);
	} else if ( page_num == 0 ) {
		agh_modelrun_remove_untried();
		agh_populate_cMeasurements();
		gtk_widget_set_sensitive( bExpChange, TRUE);
	}
}







// ================== Simulations part



static void
__widgets_to_tunables()
{
	struct SConsumerTunableSetFull tset;
	const struct STunableDescription *td;
	for ( gushort t = 0; t < _agh_basic_tunables_; ++t ) {
		td = agh_tunable_get_description(t);
		tset.tunables    [t] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eTunable[t][_val_ ])) / td->display_scale_factor;
		tset.lower_bounds[t] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eTunable[t][_min_ ])) / td->display_scale_factor;
		tset.upper_bounds[t] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eTunable[t][_max_ ])) / td->display_scale_factor;
		tset.steps       [t] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eTunable[t][_step_])) / td->display_scale_factor;
	}
	agh_tunables0_put( &tset, _agh_basic_tunables_);
}

static void
__tunables_to_widgets()
{
	struct SConsumerTunableSetFull tset;
	const struct STunableDescription *td;
	agh_tunables0_get( &tset, _agh_basic_tunables_);
	for ( gushort t = 0; t < _agh_basic_tunables_; ++t ) {
		td = agh_tunable_get_description(t);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][_val_]),	td->display_scale_factor * tset.tunables[t]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][_min_]),	td->display_scale_factor * tset.lower_bounds[t]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][_max_]),	td->display_scale_factor * tset.upper_bounds[t]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][_step_]),	td->display_scale_factor * tset.steps[t]);
	}
}


void
tSimulations_switch_page_cb( GtkNotebook     *notebook,
			     GtkNotebookPage *page,
			     guint            page_num,
			     gpointer         user_data)
{
	struct SConsumerCtlParams ctlparams;
	if ( page_num == 1 ) {  // switching to display parameters tab
		agh_ctlparams0_get( &ctlparams);

	      // Controlling parameters frame
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlNTries),	ctlparams.siman_params.n_tries);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlItersFixedT),	ctlparams.siman_params.iters_fixed_T);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlStepSize),	ctlparams.siman_params.step_size);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlBoltzmannk),	ctlparams.siman_params.k);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlDampingMu),	ctlparams.siman_params.mu_t);
		float mantissa;
		int exponent;
		decompose_double( ctlparams.siman_params.t_min, &mantissa, &exponent);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinMantissa),	mantissa);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinExponent),	exponent);
		decompose_double( ctlparams.siman_params.t_initial, &mantissa, &exponent);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTInitialMantissa),	mantissa);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTInitialExponent),	exponent);

		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment1), ctlparams.DBAmendment1);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment2), ctlparams.DBAmendment2);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamAZAmendment),  ctlparams.AZAmendment);

	      // Unconventional scores frame
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamScoreMVTAsWake),
					      ctlparams.ScoreMVTAsWake);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamScoreUnscoredAsWake),
					      ctlparams.ScoreUnscoredAsWake);

	      // Tunables tab
		__tunables_to_widgets();

	} else {
	      // Controlling parameters frame
		ctlparams.siman_params.n_tries       = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlNTries));
		ctlparams.siman_params.iters_fixed_T = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlItersFixedT));
		ctlparams.siman_params.step_size     = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlStepSize));
		ctlparams.siman_params.k             = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlBoltzmannk));
		ctlparams.siman_params.mu_t          = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlDampingMu));
		ctlparams.siman_params.t_initial     = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTInitialMantissa))
			* pow(10, gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTInitialExponent)));
		ctlparams.siman_params.t_min	     = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinMantissa))
			* pow(10, gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinExponent)));
		ctlparams.DBAmendment1 = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment1));
		ctlparams.DBAmendment2 = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment2));
		ctlparams.AZAmendment  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamAZAmendment));

	      // Unconventional scores frame
		ctlparams.ScoreMVTAsWake      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamScoreMVTAsWake));
		ctlparams.ScoreUnscoredAsWake = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamScoreUnscoredAsWake));

		agh_ctlparams0_put(&ctlparams);

	      // Tunables tab
		__widgets_to_tunables();

	      // for ctlparam chnges to take effect on virgin modruns
		agh_modelrun_remove_untried();
		agh_populate_mSimulations( TRUE);
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

// #define UPDATE_HOOK
// 	agh_collect_simulations_from_tree(), agh_populate_mSimulations()




/*
void eTunable_S0_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_S0_);  }
void eTunable_S0_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_S0_);  }
void eTunable_S0_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_S0_);  }
void eTunable_S0_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_S0_); }
void eTunable_S0_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_S0_);  }

void eTunable_SU_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_SU_);  }
void eTunable_SU_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_SU_);  }
void eTunable_SU_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_SU_);  }
void eTunable_SU_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_SU_); }
void eTunable_SU_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_SU_);  }

void eTunable_fcR_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_fcR_);  }
void eTunable_fcR_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_fcR_);  }
void eTunable_fcR_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_fcR_);  }
void eTunable_fcR_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_fcR_); }
void eTunable_fcR_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_fcR_);  }

void eTunable_fcW_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_fcW_);  }
void eTunable_fcW_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_fcW_);  }
void eTunable_fcW_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_fcW_);  }
void eTunable_fcW_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_fcW_); }
void eTunable_fcW_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_fcW_);  }

void eTunable_gc_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_gc_);  }
void eTunable_gc_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_gc_);  }
void eTunable_gc_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_gc_);  }
void eTunable_gc_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_gc_); }
void eTunable_gc_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_gc_);  }

void eTunable_rc_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_rc_);  }
void eTunable_rc_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_rc_);  }
void eTunable_rc_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_rc_);  }
void eTunable_rc_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_rc_); }
void eTunable_rc_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_rc_);  }

void eTunable_rs_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_rs_);  }
void eTunable_rs_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_rs_);  }
void eTunable_rs_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_rs_);  }
void eTunable_rs_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_rs_); }
void eTunable_rs_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_rs_);  }

void eTunable_ta_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_ta_);  }
void eTunable_ta_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_ta_);  }
void eTunable_ta_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_ta_);  }
void eTunable_ta_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_ta_); }
void eTunable_ta_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_ta_);  }

void eTunable_tp_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_tp_);  }
void eTunable_tp_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_tp_);  }
void eTunable_tp_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_tp_);  }
void eTunable_tp_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_tp_); }
void eTunable_tp_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_tp_);  }
*/


void
bSimParamRevertTunables_clicked_cb()
{
	agh_tunables0_stock_defaults();
	__tunables_to_widgets();
}



// EOF
