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
#include "scoring-facility.hh"
#include "scoring-facility_cb.hh"

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
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		eSFCurrentPos)) ||

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

	g_signal_connect( eSFCurrentPos, "clicked",
			  (GCallback)eSFCurrentPos_clicked_cb,
			  this);

	sbSFContextIdGeneral = gtk_statusbar_get_context_id( sbSF, "General context");

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
	     !(AGH_GBGETOBJ3 (builder, GtkSeparatorMenuItem,	iSFPageProfileItemsSeparator)) ||
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
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem, 		iSFPageLocateSelection)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageAnnotationSeparator)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageAnnotationDelete)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageAnnotationEdit)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionMarkArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionClearArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionFindPattern)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionAnnotate)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageSelectionDrawCourse)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageSelectionDrawEnvelope)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageSelectionDrawDzxdf)) ||

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
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFADInfo)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFADPreview)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFADApply)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFADCancel)) )
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
	// g_signal_connect( wScoringFacility, "configure-event",
	// 		  (GCallback)wScoringFacility_configure_event_cb,
	// 		  this);

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
	g_signal_connect( bSFADPreview, "toggled",
			  (GCallback)bSFADPreview_toggled_cb,
			  this);
	g_signal_connect( bSFADApply, "clicked",
			  (GCallback)bSFADApply_clicked_cb,
			  this);
	g_signal_connect( bSFADCancel, "clicked",
			  (GCallback)bSFADCancel_clicked_cb,
			  this);
	return 0;
}




int
aghui::SScoringFacility::SFindDialog::
construct_widgets()
{
	mPatterns =
		gtk_list_store_new( 1, G_TYPE_STRING);

	GtkCellRenderer *renderer;

	if ( !AGH_GBGETOBJ3 (_p.builder, GtkDialog,		wPattern) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkDrawingArea,	daPatternSelection) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkScrolledWindow,	vpPatternSelection) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternFindPrevious) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternFindNext) ||
//	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternDismiss) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternSave) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternDiscard) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternEnvTightness) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternFilterOrder) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternFilterCutoff) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternDZCDFStep) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternDZCDFSigma) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternDZCDFSmooth) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternParameterA) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternParameterB) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternParameterC) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkHBox,		cPatternLabelBox) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkLabel,		lPatternSimilarity) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkComboBox,		ePatternList) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkComboBox,		ePatternChannel) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkDialog,		wPatternName) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkEntry,		ePatternNameName) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkCheckButton,	ePatternNameSaveGlobally) )
		return -1;

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
				 (GtkTreeModel*)_p._p.mAllChannels);
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
	return 0;
}



int
aghui::SScoringFacility::SFiltersDialog::
construct_widgets()
{
	GtkCellRenderer *renderer;

      // ------- wFilter
	if ( !(AGH_GBGETOBJ3 (_p.builder, GtkDialog,		wFilters)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkLabel,		lFilterCaption)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,	eFilterLowPassCutoff)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,	eFilterHighPassCutoff)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,	eFilterLowPassOrder)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,	eFilterHighPassOrder)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkComboBox,		eFilterNotchFilter)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkListStore,		mFilterNotchFilter)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkButton,		bFilterOK)) )
		return -1;

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
	return 0;
}





int
aghui::SScoringFacility::SPhasediffDialog::
construct_widgets()
{
	GtkCellRenderer *renderer;

      // ------- wPhaseDiff
	if ( !(AGH_GBGETOBJ3 (_p.builder, GtkDialog, wSFPD)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkDrawingArea, daSFPD)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkComboBox, eSFPDChannelA)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkComboBox, eSFPDChannelB)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton, eSFPDFreqFrom)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton, eSFPDBandwidth)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkScaleButton, eSFPDSmooth)) )
		return -1;

	gtk_combo_box_set_model( eSFPDChannelA,
				 (GtkTreeModel*)_p._p.mEEGChannels);
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
				 (GtkTreeModel*)_p._p.mEEGChannels);
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
	return 0;
}






// eof
