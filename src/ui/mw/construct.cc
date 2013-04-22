/*
 *       File name:  ui/mw/construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-02
 *
 *         Purpose:  SExpDesignUI::construct_widgets
 *
 *         License:  GPL
 */


#include "ui/ui.hh"
#include "widgets.hh"
#include "mw_cb.hh"

using namespace std;
using namespace aghui;



aghui::SExpDesignUIWidgets::
SExpDesignUIWidgets ()
{
      // load glade
	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_resource( builder, "/org/gtk/aghermann/mw.glade", NULL) ||
	     !gtk_builder_add_from_resource( builder, "/org/gtk/aghermann/mw-dialogs.glade", NULL) )
		throw runtime_error ("Failed to load main resources");

	gtk_builder_connect_signals( builder, NULL);

      // ======== construct list and tree stores
	// dynamic
	mSessions =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mEEGChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mAllChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mGlobalAnnotations =
		gtk_tree_store_new( 7,
				    G_TYPE_STRING, // id
				    G_TYPE_STRING, // at pages
				    G_TYPE_STRING, // channel
				    G_TYPE_STRING, // type
				    G_TYPE_STRING, // label
				    G_TYPE_BOOLEAN, G_TYPE_POINTER);
	mGlobalADProfiles =
		gtk_list_store_new( 1, G_TYPE_STRING);
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
	// static
	if ( !AGH_GBGETOBJ (GtkListStore,	mScoringPageSize) ||
	     !AGH_GBGETOBJ (GtkListStore,	mFFTParamsPageSize) ||
	     !AGH_GBGETOBJ (GtkListStore,	mFFTParamsBinSize) ||
	     !AGH_GBGETOBJ (GtkListStore,	mFFTParamsWindowType) )
		throw runtime_error ("Failed to construct widgets");
	// (some of) these are directly attached to combo boxes in dauaghter widgets, so ref
	g_object_ref( (GObject*)mScoringPageSize);
	g_object_ref( (GObject*)mFFTParamsPageSize);
	g_object_ref( (GObject*)mFFTParamsBinSize);
	g_object_ref( (GObject*)mFFTParamsWindowType);

      // misc
	auto font_desc = pango_font_description_from_string( "Mono 9");

	GtkTreeViewColumn *col;

      // =========== 1. Measurements
      // ------------- cMeasurements
	if ( !AGH_GBGETOBJ (GtkWindow,		wMainWindow) ||
	     !AGH_GBGETOBJ (GtkVBox,		cMeasurements) )
		throw runtime_error ("Failed to construct widgets");

	wMainWindow_delete_event_cb_handler_id =
		G_CONNECT_2 (wMainWindow, delete, event);
	G_CONNECT_2 (wMainWindow, configure, event);
	G_CONNECT_3 (wMainWindow, key, press, event);

	G_CONNECT_3 (cMeasurements, drag, data, received);
	G_CONNECT_2 (cMeasurements, drag, drop);

	gtk_drag_dest_set( (GtkWidget*)cMeasurements, GTK_DEST_DEFAULT_ALL,
			   NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets( (GtkWidget*)(cMeasurements));

	if ( !AGH_GBGETOBJ (GtkMenuItem,	iiMainMenu) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpRefresh) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpPurgeComputed) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpAnnotations) ||
	     !AGH_GBGETOBJ (GtkRadioMenuItem,	iExpSubjectSortName) ||
	     !AGH_GBGETOBJ (GtkRadioMenuItem,	iExpSubjectSortAge) ||
	     !AGH_GBGETOBJ (GtkRadioMenuItem,	iExpSubjectSortAdmissionDate) ||
	     !AGH_GBGETOBJ (GtkRadioMenuItem,	iExpSubjectSortAvgPower) ||
	     !AGH_GBGETOBJ (GtkCheckMenuItem,	iExpSubjectSortAscending) ||
	     !AGH_GBGETOBJ (GtkCheckMenuItem,	iExpSubjectSortSegregate) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpBasicSADetectUltradianCycles) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iiExpGlobalOperations) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpGloballyDetectArtifacts) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpGloballySetFilters) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iMontageSetDefaults) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpClose) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iExpQuit) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iHelpAbout) ||
	     !AGH_GBGETOBJ (GtkMenuItem,	iHelpUsage) )
		throw runtime_error ("Failed to construct widgets");

	G_CONNECT_1 (iExpClose, activate);
	// g_signal_connect( iExpClose, "activate",
	// 		  (GCallback)iExpClose_activate_cb,
	// 		  this);
	G_CONNECT_1 (iExpRefresh, activate);
	G_CONNECT_1 (iExpPurgeComputed, activate);
	G_CONNECT_1 (iExpAnnotations, activate);
	G_CONNECT_1 (iExpBasicSADetectUltradianCycles, activate);
	G_CONNECT_1 (iExpGloballyDetectArtifacts, activate);
	G_CONNECT_1 (iExpGloballySetFilters, activate);
	// G_CONNECT_1 (iMontageSetDefaults, activate);
	G_CONNECT_1 (iExpQuit, activate);
	G_CONNECT_1 (iHelpAbout, activate);
	G_CONNECT_1 (iHelpUsage, activate);

	for ( auto& w : {iExpSubjectSortName, iExpSubjectSortAge, iExpSubjectSortAdmissionDate, iExpSubjectSortAvgPower} )
		g_signal_connect( w, "toggled",
				  (GCallback)iExpSubjectSortAny_toggled_cb,
				  this);
	G_CONNECT_1 (iExpSubjectSortAscending, toggled);
	G_CONNECT_1 (iExpSubjectSortSegregate, toggled);

     // --------- tabs
	if ( !AGH_GBGETOBJ (GtkNotebook,	tTaskSelector) ||
	     !AGH_GBGETOBJ (GtkNotebook,	tDesign) ||
	     !AGH_GBGETOBJ (GtkNotebook,	tSimulations) ||
	     !AGH_GBGETOBJ (GtkNotebook,	tSettings) ||
	     !AGH_GBGETOBJ (GtkLabel,		lTaskSelector1) ||
	     !AGH_GBGETOBJ (GtkLabel,		lTaskSelector2) ||
	     !AGH_GBGETOBJ (GtkLabel,		lSettings) )
		throw runtime_error ("Failed to construct widgets");

	G_CONNECT_2 (tTaskSelector, switch, page);
	G_CONNECT_2 (tDesign, switch, page);
	G_CONNECT_2 (tSimulations, switch, page);


     // ------------- eMsmtSession, eMsmtChannel
	if ( !AGH_GBGETOBJ (GtkComboBox, eMsmtSession) ||
	     !AGH_GBGETOBJ ( GtkComboBox, eMsmtChannel) )
		throw runtime_error ("Failed to construct widgets");

	gtk_combo_box_set_model_properly(
		eMsmtSession, mSessions);
	eMsmtSession_changed_cb_handler_id =
		G_CONNECT_1 (eMsmtSession, changed);

	gtk_combo_box_set_model_properly(
		eMsmtChannel, mEEGChannels);
	eMsmtChannel_changed_cb_handler_id =
		G_CONNECT_1 (eMsmtChannel, changed);

     // ------------- eMsmtProfile*
	if ( !AGH_GBGETOBJ (GtkToggleButton,	eMsmtProfileAutoscale) ||
	     !AGH_GBGETOBJ (GtkScaleButton,	eMsmtProfileSmooth) ||

	     !AGH_GBGETOBJ (GtkListStore,	mMsmtProfileType) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eMsmtProfileType) ||
	     !AGH_GBGETOBJ (GtkBox,		cMsmtProfileParamsContainer) ||
	     !AGH_GBGETOBJ (GtkBox,		cMsmtProfileParamsPSD) ||
	     !AGH_GBGETOBJ (GtkBox,		cMsmtProfileParamsSWU) ||
	     !AGH_GBGETOBJ (GtkBox,		cMsmtProfileParamsMC) ||

	     !AGH_GBGETOBJ (GtkSpinButton,	eMsmtProfileParamsPSDFreqFrom)  ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMsmtProfileParamsPSDFreqWidth) ||
	     !AGH_GBGETOBJ (GtkAdjustment,	jMsmtProfileParamsPSDFreqFrom)  ||
	     !AGH_GBGETOBJ (GtkAdjustment,	jMsmtProfileParamsPSDFreqWidth) ||

	     !AGH_GBGETOBJ (GtkSpinButton,	eMsmtProfileParamsSWUF0)  ||
	     !AGH_GBGETOBJ (GtkAdjustment,	jMsmtProfileParamsSWUF0)  ||

	     !AGH_GBGETOBJ (GtkSpinButton,	eMsmtProfileParamsMCF0)  ||
	     !AGH_GBGETOBJ (GtkAdjustment,	jMsmtProfileParamsMCF0)  ||

	     !AGH_GBGETOBJ (GtkLabel,		lMsmtProfilePSDExtra) ||
	     !AGH_GBGETOBJ (GtkLabel,		lMsmtProfileSWUExtra) ||
	     !AGH_GBGETOBJ (GtkLabel,		lMsmtProfileMCExtra) ||

	     !AGH_GBGETOBJ (GtkBox,		cMsmtMainToolbar) ||
	     !AGH_GBGETOBJ (GtkBox,		cMsmtTopArea) )
		throw runtime_error ("Failed to construct widgets");

	gtk_cell_layout_set_renderer( eMsmtProfileType);
	// and when was the list store attached to it, eh?

	G_CONNECT_1 (eMsmtProfileType, changed);
	G_CONNECT_1 (eMsmtProfileAutoscale, toggled);
	G_CONNECT_2 (eMsmtProfileSmooth, value, changed);

	G_CONNECT_2 (eMsmtProfileParamsPSDFreqFrom, value, changed);
	G_CONNECT_2 (eMsmtProfileParamsPSDFreqWidth, value, changed);
	G_CONNECT_2 (eMsmtProfileParamsSWUF0, value, changed);
	G_CONNECT_2 (eMsmtProfileParamsMCF0, value, changed);

      // ------------ menus
	if ( !(AGH_GBGETOBJ (GtkMenu,		iiSubjectTimeline)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineScore)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineDetectUltradianCycle)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineEDFInfo)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineSaveAsSVG)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineResetMontage)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSubjectTimelineBrowse)) )
		throw runtime_error ("Failed to construct widgets");

	g_object_ref( (GObject*)iiSubjectTimeline);
	g_object_ref( (GObject*)iSubjectTimelineScore);
	g_object_ref( (GObject*)iSubjectTimelineDetectUltradianCycle);
	g_object_ref( (GObject*)iSubjectTimelineEDFInfo);
	g_object_ref( (GObject*)iSubjectTimelineSaveAsSVG);
	g_object_ref( (GObject*)iSubjectTimelineResetMontage);
	g_object_ref( (GObject*)iSubjectTimelineBrowse);

	G_CONNECT_1 (iSubjectTimelineScore, activate);
	G_CONNECT_1 (iSubjectTimelineDetectUltradianCycle, activate);
	G_CONNECT_1 (iSubjectTimelineEDFInfo, activate);
	G_CONNECT_1 (iSubjectTimelineSaveAsSVG, activate);
	G_CONNECT_1 (iSubjectTimelineBrowse, activate);
	G_CONNECT_1 (iSubjectTimelineResetMontage, activate);

      // ------------ actions
	if ( !(AGH_GBGETOBJ (GtkButton,		bMainCloseThatSF)) )
		throw runtime_error ("Failed to construct widgets");

	G_CONNECT_1 (bMainCloseThatSF, clicked);

   // ================ 2. Simulations
     // ------------- tvSimulations & controls
	if ( !(AGH_GBGETOBJ (GtkTreeView, tvSimulations)) )
		throw runtime_error ("Failed to construct widgets");

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
	G_CONNECT_2 (tvSimulations, row, activated);

	for ( auto c = 0; c < msimulations_visibility_switch_col; ++c ) {
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
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
			(c == 0) ? "Id" : (c == 1) ? "Status" : agh::ach::tunable_pango_name(c-2).c_str());
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
		throw runtime_error ("Failed to construct widgets");

	G_CONNECT_1 (iSimulationsRunBatch, activate);
	G_CONNECT_1 (iSimulationsRunClearAll, activate);
	G_CONNECT_1 (iSimulationsReportGenerate, activate);

      // ------------- lSimulations{Session,Channel}
	if ( !AGH_GBGETOBJ (GtkLabel, lSimulationsProfile) ||
	     !AGH_GBGETOBJ (GtkLabel, lSimulationsChannel) ||
	     !AGH_GBGETOBJ (GtkLabel, lSimulationsSession) )
		throw runtime_error ("Failed to construct widgets");

      // ------- statusbar
	if ( !AGH_GBGETOBJ (GtkStatusbar,	sbMainStatusBar) )
		throw runtime_error ("Failed to construct widgets");

	sbMainContextIdGeneral = gtk_statusbar_get_context_id( sbMainStatusBar, "General context");

	if ( !(AGH_GBGETOBJ (GtkDialog,		wScanLog)) ||
	     !(AGH_GBGETOBJ (GtkTextView,	tScanLog)) )
		throw runtime_error ("Failed to construct widgets");

	gtk_widget_override_font( (GtkWidget*)tScanLog, font_desc);
	// free? unref? leak some?

      // ****************** settings
	if ( !AGH_GBGETOBJ (GtkSpinButton,	eSMPMaxThreads) )
		throw runtime_error ("Failed to construct widgets");

      // ------------- fFFTParams
	if ( !AGH_GBGETOBJ (GtkSpinButton,	eUltradianCycleDetectionAccuracy) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eFFTParamsBinSize) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eFFTParamsPageSize) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eFFTParamsPlanType) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eFFTParamsWindowType) )
		throw runtime_error ("Failed to construct widgets");

	for ( auto& e : {eFFTParamsBinSize, eFFTParamsPageSize, eFFTParamsPlanType, eFFTParamsWindowType} )
		gtk_cell_layout_set_renderer( e);

      // ------------- fArtifacts
	if ( !AGH_GBGETOBJ (GtkComboBox,	eArtifDampenWindowType) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eArtifDampenFactor) )
		throw runtime_error ("Failed to construct widgets");

	gtk_cell_layout_set_renderer( eArtifDampenWindowType);

      // ------------- fMicrocontinuity
	if ( !AGH_GBGETOBJ (GtkSpinButton,	eMCParamBandWidth) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamIIRBackpolate) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamMCGain) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamFreqInc) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMCParamNBins) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSWUParamMinUpswingDuration) )
		throw runtime_error ("Failed to construct widgets");

      // ------- custom score codes
	if ( !(eScoreCode[sigfile::SPage::TScore::none]		= (GtkEntry*)gtk_builder_get_object( builder, "eScoreCodeUnscored")) ||
	     !(eScoreCode[sigfile::SPage::TScore::nrem1]	= (GtkEntry*)gtk_builder_get_object( builder, "eScoreCodeNREM1")) ||
	     !(eScoreCode[sigfile::SPage::TScore::nrem2]	= (GtkEntry*)gtk_builder_get_object( builder, "eScoreCodeNREM2")) ||
	     !(eScoreCode[sigfile::SPage::TScore::nrem3]	= (GtkEntry*)gtk_builder_get_object( builder, "eScoreCodeNREM3")) ||
	     !(eScoreCode[sigfile::SPage::TScore::nrem4]	= (GtkEntry*)gtk_builder_get_object( builder, "eScoreCodeNREM4")) ||
	     !(eScoreCode[sigfile::SPage::TScore::rem]		= (GtkEntry*)gtk_builder_get_object( builder, "eScoreCodeREM")) ||
	     !(eScoreCode[sigfile::SPage::TScore::wake]		= (GtkEntry*)gtk_builder_get_object( builder, "eScoreCodeWake")))
		throw runtime_error ("Failed to construct widgets");

      // --------- Bands
	if ( !(eBand[metrics::psd::TBand::delta][0]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandDeltaFrom")) ||
	     !(eBand[metrics::psd::TBand::delta][1]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandDeltaUpto")) ||
	     !(eBand[metrics::psd::TBand::theta][0]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandThetaFrom")) ||
	     !(eBand[metrics::psd::TBand::theta][1]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandThetaUpto")) ||
	     !(eBand[metrics::psd::TBand::alpha][0]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandAlphaFrom")) ||
	     !(eBand[metrics::psd::TBand::alpha][1]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandAlphaUpto")) ||
	     !(eBand[metrics::psd::TBand::beta ][0]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandBetaFrom" )) ||
	     !(eBand[metrics::psd::TBand::beta ][1]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandBetaUpto" )) ||
	     !(eBand[metrics::psd::TBand::gamma][0]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandGammaFrom")) ||
	     !(eBand[metrics::psd::TBand::gamma][1]   = (GtkSpinButton*)gtk_builder_get_object( builder, "eBandGammaUpto")) )
		throw runtime_error ("Failed to construct widgets");

      // --------- Misc
	if ( !AGH_GBGETOBJ (GtkSpinButton, eDAMsmtPPH) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAMsmtTLHeight) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAPageHeight) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAHypnogramHeight) ||
	     !AGH_GBGETOBJ (GtkSpinButton, eDAEMGHeight) ||
	     !AGH_GBGETOBJ (GtkCheckButton, eScanTreeStrict) )
		throw runtime_error ("Failed to construct widgets");

	if ( !AGH_GBGETOBJ (GtkEntry,	eBrowseCommand) )
		throw runtime_error ("Failed to construct widgets");


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
	     !AGH_GBGETOBJ (GtkLabel,		lCtlParamDBAmendment1) ||
	     !AGH_GBGETOBJ (GtkLabel,		lCtlParamDBAmendment2) ||
	     !AGH_GBGETOBJ (GtkLabel,		lCtlParamAZAmendment1) ||
	     !AGH_GBGETOBJ (GtkRadioButton,	eCtlParamScoreUnscoredAsWake) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamNSWAPpBeforeSimStart) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eCtlParamReqScoredPercent) )
		throw runtime_error ("Failed to construct widgets");

	G_CONNECT_1 (eCtlParamDBAmendment1, toggled);
	G_CONNECT_1 (eCtlParamDBAmendment2, toggled);
	G_CONNECT_1 (eCtlParamAZAmendment1, toggled);

      // ------------- eTunable_*
	using namespace agh::ach;
	if ( !(eTunable[TTunable::rs][0]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_rs")) ||
	     !(eTunable[TTunable::rs][1]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_rs_min")) ||
	     !(eTunable[TTunable::rs][2]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_rs_max")) ||
	     !(eTunable[TTunable::rs][3]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_rs_step")) ||

	     !(eTunable[TTunable::rc][0]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_rc")) ||
	     !(eTunable[TTunable::rc][1]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_rc_min")) ||
	     !(eTunable[TTunable::rc][2]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_rc_max")) ||
	     !(eTunable[TTunable::rc][3]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_rc_step")) ||

	     !(eTunable[TTunable::fcR][0]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_fcR")) ||
	     !(eTunable[TTunable::fcR][1]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_fcR_min")) ||
	     !(eTunable[TTunable::fcR][2]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_fcR_max")) ||
	     !(eTunable[TTunable::fcR][3]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_fcR_step")) ||

	     !(eTunable[TTunable::fcW][0]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_fcW")) ||
	     !(eTunable[TTunable::fcW][1]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_fcW_min")) ||
	     !(eTunable[TTunable::fcW][2]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_fcW_max")) ||
	     !(eTunable[TTunable::fcW][3]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_fcW_step")) ||

	     !(eTunable[TTunable::S0][0]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_S0")) ||
	     !(eTunable[TTunable::S0][1]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_S0_min")) ||
	     !(eTunable[TTunable::S0][2]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_S0_max")) ||
	     !(eTunable[TTunable::S0][3]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_S0_step")) ||

	     !(eTunable[TTunable::SU][0]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_SU")) ||
	     !(eTunable[TTunable::SU][1]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_SU_min")) ||
	     !(eTunable[TTunable::SU][2]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_SU_max")) ||
	     !(eTunable[TTunable::SU][3]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_SU_step")) ||

	     !(eTunable[TTunable::ta][0]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_ta")) ||
	     !(eTunable[TTunable::ta][1]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_ta_min")) ||
	     !(eTunable[TTunable::ta][2]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_ta_max")) ||
	     !(eTunable[TTunable::ta][3]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_ta_step")) ||

	     !(eTunable[TTunable::tp][0]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_tp")) ||
	     !(eTunable[TTunable::tp][1]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_tp_min")) ||
	     !(eTunable[TTunable::tp][2]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_tp_max")) ||
	     !(eTunable[TTunable::tp][3]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_tp_step")) ||

	     !(eTunable[TTunable::gc][0]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_gc")) ||
	     !(eTunable[TTunable::gc][1]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_gc_min")) ||
	     !(eTunable[TTunable::gc][2]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_gc_max")) ||
	     !(eTunable[TTunable::gc][3]	= (GtkSpinButton*)gtk_builder_get_object( builder, "eTunable_gc_step")) )
		throw runtime_error ("Failed to construct widgets");

	for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t )
		for ( auto d = 0; d < 4; ++d )
			jTunable[t][d] = gtk_spin_button_get_adjustment( eTunable[t][d]);


	if ( !AGH_GBGETOBJ (GtkButton,	bSimParamRevertTunables) )
		throw runtime_error ("Failed to construct widgets");
	G_CONNECT_1 (bSimParamRevertTunables, clicked);

      // ------ colours
	if ( !(CwB[TColour::mw_night	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMWNight")) ||
	     !(CwB[TColour::mw_day	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMWDay")) ||
	     !(CwB[TColour::mw_profile	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMWProfile")) ||
	     !(CwB[TColour::mw_ticks	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMWTicks")) ||
	     !(CwB[TColour::mw_labels	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMWLabels")) ||

	     !(CwB[TColour::sf_profile_psd    ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFProfilePSD")) ||
	     !(CwB[TColour::sf_profile_mc     ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFProfileMC")) ||
	     !(CwB[TColour::sf_profile_swu    ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFProfileSWU")) ||
	     !(CwB[TColour::sf_phasic_spindle ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFPhasicSpindle")) ||
	     !(CwB[TColour::sf_phasic_Kcomplex].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFPhasicKComplex")) ||
	     !(CwB[TColour::sf_phasic_eyeblink].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFPhasicEyeBlink")) ||
	     !(CwB[TColour::sf_emg	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFEMG")) ||
	     !(CwB[TColour::sf_hypnogram      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFHypnogram")) ||
	     !(CwB[TColour::sf_artifact	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFArtifacts")) ||
	     !(CwB[TColour::sf_annotations    ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFAnnotations")) ||
	     !(CwB[TColour::sf_embedded_annotations]
					       .btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFEmbeddedAnnotations")) ||
	     !(CwB[TColour::sf_selection      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFSelection")) ||
	     !(CwB[TColour::sf_ticks	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFTicks")) ||
	     !(CwB[TColour::sf_labels	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFLabels")) ||
	     !(CwB[TColour::sf_cursor	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourSFCursor")) ||

	     !(CwB[TColour::mf_swa	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMFSWA")) ||
	     !(CwB[TColour::mf_swa_sim	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMFSWASim")) ||
	     !(CwB[TColour::mf_process_s      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMFProcessS")) ||
	     !(CwB[TColour::mf_paper	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMFPaper")) ||
	     !(CwB[TColour::mf_ticks	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMFTicks")) ||
	     !(CwB[TColour::mf_labels	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourMFLabels")) ||

	     !(CwB[TColour::score_none	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourScoreNONE")) ||
	     !(CwB[TColour::score_nrem1	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourScoreNREM1")) ||
	     !(CwB[TColour::score_nrem2	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourScoreNREM2")) ||
	     !(CwB[TColour::score_nrem3	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourScoreNREM3")) ||
	     !(CwB[TColour::score_nrem4	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourScoreNREM4")) ||
	     !(CwB[TColour::score_rem	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourScoreREM")) ||
	     !(CwB[TColour::score_wake	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourScoreWake")) ||

	     !(CwB[TColour::band_delta	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourBandDelta")) ||
	     !(CwB[TColour::band_theta	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourBandTheta")) ||
	     !(CwB[TColour::band_alpha	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourBandAlpha")) ||
	     !(CwB[TColour::band_beta	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourBandBeta")) ||
	     !(CwB[TColour::band_gamma	      ].btn = (GtkColorButton*)gtk_builder_get_object( builder, "bColourBandGamma")) )
		throw runtime_error ("Failed to construct widgets");

      // scrub colours
	for ( auto &C : CwB ) {
		g_signal_connect( C.second.btn, "color-set",
				  (GCallback)bColourX_color_set_cb,
				  &C.second);
	}
	// get CwB color values from glade
	for ( auto &C : CwB )
		g_signal_emit_by_name( C.second.btn, "color-set");


      // ========= child widgets
      // ----- wAbout
	if ( !AGH_GBGETOBJ (GtkDialog,		wAbout) ||
	     !AGH_GBGETOBJ (GtkNotebook,	cAboutTabs) ||
	     !AGH_GBGETOBJ (GtkLabel,		lAboutVersion) )
		throw runtime_error ("Failed to construct widgets");

      // ------- wEDFFileDetails
	if ( !AGH_GBGETOBJ (GtkDialog,		wEDFFileDetails) ||
	     !AGH_GBGETOBJ (GtkTextView,	lEDFFileDetailsReport) )
		throw runtime_error ("Failed to construct widgets");

	// used by two GtkTextView's, lEDFFileDetailsReport and lEdfImportFileInfo
	if ( !AGH_GBGETOBJ (GtkTextBuffer,	tEDFFileDetailsReport) )
		throw runtime_error ("Failed to construct widgets");

	gtk_widget_override_font( (GtkWidget*)lEDFFileDetailsReport, font_desc);
	g_object_set( lEDFFileDetailsReport,
		      "tabs", pango_tab_array_new_with_positions(
			      2, TRUE,
			      PANGO_TAB_LEFT, 180,
			      PANGO_TAB_LEFT, 190),
		      NULL);

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
		throw runtime_error ("Failed to construct widgets");


	gtk_widget_override_font( (GtkWidget*)lEdfImportFileInfo, font_desc);

	g_object_set( lEdfImportFileInfo,
		      "tabs", pango_tab_array_new_with_positions(
				2, TRUE,
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

      // ------------- wBatchSetup
	if ( !AGH_GBGETOBJ (GtkDialog,		wBatchSetup) ||
	     !AGH_GBGETOBJ (GtkEntry,		eBatchSetupSubjects) ||
	     !AGH_GBGETOBJ (GtkEntry,		eBatchSetupSessions) ||
	     !AGH_GBGETOBJ (GtkEntry,		eBatchSetupChannels) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eBatchSetupRangeFrom) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eBatchSetupRangeWidth) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eBatchSetupRangeInc) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eBatchSetupRangeSteps) )
		throw runtime_error ("Failed to construct widgets");

      // // ------------- wMontageDefaults
      // 	if ( !AGH_GBGETOBJ (GtkDialog,		wMontageDefaults) ||
      // 	     !AGH_GBGETOBJ (GtkEntry,		eMontageDefaultsChannelList) ||
      // 	     !AGH_GBGETOBJ (GtkCheckButton,	eMontageDefaultsShowPSD) ||
      // 	     !AGH_GBGETOBJ (GtkCheckButton,	eMontageDefaultsShowPSDSpectrum) ||
      // 	     !AGH_GBGETOBJ (GtkCheckButton,	eMontageDefaultsShowMC) ||
      // 	     !AGH_GBGETOBJ (GtkCheckButton,	eMontageDefaultsShowEMG) )
      // 		throw runtime_error ("Failed to construct widgets");

      // ----------- wGlobalFilters
	if ( !AGH_GBGETOBJ (GtkDialog,		wGlobalFilters) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eGlobalFiltersLowPassCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eGlobalFiltersLowPassOrder) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eGlobalFiltersHighPassCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eGlobalFiltersHighPassOrder) ||
	     !AGH_GBGETOBJ (GtkListStore,	mGlobalFiltersNotchFilter) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eGlobalFiltersNotchFilter) )
		throw runtime_error ("Failed to construct widgets");
	gtk_combo_box_set_model_properly(
		eGlobalFiltersNotchFilter, mGlobalFiltersNotchFilter);

      // ----------- wGlobalAnnotations
	if ( !AGH_GBGETOBJ (GtkDialog,		wGlobalAnnotations) ||
	     !AGH_GBGETOBJ (GtkTreeView,	tvGlobalAnnotations) ||
	     !AGH_GBGETOBJ (GtkCheckButton,	eGlobalAnnotationsShowPhasicEvents) )
		throw runtime_error ("Failed to construct widgets");

	gtk_tree_view_set_model( tvGlobalAnnotations,
				 (GtkTreeModel*)mGlobalAnnotations);

	g_object_set( (GObject*)tvGlobalAnnotations,
		      "expander-column", 0,
		      NULL);
	g_signal_connect( tvGlobalAnnotations, "map",
			  (GCallback)gtk_tree_view_expand_all,
			  NULL);
	G_CONNECT_2 (tvGlobalAnnotations, row, activated);

	int c = 0;
	for ( auto column : {"Recording", "Page(s)", "Channel", "Type", "Label"} ) {
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		g_object_set( (GObject*)renderer,
			      "editable", FALSE,
			      NULL);
		g_object_set_data( (GObject*)renderer, "column", GINT_TO_POINTER (c));
		col = gtk_tree_view_column_new_with_attributes(
			column,
			renderer,
			"text", c,
			NULL);
		gtk_tree_view_column_set_expand( col, TRUE);
		gtk_tree_view_append_column( tvGlobalAnnotations, col);
		++c;
	}
	gtk_tree_view_append_column( tvGlobalAnnotations,
				     gtk_tree_view_column_new());

	G_CONNECT_1 (eGlobalAnnotationsShowPhasicEvents, toggled);

      // ------------- wGlobalArtifactDetection
	if ( !AGH_GBGETOBJ (GtkDialog,		wGlobalArtifactDetection) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eGlobalADProfiles) ||
	     !AGH_GBGETOBJ (GtkCheckButton,	eGlobalADKeepExisting) ||
	     !AGH_GBGETOBJ (GtkButton,		bGlobalADOK) ||
	     !AGH_GBGETOBJ (GtkLabel,		lGlobalADHint) )
		throw runtime_error ("Failed to construct widgets");

	gtk_combo_box_set_model_properly(
		eGlobalADProfiles, mGlobalADProfiles);
	G_CONNECT_1 (eGlobalADProfiles, changed);

	pango_font_description_free( font_desc);
}


aghui::SExpDesignUIWidgets::
~SExpDesignUIWidgets ()
{
	g_object_unref( (GObject*)builder);

      // destroy toplevels
	gtk_widget_destroy( (GtkWidget*)wMainWindow);
	gtk_widget_destroy( (GtkWidget*)wAbout);
	gtk_widget_destroy( (GtkWidget*)wScanLog);
	gtk_widget_destroy( (GtkWidget*)wEDFFileDetails);
	gtk_widget_destroy( (GtkWidget*)wEdfImport);
	gtk_widget_destroy( (GtkWidget*)wGlobalAnnotations);
	gtk_widget_destroy( (GtkWidget*)wGlobalFilters);
	gtk_widget_destroy( (GtkWidget*)wBatchSetup);
      // and models, etc
	g_object_unref( (GObject*)mEEGChannels);
	g_object_unref( (GObject*)mAllChannels);
	g_object_unref( (GObject*)mSessions);
	g_object_unref( (GObject*)mGlobalAnnotations);
	g_object_unref( (GObject*)mGlobalADProfiles);
	g_object_unref( (GObject*)mSimulations);

	g_object_unref( (GObject*)mScoringPageSize);
	g_object_unref( (GObject*)mFFTParamsPageSize);
	g_object_unref( (GObject*)mFFTParamsBinSize);
	g_object_unref( (GObject*)mFFTParamsWindowType);

	g_object_unref( (GObject*)iiSubjectTimeline);
	g_object_unref( (GObject*)iSubjectTimelineScore);
	g_object_unref( (GObject*)iSubjectTimelineEDFInfo);
	g_object_unref( (GObject*)iSubjectTimelineSaveAsSVG);
	g_object_unref( (GObject*)iSubjectTimelineResetMontage);
	g_object_unref( (GObject*)iSubjectTimelineBrowse);
	// I'm quite possibly missing something
}



void
aghui::SExpDesignUIWidgets::
set_wMainWindow_interactive( bool indeed, bool flush)
{
	set_cursor_busy( not indeed, (GtkWidget*)wMainWindow);
	//gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, indeed);

	gtk_widget_set_sensitive( (GtkWidget*)cMsmtMainToolbar, indeed);
	gtk_widget_set_sensitive( (GtkWidget*)cMeasurements, indeed);

	gtk_widget_set_visible( (GtkWidget*)lTaskSelector2, indeed);
	gtk_widget_set_visible( gtk_notebook_get_nth_page( tTaskSelector, 1), indeed);
	gtk_widget_set_visible( (GtkWidget*)lSettings, indeed);
	gtk_widget_set_visible( gtk_notebook_get_nth_page( tDesign, 1), indeed);

	gtk_widget_set_sensitive( (GtkWidget*)iiMainMenu, indeed);
	gtk_widget_set_sensitive( (GtkWidget*)eMsmtSession, indeed);
	gtk_widget_set_sensitive( (GtkWidget*)eMsmtChannel, indeed);

	gtk_widget_set_visible( (GtkWidget*)cMsmtTopArea, indeed);

	if ( flush )
		gtk_flush();
}

void
aghui::SExpDesignUIWidgets::
set_controls_for_empty_experiment( bool indeed, bool flush)
{
	indeed = not indeed;
	gtk_widget_set_visible( (GtkWidget*)cMsmtMainToolbar, indeed);

	gtk_widget_set_visible( (GtkWidget*)lTaskSelector2, indeed);
	gtk_widget_set_visible( gtk_notebook_get_nth_page( tTaskSelector, 1), indeed);

	gtk_widget_set_visible( (GtkWidget*)iExpAnnotations, indeed);
	gtk_widget_set_visible( (GtkWidget*)iiExpGlobalOperations, indeed);

	gtk_widget_set_visible( (GtkWidget*)eMsmtSession, indeed);
	gtk_widget_set_visible( (GtkWidget*)eMsmtChannel, indeed);

	if ( flush )
		gtk_flush();
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
