// ;-*-C++-*- *  Time-stamp: "2011-05-19 02:46:54 hmmr"
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

#include <array>
#include <initializer_list>

#include "../libagh/enums.hh"
#include "ui.hh"
#include "misc.hh"
#include "settings.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;
using namespace agh;


namespace aghui {

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
		*eDAPageHeight,
		*eDAPowerHeight,
		*eDASpectrumWidth,
		*eDAEMGHeight,

		*eTunable[(size_t)TTunable::_basic_tunables][4];
}




namespace settings {

string
	LastExpdesignDir;
int
	LastExpdesignDirNo;

TFFTWinType
	AfDampingWindowType = TFFTWinType::welch;

float	OperatingRangeFrom = 2.,
	OperatingRangeUpto = 3.;


unsigned short
	DisplayPageSizeItem = 4,  // the one used to obtain FFTs
	FFTPageSizeCurrent = 2;


agh::CHypnogram::TCustomScoreCodes ExtScoreCodes =
	{{" -0", "1", "2", "3", "4", "6Rr8", "Ww5", "mM"}};

float	FreqBands[(size_t)TBand::_total][2] = {
	{  1.5,  4.0 },
	{  4.0,  8.0 },
	{  8.0, 12.0 },
	{ 15.0, 30.0 },
	{ 30.0, 40.0 },
};

bool	SimRunbatchIncludeAllChannels,
	SimRunbatchIncludeAllSessions,
	SimRunbatchIterateRanges;

unsigned int
	SFDAPageHeight,
	SFDASpectrumWidth,
	SFDAPowerProfileHeight,
	SFDAEMGProfileHeight;


const char // not quite a settings item, this
	*const FreqBandNames[(size_t)agh::TBand::_total] = {
	"Delta", "Theta", "Alpha", "Beta", "Gamma",
};


GtkListStore
	*mFFTParamsWindowType,
	*mFFTParamsPageSize,
	*mScoringPageSize,
	*mAfDampingWindowType;




// ------------------ construct

int
construct_once()
{
	mScoringPageSize =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mFFTParamsPageSize =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mFFTParamsWindowType =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mAfDampingWindowType =
		gtk_list_store_new( 1, G_TYPE_STRING);

	GtkCellRenderer *renderer;

     // ------------- fFFTParams
	if ( !AGH_GBGETOBJ (GtkSpinButton,	eFFTParamsBinSize) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eFFTParamsPageSize) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eFFTParamsWindowType) )
		return -1;

	gtk_combo_box_set_model( eFFTParamsPageSize,
				 (GtkTreeModel*)mFFTParamsPageSize);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eFFTParamsPageSize, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eFFTParamsPageSize, renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( eFFTParamsWindowType,
				 (GtkTreeModel*)mFFTParamsWindowType);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eFFTParamsWindowType, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eFFTParamsWindowType, renderer,
					"text", 0,
					NULL);

      // ------------- fArtifacts
	if ( !AGH_GBGETOBJ (GtkComboBox,	eArtifWindowType) )
		return -1;

	gtk_combo_box_set_model( eArtifWindowType,
				 (GtkTreeModel*)mAfDampingWindowType);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eArtifWindowType, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eArtifWindowType, renderer,
					"text", 0,
					NULL);

      // ------- custom score codes
	if ( !(eScoreCode[(size_t)TScore::none]		= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeUnscored")) ||
	     !(eScoreCode[(size_t)TScore::nrem1]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM1")) ||
	     !(eScoreCode[(size_t)TScore::nrem2]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM2")) ||
	     !(eScoreCode[(size_t)TScore::nrem3]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM3")) ||
	     !(eScoreCode[(size_t)TScore::nrem4]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM4")) ||
	     !(eScoreCode[(size_t)TScore::rem]		= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeREM")) ||
	     !(eScoreCode[(size_t)TScore::wake]		= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeWake")) ||
	     !(eScoreCode[(size_t)TScore::mvt]		= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeMVT")) )
		return -1;

      // --------- Bands
	if ( !(eBand[(size_t)TBand::delta][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandDeltaFrom")) ||
	     !(eBand[(size_t)TBand::delta][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandDeltaUpto")) ||
	     !(eBand[(size_t)TBand::theta][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandThetaFrom")) ||
	     !(eBand[(size_t)TBand::theta][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandThetaUpto")) ||
	     !(eBand[(size_t)TBand::alpha][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandAlphaFrom")) ||
	     !(eBand[(size_t)TBand::alpha][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandAlphaUpto")) ||
	     !(eBand[(size_t)TBand::beta ][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandBetaFrom" )) ||
	     !(eBand[(size_t)TBand::beta ][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandBetaUpto" )) ||
	     !(eBand[(size_t)TBand::gamma][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandGammaFrom")) ||
	     !(eBand[(size_t)TBand::gamma][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandGammaUpto")) )
		return -1;

      // --------- Misc
	if ( !AGH_GBGETOBJ (GtkSpinButton, eDAPageHeight) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAPowerHeight) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDASpectrumWidth) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAEMGHeight) )
		return -1;

     // ------------- eCtrlParam*
	if ( !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamAnnlNTries) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamAnnlItersFixedT) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamAnnlStepSize) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamAnnlBoltzmannk) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamAnnlDampingMu) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamAnnlTInitialMantissa) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamAnnlTInitialExponent) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamAnnlTMinMantissa) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamAnnlTMinExponent) ||
	     !AGH_GBGETOBJ (GtkCheckButton,	eCtlParamDBAmendment1) ||
	     !AGH_GBGETOBJ (GtkCheckButton,	eCtlParamDBAmendment2) ||
	     !AGH_GBGETOBJ (GtkCheckButton,	eCtlParamAZAmendment) ||
	     !AGH_GBGETOBJ (GtkRadioButton,	eCtlParamScoreMVTAsWake) ||
	     !AGH_GBGETOBJ (GtkRadioButton,	eCtlParamScoreUnscoredAsWake) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamNSWAPpBeforeSimStart) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamReqScoredPercent) )
		return -1;

      // ------------- eTunable_*
	if ( !(eTunable[(size_t)TTunable::rs][(size_t)TTIdx::val]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_rs")) ||
	     !(eTunable[(size_t)TTunable::rs][(size_t)TTIdx::min]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_rs_min")) ||
	     !(eTunable[(size_t)TTunable::rs][(size_t)TTIdx::max]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_rs_max")) ||
	     !(eTunable[(size_t)TTunable::rs][(size_t)TTIdx::step]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_rs_step")) ||

	     !(eTunable[(size_t)TTunable::rc][(size_t)TTIdx::val]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_rc")) ||
	     !(eTunable[(size_t)TTunable::rc][(size_t)TTIdx::min]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_rc_min")) ||
	     !(eTunable[(size_t)TTunable::rc][(size_t)TTIdx::max]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_rc_max")) ||
	     !(eTunable[(size_t)TTunable::rc][(size_t)TTIdx::step]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_rc_step")) ||

	     !(eTunable[(size_t)TTunable::fcR][(size_t)TTIdx::val]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_fcR")) ||
	     !(eTunable[(size_t)TTunable::fcR][(size_t)TTIdx::min]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_fcR_min")) ||
	     !(eTunable[(size_t)TTunable::fcR][(size_t)TTIdx::max]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_fcR_max")) ||
	     !(eTunable[(size_t)TTunable::fcR][(size_t)TTIdx::step]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_fcR_step")) ||

	     !(eTunable[(size_t)TTunable::fcW][(size_t)TTIdx::val]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_fcW")) ||
	     !(eTunable[(size_t)TTunable::fcW][(size_t)TTIdx::min]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_fcW_min")) ||
	     !(eTunable[(size_t)TTunable::fcW][(size_t)TTIdx::max]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_fcW_max")) ||
	     !(eTunable[(size_t)TTunable::fcW][(size_t)TTIdx::step]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_fcW_step")) ||

	     !(eTunable[(size_t)TTunable::S0][(size_t)TTIdx::val]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_S0")) ||
	     !(eTunable[(size_t)TTunable::S0][(size_t)TTIdx::min]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_S0_min")) ||
	     !(eTunable[(size_t)TTunable::S0][(size_t)TTIdx::max]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_S0_max")) ||
	     !(eTunable[(size_t)TTunable::S0][(size_t)TTIdx::step]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_S0_step")) ||

	     !(eTunable[(size_t)TTunable::SU][(size_t)TTIdx::val]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_SU")) ||
	     !(eTunable[(size_t)TTunable::SU][(size_t)TTIdx::min]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_SU_min")) ||
	     !(eTunable[(size_t)TTunable::SU][(size_t)TTIdx::max]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_SU_max")) ||
	     !(eTunable[(size_t)TTunable::SU][(size_t)TTIdx::step]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_SU_step")) ||

	     !(eTunable[(size_t)TTunable::ta][(size_t)TTIdx::val]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_ta")) ||
	     !(eTunable[(size_t)TTunable::ta][(size_t)TTIdx::min]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_ta_min")) ||
	     !(eTunable[(size_t)TTunable::ta][(size_t)TTIdx::max]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_ta_max")) ||
	     !(eTunable[(size_t)TTunable::ta][(size_t)TTIdx::step]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_ta_step")) ||

	     !(eTunable[(size_t)TTunable::tp][(size_t)TTIdx::val]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_tp")) ||
	     !(eTunable[(size_t)TTunable::tp][(size_t)TTIdx::min]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_tp_min")) ||
	     !(eTunable[(size_t)TTunable::tp][(size_t)TTIdx::max]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_tp_max")) ||
	     !(eTunable[(size_t)TTunable::tp][(size_t)TTIdx::step]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_tp_step")) ||

	     !(eTunable[(size_t)TTunable::gc][(size_t)TTIdx::val]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_gc")) ||
	     !(eTunable[(size_t)TTunable::gc][(size_t)TTIdx::min]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_gc_min")) ||
	     !(eTunable[(size_t)TTunable::gc][(size_t)TTIdx::max]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_gc_max")) ||
	     !(eTunable[(size_t)TTunable::gc][(size_t)TTIdx::step]	= (GtkSpinButton*)gtk_builder_get_object( __builder, "eTunable_gc_step")) )
		return -1;

	return 0;
}

} // namespace settings


// ============== Measurements settings tab

// --- PSD & Scoring


using namespace settings;

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
				assert ( ++DisplayPageSizeItem < 10 );

			AghCC->fft_params.welch_window_type =
				(TFFTWinType)gtk_combo_box_get_active( GTK_COMBO_BOX (eFFTParamsWindowType));
			AghCC->fft_params.bin_size =
				gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFFTParamsBinSize));
			AghCC->af_dampen_window_type =
				(TFFTWinType)gtk_combo_box_get_active( GTK_COMBO_BOX (eArtifWindowType));

			for ( gushort i = 0; i < (size_t)TScore::_total; ++i )
				ExtScoreCodes[i] = gtk_entry_get_text( GTK_ENTRY (eScoreCode[i]));

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
			gtk_combo_box_set_active( eFFTParamsPageSize, FFTPageSizeCurrent = i);

			gtk_combo_box_set_active( eFFTParamsWindowType, (int)AghCC->fft_params.welch_window_type);
			gtk_spin_button_set_value( eFFTParamsBinSize, (int)AghCC->fft_params.bin_size);

			// artifacts
			gtk_combo_box_set_active( eArtifWindowType, (int)AghCC->af_dampen_window_type);

			// custom score codes
			for ( gushort i = 0; i < (size_t)TScore::_total; ++i )
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

			gtk_spin_button_set_value( eDAPageHeight,    SFDAPageHeight);
			gtk_spin_button_set_value( eDASpectrumWidth, SFDASpectrumWidth);
			gtk_spin_button_set_value( eDAPowerHeight,   SFDAPowerProfileHeight);
			gtk_spin_button_set_value( eDAEMGHeight,     SFDAEMGProfileHeight);

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
			msmtview::populate();
			gtk_widget_set_sensitive( (GtkWidget*)bExpChange, TRUE);
		}
	}
} // extern "C"






// ================== Simulations part



inline namespace {
void
__widgets_to_tunables()
{
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
			gtk_toggle_button_set_active( (GtkToggleButton*)eCtlParamDBAmendment1, AghCC->ctl_params0.DBAmendment1);
			gtk_toggle_button_set_active( (GtkToggleButton*)eCtlParamDBAmendment2, AghCC->ctl_params0.DBAmendment2);
			gtk_toggle_button_set_active( (GtkToggleButton*)eCtlParamAZAmendment,  AghCC->ctl_params0.AZAmendment);
			gtk_spin_button_set_value( eCtlParamNSWAPpBeforeSimStart, AghCC->ctl_params0.swa_laden_pages_before_SWA_0);
			gtk_spin_button_set_value( eCtlParamReqScoredPercent, AghCC->ctl_params0.req_percent_scored);

		      // Unconventional scores frame
			gtk_toggle_button_set_active( (GtkToggleButton*)eCtlParamScoreMVTAsWake,
						      AghCC->ctl_params0.ScoreMVTAsWake);
			gtk_toggle_button_set_active( (GtkToggleButton*)eCtlParamScoreUnscoredAsWake,
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
			AghCC->ctl_params0.DBAmendment1 = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment1));
			AghCC->ctl_params0.DBAmendment2 = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment2));
			AghCC->ctl_params0.AZAmendment  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamAZAmendment));
			AghCC->ctl_params0.swa_laden_pages_before_SWA_0	= gtk_spin_button_get_value( eCtlParamNSWAPpBeforeSimStart);
			AghCC->ctl_params0.req_percent_scored		= gtk_spin_button_get_value( eCtlParamReqScoredPercent);

		      // Unconventional scores frame
			AghCC->ctl_params0.ScoreMVTAsWake      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamScoreMVTAsWake));
			AghCC->ctl_params0.ScoreUnscoredAsWake = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eCtlParamScoreUnscoredAsWake));

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

} // namespace aghui

// EOF
