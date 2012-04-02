// ;-*-C++-*-
/*
 *       File name:  ui/expdesign-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-02
 *
 *         Purpose:  SExpDesignUI::construct_widgets
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "expdesign.hh"

using namespace std;

using namespace aghui;


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
	mGlobalAnnotations =
		gtk_tree_store_new( 6,
				    G_TYPE_STRING, // id
				    G_TYPE_STRING, // at pages
				    G_TYPE_STRING, // channel
				    G_TYPE_STRING, // label
				    G_TYPE_BOOLEAN, G_TYPE_POINTER);
	mSimulations =
		gtk_tree_store_new( 16,
				    G_TYPE_STRING,	// group, subject, channel, from-upto
				    G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,	// tunables
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_BOOLEAN,
				    G_TYPE_POINTER);

      // misc
	auto font_desc = pango_font_description_from_string( "Mono 9");

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;

      // =========== 1. Measurements
      // ------------- cMeasurements
	if ( !AGH_GBGETOBJ (GtkMenu,		iiMainMenu) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpChange) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpRefresh) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpAnnotations) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpQuit) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iMontageResetAll) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iMontageNotchNone) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iMontageNotch50Hz) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iMontageNotch60Hz) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iHelpAbout) )
		return -1;

	g_signal_connect( iExpChange, "activate",
			  (GCallback)iExpChange_activate_cb,
			  this);
	g_signal_connect( iExpRefresh, "activate",
			  (GCallback)iExpRefresh_activate_cb,
			  this);
	g_signal_connect( iExpAnnotations, "activate",
			  (GCallback)iExpAnnotations_activate_cb,
			  this);
	g_signal_connect( iExpQuit, "activate",
			  (GCallback)iExpQuit_activate_cb,
			  this);
	g_signal_connect( iHelpAbout, "activate",
			  (GCallback)iHelpAbout_activate_cb,
			  this);

	g_signal_connect( iMontageResetAll, "activate",
			  (GCallback)iMontageResetAll_activate_cb,
			  this);
	g_signal_connect( iMontageNotchNone, "activate",
			  (GCallback)iMontageNotchNone_activate_cb,
			  this);
	g_signal_connect( iMontageNotch50Hz, "activate",
			  (GCallback)iMontageNotch50Hz_activate_cb,
			  this);
	g_signal_connect( iMontageNotch60Hz, "activate",
			  (GCallback)iMontageNotch60Hz_activate_cb,
			  this);

	if ( !AGH_GBGETOBJ (GtkWindow,		wMainWindow) ||
	     !AGH_GBGETOBJ (GtkVBox,		cMeasurements) ||
	     !AGH_GBGETOBJ (GtkLabel,		lMsmtPSDInfo) )
		return -1;

	wMainWindow_delete_event_cb_handler_id =
		g_signal_connect( wMainWindow, "delete-event",
				  (GCallback)wMainWindow_delete_event_cb,
				  this);
	g_signal_connect( wMainWindow, "configure-event",
			  (GCallback)wMainWindow_configure_event_cb,
			  this);

	g_signal_connect( cMeasurements, "drag-data-received",
			  (GCallback)cMeasurements_drag_data_received_cb,
			  this);
	g_signal_connect( cMeasurements, "drag-drop",
			  (GCallback)cMeasurements_drag_drop_cb,
			  this);


	gtk_drag_dest_set( (GtkWidget*)cMeasurements, GTK_DEST_DEFAULT_ALL,
			   NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets( (GtkWidget*)(cMeasurements));

	// annotations
	if ( !AGH_GBGETOBJ (GtkDialog,		wGlobalAnnotations) ||
	     !AGH_GBGETOBJ (GtkTreeView,	tvGlobalAnnotations) )
		return -1;
	gtk_tree_view_set_model( tvGlobalAnnotations,
				 (GtkTreeModel*)mGlobalAnnotations);

	g_object_set( (GObject*)tvGlobalAnnotations,
		      "expander-column", 0,
		      NULL);
	g_signal_connect( tvGlobalAnnotations, "map",
			  (GCallback)gtk_tree_view_expand_all,
			  NULL);
	g_signal_connect( tvGlobalAnnotations, "row-activated",
			  (GCallback)tvGlobalAnnotations_row_activated_cb,
			  this);

	renderer = gtk_cell_renderer_text_new();
	for ( auto t = 0; t < mannotations_visibility_switch_col; ++t ) {
		renderer = gtk_cell_renderer_text_new();
		g_object_set( (GObject*)renderer,
			      "editable", FALSE,
			      NULL);
		g_object_set_data( (GObject*)renderer, "column", GINT_TO_POINTER (t));
		col = gtk_tree_view_column_new_with_attributes( mannotations_column_names[t],
								renderer,
								"text", t,
								NULL);
		gtk_tree_view_column_set_expand( col, TRUE);
		gtk_tree_view_append_column( tvGlobalAnnotations, col);
	}
	gtk_tree_view_append_column( tvGlobalAnnotations,
				     gtk_tree_view_column_new());

     // --------- tabs
	if ( !AGH_GBGETOBJ (GtkNotebook,	tTaskSelector) ||
	     !AGH_GBGETOBJ (GtkNotebook,	tDesign) ||
	     !AGH_GBGETOBJ (GtkNotebook,	tSimulations) ||
	     !AGH_GBGETOBJ (GtkNotebook,	tSettings) ||
	     !AGH_GBGETOBJ (GtkLabel,		lTaskSelector1) ||
	     !AGH_GBGETOBJ (GtkLabel,		lTaskSelector2) ||
	     !AGH_GBGETOBJ (GtkLabel,		lSettings) )
		return -1;

	g_signal_connect( tTaskSelector, "switch-page",
			  (GCallback)tTaskSelector_switch_page_cb,
			  this);
	g_signal_connect( tDesign, "switch-page",
			  (GCallback)tDesign_switch_page_cb,
			  this);
	g_signal_connect( tSimulations, "switch-page",
			  (GCallback)tSimulations_switch_page_cb,
			  this);


     // ------------- eMsmtSession
	if ( !AGH_GBGETOBJ (GtkComboBox, eMsmtSession) )
		return -1;

	gtk_combo_box_set_model( eMsmtSession,
				 (GtkTreeModel*)mSessions);
	gtk_combo_box_set_id_column( eMsmtSession, 0);

	eMsmtSession_changed_cb_handler_id =
		g_signal_connect( eMsmtSession, "changed",
				  (GCallback)eMsmtSession_changed_cb,
				  this);
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
		g_signal_connect( eMsmtChannel, "changed",
				  (GCallback)eMsmtChannel_changed_cb,
				  this);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eMsmtChannel, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eMsmtChannel, renderer,
					"text", 0,
					NULL);

     // ------------- eMsmtProfile*
	if ( !AGH_GBGETOBJ (GtkToggleButton,	eMsmtProfileAutoscale) ||
	     !AGH_GBGETOBJ (GtkListStore,	mMsmtProfileType) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eMsmtProfileType) ||
	     !AGH_GBGETOBJ (GtkBox,		cMsmtProfileParams1) ||
	     !AGH_GBGETOBJ (GtkBox,		cMsmtProfileParams2) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMsmtPSDFreqFrom) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMsmtPSDFreqWidth) ||
	     !AGH_GBGETOBJ (GtkBox,		cMsmtProfileParamsContainer) )
		return -1;

	g_signal_connect( eMsmtProfileType, "changed",
			  (GCallback)eMsmtProfileType_changed_cb,
			  this);
	g_signal_connect( eMsmtProfileAutoscale, "toggled",
			  (GCallback)eMsmtProfileAutoscale_toggled_cb,
			  this);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eMsmtProfileType, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eMsmtProfileType, renderer,
					"text", 0,
					NULL);

	eMsmtPSDFreqFrom_value_changed_cb_handler_id =
		g_signal_connect_after( eMsmtPSDFreqFrom, "value-changed",
					(GCallback)eMsmtPSDFreqFrom_value_changed_cb,
					this);
	eMsmtPSDFreqWidth_value_changed_cb_handler_id =
		g_signal_connect_after( eMsmtPSDFreqWidth, "value-changed",
					(GCallback)eMsmtPSDFreqWidth_value_changed_cb,
					this);

      // ------------ menus
	if ( !(AGH_GBGETOBJ (GtkMenu,		iiSubjectTimeline)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineScore)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineSubjectInfo)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineEDFInfo)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineSaveAsSVG)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineResetMontage)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineBrowse)) )
		return -1;

	g_signal_connect( iSubjectTimelineScore, "activate",
			  (GCallback)iSubjectTimelineScore_activate_cb,
			  this);
	g_signal_connect( iSubjectTimelineSubjectInfo, "activate",
			  (GCallback)iSubjectTimelineSubjectInfo_activate_cb,
			  this);
	g_signal_connect( iSubjectTimelineEDFInfo, "activate",
			  (GCallback)iSubjectTimelineEDFInfo_activate_cb,
			  this);
	g_signal_connect( iSubjectTimelineSaveAsSVG, "activate",
			  (GCallback)iSubjectTimelineSaveAsSVG_activate_cb,
			  this);
	g_signal_connect( iSubjectTimelineBrowse, "activate",
			  (GCallback)iSubjectTimelineBrowse_activate_cb,
			  this);
	g_signal_connect( iSubjectTimelineResetMontage, "activate",
			  (GCallback)iSubjectTimelineResetMontage_activate_cb,
			  this);


   // ================ 2. Simulations
     // ------------- tvSimulations & controls
	if ( !(AGH_GBGETOBJ (GtkTreeView, tvSimulations)) )
		return -1;

	gtk_tree_view_set_model( tvSimulations,
				 (GtkTreeModel*)mSimulations);

	g_object_set( (GObject*)tvSimulations,
		      "expander-column", 0,
		      "enable-tree-lines", FALSE,
		      "headers-clickable", FALSE,
		      NULL);
	g_signal_connect( tvSimulations, "map",
			  (GCallback)gtk_tree_view_expand_all,
			  NULL);
	g_signal_connect( tvSimulations, "row-activated",
			  (GCallback)tvSimulations_row_activated_cb,
			  this);

	for ( auto c = 0; c < msimulations_visibility_switch_col; ++c ) {
		renderer = gtk_cell_renderer_text_new();
		g_object_set( (GObject*)renderer,
			      "editable", FALSE,
			      "xalign", (c >= 2) ? .5 : 0.,
			      NULL);
		g_object_set_data( (GObject*)renderer,
				   "column", GINT_TO_POINTER (c));

		col = gtk_tree_view_column_new_with_attributes(
			"replaceme",
			renderer,
			"text", c,
			NULL);
		gtk_tree_view_column_set_expand( col, TRUE);
		gtk_tree_view_column_set_alignment( col, (c >= 2) ? .5 : 0.);
		GtkWidget *lbl = gtk_label_new( NULL);
		gtk_label_set_markup(
			(GtkLabel*)lbl,
			(c == 0) ? "Id" : (c == 1) ? "Status" : agh::STunableSet::tunable_pango_name(c-2).c_str());
		gtk_widget_set_visible( lbl, TRUE);
		gtk_tree_view_column_set_widget(
			col, lbl);
		gtk_tree_view_append_column( tvSimulations, col);
	}
	gtk_tree_view_append_column( tvSimulations,
				     col = gtk_tree_view_column_new());
	gtk_tree_view_column_set_visible( col, FALSE);

      // ------------ iSimulations*
	if ( !(AGH_GBGETOBJ (GtkMenuItem, iSimulationsRunBatch)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem, iSimulationsRunClearAll)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem, iSimulationsReportGenerate)) )
		return -1;

	g_signal_connect( iSimulationsRunBatch, "activate",
			  (GCallback)iSimulationsRunBatch_activate_cb,
			  this);
	g_signal_connect( iSimulationsRunClearAll, "activate",
			  (GCallback)iSimulationsRunClearAll_activate_cb,
			  this);
	g_signal_connect( iSimulationsReportGenerate, "activate",
			  (GCallback)iSimulationsReportGenerate_activate_cb,
			  this);

      // ------------- lSimulations{Session,Channel}
	if ( !AGH_GBGETOBJ (GtkLabel, lSimulationsSession) ||
	     !AGH_GBGETOBJ (GtkLabel, lSimulationsChannel) )
		return -1;

    // ======= statusbar
	if ( !AGH_GBGETOBJ (GtkStatusbar,	sbMainStatusBar) )
		return -1;

	sbContextIdGeneral = gtk_statusbar_get_context_id( sbMainStatusBar, "General context");

	if ( !(AGH_GBGETOBJ (GtkDialog,		wScanLog)) ||
	     !(AGH_GBGETOBJ (GtkTextView,	tScanLog)) )
		return -1;

	gtk_widget_override_font( (GtkWidget*)tScanLog, font_desc);
	// free? unref? leak some?

      // ****************** settings
	if ( !AGH_GBGETOBJ (GtkListStore,	mScoringPageSize) ||
	     !AGH_GBGETOBJ (GtkListStore,	mFFTParamsPageSize) ||
	     !AGH_GBGETOBJ (GtkListStore,	mFFTParamsBinSize) ||
	     !AGH_GBGETOBJ (GtkListStore,	mFFTParamsWindowType) )
		return -1;
      // ------------- fFFTParams
	if ( !AGH_GBGETOBJ (GtkComboBox,	eFFTParamsBinSize) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eFFTParamsPageSize) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eFFTParamsWindowType) )
		return -1;

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eFFTParamsPageSize, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eFFTParamsPageSize, renderer,
					"text", 0,
					NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eFFTParamsBinSize, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eFFTParamsBinSize, renderer,
					"text", 0,
					NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eFFTParamsWindowType, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eFFTParamsWindowType, renderer,
					"text", 0,
					NULL);
      // ------------- fArtifacts
	if ( !AGH_GBGETOBJ (GtkComboBox,	eArtifDampenWindowType) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eArtifDampenFactor) )
		return -1;

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eArtifDampenWindowType, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eArtifDampenWindowType, renderer,
					"text", 0,
					NULL);
      // ------------- fMicrocontinuity
	if ( !AGH_GBGETOBJ (GtkSpinButton,	eMCParamXpiBPlus) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamXpiBMinus) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamXpiBZero) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamF0) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamFC) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamBandWidth) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamIIRBackpolate) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamSSSUMin) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamSSSUMax) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamPiBPeakWidth) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamMCGain) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamArtMax) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamMCEventDuration) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamMCEventReject) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamMCJumpFind) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamSmoothRate) )
		return -1;

      // ------- custom score codes
	if ( !(eScoreCode[(size_t)sigfile::SPage::TScore::none]		= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeUnscored")) ||
	     !(eScoreCode[(size_t)sigfile::SPage::TScore::nrem1]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM1")) ||
	     !(eScoreCode[(size_t)sigfile::SPage::TScore::nrem2]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM2")) ||
	     !(eScoreCode[(size_t)sigfile::SPage::TScore::nrem3]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM3")) ||
	     !(eScoreCode[(size_t)sigfile::SPage::TScore::nrem4]	= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeNREM4")) ||
	     !(eScoreCode[(size_t)sigfile::SPage::TScore::rem]		= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeREM")) ||
	     !(eScoreCode[(size_t)sigfile::SPage::TScore::wake]		= (GtkEntry*)gtk_builder_get_object( __builder, "eScoreCodeWake")))
		return -1;

      // --------- Bands
	if ( !(eBand[(size_t)sigfile::TBand::delta][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandDeltaFrom")) ||
	     !(eBand[(size_t)sigfile::TBand::delta][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandDeltaUpto")) ||
	     !(eBand[(size_t)sigfile::TBand::theta][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandThetaFrom")) ||
	     !(eBand[(size_t)sigfile::TBand::theta][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandThetaUpto")) ||
	     !(eBand[(size_t)sigfile::TBand::alpha][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandAlphaFrom")) ||
	     !(eBand[(size_t)sigfile::TBand::alpha][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandAlphaUpto")) ||
	     !(eBand[(size_t)sigfile::TBand::beta ][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandBetaFrom" )) ||
	     !(eBand[(size_t)sigfile::TBand::beta ][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandBetaUpto" )) ||
	     !(eBand[(size_t)sigfile::TBand::gamma][0]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandGammaFrom")) ||
	     !(eBand[(size_t)sigfile::TBand::gamma][1]   = (GtkSpinButton*)gtk_builder_get_object( __builder, "eBandGammaUpto")) )
		return -1;

      // --------- Misc
	if ( !AGH_GBGETOBJ (GtkSpinButton, eDAPageHeight) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAHypnogramHeight) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAEMGHeight) )
		return -1;

	if ( !AGH_GBGETOBJ (GtkEntry,	eBrowseCommand) )
		return -1;

	gtk_entry_set_text( eBrowseCommand, browse_command.c_str());


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
	     !AGH_GBGETOBJ (GtkCheckButton,	eCtlParamAZAmendment1) ||
	     !AGH_GBGETOBJ (GtkCheckButton,	eCtlParamAZAmendment2) ||
	     !AGH_GBGETOBJ (GtkLabel,		lCtlParamDBAmendment1) ||
	     !AGH_GBGETOBJ (GtkLabel,		lCtlParamDBAmendment2) ||
	     !AGH_GBGETOBJ (GtkLabel,		lCtlParamAZAmendment1) ||
	     !AGH_GBGETOBJ (GtkLabel,		lCtlParamAZAmendment2) ||
	     !AGH_GBGETOBJ (GtkRadioButton,	eCtlParamScoreUnscoredAsWake) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamNSWAPpBeforeSimStart) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamReqScoredPercent) )
		return -1;

	g_signal_connect( eCtlParamDBAmendment1, "toggled",
			  (GCallback)eCtlParamDBAmendment1_toggled_cb,
			  this);
	g_signal_connect( eCtlParamDBAmendment2, "toggled",
			  (GCallback)eCtlParamDBAmendment2_toggled_cb,
			  this);
	g_signal_connect( eCtlParamAZAmendment1, "toggled",
			  (GCallback)eCtlParamAZAmendment1_toggled_cb,
			  this);
	g_signal_connect( eCtlParamAZAmendment2, "toggled",
			  (GCallback)eCtlParamAZAmendment2_toggled_cb,
			  this);

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

	for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t )
		for ( auto d = 0; d < 4; ++d )
			jTunable[t][d] = gtk_spin_button_get_adjustment( eTunable[t][d]);


	if ( !AGH_GBGETOBJ (GtkButton,	bSimParamRevertTunables) )
		return -1;
	g_signal_connect( bSimParamRevertTunables, "clicked",
			  (GCallback)bSimParamRevertTunables_clicked_cb,
			  this);

      // ------ colours
	if ( !(CwB[TColour::night	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNight")) ||
	     !(CwB[TColour::day		].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourDay")) ||
	     !(CwB[TColour::power_mt	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourPowerMT")) ||
	     !(CwB[TColour::ticks_mt	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourTicksMT")) ||
	     !(CwB[TColour::labels_mt	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourLabelsMT")) ||

	     !(CwB[TColour::swa	     	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourSWA")) ||
	     !(CwB[TColour::swa_sim  	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourSWASim")) ||
	     !(CwB[TColour::process_s	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourProcessS")) ||
	     !(CwB[TColour::paper_mr	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourPaperMR")) ||
	     !(CwB[TColour::ticks_mr	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourTicksMR")) ||
	     !(CwB[TColour::labels_mr	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourLabelsMR")) ||

	     !(CwB[TColour::score_none	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNONE")) ||
	     !(CwB[TColour::score_nrem1	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNREM1")) ||
	     !(CwB[TColour::score_nrem2	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNREM2")) ||
	     !(CwB[TColour::score_nrem3	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNREM3")) ||
	     !(CwB[TColour::score_nrem4	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNREM4")) ||
	     !(CwB[TColour::score_rem	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourREM")) ||
	     !(CwB[TColour::score_wake	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourWake")) ||
	     !(CwB[TColour::power_sf	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourPowerSF")) ||
	     !(CwB[TColour::emg		].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourEMG")) ||
	     !(CwB[TColour::hypnogram	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourHypnogram")) ||
	     !(CwB[TColour::artifact	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourArtifacts")) ||
	     !(CwB[TColour::annotations	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourAnnotations")) ||
	     !(CwB[TColour::selection	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourSelection")) ||
	     !(CwB[TColour::ticks_sf	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourTicksSF")) ||
	     !(CwB[TColour::labels_sf	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourLabelsSF")) ||
	     !(CwB[TColour::cursor	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourCursor")) ||
	     !(CwB[TColour::band_delta	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandDelta")) ||
	     !(CwB[TColour::band_theta	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandTheta")) ||
	     !(CwB[TColour::band_alpha	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandAlpha")) ||
	     !(CwB[TColour::band_beta	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandBeta")) ||
	     !(CwB[TColour::band_gamma	].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandGamma")) )
		return -1;

      // scrub colours
	for ( auto &C : CwB ) {
		g_signal_connect( C.second.btn, "color-set",
				  (GCallback)bColourX_color_set_cb,
				  &C.second);
		g_signal_emit_by_name( C.second.btn, "color-set");
	}


      // ========= child widgets
      // ----- wAbout
	if ( !(AGH_GBGETOBJ (GtkDialog,		wAbout)) ||
	     !(AGH_GBGETOBJ (GtkTextView,	tREADME)) )
		return -1;

	{
		gtk_widget_override_font( (GtkWidget*)tREADME, font_desc);
		char *contents;
		snprintf_buf( "%s/doc/%s/README", PACKAGE_DATADIR, PACKAGE);
		GFile *file = g_file_new_for_path( __buf__);
		gtk_text_buffer_set_text(
			gtk_text_view_get_buffer( tREADME),
			g_file_load_contents( file, NULL, &contents, NULL, NULL, NULL)
			? contents
			: "(The contents of " PACKAGE_DATADIR "/README was supposed to be here;\n"
			"this file was not found in that location, too bad.)", -1);
		g_object_unref( file);
	}


      // ------- wEDFFileDetails
	if ( !AGH_GBGETOBJ (GtkDialog,		wEDFFileDetails) ||
	     !AGH_GBGETOBJ (GtkTextView,	lEDFFileDetailsReport) )
		return -1;

	// used by two GtkTextView's, lEDFFileDetailsReport and lEdfImportFileInfo
	if ( !AGH_GBGETOBJ (GtkTextBuffer,	tEDFFileDetailsReport) )
		return -1;
	// g_object_get( tEDFFileDetailsReport, "style", &sty, NULL);
	// sty->font_desc = monofont;

	g_object_set( lEDFFileDetailsReport,
		      "tabs", pango_tab_array_new_with_positions(
			      2, TRUE,
			      PANGO_TAB_LEFT, 180,
			      PANGO_TAB_LEFT, 190),
		      NULL);
	gtk_widget_override_font( (GtkWidget*)lEDFFileDetailsReport, font_desc);

      // ------- wEdfImport
	if ( !AGH_GBGETOBJ (GtkDialog,		wEdfImport) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eEdfImportGroup) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eEdfImportSession) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eEdfImportEpisode) ||
	     !AGH_GBGETOBJ (GtkEntry,		eEdfImportGroupEntry) ||
	     !AGH_GBGETOBJ (GtkEntry,		eEdfImportSessionEntry) ||
	     !AGH_GBGETOBJ (GtkEntry,		eEdfImportEpisodeEntry) ||
	     !AGH_GBGETOBJ (GtkLabel,		lEdfImportSubject) ||
	     !AGH_GBGETOBJ (GtkLabel,		lEdfImportCaption) ||
	     !AGH_GBGETOBJ (GtkTextView,	lEdfImportFileInfo) ||
	     !AGH_GBGETOBJ (GtkButton,		bEdfImportAttachCopy) ||
	     !AGH_GBGETOBJ (GtkButton,		bEdfImportAttachMove) ||
	     !AGH_GBGETOBJ (GtkButton,		bEdfImportAdmit) ||
	     !AGH_GBGETOBJ (GtkButton,		bEdfImportEdfhed) )
		return -1;

	g_object_set( lEdfImportFileInfo,
		      "tabs", pango_tab_array_new_with_positions( 2, TRUE,
								  PANGO_TAB_LEFT, 130,
								  PANGO_TAB_LEFT, 190),
		      NULL);

	g_signal_connect( eEdfImportGroupEntry,
			  "changed", (GCallback)check_gtk_entry_nonempty_cb,
			  this);
	g_signal_connect( eEdfImportSessionEntry,
			  "changed", (GCallback)check_gtk_entry_nonempty_cb,
			  this);
	g_signal_connect( eEdfImportEpisodeEntry,
			  "changed", (GCallback)check_gtk_entry_nonempty_cb,
			  this);

      // ------- wEdfImport
	if ( !AGH_GBGETOBJ (GtkDialog,		wSubjectDetails) ||
	     !AGH_GBGETOBJ (GtkEntry,		eSubjectDetailsName) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSubjectDetailsAge) ||
	     !AGH_GBGETOBJ (GtkRadioButton,	eSubjectDetailsGenderMale) ||
	     !AGH_GBGETOBJ (GtkRadioButton,	eSubjectDetailsGenderFemale) ||
	     !AGH_GBGETOBJ (GtkEntry,		eSubjectDetailsComment) )
		return -1;

      // ------------- wBatchSetup
	if ( !AGH_GBGETOBJ (GtkDialog,		wBatchSetup) ||
	     !AGH_GBGETOBJ (GtkEntry,		eBatchSetupSubjects) ||
	     !AGH_GBGETOBJ (GtkEntry,		eBatchSetupSessions) ||
	     !AGH_GBGETOBJ (GtkEntry,		eBatchSetupChannels) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eBatchSetupRangeFrom) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eBatchSetupRangeWidth) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eBatchSetupRangeInc) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eBatchSetupRangeSteps) )
		return -1;


      // ========= sister widget
	mExpDesignChooserList =
		gtk_list_store_new( 1, G_TYPE_STRING);

	if ( !(AGH_GBGETOBJ (GtkDialog, 	wExpDesignChooser)) ||
	     !(AGH_GBGETOBJ (GtkTreeView,	tvExpDesignChooserList)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bExpDesignChooserSelect)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bExpDesignChooserCreateNew)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bExpDesignChooserRemove)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bExpDesignChooserQuit)) )
		return -1;

	g_signal_connect( wExpDesignChooser, "show",
			  (GCallback)wExpDesignChooser_show_cb,
			  this);
	g_signal_connect( wExpDesignChooser, "hide",
			  (GCallback)wExpDesignChooser_hide_cb,
			  this);

	g_signal_connect( gtk_tree_view_get_selection( tvExpDesignChooserList), "changed",
			  (GCallback)tvExpDesignChooserList_changed_cb,
			  this);
	g_signal_connect( bExpDesignChooserSelect, "clicked",
			  (GCallback)bExpDesignChooserSelect_clicked_cb,
			  this);
	g_signal_connect( bExpDesignChooserCreateNew, "clicked",
			  (GCallback)bExpDesignChooserCreateNew_clicked_cb,
			  this);
	g_signal_connect( bExpDesignChooserRemove, "clicked",
			  (GCallback)bExpDesignChooserRemove_clicked_cb,
			  this);
	g_signal_connect( bExpDesignChooserQuit, "clicked",
			  (GCallback)bExpDesignChooserQuit_clicked_cb,
			  this);

	gtk_tree_view_set_model( tvExpDesignChooserList,
				 (GtkTreeModel*)mExpDesignChooserList);

	g_object_set( (GObject*)tvExpDesignChooserList,
		      "headers-visible", FALSE,
		      NULL);

	renderer = gtk_cell_renderer_text_new();
	g_object_set( (GObject*)renderer, "editable", FALSE, NULL);
	g_object_set_data( (GObject*)renderer, "column", GINT_TO_POINTER (0));
	gtk_tree_view_insert_column_with_attributes( tvExpDesignChooserList,
						     -1, "ExpDesign", renderer,
						     "text", 0,
						     NULL);

	pango_font_description_free( font_desc);

	return 0;
}


// eof
