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


#include "scoring-facility.hh"

using namespace std;


int
aghui::SScoringFacility::
construct_widgets()
{
	GtkCellRenderer *renderer;

	if ( !(AGH_GBGETOBJ3 (builder, GtkWindow,		wScoringFacility)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFHint)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eSFPageSize)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFCurrentPage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkAdjustment,		jPageNo)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFTotalPages)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFClockTime)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFCurrentPos)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkExpander,		cSFHypnogram)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkHBox,			cSFControlBar)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkBox,			cSFScoringModeContainer)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkBox,			cSFICAModeContainer)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFBack)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFForward)) ||

	     // 1. scoring
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreClear)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreNREM1)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreNREM2)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreNREM3)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreNREM4)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreREM))   ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreWake))  ||
	     !(AGH_GBGETOBJ3 (builder, GtkTable,		cSFSleepStageStats)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFPercentScored)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoreStatsNREMPercent)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoreStatsREMPercent)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoreStatsWakePercent)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreGotoPrevUnscored)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreGotoNextUnscored)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreGotoPrevArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreGotoNextArtifact)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFShowFindDialog)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFShowPhaseDiffDialog)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFDrawCrosshair)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFRunICA)) ||

	     // 2. ICA
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eSFICARemixMode)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eSFICANonlinearity)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eSFICAApproach)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkListStore,		mSFICARemixMode)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkListStore,		mSFICANonlinearity)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkListStore,		mSFICAApproach)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,		eSFICAFineTune)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,		eSFICAStabilizationMode)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAa1)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAa2)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAmu)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAepsilon)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICANofICs)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkAdjustment,		jSFICANofICs)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAEigVecFirst)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAEigVecLast)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkAdjustment,		jSFICAEigVecFirst)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkAdjustment,		jSFICAEigVecLast)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICASampleSizePercent)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAMaxIterations)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFICATry)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFICAPreview)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFICAShowMatrix)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFICAApply)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFICACancel)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDialog,		wSFICAMatrix)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkTextView,		tSFICAMatrix)) ||

	     // rest
	     !(AGH_GBGETOBJ3 (builder, GtkDrawingArea,		daSFMontage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDrawingArea,		daSFHypnogram)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuToolButton,	bSFAccept)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu,			mSFAccept)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkStatusbar,		sbSF)) )
		return -1;

	gtk_combo_box_set_model( eSFPageSize, // reuse the one previously constructed in SExpDesignUI
				 (GtkTreeModel*)_p.mScoringPageSize);
	gtk_combo_box_set_id_column( eSFPageSize, 0);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFPageSize, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFPageSize, renderer,
					"text", 0,
					NULL);

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

	// ------- menus
	if ( !(AGH_GBGETOBJ3 (builder, GtkLabel, 		lSFOverChannel)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFICAPage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPageSelection)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPageAnnotation)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPageHidden)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPower)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFScore)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageShowOriginal)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageShowProcessed)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageUseResample)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageDrawZeroline)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem, 	iSFPageDrawPSDProfile)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem, 	iSFPageDrawPSDSpectrum)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem, 	iSFPageDrawMCProfile)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem, 	iSFPageDrawEMGProfile)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageFilter)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSaveChannelAsSVG)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSaveMontageAsSVG)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageExportSignal)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageUseThisScale)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageDetectArtifacts)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageClearArtifacts)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageHide)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem, 		iSFPageHidden)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem, 		iSFPageSpaceEvenly)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageAnnotationSeparator)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageAnnotationDelete)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageAnnotationEdit)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionMarkArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionClearArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionFindPattern)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionAnnotate)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerExportRange)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerExportAll)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPowerSmooth)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPowerDrawBands)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerUseThisScale)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPowerAutoscale)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreAssist)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreImport)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreExport)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreClear)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFAcceptAndTakeNext)) )
		return -1;

	gtk_menu_tool_button_set_menu( bSFAccept, (GtkWidget*)mSFAccept);

	gtk_menu_item_set_submenu( iSFPageHidden, (GtkWidget*)mSFPageHidden);

	// petty dialogs
	if ( !(AGH_GBGETOBJ3 (builder, GtkDialog,		wAnnotationLabel)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkEntry,		eAnnotationLabel)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDialog,		wAnnotationSelector)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eAnnotationSelectorWhich)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkDialog,		wSFArtifactDetectionSetup)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADScope)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADUpperThr)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADLowerThr)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADF0)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADFc)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADBandwidth)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADMCGain)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADBackpolate)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADEValue)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADHistRangeMin)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADHistRangeMax)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADHistBins)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFADSmoothSide)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,		eSFADClearOldArtifacts)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,		eSFADEstimateE)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkRadioButton,		eSFADUseThisRange)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkRadioButton,		eSFADUseComputedRange)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkTable,		cSFADWhenEstimateEOn)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkTable,		cSFADWhenEstimateEOff)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFADInfo)) )
		return -1;

	mAnnotationsAtCursor = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_combo_box_set_model( eAnnotationSelectorWhich,
				 (GtkTreeModel*)mAnnotationsAtCursor);
	gtk_combo_box_set_id_column( eAnnotationSelectorWhich, 0);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eAnnotationSelectorWhich, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eAnnotationSelectorWhich, renderer,
					"text", 0,
					NULL);


      // orient control widget callbacks
	g_signal_connect( eSFPageSize, "changed",
			  (GCallback)eSFPageSize_changed_cb,
			  this);
	g_signal_connect( eSFCurrentPage, "value-changed",
			  (GCallback)eSFCurrentPage_value_changed_cb,
			  this);

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

	g_signal_connect( bSFRunICA, "clicked",
			  (GCallback)bSFRunICA_clicked_cb,
			  this);
	// g_signal_connect( bSFResetMontage, "clicked",
	// 		  (GCallback)bSFResetMontage_clicked_cb,
	// 		  this);


	g_signal_connect( bSFDrawCrosshair, "toggled",
			  (GCallback)bSFDrawCrosshair_toggled_cb,
			  this);

	g_signal_connect( bSFShowFindDialog, "toggled",
			  (GCallback)bSFShowFindDialog_toggled_cb,
			  this);
	g_signal_connect( bSFShowPhaseDiffDialog, "toggled",
			  (GCallback)bSFShowPhaseDiffDialog_toggled_cb,
			  this);

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


	g_signal_connect( bSFAccept, "clicked",
			  (GCallback)bSFAccept_clicked_cb,
			  this);
	g_signal_connect( iSFAcceptAndTakeNext, "activate",
			  (GCallback)iSFAcceptAndTakeNext_activate_cb,
			  this);

	g_signal_connect( wScoringFacility, "delete-event",
			  (GCallback)wScoringFacility_delete_event_cb,
			  this);

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

	g_signal_connect( eSFADEstimateE, "toggled",
			  (GCallback)eSFADEstimateE_toggled_cb,
			  this);
	g_signal_connect( eSFADUseThisRange, "toggled",
			  (GCallback)eSFADUseThisRange_toggled_cb,
			  this);
	return 0;
}

