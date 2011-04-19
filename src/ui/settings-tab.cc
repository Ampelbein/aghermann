// ;-*-C++-*- *  Time-stamp: "2011-04-19 01:55:10 hmmr"
/*
 *       File name:  ui/settings-tab.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  settings tab construct and callbacks
 *
 *         License:  GPL
 */


#include <cstring>
#include <glade/glade.h>

#include <array>
#include <initializer_list>

#include "misc.hh"
#include "ui.hh"
#include "settings.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


namespace aghui {

using namespace std;
using namespace agh;


list<SChannel>::iterator
	_AghTi,  // eeg channels
	_AghHi;  // all channels
list<string>::iterator
	_AghDi,
	_AghGi,  // groups
	_AghEi;  // going deprecated?

const CSubject
	*AghJ;




TFFTWinType
	AfDampingWindowType = TFFTWinType::welch;

float	OperatingRangeFrom = 2.,
	OperatingRangeUpto = 3.;


unsigned short
	DisplayPageSizeItem = 4,  // the one used to obtain FFTs
	FFTPageSizeCurrent = 2;


array<string, (size_t)TScore::_total> ScoreNames =
	{{ "Unscored", "NREM1", "NREM2", "NREM3", "NREM4", "REM", "Wake", "MVT"}};
array<string, (size_t)TScore::_total> ExtScoreCodes =
	{{" -0", "1", "2", "3", "4", "6Rr8", "Ww5", "mM"}};

float
	FreqBands[(size_t)TBand::_total][2] = {
	{  1.5,  4.0 },
	{  4.0,  8.0 },
	{  8.0, 12.0 },
	{ 15.0, 30.0 },
	{ 30.0, 40.0 },
};




GtkSpinButton
	*eBand[(size_t)TBand::_total][2];


inline namespace {
	GtkSpinButton
		*eCtlParamAnnlNTries,
		*eCtlParamAnnlItersFixedT,
		*eCtlParamAnnlStepSize,
		*eCtlParamAnnlBoltzmannk,
		*eCtlParamAnnlTInitialMantissa,
		*eCtlParamAnnlTInitialExponent,
		*eCtlParamAnnlDampingMu,
		*eCtlParamAnnlTMinMantissa,
		*eCtlParamAnnlTMinExponent,
		*eCtlParamNSWAPpBeforeSimStart,
		*eCtlParamReqScoredPercent;

	GtkCheckButton
		*eCtlParamDBAmendment1,
		*eCtlParamDBAmendment2,
		*eCtlParamAZAmendment;

	GtkRadioButton
		*eCtlParamScoreMVTAsWake,
		*eCtlParamScoreUnscoredAsWake;
	//	*eCtlParamScoreMVTAsPrevScore,
	//	*eCtlParamScoreUnscoredAsPrevScore,

	GtkSpinButton
		*eFFTParamsBinSize;
	GtkComboBox
		*eFFTParamsWindowType,
		*eFFTParamsPageSize,
	//	*eArtifSmoothOver,
		*eArtifWindowType;
	GtkEntry
		*eScoreCode[(size_t)TScore::_total];
	GtkSpinButton
		*ePatternDZCDFSigmaDefault,
		*ePatternDZCDFStepDefault,
		*ePatternDZCDFSmoothDefault,
		*ePatternFilterCutoffDefault,
		*ePatternFilterOrderDefault,
		*ePatternEnvTightnessDefault;
	GtkCheckButton
		*eSignalAnalysisUseOnNonEEG;
	GtkSpinButton
		*eDAPageHeight,
		*eDAPowerHeight,
		*eDASpectrumWidth,
		*eDAEMGHeight,

		*eTunable[(size_t)TTunable::_basic_tunables][4];
}


// ------------------ construct

int
construct_Settings( GladeXML *xml)
{
	GtkCellRenderer *renderer;

     // ------------- fFFTParams
	if ( !GLADEXML2 (GtkSpinButton,	eFFTParamsBinSize) ||
	     !GLADEXML2 (GtkComboBox,	eFFTParamsPageSize) ||
	     !GLADEXML2 (GtkComboBox,	eFFTParamsWindowType) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eFFTParamsPageSize),
				 GTK_TREE_MODEL (mFFTParamsPageSize));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT (eFFTParamsPageSize), renderer, FALSE);
	gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT (eFFTParamsPageSize), renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( GTK_COMBO_BOX (eFFTParamsWindowType),
				 GTK_TREE_MODEL (mFFTParamsWindowType));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eFFTParamsWindowType), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eFFTParamsWindowType), renderer,
					"text", 0,
					NULL);

      // ------------- fArtifacts
	if ( !GLADEXML2 (GtkComboBox,	eArtifWindowType) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eArtifWindowType),
				 GTK_TREE_MODEL (mAfDampingWindowType));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eArtifWindowType), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eArtifWindowType), renderer,
					"text", 0,
					NULL);

      // ------- custom score codes
	if ( !(eScoreCode[(size_t)TScore::none]		= (GtkEntry*)glade_xml_get_widget( xml, "eScoreCodeUnscored")) ||
	     !(eScoreCode[(size_t)TScore::nrem1]	= (GtkEntry*)glade_xml_get_widget( xml, "eScoreCodeNREM1")) ||
	     !(eScoreCode[(size_t)TScore::nrem2]	= (GtkEntry*)glade_xml_get_widget( xml, "eScoreCodeNREM2")) ||
	     !(eScoreCode[(size_t)TScore::nrem3]	= (GtkEntry*)glade_xml_get_widget( xml, "eScoreCodeNREM3")) ||
	     !(eScoreCode[(size_t)TScore::nrem4]	= (GtkEntry*)glade_xml_get_widget( xml, "eScoreCodeNREM4")) ||
	     !(eScoreCode[(size_t)TScore::rem]		= (GtkEntry*)glade_xml_get_widget( xml, "eScoreCodeREM")) ||
	     !(eScoreCode[(size_t)TScore::wake]		= (GtkEntry*)glade_xml_get_widget( xml, "eScoreCodeWake")) ||
	     !(eScoreCode[(size_t)TScore::mvt]		= (GtkEntry*)glade_xml_get_widget( xml, "eScoreCodeMVT")) )
		return -1;

      // ----------- fSignalCriteria
	if ( !GLADEXML2 (GtkSpinButton,	ePatternDZCDFStepDefault) ||
	     !GLADEXML2 (GtkSpinButton,	ePatternDZCDFSigmaDefault) ||
	     !GLADEXML2 (GtkSpinButton,	ePatternDZCDFSmoothDefault) ||
	     !GLADEXML2 (GtkSpinButton, ePatternFilterCutoffDefault) ||
	     !GLADEXML2 (GtkSpinButton,	ePatternFilterOrderDefault) ||
	     !GLADEXML2 (GtkSpinButton,	ePatternEnvTightnessDefault) ||
	     !GLADEXML2 (GtkCheckButton, eSignalAnalysisUseOnNonEEG) )
		return -1;

      // --------- Bands
	if ( !(eBand[(size_t)TBand::delta][0]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandDeltaFrom")) ||
	     !(eBand[(size_t)TBand::delta][1]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandDeltaUpto")) ||
	     !(eBand[(size_t)TBand::theta][0]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandThetaFrom")) ||
	     !(eBand[(size_t)TBand::theta][1]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandThetaUpto")) ||
	     !(eBand[(size_t)TBand::alpha][0]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandAlphaFrom")) ||
	     !(eBand[(size_t)TBand::alpha][1]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandAlphaUpto")) ||
	     !(eBand[(size_t)TBand::beta ][0]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandBetaFrom" )) ||
	     !(eBand[(size_t)TBand::beta ][1]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandBetaUpto" )) ||
	     !(eBand[(size_t)TBand::gamma][0]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandGammaFrom")) ||
	     !(eBand[(size_t)TBand::gamma][1]   = (GtkSpinButton*)glade_xml_get_widget( xml, "eBandGammaUpto")) )
		return -1;

      // --------- Misc
	if ( !GLADEXML2 (GtkSpinButton, eDAPageHeight) ||
	     !GLADEXML2 (GtkSpinButton, eDAPowerHeight) ||
	     !GLADEXML2 (GtkSpinButton, eDASpectrumWidth) ||
	     !GLADEXML2 (GtkSpinButton, eDAEMGHeight) )
		return -1;

     // ------------- eCtrlParam*
	if ( !GLADEXML2 (GtkSpinButton,		eCtlParamAnnlNTries) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamAnnlItersFixedT) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamAnnlStepSize) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamAnnlBoltzmannk) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamAnnlDampingMu) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamAnnlTInitialMantissa) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamAnnlTInitialExponent) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamAnnlTMinMantissa) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamAnnlTMinExponent) ||
	     !GLADEXML2 (GtkCheckButton,	eCtlParamDBAmendment1) ||
	     !GLADEXML2 (GtkCheckButton,	eCtlParamDBAmendment2) ||
	     !GLADEXML2 (GtkCheckButton,	eCtlParamAZAmendment) ||
	     !GLADEXML2 (GtkRadioButton,	eCtlParamScoreMVTAsWake) ||
	     !GLADEXML2 (GtkRadioButton,	eCtlParamScoreUnscoredAsWake) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamNSWAPpBeforeSimStart) ||
	     !GLADEXML2 (GtkSpinButton,		eCtlParamReqScoredPercent) )
		return -1;

      // ------------- eTunable_*
	if ( !(eTunable[(size_t)TTunable::rs][(size_t)TTIdx::val]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_rs")) ||
	     !(eTunable[(size_t)TTunable::rs][(size_t)TTIdx::min]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_rs_min")) ||
	     !(eTunable[(size_t)TTunable::rs][(size_t)TTIdx::max]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_rs_max")) ||
	     !(eTunable[(size_t)TTunable::rs][(size_t)TTIdx::step]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_rs_step")) ||

	     !(eTunable[(size_t)TTunable::rc][(size_t)TTIdx::val]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_rc")) ||
	     !(eTunable[(size_t)TTunable::rc][(size_t)TTIdx::min]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_rc_min")) ||
	     !(eTunable[(size_t)TTunable::rc][(size_t)TTIdx::max]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_rc_max")) ||
	     !(eTunable[(size_t)TTunable::rc][(size_t)TTIdx::step]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_rc_step")) ||

	     !(eTunable[(size_t)TTunable::fcR][(size_t)TTIdx::val]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_fcR")) ||
	     !(eTunable[(size_t)TTunable::fcR][(size_t)TTIdx::min]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_fcR_min")) ||
	     !(eTunable[(size_t)TTunable::fcR][(size_t)TTIdx::max]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_fcR_max")) ||
	     !(eTunable[(size_t)TTunable::fcR][(size_t)TTIdx::step]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_fcR_step")) ||

	     !(eTunable[(size_t)TTunable::fcW][(size_t)TTIdx::val]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_fcW")) ||
	     !(eTunable[(size_t)TTunable::fcW][(size_t)TTIdx::min]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_fcW_min")) ||
	     !(eTunable[(size_t)TTunable::fcW][(size_t)TTIdx::max]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_fcW_max")) ||
	     !(eTunable[(size_t)TTunable::fcW][(size_t)TTIdx::step]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_fcW_step")) ||

	     !(eTunable[(size_t)TTunable::S0][(size_t)TTIdx::val]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_S0")) ||
	     !(eTunable[(size_t)TTunable::S0][(size_t)TTIdx::min]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_S0_min")) ||
	     !(eTunable[(size_t)TTunable::S0][(size_t)TTIdx::max]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_S0_max")) ||
	     !(eTunable[(size_t)TTunable::S0][(size_t)TTIdx::step]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_S0_step")) ||

	     !(eTunable[(size_t)TTunable::SU][(size_t)TTIdx::val]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_SU")) ||
	     !(eTunable[(size_t)TTunable::SU][(size_t)TTIdx::min]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_SU_min")) ||
	     !(eTunable[(size_t)TTunable::SU][(size_t)TTIdx::max]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_SU_max")) ||
	     !(eTunable[(size_t)TTunable::SU][(size_t)TTIdx::step]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_SU_step")) ||

	     !(eTunable[(size_t)TTunable::ta][(size_t)TTIdx::val]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_ta")) ||
	     !(eTunable[(size_t)TTunable::ta][(size_t)TTIdx::min]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_ta_min")) ||
	     !(eTunable[(size_t)TTunable::ta][(size_t)TTIdx::max]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_ta_max")) ||
	     !(eTunable[(size_t)TTunable::ta][(size_t)TTIdx::step]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_ta_step")) ||

	     !(eTunable[(size_t)TTunable::tp][(size_t)TTIdx::val]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_tp")) ||
	     !(eTunable[(size_t)TTunable::tp][(size_t)TTIdx::min]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_tp_min")) ||
	     !(eTunable[(size_t)TTunable::tp][(size_t)TTIdx::max]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_tp_max")) ||
	     !(eTunable[(size_t)TTunable::tp][(size_t)TTIdx::step]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_tp_step")) ||

	     !(eTunable[(size_t)TTunable::gc][(size_t)TTIdx::val]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_gc")) ||
	     !(eTunable[(size_t)TTunable::gc][(size_t)TTIdx::min]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_gc_min")) ||
	     !(eTunable[(size_t)TTunable::gc][(size_t)TTIdx::max]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_gc_max")) ||
	     !(eTunable[(size_t)TTunable::gc][(size_t)TTIdx::step]		= (GtkSpinButton*)glade_xml_get_widget( xml, "eTunable_gc_step")) )
		return -1;

	return 0;
}


// ============== Measurements settings tab

// --- PSD & Scoring



extern "C" {
void
tDesign_switch_page_cb( GtkNotebook     *notebook,
			gpointer	 unused,
			guint            page_num,
			gpointer         user_data)
{
      // save parameters changing which should trigger tree rescan
	static size_t
		FFTPageSizeCurrent_saved;
	static TFFTWinType
		FFTWindowType_saved,
		AfDampingWindowType_saved;
	static float
		FFTBinSize_saved;

	if ( page_num == 0 ) {  // switching back from settings tab

	      // collect values from widgets
		AghCC->fft_params.page_size =
			FFTPageSizeValues[ FFTPageSizeCurrent = gtk_combo_box_get_active( GTK_COMBO_BOX (eFFTParamsPageSize))];
		DisplayPageSizeItem = 0;
		while ( DisplayPageSizeValues[DisplayPageSizeItem] != FFTPageSizeValues[FFTPageSizeCurrent] )
			assert ( ++DisplayPageSizeItem > 10 );

		AghCC->fft_params.welch_window_type =
			(TFFTWinType)gtk_combo_box_get_active( GTK_COMBO_BOX (eFFTParamsWindowType));
		AghCC->fft_params.bin_size =
			gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFFTParamsBinSize));
		AghCC->af_dampen_window_type =
			(TFFTWinType)gtk_combo_box_get_active( GTK_COMBO_BOX (eArtifWindowType));

		for ( gushort i = 0; i < (size_t)TScore::_total; ++i )
			ExtScoreCodes[i] = gtk_entry_get_text( GTK_ENTRY (eScoreCode[i]));

		EnvTightness	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternEnvTightnessDefault));
		BWFCutoff	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterCutoffDefault));
		BWFOrder	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterOrderDefault));
		DZCDFStep	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFStepDefault));
		DZCDFSigma	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSigmaDefault));
		DZCDFSmooth	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSmoothDefault));
		UseSigAnOnNonEEGChannels =
			gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eSignalAnalysisUseOnNonEEG));

		FreqBands[(size_t)TBand::delta][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::delta][0]));
		FreqBands[(size_t)TBand::delta][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::delta][1]));
		FreqBands[(size_t)TBand::theta][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::theta][0]));
		FreqBands[(size_t)TBand::theta][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::theta][1]));
		FreqBands[(size_t)TBand::alpha][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::alpha][0]));
		FreqBands[(size_t)TBand::alpha][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::alpha][1]));
		FreqBands[(size_t)TBand::beta ][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::beta ][0]));
		FreqBands[(size_t)TBand::beta ][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::beta ][1]));
		FreqBands[(size_t)TBand::gamma][0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::gamma][0]));
		FreqBands[(size_t)TBand::gamma][1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::gamma][1]));

		SFDAPageHeight		= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eDAPageHeight));
		SFDASpectrumWidth	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eDASpectrumWidth));
		SFDAPowerProfileHeight	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eDAPowerHeight));
		SFDAEMGProfileHeight	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eDAEMGHeight));

	      // scan as necessary
		if ( FFTPageSizeCurrent_saved != FFTPageSizeCurrent ||
		     FFTWindowType_saved != AghCC->fft_params.welch_window_type ||
		     AfDampingWindowType_saved != AghCC->af_dampen_window_type ||
		     FFTBinSize_saved != AghCC->fft_params.bin_size ) {
		      // rescan tree
			set_cursor_busy( TRUE, GTK_WIDGET (wMainWindow));
			gtk_widget_set_sensitive( GTK_WIDGET (wMainWindow), FALSE);
			while ( gtk_events_pending() )
				gtk_main_iteration();
			AghCC->scan_tree( progress_indicator);
			populate( 0); // no new objects expected, don't depopulate (don't rebuild gtk tree storaage)

			set_cursor_busy( false, GTK_WIDGET (wMainWindow));
			gtk_widget_set_sensitive( GTK_WIDGET (wMainWindow), TRUE);
			gtk_statusbar_push( GTK_STATUSBAR (sbMainStatusBar), sbContextIdGeneral,
					    "Scanning complete");
		}
	} else {
		FFTPageSizeCurrent_saved  = FFTPageSizeCurrent;
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
		gtk_combo_box_set_active( GTK_COMBO_BOX (eFFTParamsPageSize), FFTPageSizeCurrent = i);

		gtk_combo_box_set_active( GTK_COMBO_BOX (eFFTParamsWindowType), (int)AghCC->fft_params.welch_window_type);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eFFTParamsBinSize), (int)AghCC->fft_params.bin_size);

		// artifacts
