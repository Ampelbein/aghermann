/*
 *       File name:  aghermann/ui/sf/construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-13
 *
 *         Purpose:  scoring facility widget construct megafun
 *
 *         License:  GPL
 */

#include <stdexcept>

#include "aghermann/ui/mw/mw.hh"
#include "aghermann/ui/ui.hh"
#include "sf.hh"
#include "sf_cb.hh"

using namespace std;


aghui::SScoringFacilityWidgets::
SScoringFacilityWidgets ()
{
	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_resource( builder, "/org/gtk/aghermann/sf.glade", NULL) )
		throw runtime_error( "Failed to load SF glade resource");

	gtk_builder_connect_signals( builder, NULL);
	//  we do it all mostly ourself, except for some delete-event binding to gtk_true()

	// general & montage page navigation
	if ( !AGH_GBGETOBJ (GtkWindow,		wSF) ||
	     !AGH_GBGETOBJ (GtkLabel,		lSFHint) ||
	     !AGH_GBGETOBJ (GtkListStore,	mSFScoringPageSize) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eSFPageSize) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFCurrentPage) ||
	     !AGH_GBGETOBJ (GtkAdjustment,	jSFPageNo) ||
	     !AGH_GBGETOBJ (GtkLabel,		lSFTotalPages) ||
	     !AGH_GBGETOBJ (GtkButton,		eSFCurrentPos) ||
	     !AGH_GBGETOBJ (GtkExpander,	cSFHypnogram) ||
	     !AGH_GBGETOBJ (GtkHBox,		cSFControlBar) ||
	     !AGH_GBGETOBJ (GtkBox,		cSFScoringModeContainer) ||
	     !AGH_GBGETOBJ (GtkBox,		cSFICAModeContainer) )
		throw runtime_error ("Failed to construct SF widgets (0)");

	if ( !AGH_GBGETOBJ (GtkButton,		bSFBack) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFForward) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFGotoPrevUnscored) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFGotoNextUnscored) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFGotoPrevArtifact) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFGotoNextArtifact) ||

	     !AGH_GBGETOBJ (GtkToggleButton,	bSFShowFindDialog) ||
	     !AGH_GBGETOBJ (GtkToggleButton,	bSFShowPhaseDiffDialog) ||
	     !AGH_GBGETOBJ (GtkToggleButton,	bSFDrawCrosshair) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFRunICA) )
		throw runtime_error ("Failed to construct SF widgets (1)");

	G_CONNECT_2 (wSF, delete, event);
	G_CONNECT_3 (wSF, key, press, event);

	gtk_combo_box_set_model_properly(
		eSFPageSize, mSFScoringPageSize);

	G_CONNECT_1 (eSFPageSize, changed);
	G_CONNECT_2 (eSFCurrentPage, value, changed);

	G_CONNECT_1 (eSFCurrentPos, clicked);

	G_CONNECT_1 (bSFForward, clicked);
	G_CONNECT_1 (bSFBack, clicked);

	G_CONNECT_1 (bSFGotoNextUnscored, clicked);
	G_CONNECT_1 (bSFGotoPrevUnscored, clicked);
	G_CONNECT_1 (bSFGotoNextArtifact, clicked);
	G_CONNECT_1 (bSFGotoPrevArtifact, clicked);

	G_CONNECT_1 (bSFDrawCrosshair, toggled);
	G_CONNECT_1 (bSFShowFindDialog, toggled);
	G_CONNECT_1 (bSFShowPhaseDiffDialog, toggled);
	G_CONNECT_1 (bSFRunICA, clicked);

	if ( !(AGH_GBGETOBJ (GtkButton,		bSFScoreClear)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFScoreNREM1)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFScoreNREM2)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFScoreNREM3)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFScoreNREM4)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFScoreREM))   ||
	     !(AGH_GBGETOBJ (GtkButton,		bSFScoreWake))  ||
	     !(AGH_GBGETOBJ (GtkTable,		cSFSleepStageStats)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lSFPercentScored)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lScoreStatsNREMPercent)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lScoreStatsREMPercent)) ||
	     !(AGH_GBGETOBJ (GtkLabel,		lScoreStatsWakePercent)) )
		throw runtime_error ("Failed to construct SF widgets (2)");

	G_CONNECT_1 (bSFScoreClear, clicked);
	G_CONNECT_1 (bSFScoreNREM1, clicked);
	G_CONNECT_1 (bSFScoreNREM2, clicked);
	G_CONNECT_1 (bSFScoreNREM3, clicked);
	G_CONNECT_1 (bSFScoreNREM4, clicked);
	G_CONNECT_1 (bSFScoreREM, clicked);
	G_CONNECT_1 (bSFScoreWake, clicked);

	if ( !(AGH_GBGETOBJ (GtkDrawingArea,	daSFMontage)) ||
	     !(AGH_GBGETOBJ (GtkDrawingArea,	daSFHypnogram)) ||
	     !(AGH_GBGETOBJ (GtkMenuToolButton,	bSFAccept)) ||
	     !(AGH_GBGETOBJ (GtkMenu,		iiSFAccept)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSFAcceptAndTakeNext)) ||
	     !(AGH_GBGETOBJ (GtkStatusbar,	sbSF)) )
		throw runtime_error ("Failed to construct SF widgets (3)");

	sbSFContextIdGeneral = gtk_statusbar_get_context_id( sbSF, "General context");

	G_CONNECT_1 (daSFMontage, draw);
	G_CONNECT_2 (daSFMontage, configure, event);
	G_CONNECT_3 (daSFMontage, button, press, event);
	G_CONNECT_3 (daSFMontage, button, release, event);
	G_CONNECT_2 (daSFMontage, scroll, event);
	G_CONNECT_3 (daSFMontage, motion, notify, event);
	G_CONNECT_3 (daSFMontage, leave, notify, event);

	G_CONNECT_1 (daSFHypnogram, draw);
	G_CONNECT_3 (daSFHypnogram, button, press, event);
	G_CONNECT_3 (daSFHypnogram, button, release, event);
	G_CONNECT_3 (daSFHypnogram, motion, notify, event);

	gtk_menu_tool_button_set_menu( bSFAccept, (GtkWidget*)iiSFAccept);

	G_CONNECT_1 (bSFAccept, clicked);
	G_CONNECT_1 (iSFAcceptAndTakeNext, activate);

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
		throw runtime_error ("Failed to construct SF widgets (4)");

	gtk_combo_box_set_model_properly( eSFICANonlinearity, mSFICANonlinearity);
	gtk_combo_box_set_model_properly( eSFICAApproach, mSFICAApproach);
	gtk_combo_box_set_model_properly( eSFICARemixMode, mSFICARemixMode);

	auto tabarray = pango_tab_array_new( 20, FALSE);  // 20 channels is good enough
	for ( int t = 1; t < 20; ++t )
		pango_tab_array_set_tab( tabarray, t-1, PANGO_TAB_LEFT, t * 12);
	g_object_set( tSFICAMatrix,
		      "tabs", tabarray,
		      NULL);

	G_CONNECT_1 (eSFICARemixMode, changed);
	G_CONNECT_1 (eSFICANonlinearity, changed);
	G_CONNECT_1 (eSFICAApproach, changed);
	G_CONNECT_1 (eSFICAFineTune, toggled);
	G_CONNECT_1 (eSFICAStabilizationMode, toggled);
	G_CONNECT_2 (eSFICAa1, value, changed);
	G_CONNECT_2 (eSFICAa2, value, changed);
	G_CONNECT_2 (eSFICAmu, value, changed);
	G_CONNECT_2 (eSFICAepsilon, value, changed);
	G_CONNECT_2 (eSFICANofICs, value, changed);
	G_CONNECT_2 (eSFICAEigVecFirst, value, changed);
	G_CONNECT_2 (eSFICAEigVecLast, value, changed);
	G_CONNECT_2 (eSFICASampleSizePercent, value, changed);
	G_CONNECT_2 (eSFICAMaxIterations, value, changed);

	G_CONNECT_1 (bSFICATry, clicked);
	G_CONNECT_1 (bSFICAPreview, toggled);
	G_CONNECT_1 (bSFICAShowMatrix, toggled);
	G_CONNECT_1 (wSFICAMatrix, hide);

	G_CONNECT_1 (bSFICAApply, clicked);
	G_CONNECT_1 (bSFICACancel, clicked);

	// ------- menus
	if ( !(AGH_GBGETOBJ (GtkLabel, 		lSFOverChannel)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		iiSFPage)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		iiSFICAPage)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		iiSFPageSelection)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		iiSFPageAnnotation)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		iiSFPageProfiles)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		iiSFPagePhasicEvents)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		iiSFPageHidden)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		iiSFPower)) ||
	     !(AGH_GBGETOBJ (GtkMenu, 		iiSFScore)) ||

	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageShowOriginal)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageShowProcessed)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageUseResample)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem,		iSFPageDrawZeroline)) ||
	     !(AGH_GBGETOBJ (GtkSeparatorMenuItem,	iSFPageProfilesSubmenuSeparator)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawPSDProfile)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawPSDSpectrum)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawSWUProfile)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawMCProfile)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawEMGProfile)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawPhasicSpindles)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawPhasicKComplexes)) ||
	     !(AGH_GBGETOBJ (GtkCheckMenuItem, 		iSFPageDrawPhasicEyeBlinks)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageFilter)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageSaveChannelAsSVG)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageSaveMontageAsSVG)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageExportSignal)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageUseThisScale)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageArtifactsDetect)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageArtifactsClear)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageArtifactsMarkFlat)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageHide)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem, 		iSFPageHidden)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem, 		iSFPageSpaceEvenly)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem, 		iSFPageLocateSelection)) ||

	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageAnnotationSeparator)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageAnnotationDelete)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageAnnotationEdit)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageAnnotationClearAll)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageAnnotationGotoNext)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,		iSFPageAnnotationGotoPrev)) ||

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
		throw runtime_error ("Failed to construct SF widgets (5)");

	gtk_menu_item_set_submenu( iSFPageHidden, (GtkWidget*)iiSFPageHidden);

	G_CONNECT_1 (iSFPageShowOriginal, toggled);
	G_CONNECT_1 (iSFPageShowProcessed, toggled);
	G_CONNECT_1 (iSFPageUseResample, toggled);
	G_CONNECT_1 (iSFPageDrawZeroline, toggled);

	G_CONNECT_1 (iSFPageAnnotationDelete, activate);
	G_CONNECT_1 (iSFPageAnnotationEdit, activate);
	G_CONNECT_1 (iSFPageAnnotationClearAll, activate);
	G_CONNECT_1 (iSFPageAnnotationGotoPrev, activate);
	G_CONNECT_1 (iSFPageAnnotationGotoNext, activate);

	G_CONNECT_1 (iSFPageSelectionMarkArtifact, activate);
	G_CONNECT_1 (iSFPageSelectionClearArtifact, activate);
	G_CONNECT_1 (iSFPageSelectionFindPattern, activate);
	G_CONNECT_1 (iSFPageSelectionAnnotate, activate);
	G_CONNECT_1 (iSFPageSelectionDrawCourse, toggled);
	G_CONNECT_1 (iSFPageSelectionDrawEnvelope, toggled);
	G_CONNECT_1 (iSFPageSelectionDrawDzxdf, toggled);

	G_CONNECT_1 (iSFPageFilter, activate);
	G_CONNECT_1 (iSFPageSaveChannelAsSVG, activate);
	G_CONNECT_1 (iSFPageSaveMontageAsSVG, activate);
	G_CONNECT_1 (iSFPageExportSignal, activate);
	G_CONNECT_1 (iSFPageUseThisScale, activate);
	G_CONNECT_1 (iSFPageArtifactsDetect, activate);
	G_CONNECT_1 (iSFPageArtifactsMarkFlat, activate);
	G_CONNECT_1 (iSFPageArtifactsClear, activate);
	G_CONNECT_1 (iSFPageHide, activate);

	G_CONNECT_1 (iSFPageSpaceEvenly, activate);
	G_CONNECT_1 (iSFPageLocateSelection, activate);

	G_CONNECT_1 (iSFPageDrawPSDProfile, toggled);
	G_CONNECT_1 (iSFPageDrawPSDSpectrum, toggled);
	G_CONNECT_1 (iSFPageDrawMCProfile, toggled);
	G_CONNECT_1 (iSFPageDrawSWUProfile, toggled);
	G_CONNECT_1 (iSFPageDrawEMGProfile, toggled);

	G_CONNECT_1 (iSFPageDrawPhasicSpindles, toggled);
	G_CONNECT_1 (iSFPageDrawPhasicKComplexes, toggled);
	G_CONNECT_1 (iSFPageDrawPhasicEyeBlinks, toggled);

	G_CONNECT_1 (iSFPowerExportRange, activate);
	G_CONNECT_1 (iSFPowerExportAll, activate);
	G_CONNECT_1 (iSFPowerSmooth, toggled);
	G_CONNECT_1 (iSFPowerDrawBands, toggled);
	G_CONNECT_1 (iSFPowerUseThisScale, activate);
	G_CONNECT_1 (iSFPowerAutoscale, toggled);

	G_CONNECT_1 (iSFScoreAssist, activate);
	G_CONNECT_1 (iSFScoreExport, activate);
	G_CONNECT_1 (iSFScoreImport, activate);
	G_CONNECT_1 (iSFScoreClear, activate);

      // petty dialogs
	// annotations
	if ( !AGH_GBGETOBJ (GtkDialog,			wSFAnnotationLabel) ||
	     !AGH_GBGETOBJ (GtkEntry,			eSFAnnotationLabel) ||
	     !AGH_GBGETOBJ (GtkRadioButton,		eSFAnnotationTypePlain) ||
	     !AGH_GBGETOBJ (GtkRadioButton,		eSFAnnotationTypeSpindle) ||
	     !AGH_GBGETOBJ (GtkRadioButton,		eSFAnnotationTypeKComplex) ||
	     !AGH_GBGETOBJ (GtkRadioButton,		eSFAnnotationTypeBlink) ||
	     !AGH_GBGETOBJ (GtkDialog,			wSFAnnotationSelector) ||
	     !AGH_GBGETOBJ (GtkComboBox,		eSFAnnotationSelectorWhich) )
		throw runtime_error ("Failed to construct SF widgets (6)");

	mSFAnnotationsAtCursor = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_combo_box_set_model_properly( eSFAnnotationSelectorWhich, mSFAnnotationsAtCursor);
}




aghui::SScoringFacilityWidgets::
~SScoringFacilityWidgets ()
{
	// destroy toplevels
	gtk_widget_destroy( (GtkWidget*)wSF);
	g_object_unref( (GObject*)builder);
}

// Local Variables:
// indent-tabs-mode: 8
// tab-width: 8
// End:
