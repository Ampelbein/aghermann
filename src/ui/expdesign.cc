// ;-*-C++-*- *  Time-stamp: "2011-06-30 20:41:07 hmmr"
/*
 *       File name:  ui/measurements.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  measurements overview view
 *
 *         License:  GPL
 */


#include <cstring>
#include <ctime>

#include <cairo.h>
#include <cairo-svg.h>

#include "misc.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

using namespace aghui;

inline namespace {

      // supporting ui stuff
	GtkTextBuffer
		*textbuf2;

	enum class TTipIdx {
		general = 0,
	};

	const char*
       		__tooltips[] = {
			"<b>Subject timeline:</b>\n"
			"	Ctrl+Wheel:	change scale;\n"
			"	Click1:		view/score episode;\n"
			"	Click3:		show edf file info;\n"
			"	Alt+Click3:	save timeline as svg.",
	};
} // inline namespace




aghui::SSubjectPresentation::SSubjectPresentation( agh::CSubject& _j, SGroupPresentation& parent)
      : csubject (_j),
	da (NULL),
	is_focused (false),
	_parent (parent)
{
	try {
		cscourse = new agh::CSCourse (csubject, *_AghDi, *_AghTi,
					      OperatingRangeFrom, OperatingRangeUpto,
					      0., 0, false, false);
		tl_start = csubject.measurements[*_AghDi].episodes.front().start_rel;
	} catch (...) {  // can be invalid_argument (no recording in such session/channel) or some TSimPrepError
		cscourse = NULL;
	}
	episode_focused = csubject.measurements[*_parent._parent._AghDi].episodes.end();
}


aghui::SSubjectPresentation::~SSubjectPresentation()
{
	if ( cscourse )
		delete cscourse;
}








static const char
	*const aghui::SExpDesignUI::FreqBandNames[(size_t)agh::TBand::_total] = {
	"Delta", "Theta", "Alpha", "Beta", "Gamma",
};

static const array<unsigned, 4>
	aghui::SExpDesignUI::FFTPageSizeValues = {15, 20, 30, 60};


aghui::SExpDesignUI::SExpDesignUI( const string& dir)
      : operating_range_from (2.),
	operating_range_upto (3.),
	pagesize_item (3),
	fft_window_type (agh::SFFTParamSet::TWinType::welch),
	af_damping_window_type (agh::SFFTParamSet::TWinType::welch),
	ext_score_codes ({
		{" -0", "1", "2", "3", "4", "6Rr8", "Ww5", "mM"}
	}),
	freq_bands ({
		{  1.5,  4.0 },
		{  4.0,  8.0 },
		{  8.0, 12.0 },
		{ 15.0, 30.0 },
		{ 30.0, 40.0 },
	}),
	ppuv2 (1e-5),
	timeline_height (70),
	timeline_pph (20),
	runbatch_include_all_channels (false),
	runbatch_include_all_sessions (false),
	runbatch_iterate_ranges (false),
	config_keys_s ({
		SValidator<string>("WindowGeometry.Main",	_geometry_placeholder),
		SValidator<string>("Common.CurrentSession",	_aghdd_placeholder),
		SValidator<string>("Common.CurrentChannel",	_aghtt_placeholder),
	}),
	config_keys_g ({
		SValidator<double>("Common.OperatingRangeFrom",		operating_range_from,			aghui::SExpDesignUI::SValidator::SVFRange (0., 20.)),
		SValidator<double>("Common.OperatingRangeUpto",		operating_range_upto,			aghui::SExpDesignUI::SValidator::SVFRange (0., 20.)),
		SValidator<double>("ScoringFacility.NeighPagePeek",	SScoringFacility::NeighPagePeek,	aghui::SExpDesignUI::SValidator::SVFRange (0., 40.)),
	}),
	config_keys_z ({
		SValidator<size_t>("Measurements.TimelineHeight",	timeline_height,			aghui::SExpDesignUI::SValidator::SVFRange (10, 600)),
		SValidator<size_t>("Measurements.TimelinePPuV2",	ppuv2,					aghui::SExpDesignUI::SValidator::SVFRange (1e-10, 1e10)),
		SValidator<size_t>("Measurements.TimelinePPH",		timeline_height,			aghui::SExpDesignUI::SValidator::SVFRange (10, 600)),
		SValidator<size_t>("ScoringFacility.IntersignalSpace",	SScoringFacility::IntersignalSpace,	aghui::SExpDesignUI::SValidator::SVFRange (10, 800)),
		SValidator<size_t>("ScoringFacility.SpectrumWidth",	SScoringFacility::SpectrumWidth,	aghui::SExpDesignUI::SValidator::SVFRange (10, 800)),
		SValidator<size_t>("ScoringFacility.HypnogramHeight",	SScoringFacility::HypnogramHeight,	aghui::SExpDesignUI::SValidator::SVFRange (10, 300)),
	}),
	config_keys_z ({
		SValidator<bool>("BatchRun.IncludeAllChannels",	runbatch_include_all_channels),
		SValidator<bool>("BatchRun.IncludeAllSessions",	runbatch_include_all_sessions),
		SValidator<bool>("BatchRun.IterateRanges",	runbatch_iterate_ranges),
	})
{
	if ( construct_widgets() )
		throw runtime_error ("SExpDesignUI::SExpDesignUI(): failed to construct widgets");

}