//		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eArtifSmoothOver), agh_af_get_smoothover());
		gtk_combo_box_set_active( GTK_COMBO_BOX (eArtifWindowType), (int)AghCC->af_dampen_window_type);

		// custom score codes
		for ( gushort i = 0; i < (size_t)TScore::_total; ++i )
			gtk_entry_set_text( GTK_ENTRY (eScoreCode[i]), ExtScoreCodes[i].c_str());

		// signal criteria
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternEnvTightnessDefault),	EnvTightness);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternFilterCutoffDefault),	BWFCutoff);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternFilterOrderDefault),	BWFOrder);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFStepDefault),		DZCDFStep);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFSigmaDefault),		DZCDFSigma);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFSmoothDefault),	DZCDFSmooth);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eSignalAnalysisUseOnNonEEG),	UseSigAnOnNonEEGChannels);

		// misc
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::delta][0]), FreqBands[(size_t)TBand::delta][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::delta][1]), FreqBands[(size_t)TBand::delta][1]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::theta][0]), FreqBands[(size_t)TBand::theta][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::theta][1]), FreqBands[(size_t)TBand::theta][1]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::alpha][0]), FreqBands[(size_t)TBand::alpha][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::alpha][1]), FreqBands[(size_t)TBand::alpha][1]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::beta ][0]), FreqBands[(size_t)TBand::beta ][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::beta ][1]), FreqBands[(size_t)TBand::beta ][1]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::gamma][0]), FreqBands[(size_t)TBand::gamma][0]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[(size_t)TBand::gamma][1]), FreqBands[(size_t)TBand::gamma][1]);

		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eDAPageHeight),    SFDAPageHeight);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eDASpectrumWidth), SFDASpectrumWidth);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eDAPowerHeight),   SFDAPowerProfileHeight);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eDAEMGHeight),     SFDAEMGProfileHeight);

		// colours are served specially elsewhere
	}
}


