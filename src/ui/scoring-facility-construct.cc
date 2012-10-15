// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-13
 *
 *         Purpose:  scoring facility widget construct megafun
 *
 *         License:  GPL
 */


#include "expdesign.hh"
#include <stdexcept>
#include "ui.hh"
#include "scoring-facility-widgets.hh"
#include "scoring-facility_cb.hh"

using namespace std;


aghui::SScoringFacilityWidgets::
SScoringFacilityWidgets (SExpDesignUI& _p)
{
	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_resource( builder, "/org/gtk/aghermann/sf.glade", NULL) ) {
		g_object_unref( (GObject*)builder);
		throw runtime_error( "Failed to load SF glade resource");
	}
	gtk_builder_connect_signals( builder, NULL);
	//  we do it all mostly ourself, except for some delete-event binding to gtk_true()

	GtkCellRenderer *renderer;

	// general & montage page navigation
	if ( !(AGH_GBGETOBJ (GtkWindow,		wScoringFacility)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lSFHint)) ||
	     !(AGH_GBGETOBJ (GtkListStore,	mScoringPageSize) ) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eSFPageSize)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFCurrentPage)) ||
	     !(AGH_GBGETOBJ (GtkAdjustment,	jPageNo)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lSFTotalPages)) ||
	     !(AGH_GBGETOBJ (GtkButton,		eSFCurrentPos)) ||

	     !(AGH_GBGETOBJ (GtkExpander,	cSFHypnogram)) ||
	     !(AGH_GBGETOBJ (GtkHBox,		cSFControlBar)) ||
	     !(AGH_GBGETOBJ (GtkBox,		cSFScoringModeContainer)) ||
	     !(AGH_GBGETOBJ (GtkBox,		cSFICAModeContainer)) ||

	     !(AGH_GBGETOBJ (GtkButton,		bSFBack)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFForward)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreGotoPrevUnscored)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreGotoNextUnscored)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreGotoPrevArtifact)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreGotoNextArtifact)) ||

	     !(AGH_GBGETOBJ (GtkToggleButton,	bSFShowFindDialog)) ||
	     !(AGH_GBGETOBJ (GtkToggleButton,	bSFShowPhaseDiffDialog)) ||
	     !(AGH_GBGETOBJ (GtkToggleButton,	bSFDrawCrosshair)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFRunICA)) )
		throw runtime_error ("Failed to contruct SF widgets");

	g_signal_connect( wScoringFacility, "delete-event",
			  (GCallback)wScoringFacility_delete_event_cb,
			  this);

	gtk_combo_box_set_model( eSFPageSize,
				 (GtkTreeModel*)mScoringPageSize);
	gtk_combo_box_set_id_column( eSFPageSize, 0);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFPageSize, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFPageSize, renderer,
					"text", 0,
					NULL);

	g_signal_connect( eSFPageSize, "changed",
			  (GCallback)eSFPageSize_changed_cb,
			  this);
	g_signal_connect( eSFCurrentPage, "value-changed",
			  (GCallback)eSFCurrentPage_value_changed_cb,
			  this);

	g_signal_connect( eSFCurrentPos, "clicked",
			  (GCallback)eSFCurrentPos_clicked_cb,
			  this);

	g_signal_connect( bSFForward, "clicked",
			  (GCallback)bSFForward_clicked_cb,
			  this);
	g_signal_connect( bSFBack, "clicked",
			  (GCallback)bSFBack_clicked_cb,
			  this);

	g_signal_connect( bScoreGotoNextUnscored, "clicked",
			  (GCallback)bScoreGotoNextUnscored_clicked_cb,
			  this);
	g_signal_connect( bScoreGotoPrevUnscored, "clicked",
			  (GCallback)bScoreGotoPrevUnscored_clicked_cb,
			  this);

	g_signal_connect( bScoreGotoNextArtifact, "clicked",
			  (GCallback)bScoreGotoNextArtifact_clicked_cb,
			  this);
	g_signal_connect( bScoreGotoPrevArtifact, "clicked",
			  (GCallback)bScoreGotoPrevArtifact_clicked_cb,
			  this);

	g_signal_connect( bSFDrawCrosshair, "toggled",
			  (GCallback)bSFDrawCrosshair_toggled_cb,
			  this);
	g_signal_connect( bSFShowFindDialog, "toggled",
			  (GCallback)bSFShowFindDialog_toggled_cb,
			  this);
	g_signal_connect( bSFShowPhaseDiffDialog, "toggled",
			  (GCallback)bSFShowPhaseDiffDialog_toggled_cb,
			  this);
	g_signal_connect( bSFRunICA, "clicked",
			  (GCallback)bSFRunICA_clicked_cb,
			  this);

	if ( !(AGH_GBGETOBJ (GtkButton,		bScoreClear)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreNREM1)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreNREM2)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreNREM3)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreNREM4)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreREM))   ||
	     !(AGH_GBGETOBJ (GtkButton,		bScoreWake))  ||
	     !(AGH_GBGETOBJ (GtkTable,		cSFSleepStageStats)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lSFPercentScored)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lScoreStatsNREMPercent)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lScoreStatsREMPercent)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lScoreStatsWakePercent)) )
		throw runtime_error ("Failed to contruct SF widgets");

	g_signal_connect( bScoreClear, "clicked",
			  (GCallback)bScoreClear_clicked_cb,
			  this);
	g_signal_connect( bScoreNREM1, "clicked",
			  (GCallback)bScoreNREM1_clicked_cb,
			  this);
	g_signal_connect( bScoreNREM2, "clicked",
			  (GCallback)bScoreNREM2_clicked_cb,
			  this);
	g_signal_connect( bScoreNREM3, "clicked",
			  (GCallback)bScoreNREM3_clicked_cb,
			  this);
	g_signal_connect( bScoreNREM4, "clicked",
			  (GCallback)bScoreNREM4_clicked_cb,
			  this);
	g_signal_connect( bScoreREM, "clicked",
			  (GCallback)bScoreREM_clicked_cb,
			  this);
	g_signal_connect( bScoreWake, "clicked",
			  (GCallback)bScoreWake_clicked_cb,
			  this);

	if ( !(AGH_GBGETOBJ (GtkDrawingArea,	daSFMontage)) ||
	     !(AGH_GBGETOBJ (GtkDrawingArea,	daSFHypnogram)) ||
	     !(AGH_GBGETOBJ (GtkMenuToolButton,	bSFAccept)) ||
	     !(AGH_GBGETOBJ (GtkMenu,		iiSFAccept)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSFAcceptAndTakeNext)) ||
	     !(AGH_GBGETOBJ (GtkStatusbar,	sbSF)) )
		throw runtime_error ("Failed to contruct SF widgets");

	sbSFContextIdGeneral = gtk_statusbar_get_context_id( sbSF, "General context");

	g_signal_connect( daSFMontage, "draw",
			  (GCallback)daSFMontage_draw_cb,
			  this);
	g_signal_connect( daSFMontage, "configure-event",
			  (GCallback)daSFMontage_configure_event_cb,
			  this);
	g_signal_connect( daSFMontage, "button-press-event",
			  (GCallback)daSFMontage_button_press_event_cb,
			  this);
	g_signal_connect( daSFMontage, "button-release-event",
			  (GCallback)daSFMontage_button_release_event_cb,
			  this);
	g_signal_connect( daSFMontage, "scroll-event",
			  (GCallback)daSFMontage_scroll_event_cb,
			  this);
	g_signal_connect( daSFMontage, "motion-notify-event",
			  (GCallback)daSFMontage_motion_notify_event_cb,
			  this);
	g_signal_connect( daSFMontage, "leave-notify-event",
			  (GCallback)daSFMontage_leave_notify_event_cb,
			  this);

	g_signal_connect( daSFHypnogram, "draw",
			  (GCallback)daSFHypnogram_draw_cb,
			  this);
	g_signal_connect( daSFHypnogram, "button-press-event",
			  (GCallback)daSFHypnogram_button_press_event_cb,
			  this);
	g_signal_connect( daSFHypnogram, "button-release-event",
			  (GCallback)daSFHypnogram_button_release_event_cb,
			  this);
	g_signal_connect( daSFHypnogram, "motion-notify-event",
			  (GCallback)daSFHypnogram_motion_notify_event_cb,
			  this);

	gtk_menu_tool_button_set_menu( bSFAccept, (GtkWidget*)iiSFAccept);

	g_signal_connect( bSFAccept, "clicked",
			  (GCallback)bSFAccept_clicked_cb,
			  this);
	g_signal_connect( iSFAcceptAndTakeNext, "activate",
			  (GCallback)iSFAcceptAndTakeNext_activate_cb,
			  this);

	// ICA
	if ( !(AGH_GBGETOBJ (GtkComboBox,	eSFICARemixMode)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eSFICANonlinearity)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eSFICAApproach)) ||
	     !(AGH_GBGETOBJ (GtkListStore,	mSFICARemixMode)) ||
	     !(AGH_GBGETOBJ (GtkListStore,	mSFICANonlinearity)) ||
	     !(AGH_GBGETOBJ (GtkListStore,	mSFICAApproach)) ||
	     !(AGH_GBGETOBJ (GtkCheckButton,	eSFICAFineTune)) ||
	     !(AGH_GBGETOBJ (GtkCheckButton,	eSFICAStabilizationMode)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFICAa1)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFICAa2)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFICAmu)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFICAepsilon)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFICANofICs)) ||
	     !(AGH_GBGETOBJ (GtkAdjustment,	jSFICANofICs)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFICAEigVecFirst)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFICAEigVecLast)) ||
	     !(AGH_GBGETOBJ (GtkAdjustment,	jSFICAEigVecFirst)) ||
	     !(AGH_GBGETOBJ (GtkAdjustment,	jSFICAEigVecLast)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFICASampleSizePercent)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFICAMaxIterations)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFICATry)) ||
	     !(AGH_GBGETOBJ (GtkToggleButton,	bSFICAPreview)) ||
	     !(AGH_GBGETOBJ (GtkToggleButton,	bSFICAShowMatrix)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFICAApply)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFICACancel)) ||
	     !(AGH_GBGETOBJ (GtkDialog,		wSFICAMatrix)) ||
	     !(AGH_GBGETOBJ (GtkTextView,	tSFICAMatrix)) )
		throw runtime_error ("Failed to contruct SF widgets");

	gtk_combo_box_set_model( eSFICANonlinearity,
				 (GtkTreeModel*)mSFICANonlinearity);
	gtk_combo_box_set_id_column( eSFICANonlinearity, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFICANonlinearity, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFICANonlinearity, renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( eSFICAApproach,
				 (GtkTreeModel*)mSFICAApproach);
	gtk_combo_box_set_id_column( eSFICAApproach, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFICAApproach, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFICAApproach, renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( eSFICARemixMode,
				 (GtkTreeModel*)mSFICARemixMode);
	gtk_combo_box_set_id_column( eSFICARemixMode, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFICARemixMode, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFICARemixMode, renderer,
					"text", 0,
					NULL);

	auto tabarray = pango_tab_array_new( 20, FALSE);  // 20 channels is good enough
	for ( int t = 1; t < 20; ++t )
		pango_tab_array_set_tab( tabarray, t-1, PANGO_TAB_LEFT, t * 12);
	g_object_set( tSFICAMatrix,
		      "tabs", tabarray,
		      NULL);

	g_signal_connect( eSFICARemixMode, "changed",
			  (GCallback)eSFICARemixMode_changed_cb,
			  this);
	g_signal_connect( eSFICANonlinearity, "changed",
			  (GCallback)eSFICANonlinearity_changed_cb,
			  this);
	g_signal_connect( eSFICAApproach, "changed",
			  (GCallback)eSFICAApproach_changed_cb,
			  this);
	g_signal_connect( eSFICAFineTune, "toggled",
			  (GCallback)eSFICAFineTune_toggled_cb,
			  this);
	g_signal_connect( eSFICAStabilizationMode, "toggled",
			  (GCallback)eSFICAStabilizationMode_toggled_cb,
			  this);
	g_signal_connect( eSFICAa1, "value-changed",
			  (GCallback)eSFICAa1_value_changed_cb,
			  this);
	g_signal_connect( eSFICAa2, "value-changed",
			  (GCallback)eSFICAa2_value_changed_cb,
			  this);
	g_signal_connect( eSFICAmu, "value-changed",
			  (GCallback)eSFICAmu_value_changed_cb,
			  this);
	g_signal_connect( eSFICAepsilon, "value-changed",
			  (GCallback)eSFICAepsilon_value_changed_cb,
			  this);
	g_signal_connect( eSFICANofICs, "value-changed",
			  (GCallback)eSFICANofICs_value_changed_cb,
			  this);
	g_signal_connect( eSFICAEigVecFirst, "value-changed",
			  (GCallback)eSFICAEigVecFirst_value_changed_cb,
			  this);
	g_signal_connect( eSFICAEigVecLast, "value-changed",
			  (GCallback)eSFICAEigVecLast_value_changed_cb,
			  this);
	g_signal_connect( eSFICASampleSizePercent, "value-changed",
			  (GCallback)eSFICASampleSizePercent_value_changed_cb,
			  this);
	g_signal_connect( eSFICAMaxIterations, "value-changed",
			  (GCallback)eSFICAMaxIterations_value_changed_cb,
			  this);

	g_signal_connect( bSFICATry, "clicked",
			  (GCallback)bSFICATry_clicked_cb,
			  this);
	g_signal_connect( bSFICAPreview, "toggled",
			  (GCallback)bSFICAPreview_toggled_cb,
			  this);
	g_signal_connect( bSFICAShowMatrix, "toggled",
			  (GCallback)bSFICAShowMatrix_toggled_cb,
			  this);
	g_signal_connect( wSFICAMatrix, "hide",
			  G_CALLBACK (wSFICAMatrix_hide_cb),
			  this);

	g_signal_connect( bSFICAApply, "clicked",
			  (GCallback)bSFICAApply_clicked_cb,
			  this);
	g_signal_connect( bSFICACancel, "clicked",
			  (GCallback)bSFICACancel_clicked_cb,
			  this);

	// ------- menus
	if ( !(AGH_GBGETOBJ (GtkLabel, 		lSFOverChannel)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		mSFPage)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		mSFICAPage)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		mSFPageSelection)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		mSFPageAnnotation)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		mSFPageHidden)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		mSFPower)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		mSFScore)) ||

	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageShowOriginal)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageShowProcessed)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageUseResample)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageDrawZeroline)) ||
	     !(AGH_GBGETOBJ (GtkSeparatorMenuItem,	iSFPageProfileItemsSeparator)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawPSDProfile)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawPSDSpectrum)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawMCProfile)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawEMGProfile)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageFilter)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageSaveChannelAsSVG)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageSaveMontageAsSVG)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageExportSignal)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageUseThisScale)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageDetectArtifacts)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageClearArtifacts)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageHide)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem, 		iSFPageHidden)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem, 		iSFPageSpaceEvenly)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem, 		iSFPageLocateSelection)) ||

	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageAnnotationSeparator)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageAnnotationDelete)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageAnnotationEdit)) ||

	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageSelectionMarkArtifact)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageSelectionClearArtifact)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageSelectionFindPattern)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageSelectionAnnotate)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageSelectionDrawCourse)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageSelectionDrawEnvelope)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageSelectionDrawDzxdf)) ||

	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPowerExportRange)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPowerExportAll)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPowerSmooth)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPowerDrawBands)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPowerUseThisScale)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPowerAutoscale)) ||

	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFScoreAssist)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFScoreImport)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFScoreExport)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFScoreClear)) )
		throw runtime_error ("Failed to contruct SF widgets");

	gtk_menu_item_set_submenu( iSFPageHidden, (GtkWidget*)mSFPageHidden);

	g_signal_connect( iSFPageShowOriginal, "toggled",
			  (GCallback)iSFPageShowOriginal_toggled_cb,
			  this);
	g_signal_connect( iSFPageShowProcessed, "toggled",
			  (GCallback)iSFPageShowProcessed_toggled_cb,
			  this);
	g_signal_connect( iSFPageUseResample, "toggled",
			  (GCallback)iSFPageUseResample_toggled_cb,
			  this);
	g_signal_connect( iSFPageDrawZeroline, "toggled",
			  (GCallback)iSFPageDrawZeroline_toggled_cb,
			  this);

	g_signal_connect( iSFPageAnnotationDelete, "activate",
			  (GCallback)iSFPageAnnotationDelete_activate_cb,
			  this);
	g_signal_connect( iSFPageAnnotationEdit, "activate",
			  (GCallback)iSFPageAnnotationEdit_activate_cb,
			  this);

	g_signal_connect( iSFPageSelectionMarkArtifact, "activate",
			  (GCallback)iSFPageSelectionMarkArtifact_activate_cb,
			  this);
	g_signal_connect( iSFPageSelectionClearArtifact, "activate",
			  (GCallback)iSFPageSelectionClearArtifact_activate_cb,
			  this);
	g_signal_connect( iSFPageSelectionFindPattern, "activate",
			  (GCallback)iSFPageSelectionFindPattern_activate_cb,
			  this);
	g_signal_connect( iSFPageSelectionAnnotate, "activate",
			  (GCallback)iSFPageSelectionAnnotate_activate_cb,
			  this);

	g_signal_connect( iSFPageSelectionDrawCourse, "toggled",
			  (GCallback)iSFPageSelectionDrawCourse_toggled_cb,
			  this);
	g_signal_connect( iSFPageSelectionDrawEnvelope, "toggled",
			  (GCallback)iSFPageSelectionDrawEnvelope_toggled_cb,
			  this);
	g_signal_connect( iSFPageSelectionDrawDzxdf, "toggled",
			  (GCallback)iSFPageSelectionDrawDzxdf_toggled_cb,
			  this);


	g_signal_connect( iSFPageFilter, "activate",
			  (GCallback)iSFPageFilter_activate_cb,
			  this);
	g_signal_connect( iSFPageSaveChannelAsSVG, "activate",
			  (GCallback)iSFPageSaveChannelAsSVG_activate_cb,
			  this);
	g_signal_connect( iSFPageSaveMontageAsSVG, "activate",
			  (GCallback)iSFPageSaveMontageAsSVG_activate_cb,
			  this);
	g_signal_connect( iSFPageExportSignal, "activate",
			  (GCallback)iSFPageExportSignal_activate_cb,
			  this);
	g_signal_connect( iSFPageUseThisScale, "activate",
			  (GCallback)iSFPageUseThisScale_activate_cb,
			  this);
	g_signal_connect( iSFPageDetectArtifacts, "activate",
			  (GCallback)iSFPageDetectArtifacts_activate_cb,
			  this);
	g_signal_connect( iSFPageClearArtifacts, "activate",
			  (GCallback)iSFPageClearArtifacts_activate_cb,
			  this);
	g_signal_connect( iSFPageHide, "activate",
			  (GCallback)iSFPageHide_activate_cb,
			  this);

	g_signal_connect( iSFPageSpaceEvenly, "activate",
			  (GCallback)iSFPageSpaceEvenly_activate_cb,
			  this);
	g_signal_connect( iSFPageLocateSelection, "activate",
			  (GCallback)iSFPageLocateSelection_activate_cb,
			  this);

	g_signal_connect( iSFPageDrawPSDProfile, "toggled",
			  (GCallback)iSFPageDrawPSDProfile_toggled_cb,
			  this);
	g_signal_connect( iSFPageDrawPSDSpectrum, "toggled",
			  (GCallback)iSFPageDrawPSDSpectrum_toggled_cb,
			  this);
	g_signal_connect( iSFPageDrawMCProfile, "toggled",
			  (GCallback)iSFPageDrawMCProfile_toggled_cb,
			  this);
	g_signal_connect( iSFPageDrawEMGProfile, "toggled",
			  (GCallback)iSFPageDrawEMGProfile_toggled_cb,
			  this);

	g_signal_connect( iSFPowerExportRange, "activate",
			  (GCallback)iSFPowerExportRange_activate_cb,
			  this);
	g_signal_connect( iSFPowerExportAll, "activate",
			  (GCallback)iSFPowerExportAll_activate_cb,
			  this);
	g_signal_connect( iSFPowerSmooth, "toggled",
			  (GCallback)iSFPowerSmooth_toggled_cb,
			  this);
	g_signal_connect( iSFPowerDrawBands, "toggled",
			  (GCallback)iSFPowerDrawBands_toggled_cb,
			  this);
	g_signal_connect( iSFPowerUseThisScale, "activate",
			  (GCallback)iSFPowerUseThisScale_activate_cb,
			  this);
	g_signal_connect( iSFPowerAutoscale, "toggled",
			  (GCallback)iSFPowerAutoscale_toggled_cb,
			  this);

	g_signal_connect( iSFScoreAssist, "activate",
			  (GCallback)iSFScoreAssist_activate_cb,
			  this);
	g_signal_connect( iSFScoreExport, "activate",
			  (GCallback)iSFScoreExport_activate_cb,
			  this);
	g_signal_connect( iSFScoreImport, "activate",
			  (GCallback)iSFScoreImport_activate_cb,
			  this);
	g_signal_connect( iSFScoreClear, "activate",
			  (GCallback)iSFScoreClear_activate_cb,
			  this);
      // petty dialogs
	// annotations
	if ( !(AGH_GBGETOBJ (GtkDialog,			wAnnotationLabel)) ||
	     !(AGH_GBGETOBJ (GtkEntry,			eAnnotationLabel)) ||
	     !(AGH_GBGETOBJ (GtkDialog,			wAnnotationSelector)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,		eAnnotationSelectorWhich)) )
		throw runtime_error ("Failed to construct widgets");

	mAnnotationsAtCursor = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_combo_box_set_model( eAnnotationSelectorWhich,
				 (GtkTreeModel*)mAnnotationsAtCursor);
	gtk_combo_box_set_id_column( eAnnotationSelectorWhich, 0);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eAnnotationSelectorWhich, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eAnnotationSelectorWhich, renderer,
					"text", 0,
					NULL);
	// artifact detection
	if ( !(AGH_GBGETOBJ (GtkDialog,			wSFArtifactDetection)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,		eSFADProfiles)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADProfileSave)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADProfileDelete)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADScope)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADUpperThr)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADLowerThr)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADF0)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADFc)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADBandwidth)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADMCGain)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADBackpolate)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADEValue)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADHistRangeMin)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADHistRangeMax)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADHistBins)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADSmoothSide)) ||
	     !(AGH_GBGETOBJ (GtkCheckButton,		eSFADSingleChannelPreview)) ||
	     !(AGH_GBGETOBJ (GtkCheckButton,		eSFADEstimateE)) ||
	     !(AGH_GBGETOBJ (GtkRadioButton,		eSFADUseThisRange)) ||
	     !(AGH_GBGETOBJ (GtkRadioButton,		eSFADUseComputedRange)) ||
	     !(AGH_GBGETOBJ (GtkTable,			cSFADWhenEstimateEOn)) ||
	     !(AGH_GBGETOBJ (GtkTable,			cSFADWhenEstimateEOff)) ||
	     !(AGH_GBGETOBJ (GtkLabel,			lSFADInfo)) ||
	     !(AGH_GBGETOBJ (GtkToggleButton,		bSFADPreview)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADApply)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADCancel)) )
		throw runtime_error ("Failed to contruct SF widgets");

	mSFADProfiles = gtk_list_store_new( 1, G_TYPE_STRING);
	// this GtkListStore is populated from the same source, but something
	// haunting GTK+ forbids reuse of _p.mGlobalArtifactDetectionProfiles
	gtk_combo_box_set_model( eSFADProfiles,
				 (GtkTreeModel*)mSFADProfiles);
	gtk_combo_box_set_id_column( eSFADProfiles, 0);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFADProfiles, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFADProfiles, renderer,
					"text", 0,
					NULL);

	g_signal_connect( eSFADProfiles, "changed",
			  (GCallback)eSFADProfiles_changed_cb,
			  this);
	g_signal_connect( bSFADProfileSave, "clicked",
			  (GCallback)bSFADProfileSave_clicked_cb,
			  this);
	g_signal_connect( bSFADProfileDelete, "clicked",
			  (GCallback)bSFADProfileDelete_clicked_cb,
			  this);
	g_signal_connect( eSFADEstimateE, "toggled",
			  (GCallback)eSFADEstimateE_toggled_cb,
			  this);
	g_signal_connect( eSFADUseThisRange, "toggled",
			  (GCallback)eSFADUseThisRange_toggled_cb,
			  this);
	g_signal_connect( bSFADPreview, "toggled",
			  (GCallback)bSFADPreview_toggled_cb,
			  this);
	g_signal_connect( bSFADApply, "clicked",
			  (GCallback)bSFADApply_clicked_cb,
			  this);
	g_signal_connect( bSFADCancel, "clicked",
			  (GCallback)bSFADCancel_clicked_cb,
			  this);



      // find/manage patterns
	mPatterns =
		gtk_list_store_new( 1, G_TYPE_STRING);

	if ( !AGH_GBGETOBJ (GtkDialog,		wPattern) ||
	     !AGH_GBGETOBJ (GtkDrawingArea,	daPatternSelection) ||
	     !AGH_GBGETOBJ (GtkScrolledWindow,	vpPatternSelection) ||
	     !AGH_GBGETOBJ (GtkButton,		bPatternFindPrevious) ||
	     !AGH_GBGETOBJ (GtkButton,		bPatternFindNext) ||
//	     !AGH_GBGETOBJ (GtkButton,		bPatternDismiss) ||
	     !AGH_GBGETOBJ (GtkButton,		bPatternSave) ||
	     !AGH_GBGETOBJ (GtkButton,		bPatternDiscard) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternEnvTightness) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternFilterOrder) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternFilterCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternDZCDFStep) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternDZCDFSigma) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternDZCDFSmooth) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternParameterA) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternParameterB) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternParameterC) ||
	     !AGH_GBGETOBJ (GtkHBox,		cPatternLabelBox) ||
	     !AGH_GBGETOBJ (GtkLabel,		lPatternSimilarity) ||
	     !AGH_GBGETOBJ (GtkComboBox,	ePatternList) ||
	     !AGH_GBGETOBJ (GtkComboBox,	ePatternChannel) ||
	     !AGH_GBGETOBJ (GtkDialog,		wPatternName) ||
	     !AGH_GBGETOBJ (GtkEntry,		ePatternNameName) ||
	     !AGH_GBGETOBJ (GtkCheckButton,	ePatternNameSaveGlobally) )
		throw runtime_error ("Failed to contruct SF widgets");

	gtk_combo_box_set_model( ePatternList,
				 (GtkTreeModel*)mPatterns);
	gtk_combo_box_set_id_column( ePatternList, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)ePatternList, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)ePatternList, renderer,
					"text", 0,
					NULL);
	ePatternList_changed_cb_handler_id =
		g_signal_connect( ePatternList, "changed",
				  G_CALLBACK (ePatternList_changed_cb),
				  this);

	gtk_combo_box_set_model( ePatternChannel,
				 (GtkTreeModel*)_p.mAllChannels);
	gtk_combo_box_set_id_column( ePatternChannel, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)ePatternChannel, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)ePatternChannel, renderer,
					"text", 0,
					NULL);

	ePatternChannel_changed_cb_handler_id =
		g_signal_connect( ePatternChannel, "changed",
				  G_CALLBACK (ePatternChannel_changed_cb),
				  this);

	g_signal_connect( daPatternSelection, "draw",
			  G_CALLBACK (daPatternSelection_draw_cb),
			  this);
	g_signal_connect( daPatternSelection, "scroll-event",
			  G_CALLBACK (daPatternSelection_scroll_event_cb),
			  this);
	g_signal_connect( bPatternFindNext, "clicked",
			  G_CALLBACK (bPatternFind_clicked_cb),
			  this);
	g_signal_connect( bPatternFindPrevious, "clicked",
			  G_CALLBACK (bPatternFind_clicked_cb),
			  this);
	g_signal_connect( bPatternSave, "clicked",
			  G_CALLBACK (bPatternSave_clicked_cb),
			  this);
	g_signal_connect( bPatternDiscard, "clicked",
			  G_CALLBACK (bPatternDiscard_clicked_cb),
			  this);

	g_signal_connect( ePatternEnvTightness, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternFilterCutoff, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternFilterOrder, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternDZCDFStep, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternDZCDFSigma, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternDZCDFSmooth, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternParameterA, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternParameterB, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternParameterC, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);

	g_signal_connect( wPattern, "show",
			  G_CALLBACK (wPattern_show_cb),
			  this);
	g_signal_connect( wPattern, "hide",
			  G_CALLBACK (wPattern_hide_cb),
			  this);


	// aghui::SScoringFacility::SFiltersDialog::

      // ------- wFilter
	if ( !(AGH_GBGETOBJ (GtkDialog,		wFilters)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lFilterCaption)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eFilterLowPassCutoff)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eFilterHighPassCutoff)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eFilterLowPassOrder)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eFilterHighPassOrder)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eFilterNotchFilter)) ||
	     !(AGH_GBGETOBJ (GtkListStore,	mFilterNotchFilter)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bFilterOK)) )
		throw runtime_error ("Failed to contruct SF widgets");

	gtk_combo_box_set_model( eFilterNotchFilter,
				 (GtkTreeModel*)mFilterNotchFilter);
	gtk_combo_box_set_id_column( eFilterNotchFilter, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eFilterNotchFilter, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eFilterNotchFilter, renderer,
					"text", 0,
					NULL);

	g_signal_connect( (GObject*)eFilterHighPassCutoff, "value-changed",
			  (GCallback)eFilterHighPassCutoff_value_changed_cb,
			  this);
	g_signal_connect( (GObject*)eFilterLowPassCutoff, "value-changed",
			  (GCallback)eFilterLowPassCutoff_value_changed_cb,
			  this);

      // ------- wPhaseDiff
	if ( !(AGH_GBGETOBJ (GtkDialog,		wSFPD)) ||
	     !(AGH_GBGETOBJ (GtkDrawingArea,	daSFPD)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eSFPDChannelA)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eSFPDChannelB)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFPDFreqFrom)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFPDBandwidth)) ||
	     !(AGH_GBGETOBJ (GtkScaleButton,	eSFPDSmooth)) )
		throw runtime_error ("Failed to contruct SF widgets");

	gtk_combo_box_set_model( eSFPDChannelA,
				 (GtkTreeModel*)_p.mEEGChannels);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFPDChannelA, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFPDChannelA, renderer,
					"text", 0,
					NULL);
	eSFPDChannelA_changed_cb_handler_id =
		g_signal_connect( eSFPDChannelA, "changed",
				  G_CALLBACK (eSFPDChannelA_changed_cb),
				  this);

	gtk_combo_box_set_model( eSFPDChannelB,
				 (GtkTreeModel*)_p.mEEGChannels);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFPDChannelB, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFPDChannelB, renderer,
					"text", 0,
					NULL);
	eSFPDChannelB_changed_cb_handler_id =
		g_signal_connect( eSFPDChannelB, "changed",
				  G_CALLBACK (eSFPDChannelB_changed_cb),
				  this);

	g_signal_connect( daSFPD, "draw",
			  G_CALLBACK (daSFPD_draw_cb),
			  this);
	g_signal_connect( daSFPD, "scroll-event",
			  G_CALLBACK (daSFPD_scroll_event_cb),
			  this);
	g_signal_connect( eSFPDChannelA, "changed",
			  G_CALLBACK (eSFPDChannelA_changed_cb),
			  this);
	g_signal_connect( eSFPDChannelB, "changed",
			  G_CALLBACK (eSFPDChannelB_changed_cb),
			  this);
	g_signal_connect( eSFPDFreqFrom, "value-changed",
			  G_CALLBACK (eSFPDFreqFrom_value_changed_cb),
			  this);
	g_signal_connect( eSFPDBandwidth, "value-changed",
			  G_CALLBACK (eSFPDBandwidth_value_changed_cb),
			  this);
	g_signal_connect( eSFPDSmooth, "value-changed",
			  G_CALLBACK (eSFPDSmooth_value_changed_cb),
			  this);
	g_signal_connect( wSFPD, "show",
			  G_CALLBACK (wSFPD_show_cb),
			  this);
	g_signal_connect( wSFPD, "hide",
			  G_CALLBACK (wSFPD_hide_cb),
			  this);
}




aghui::SScoringFacilityWidgets::
~SScoringFacilityWidgets ()
{
	// destroy toplevels
	gtk_widget_destroy( (GtkWidget*)wScoringFacility);
	g_object_unref( (GObject*)builder);
}

// eof