int
aghui::SExpDesignUI::construct_widgets()
{
      // ======== construct list and tree stores
	mSessions =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mEEGChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mAllChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);


	GtkCellRenderer *renderer;

      // =========== 1. Measurements
      // ------------- cMeasurements
	if ( !AGH_GBGETOBJ (GtkVBox,	cMeasurements) ||
	     !AGH_GBGETOBJ (GtkLabel,	lMsmtHint) ||
	     !AGH_GBGETOBJ (GtkLabel,	lMsmtInfo) )
		return -1;

	gtk_drag_dest_set( (GtkWidget*)cMeasurements, GTK_DEST_DEFAULT_ALL,
			   NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets( (GtkWidget*)(cMeasurements));


     // ------------- eMsmtSession
	if ( !AGH_GBGETOBJ (GtkComboBox, eMsmtSession) )
		return -1;

	gtk_combo_box_set_model( eMsmtSession,
				 (GtkTreeModel*)mSessions);
	gtk_combo_box_set_id_column( eMsmtSession, 0);

	eMsmtSession_changed_cb_handler_id =
		g_signal_connect( eMsmtSession, "changed", G_CALLBACK (eMsmtSession_changed_cb), NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eMsmtSession, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eMsmtSession, renderer,
					"text", 0,
					NULL);

     // ------------- eMsmtChannel
	if ( !AGH_GBGETOBJ ( GtkComboBox, eMsmtChannel) )
		return -1;

	gtk_combo_box_set_model( eMsmtChannel,
				 (GtkTreeModel*)mEEGChannels);
	gtk_combo_box_set_id_column( eMsmtChannel, 0);
	eMsmtChannel_changed_cb_handler_id =
		g_signal_connect( eMsmtChannel, "changed", G_CALLBACK (eMsmtChannel_changed_cb), NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eMsmtChannel, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eMsmtChannel, renderer,
					"text", 0,
					NULL);

     // ------------- eMsmtPSDFreq
	if ( !AGH_GBGETOBJ (GtkSpinButton,	eMsmtPSDFreqFrom) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMsmtPSDFreqWidth) )
		return -1;
	eMsmtPSDFreqFrom_value_changed_cb_handler_id =
		g_signal_connect_after( eMsmtPSDFreqFrom, "value-changed",
					G_CALLBACK (eMsmtPSDFreqFrom_value_changed_cb),
					NULL);
	eMsmtPSDFreqWidth_value_changed_cb_handler_id =
		g_signal_connect_after( eMsmtPSDFreqWidth, "value-changed",
					G_CALLBACK (eMsmtPSDFreqWidth_value_changed_cb),
					NULL);


      // --- assorted static objects
	gtk_widget_set_tooltip_markup( (GtkWidget*)(lMsmtHint), __tooltips[(size_t)TTipIdx::general]);



      // ============= settings
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
	if ( !(eScoreCode[(size_t)agh::SPage::TScore::none]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeUnscored")) ||
	     !(eScoreCode[(size_t)agh::SPage::TScore::nrem1]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM1")) ||
	     !(eScoreCode[(size_t)agh::SPage::TScore::nrem2]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM2")) ||
	     !(eScoreCode[(size_t)agh::SPage::TScore::nrem3]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM3")) ||
	     !(eScoreCode[(size_t)agh::SPage::TScore::nrem4]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM4")) ||
	     !(eScoreCode[(size_t)agh::SPage::TScore::rem]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeREM")) ||
	     !(eScoreCode[(size_t)agh::SPage::TScore::wake]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeWake")) ||
	     !(eScoreCode[(size_t)agh::SPage::TScore::mvt]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeMVT")) )
		return -1;

      // --------- Bands
	if ( !(eBand[(size_t)agh::TBand::delta][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandDeltaFrom")) ||
	     !(eBand[(size_t)agh::TBand::delta][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandDeltaUpto")) ||
	     !(eBand[(size_t)agh::TBand::theta][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandThetaFrom")) ||
	     !(eBand[(size_t)agh::TBand::theta][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandThetaUpto")) ||
	     !(eBand[(size_t)agh::TBand::alpha][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandAlphaFrom")) ||
	     !(eBand[(size_t)agh::TBand::alpha][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandAlphaUpto")) ||
	     !(eBand[(size_t)agh::TBand::beta ][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandBetaFrom" )) ||
	     !(eBand[(size_t)agh::TBand::beta ][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandBetaUpto" )) ||
	     !(eBand[(size_t)agh::TBand::gamma][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandGammaFrom")) ||
	     !(eBand[(size_t)agh::TBand::gamma][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandGammaUpto")) )
		return -1;

      // --------- Misc
	if ( !AGH_GBGETOBJ (GtkSpinButton, eSFNeighPagePeekPercent) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAPageHeight) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAHypnogramHeight) ||
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
	using namespace agh;
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



      // ------ colours
	if ( !(CwB[TColour::power_mt].btn	= (GtkColorButton*)gtk_builder_get_object( __builder, "bColourPowerMT")) ||
	     !(CwB[TColour::ticks_mt].btn	= (GtkColorButton*)gtk_builder_get_object( __builder, "bColourTicksMT")) ||
	     !(CwB[TColour::labels_mt].btn	= (GtkColorButton*)gtk_builder_get_object( __builder, "bColourLabelsMT")) )
		return -1;

      // scrub colours
	for_each( CwB.begin(), CwB.end(),
		  [] ( const pair<TColour, SManagedColor>& p)
		  {
			  g_signal_emit_by_name( p.second.btn, "color-set");
		  });


      // ========= child widgets
      // ------- wEDFFileDetails
	if ( !AGH_GBGETOBJ (GtkDialog,		wEDFFileDetails) ||
	     !AGH_GBGETOBJ (GtkTextView,	lEDFFileDetailsReport) )
		return -1;

	g_object_set( lEDFFileDetailsReport,
		      "tabs", pango_tab_array_new_with_positions( 2, TRUE,
								  PANGO_TAB_LEFT, 130,
								  PANGO_TAB_LEFT, 190),
		      NULL);
	textbuf2 = gtk_text_view_get_buffer( lEDFFileDetailsReport);


	return 0;
}



int
aghui::SExpDesignUI::construct_once()
{
      // ========= construct static storage
	if ( !AGH_GBGETOBJ (GtkWindow, wMainWindow) ||
	     !AGH_GBGETOBJ (GtkListStore, mScoringPageSize) ||
	     !AGH_GBGETOBJ (GtkListStore, mFFTParamsPageSize) ||
	     !AGH_GBGETOBJ (GtkListStore, mFFTParamsWindowType) ) {
		return -1;
	}

	return 0;
}



inline namespace {
	template <class T>
	void
	print_xx( const char *pre, const list<T>& ss)
	{
		printf( "%s", pre);
		for ( auto S = ss.begin(); S != ss.end(); ++S )
			printf( " %s;", S->c_str());
		printf("\n");
	}
}



int
aghui::SExpDesignUI::populate_1( bool do_load)
{
	printf( "\nSExpDesignUI::populate_1():\n");
	AghDD = AghCC->enumerate_sessions();
	_AghDi = AghDD.begin();
	print_xx( "* Sessions:", AghDD);
	AghGG = AghCC->enumerate_groups();
	_AghGi = AghGG.begin();
	print_xx( "* Groups:", AghGG);
	AghHH = AghCC->enumerate_all_channels();
	_AghHi = AghHH.begin();
	print_xx( "* All Channels:", AghHH);
	AghTT = AghCC->enumerate_eeg_channels();
	_AghTi = AghTT.begin();
	print_xx( "* EEG channels:", AghTT);
	AghEE = AghCC->enumerate_episodes();
	_AghEi = AghEE.begin();
	print_xx( "* Episodes:", AghEE);
	printf( "\n");

	if ( do_load ) {
		if ( settings::load() )
			;
		else
			if ( GeometryMain.w > 0 ) // implies the rest are, too
				gdk_window_move_resize( gtk_widget_get_window( (GtkWidget*)wMainWindow),
							GeometryMain.x, GeometryMain.y,
							GeometryMain.w, GeometryMain.h);
	}

	if ( AghGG.empty() ) {
		msmt::show_empty_experiment_blurb();
	} else {
		populate_mChannels();
		populate_mSessions();
		msmt::populate();
//		populate_mSimulations( FALSE);
	}

	return 0;
}


void
aghui::depopulate( bool do_save)
{
	if ( do_save )
		settings::save();

	// these are freed on demand immediately before reuse; leave them alone
	AghGG.clear();
	AghDD.clear();
	AghEE.clear();
	AghHH.clear();
	AghTT.clear();
}




void
aghui::do_rescan_tree()
{
	set_cursor_busy( true, (GtkWidget*)wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, FALSE);
	while ( gtk_events_pending() )
		gtk_main_iteration();

	depopulate( false);
	AghCC -> scan_tree( sb::progress_indicator);
	populate( false);

	set_cursor_busy( false, (GtkWidget*)wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, TRUE);
	gtk_statusbar_push( sbMainStatusBar, sb::sbContextIdGeneral,
			    "Scanning complete");
}




void
aghui::populate_mSessions()
{
	g_signal_handler_block( eMsmtSession, msmt::eMsmtSession_changed_cb_handler_id);
	gtk_list_store_clear( mSessions);
	GtkTreeIter iter;
	for ( auto D = AghDD.begin(); D != AghDD.end(); ++D ) {
		gtk_list_store_append( mSessions, &iter);
		gtk_list_store_set( mSessions, &iter,
				    0, D->c_str(),
				    -1);
	}
	__reconnect_sessions_combo();
	g_signal_handler_unblock( eMsmtSession, msmt::eMsmtSession_changed_cb_handler_id);
}






void
aghui::populate_mChannels()
{
	g_signal_handler_block( eMsmtChannel, msmt::eMsmtChannel_changed_cb_handler_id);
	gtk_list_store_clear( mEEGChannels);
	gtk_list_store_clear( mAllChannels);
	// users of mAllChannels (SF pattern) connect to model dynamically

	// for ( auto H = AghTT.begin(); H != AghTT.end(); ++H ) {
	// 	gtk_list_store_append( agh_mEEGChannels, &iter);
	// 	gtk_list_store_set( agh_mEEGChannels, &iter,
	// 			    0, H->c_str(),
	// 			    -1);
	// }
	for_each( AghTT.begin(), AghTT.end(),
		  [&] ( const agh::SChannel& H) {
			  GtkTreeIter iter;
			  gtk_list_store_append( mEEGChannels, &iter);
			  gtk_list_store_set( mEEGChannels, &iter,
					      0, H.c_str(),
					      -1);
		  });

	for_each( AghHH.begin(), AghHH.end(),
		  [&] ( const agh::SChannel& H) {
			  GtkTreeIter iter;
			  gtk_list_store_append( mAllChannels, &iter);
			  gtk_list_store_set( mAllChannels, &iter,
					      0, H.c_str(),
					      -1);
		  });

	__reconnect_channels_combo();

	g_signal_handler_unblock( eMsmtChannel, msmt::eMsmtChannel_changed_cb_handler_id);
}






void
aghui::__reconnect_channels_combo()
{
	gtk_combo_box_set_model( eMsmtChannel, (GtkTreeModel*)mEEGChannels);

	if ( !AghTT.empty() ) {
		int Ti = AghTi();
		if ( Ti != -1 )
			gtk_combo_box_set_active( eMsmtChannel, Ti);
		else
			gtk_combo_box_set_active( eMsmtChannel, 0);
	}
}


void
aghui::__reconnect_sessions_combo()
{
	gtk_combo_box_set_model( eMsmtSession, (GtkTreeModel*)mSessions);

	if ( !AghDD.empty() ) {
		int Di = AghDi();
		if ( Di != -1 )
			gtk_combo_box_set_active( eMsmtSession, Di);
		else
			gtk_combo_box_set_active( eMsmtSession, 0);
	}
}





void
aghui::msmt::populate()
{
	if ( AghCC->n_groups() == 0 )
		return;

      // touch toolbar controls
	g_signal_handler_block( eMsmtPSDFreqFrom, eMsmtPSDFreqFrom_value_changed_cb_handler_id);
	g_signal_handler_block( eMsmtPSDFreqWidth, eMsmtPSDFreqWidth_value_changed_cb_handler_id);
	gtk_spin_button_set_value( eMsmtPSDFreqFrom, OperatingRangeFrom);
	gtk_spin_button_set_value( eMsmtPSDFreqWidth, OperatingRangeUpto - OperatingRangeFrom);
	g_signal_handler_unblock( eMsmtPSDFreqFrom, eMsmtPSDFreqFrom_value_changed_cb_handler_id);
	g_signal_handler_unblock( eMsmtPSDFreqWidth, eMsmtPSDFreqWidth_value_changed_cb_handler_id);

      // deal with the main drawing area
	GG.clear();
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);

	time_t	earliest_start = (time_t)-1,
		latest_end = (time_t)-1;

	printf( "msmt:populate(): session %s, channel %s\n", AghD(), AghT());
      // first pass: determine common timeline
	for ( auto g = AghCC->groups_begin(); g != AghCC->groups_end(); ++g ) {
		GG.emplace_back( g); // precisely need the iterator, not object by reference
		SGroupPresentation& G = GG.back();
		for_each( g->second.begin(), g->second.end(),
			  [&] (agh::CSubject& j)
			  {
				  G.emplace_back( j);
				  const SSubjectPresentation& J = G.back();
				  if ( J.cscourse ) {
					  auto& ee = J.csubject.measurements[*_AghDi].episodes;
					  if ( earliest_start == (time_t)-1 || earliest_start > ee.front().start_rel )
						  earliest_start = ee.front().start_rel;
					  if ( latest_end == (time_t)-1 || latest_end < ee.back().end_rel )
						  latest_end = ee.back().end_rel;
				  } else
					  fprintf( stderr, "msmt::populate(): subject %s has no recordings in session %s channel %s\n",
						   j.name(), AghD(), AghT());
			  });
	};

	__timeline_start = earliest_start;
	__timeline_end   = latest_end;
	__timeline_pixels = (__timeline_end - __timeline_start) / 3600 * TimelinePPH;
	__timeline_pages  = (__timeline_end - __timeline_start) / AghCC->fft_params.page_size;

	fprintf( stderr, "msmt::populate(): common timeline:\n");
	fputs( asctime( localtime(&earliest_start)), stderr);
	fputs( asctime( localtime(&latest_end)), stderr);

	__tl_left_margin = 0;

      // walk again thoroughly, set timeline drawing area length
	for ( auto G = GG.begin(); G != GG.end(); ++G ) {
	      // convert avg episode times
		g_string_assign( __ss__, "");
		for ( auto E = AghEE.begin(); E != AghEE.end(); ++E ) {
			pair<float, float>& avge = G->group().avg_episode_times[*_AghDi][*E];
			unsigned seconds, h0, m0, s0, h9, m9, s9;
			seconds = avge.first * 24 * 60 * 60;
			h0 = seconds / 60 / 60;
			m0  = seconds % 3600 / 60;
			s0  = seconds % 60;
			seconds = avge.second * 24 * 60 * 60;
			h9 = seconds / 60 / 60;
			m9  = seconds % 3600 / 60;
			s9  = seconds % 60;

			g_string_append_printf( __ss__,
						"       <i>%s</i> %02d:%02d:%02d ~ %02d:%02d:%02d",
						E->c_str(),
						h0 % 24, m0, s0,
						h9 % 24, m9, s9);
		}

		gchar *g_escaped = g_markup_escape_text( G->name(), -1);
		snprintf_buf( "<b>%s</b> (%zu) %s", g_escaped, G->size(), __ss__->str);
		g_free( g_escaped);

		G->expander = (GtkExpander*)gtk_expander_new( __buf__);
		gtk_expander_set_use_markup( G->expander, TRUE);
		g_object_set( (GObject*)G->expander,
			      "visible", TRUE,
			      "expanded", TRUE,
			      "height-request", -1,
			      NULL);
		gtk_box_pack_start( (GtkBox*)cMeasurements,
				    (GtkWidget*)G->expander, TRUE, TRUE, 3);
		gtk_container_add( (GtkContainer*)G->expander,
				   (GtkWidget*) (G->vbox = (GtkExpander*)gtk_vbox_new( TRUE, 1)));
		g_object_set( (GObject*)G->vbox,
			      "height-request", -1,
			      NULL);

		for ( auto J = G->begin(); J != G->end(); ++J ) {
			J->da = gtk_drawing_area_new();
			gtk_box_pack_start( (GtkBox*)G->vbox,
					    J->da, TRUE, TRUE, 2);

			// determine __tl_left_margin
			{
				cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( J->da));
				cairo_text_extents_t extents;
				cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
				cairo_set_font_size( cr, 11);
				cairo_text_extents( cr, J->csubject.name(), &extents);
				if ( __tl_left_margin < extents.width )
					__tl_left_margin = extents.width;
				cairo_destroy( cr);
			}

			// set it later
//			g_object_set( G_OBJECT (GG[g].subjects[j].da),
//				      "app-paintable", TRUE,
//				      "double-buffered", TRUE,
//				      "height-request", settings::WidgetSize_MVTimelineHeight,
//				      "width-request", __timeline_pixels + __tl_left_margin + __tl_right_margin,
//				      NULL);

			gtk_widget_add_events( J->da,
					       (GdkEventMask)
					       GDK_EXPOSURE_MASK |
					       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
					       GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
					       GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
					       GDK_POINTER_MOTION_MASK);
			g_signal_connect_after( J->da, "draw",
						G_CALLBACK (daSubjectTimeline_draw_cb),
						&*J);
			g_signal_connect_after( J->da, "enter-notify-event",
						G_CALLBACK (daSubjectTimeline_enter_notify_event_cb),
						&*J);
			g_signal_connect_after( J->da, "leave-notify-event",
						G_CALLBACK (daSubjectTimeline_leave_notify_event_cb),
						&*J);
			g_signal_connect_after( J->da, "scroll-event",
						G_CALLBACK (daSubjectTimeline_scroll_event_cb),
						&*J);
			if ( J->cscourse ) {
				g_signal_connect_after( J->da, "button-press-event",
							G_CALLBACK (daSubjectTimeline_button_press_event_cb),
							&*J);
				g_signal_connect_after( J->da, "motion-notify-event",
							G_CALLBACK (daSubjectTimeline_motion_notify_event_cb),
							&*J);
			}
			g_signal_connect_after( J->da, "drag-data-received",
						G_CALLBACK (cMeasurements_drag_data_received_cb),
						&*J);
			g_signal_connect_after( J->da, "drag-drop",
						G_CALLBACK (cMeasurements_drag_drop_cb),
						&*J);
			gtk_drag_dest_set( J->da, GTK_DEST_DEFAULT_ALL,
					   NULL, 0, GDK_ACTION_COPY);
			gtk_drag_dest_add_uri_targets( J->da);
		}
	}

      // walk quickly one last time to set widget attributes (importantly, involving __tl_left_margin)
	__tl_left_margin += 10;
	for_each( GG.begin(), GG.end(),
		  [&] (SGroupPresentation& G)
		  {
			  for_each( G.begin(), G.end(),
				    [&] (SSubjectPresentation& J)
				    {
					    g_object_set( (GObject*)J.da,
							  "can-focus", FALSE,
							  "app-paintable", TRUE,
							  "double-buffered", TRUE,
							  "height-request", settings::WidgetSize_MVTimelineHeight,
							  "width-request", __timeline_pixels + __tl_left_margin + __tl_right_margin,
							  NULL);
				    });
		  });

	snprintf_buf( "<b><small>page: %zu sec  bin: %g Hz  %s</small></b>",
		      AghCC -> fft_params.page_size,
		      AghCC -> fft_params.bin_size,
		      agh::SFFTParamSet::welch_window_type_name( AghCC->fft_params.welch_window_type));
	gtk_label_set_markup( lMsmtInfo, __buf__);
	gtk_widget_show_all( (GtkWidget*)(cMeasurements));
}



void
aghui::msmt::show_empty_experiment_blurb()
{
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	const char *briefly =
		"<b><big>Empty experiment\n</big></b>\n"
		"When you have your recordings ready as a set of .edf files,\n"
		"• Create your experiment tree as follows: <i>Experiment/Group/Subject/Session</i>;\n"
		"• Have your EDF sources named <i>Episode</i>.edf, and placed in the corresponding <i>Session</i> directory, or\n"
		"• Drop EDF sources onto here and identify and place them individually.\n\n"
		"Once set up, either:\n"
		"• click <b>⎇</b> and select the top directory of the (newly created) experiment tree, or\n"
		"• click <b>Rescan</b> if this is the tree you have just populated.";
	GtkLabel *text = (GtkLabel*)gtk_label_new( "");
	gtk_label_set_markup( text, briefly);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)text,
			    TRUE, TRUE, 0);

	snprintf_buf( "%s/%s/%s", PACKAGE_DATADIR, PACKAGE, AGH_BG_IMAGE_FNAME);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)gtk_image_new_from_file( __buf__),
			    TRUE, FALSE, 0);
	gtk_widget_show_all( (GtkWidget*)cMeasurements);
}









// callbacks


// EOF