void
ePatternDZCDFStepFineDefault_toggled_cb( GtkToggleButton *togglebutton, gpointer unused)
{
	gtk_widget_set_sensitive( GTK_WIDGET (ePatternDZCDFStepDefault),
				  !gtk_toggle_button_get_active( togglebutton));
}


void
tTaskSelector_switch_page_cb( GtkNotebook     *notebook,
			      gpointer	       unused,
			      guint            page_num,
			      gpointer         user_data)
{
	if ( page_num == 1 ) {
		simview::populate( true);
		snprintf_buf( "Session: <b>%s</b>", AghD());
		gtk_label_set_markup( GTK_LABEL (lSimulationsSession), __buf__);
		snprintf_buf( "Channel: <b>%s</b>", AghT());
		gtk_label_set_markup( GTK_LABEL (lSimulationsChannel), __buf__);
		gtk_widget_set_sensitive( GTK_WIDGET (bExpChange), FALSE);
	} else if ( page_num == 0 ) {
		remove_untried_modruns();
		msmtview::populate();
		gtk_widget_set_sensitive( GTK_WIDGET (bExpChange), TRUE);
	}
}
} // extern "C"






// ================== Simulations part



inline namespace {
void
__widgets_to_tunables()
{
	for ( TTunable_underlying_type t = 0; t < TTunable::_basic_tunables; ++t ) {
		AghCC->tunables0.value [t] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eTunable[t][(size_t)TTIdx::val ])) / __AGHTT[t].display_scale_factor;
		AghCC->tunables0.lo    [t] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eTunable[t][(size_t)TTIdx::min ])) / __AGHTT[t].display_scale_factor;
		AghCC->tunables0.hi    [t] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eTunable[t][(size_t)TTIdx::max ])) / __AGHTT[t].display_scale_factor;
		AghCC->tunables0.step  [t] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eTunable[t][(size_t)TTIdx::step])) / __AGHTT[t].display_scale_factor;
	}
}


