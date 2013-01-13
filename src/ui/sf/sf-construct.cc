// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-13
 *
 *         Purpose:  scoring facility widget construct megafun
 *
 *         License:  GPL
 */

#include <stdexcept>

#include "ui/mw/mw.hh"
#include "ui/ui.hh"
#include "sf-widgets.hh"
#include "sf_cb.hh"

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
		throw runtime_error ("Failed to construct SF widgets");

	G_CONNECT_2 (wScoringFacility, delete, event);

	gtk_combo_box_set_model_properly(
		eSFPageSize, mScoringPageSize);

	G_CONNECT_1 (eSFPageSize, changed);
	G_CONNECT_2 (eSFCurrentPage, value, changed);

	G_CONNECT_1 (eSFCurrentPos, clicked);

	G_CONNECT_1 (bSFForward, clicked);
	G_CONNECT_1 (bSFBack, clicked);

	G_CONNECT_1 (bScoreGotoNextUnscored, clicked);
	G_CONNECT_1 (bScoreGotoPrevUnscored, clicked);

	G_CONNECT_1 (bScoreGotoNextArtifact, clicked);
	G_CONNECT_1 (bScoreGotoPrevArtifact, clicked);

	G_CONNECT_1 (bSFDrawCrosshair, toggled);
	G_CONNECT_1 (bSFShowFindDialog, toggled);
	G_CONNECT_1 (bSFShowPhaseDiffDialog, toggled);
	G_CONNECT_1 (bSFRunICA, clicked);

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
		throw runtime_error ("Failed to construct SF widgets");

	G_CONNECT_1 (bScoreClear, clicked);
	G_CONNECT_1 (bScoreNREM1, clicked);
	G_CONNECT_1 (bScoreNREM2, clicked);
	G_CONNECT_1 (bScoreNREM3, clicked);
	G_CONNECT_1 (bScoreNREM4, clicked);
	G_CONNECT_1 (bScoreREM, clicked);
	G_CONNECT_1 (bScoreWake, clicked);

	if ( !(AGH_GBGETOBJ (GtkDrawingArea,	daSFMontage)) ||
	     !(AGH_GBGETOBJ (GtkDrawingArea,	daSFHypnogram)) ||
	     !(AGH_GBGETOBJ (GtkMenuToolButton,	bSFAccept)) ||
	     !(AGH_GBGETOBJ (GtkMenu,		iiSFAccept)) ||
	     !(AGH_GBGETOBJ (GtkMenuItem,	iSFAcceptAndTakeNext)) ||
	     !(AGH_GBGETOBJ (GtkStatusbar,	sbSF)) )
		throw runtime_error ("Failed to construct SF widgets");

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
		throw runtime_error ("Failed to construct SF widgets");

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
		throw runtime_error ("Failed to construct SF widgets");

	gtk_menu_item_set_submenu( iSFPageHidden, (GtkWidget*)iiSFPageHidden);

	G_CONNECT_1 (iSFPageShowOriginal, toggled);
	G_CONNECT_1 (iSFPageShowProcessed, toggled);
	G_CONNECT_1 (iSFPageUseResample, toggled);
	G_CONNECT_1 (iSFPageDrawZeroline, toggled);

	G_CONNECT_1 (iSFPageAnnotationDelete, activate);
	G_CONNECT_1 (iSFPageAnnotationEdit, activate);

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
	if ( !(AGH_GBGETOBJ (GtkDialog,			wAnnotationLabel)) ||
	     !(AGH_GBGETOBJ (GtkEntry,			eAnnotationLabel)) ||
	     !(AGH_GBGETOBJ (GtkDialog,			wAnnotationSelector)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,		eAnnotationSelectorWhich)) )
		throw runtime_error ("Failed to construct widgets");

	mAnnotationsAtCursor = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_combo_box_set_model_properly( eAnnotationSelectorWhich, mAnnotationsAtCursor);

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
	     !(AGH_GBGETOBJ (GtkLabel,			lSFADDirtyPercent)) ||
	     !(AGH_GBGETOBJ (GtkToggleButton,		bSFADPreview)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADApply)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADCancel)) ||
	     !(AGH_GBGETOBJ (GtkDialog,			wSFADSaveProfileName)) ||
	     !(AGH_GBGETOBJ (GtkEntry,			eSFADSaveProfileNameName)) )
		throw runtime_error ("Failed to construct SF widgets");

	mSFADProfiles = gtk_list_store_new( 1, G_TYPE_STRING);
	// this GtkListStore is populated from the same source, but something
	// haunting GTK+ forbids reuse of _p.mGlobalArtifactDetectionProfiles
	gtk_combo_box_set_model_properly( eSFADProfiles, mSFADProfiles);

	G_CONNECT_1 (wSFArtifactDetection, close);
	G_CONNECT_2 (wSFArtifactDetection, delete, event);
	eSFADProfiles_changed_cb_handler_id =
		G_CONNECT_1 (eSFADProfiles, changed);
	G_CONNECT_1 (bSFADProfileSave, clicked);
	G_CONNECT_1 (bSFADProfileDelete, clicked);
	G_CONNECT_1 (eSFADEstimateE, toggled);
	G_CONNECT_1 (eSFADUseThisRange, toggled);
	G_CONNECT_1 (bSFADPreview, toggled);
	G_CONNECT_1 (bSFADApply, clicked);
	G_CONNECT_1 (bSFADCancel, clicked);

	// simple artifact detection
	if ( !AGH_GBGETOBJ (GtkDialog,		wSFSimpleArtifactDetectionParams) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFSimpleArtifactDetectionMinFlatRegionSize) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFSimpleArtifactDetectionPad) )
		throw runtime_error ("Failed to construct SF widgets");


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
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternBandPassOrder) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternBandPassFrom) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternBandPassUpto) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternDZCDFStep) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternDZCDFSigma) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternDZCDFSmooth) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternParameterA) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternParameterB) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternParameterC) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	ePatternParameterD) ||
	     !AGH_GBGETOBJ (GtkHBox,		cPatternLabelBox) ||
	     !AGH_GBGETOBJ (GtkLabel,		lPatternSimilarity) ||
	     !AGH_GBGETOBJ (GtkComboBox,	ePatternList) ||
	     !AGH_GBGETOBJ (GtkComboBox,	ePatternChannel) ||
	     !AGH_GBGETOBJ (GtkDialog,		wPatternName) ||
	     !AGH_GBGETOBJ (GtkEntry,		ePatternNameName) ||
	     !AGH_GBGETOBJ (GtkCheckButton,	ePatternNameSaveGlobally) )
		throw runtime_error ("Failed to construct SF widgets");

	gtk_combo_box_set_model_properly( ePatternList, mPatterns);
	ePatternList_changed_cb_handler_id =
		G_CONNECT_1 (ePatternList, changed);

	gtk_combo_box_set_model_properly( ePatternChannel, _p.mAllChannels);
	ePatternChannel_changed_cb_handler_id =
		G_CONNECT_1 (ePatternChannel, changed);

	G_CONNECT_1 (daPatternSelection, draw);
	G_CONNECT_2( daPatternSelection, scroll, event);
	g_signal_connect( bPatternFindNext, "clicked",
			  G_CALLBACK (bPatternFind_clicked_cb),
			  this);
	g_signal_connect( bPatternFindPrevious, "clicked",
			  G_CALLBACK (bPatternFind_clicked_cb),
			  this);
	G_CONNECT_1 (bPatternSave, clicked);
	G_CONNECT_1 (bPatternDiscard, clicked);

	for ( auto& W : {ePatternEnvTightness, ePatternBandPassFrom, ePatternBandPassUpto,
			 ePatternBandPassOrder, ePatternDZCDFStep, ePatternDZCDFSigma, ePatternDZCDFSmooth,
			 // ePatternParameterA, ePatternParameterB, ePatternParameterC, ePatternParameterD
			 } )
		g_signal_connect( W, "value-changed",
				  (GCallback)ePattern_any_pattern_value_changed_cb,
				  this);
	for ( auto& W : {ePatternParameterA, ePatternParameterB, ePatternParameterC, ePatternParameterD} )
		g_signal_connect( W, "value-changed",
				  (GCallback)ePattern_any_criteria_value_changed_cb,
				  this);

	G_CONNECT_1 (wPattern, show);
	G_CONNECT_1 (wPattern, hide);


	// aghui::SScoringFacility::SFiltersDialog::

      // ------- wFilter
	if ( !AGH_GBGETOBJ (GtkDialog,		wFilters) ||
	     !AGH_GBGETOBJ (GtkLabel,		lFilterCaption) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eFilterLowPassCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eFilterLowPassOrder) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eFilterHighPassCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eFilterHighPassOrder) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eFilterNotchFilter) ||
	     !AGH_GBGETOBJ (GtkListStore,	mFilterNotchFilter) ||
	     !AGH_GBGETOBJ (GtkButton,		bFilterOK) )
		throw runtime_error ("Failed to construct SF widgets");

	gtk_combo_box_set_model_properly(
		eFilterNotchFilter, mFilterNotchFilter); // can't reuse _p.mNotchFilter

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
		throw runtime_error ("Failed to construct SF widgets");

	gtk_combo_box_set_model_properly(
		eSFPDChannelA, _p.mEEGChannels);
	eSFPDChannelA_changed_cb_handler_id =
		g_signal_connect( eSFPDChannelA, "changed",
				  G_CALLBACK (eSFPDChannelA_changed_cb),
				  this);

	gtk_combo_box_set_model_properly( eSFPDChannelB, _p.mEEGChannels);
	eSFPDChannelB_changed_cb_handler_id =
		g_signal_connect( eSFPDChannelB, "changed",
				  G_CALLBACK (eSFPDChannelB_changed_cb),
				  this);

	g_signal_connect( daSFPD, "draw",
			  G_CALLBACK (daSFPD_draw_cb),
			  this);
	G_CONNECT_2 (daSFPD, scroll, event);
	G_CONNECT_1 (eSFPDChannelA, changed);
	G_CONNECT_1 (eSFPDChannelB, changed);
	G_CONNECT_2 (eSFPDFreqFrom, value, changed);
	G_CONNECT_2 (eSFPDBandwidth, value, changed);
	G_CONNECT_2 (eSFPDSmooth, value, changed);
	G_CONNECT_1 (wSFPD, show);
	G_CONNECT_1 (wSFPD, hide);
}




aghui::SScoringFacilityWidgets::
~SScoringFacilityWidgets ()
{
	// destroy toplevels
	gtk_widget_destroy( (GtkWidget*)wScoringFacility);
	g_object_unref( (GObject*)builder);
}

// eof