void
__tunables_to_widgets()
{
	for ( TTunable_underlying_type t = 0; t < TTunable::_basic_tunables; ++t ) {
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][(size_t)TTIdx::val]),	__AGHTT[t].display_scale_factor * AghCC->tunables0.value[t]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][(size_t)TTIdx::min]),	__AGHTT[t].display_scale_factor * AghCC->tunables0.lo   [t]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][(size_t)TTIdx::max]),	__AGHTT[t].display_scale_factor * AghCC->tunables0.hi   [t]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][(size_t)TTIdx::step]),	__AGHTT[t].display_scale_factor * AghCC->tunables0.step [t]);
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
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlNTries),	AghCC->ctl_params0.siman_params.n_tries);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlItersFixedT),	AghCC->ctl_params0.siman_params.iters_fixed_T);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlStepSize),	AghCC->ctl_params0.siman_params.step_size);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlBoltzmannk),	AghCC->ctl_params0.siman_params.k);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlDampingMu),	AghCC->ctl_params0.siman_params.mu_t);
		float mantissa;
		int exponent;
		decompose_double( AghCC->ctl_params0.siman_params.t_min, &mantissa, &exponent);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinMantissa),	mantissa);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinExponent),	exponent);
		decompose_double( AghCC->ctl_params0.siman_params.t_initial, &mantissa, &exponent);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTInitialMantissa),	mantissa);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTInitialExponent),	exponent);

	      // Achermann parameters
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment1), AghCC->ctl_params0.DBAmendment1);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment2), AghCC->ctl_params0.DBAmendment2);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamAZAmendment),  AghCC->ctl_params0.AZAmendment);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamNSWAPpBeforeSimStart), AghCC->ctl_params0.swa_laden_pages_before_SWA_0);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamReqScoredPercent), AghCC->ctl_params0.req_percent_scored);

	      // Unconventional scores frame
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamScoreMVTAsWake),
					      AghCC->ctl_params0.ScoreMVTAsWake);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamScoreUnscoredAsWake),
					      AghCC->ctl_params0.ScoreUnscoredAsWake);

	      // Tunables tab
		__tunables_to_widgets();

	} else {
	      // Controlling parameters frame
		AghCC->ctl_params0.siman_params.n_tries       = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlNTries));
		AghCC->ctl_params0.siman_params.iters_fixed_T = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlItersFixedT));
		AghCC->ctl_params0.siman_params.step_size     = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlStepSize));
		AghCC->ctl_params0.siman_params.k             = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlBoltzmannk));
		AghCC->ctl_params0.siman_params.mu_t          = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlDampingMu));
		AghCC->ctl_params0.siman_params.t_initial     = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTInitialMantissa))
			* pow(10, gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTInitialExponent)));
		AghCC->ctl_params0.siman_params.t_min	     = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinMantissa))
			* pow(10, gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinExponent)));
	      // Achermann parameters
		AghCC->ctl_params0.DBAmendment1 = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment1));
		AghCC->ctl_params0.DBAmendment2 = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment2));
		AghCC->ctl_params0.AZAmendment  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamAZAmendment));
		AghCC->ctl_params0.swa_laden_pages_before_SWA_0	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamNSWAPpBeforeSimStart));
		AghCC->ctl_params0.req_percent_scored		= gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamReqScoredPercent));

	      // Unconventional scores frame
		AghCC->ctl_params0.ScoreMVTAsWake      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamScoreMVTAsWake));
		AghCC->ctl_params0.ScoreUnscoredAsWake = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamScoreUnscoredAsWake));

	      // Tunables tab
		__widgets_to_tunables();

	      // for ctlparam chnges to take effect on virgin modruns
		remove_untried_modruns();
		simview::populate( true);
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
	AghCC->tunables0.assign_defaults();
	__tunables_to_widgets();
}

} // extern "C"

} // namespace aghui

// EOF
