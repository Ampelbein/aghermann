/*
 *       File name:  aghermann/ui/sf/widgets.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-06
 *
 *         Purpose:  scoring facility widgets class
 *
 *         License:  GPL
 */

#ifndef AGH_AGHERMANN_UI_SF_WIDGETS_H_
#define AGH_AGHERMANN_UI_SF_WIDGETS_H_

#include <gtk/gtk.h>

// stow this crowd away from first-class gents
namespace agh {
namespace ui {

struct SScoringFacilityWidgets {

	SScoringFacilityWidgets ();
       ~SScoringFacilityWidgets ();

	GtkBuilder *builder;

	// storage
	GtkListStore
		*mSFScoringPageSize,
		*mSFAnnotationsAtCursor;

	// window
	GtkWindow
		*wSF;
	// control bar
	GtkLabel
		*lSFHint;
	GtkHBox
		*cSFControlBar;
	GtkComboBox
		*eSFPageSize;
	GtkSpinButton
		*eSFCurrentPage;
	GtkAdjustment
		*jSFPageNo;
	GtkLabel
		*lSFTotalPages;
	GtkBox
		*cSFScoringModeContainer,
		*cSFICAModeContainer;
	// 1. scoring mode
	GtkCheckMenuItem
		*iSFMontageDrawOriginalSignal,
		*iSFMontageDrawProcessedSignal,
		*iSFMontageDrawZeroLine,
		*iSFMontageDrawFast;
	GtkMenuItem
		*iSFMontagePatterns,
		*iSFMontageICA,
		*iSFMontagePhaseDiff,
		*iSFMontageScoreAssist,
		*iSFMontageScoreImport,
		*iSFMontageScoreExport,
		*iSFMontageScoreClear,
		*iSFMontageClose,
		*iSFMontageCloseAndNext;
	GtkButton  // acting label
		*eSFCurrentPos;
	GtkButton
		*bSFBack, *bSFForward,
		*bSFScoreClear, *bSFScoreNREM1, *bSFScoreNREM2, *bSFScoreNREM3, *bSFScoreNREM4,
		*bSFScoreREM, *bSFScoreWake,
		*bSFGotoPrevUnscored, *bSFGotoNextUnscored,
		*bSFGotoPrevArtifact, *bSFGotoNextArtifact;
	GtkToggleButton
		*bSFDrawCrosshair;
	GtkTable
		*cSFSleepStageStats;
	GtkLabel
		*lScoreStatsNREMPercent, *lScoreStatsREMPercent, *lScoreStatsWakePercent,
		*lSFPercentScored;
	GtkStatusbar
		*sbSF;
	guint	sbSFContextIdGeneral;

	// 2. ICA mode
	GtkComboBox
		*eSFICARemixMode,
		*eSFICANonlinearity,
		*eSFICAApproach;
	GtkListStore
		*mSFICARemixMode,
		*mSFICANonlinearity,
		*mSFICAApproach;
	GtkCheckButton
		*eSFICAFineTune,
		*eSFICAStabilizationMode;
	GtkSpinButton
		*eSFICAa1,
		*eSFICAa2,
		*eSFICAmu,
		*eSFICAepsilon,
		*eSFICANofICs,
		*eSFICAEigVecFirst,
		*eSFICAEigVecLast,
		*eSFICASampleSizePercent,
		*eSFICAMaxIterations;
	GtkAdjustment
		*jSFICANofICs,
		*jSFICAEigVecFirst,
		*jSFICAEigVecLast;
	GtkButton
		*bSFICATry,
		*bSFICAApply,
		*bSFICACancel;
	GtkToggleButton
		*bSFICAPreview,
		*bSFICAShowMatrix;
	GtkTextView
		*tSFICAMatrix;
	GtkDialog
		*wSFICAMatrix;

	// montage area
	GtkDrawingArea
		*daSFMontage,
		*daSFHypnogram;
	GtkExpander
		*cSFHypnogram;
	GtkLabel
		*lSFOverChannel;
	// menus
	GtkMenu
		*iiSFPage,
		*iiSFPageSelection,
		*iiSFPageAnnotation,
		*iiSFPageProfiles,
		*iiSFPagePhasicEvents,
		*iiSFPageHidden,
		*iiSFPower,
		*iiSFScore,
		*iiSFICAPage;
	GtkMenuItem
		*iSFMontageMenu;
	GtkCheckMenuItem
		*iSFPageShowOriginal, *iSFPageShowProcessed,
		*iSFPageUseResample, *iSFPageDrawZeroline,
		*iSFPageDrawPSDProfile,
		*iSFPageDrawPSDSpectrum,
		*iSFPageDrawSWUProfile,
		*iSFPageDrawMCProfile,
		*iSFPageDrawEMGProfile,
		*iSFPowerDrawBands,
		*iSFPowerSmooth,
		*iSFPowerAutoscale,
		*iSFPageSelectionDrawCourse,
		*iSFPageSelectionDrawEnvelope,
		*iSFPageSelectionDrawDzxdf,
		*iSFPageDrawPhasicSpindles,
		*iSFPageDrawPhasicKComplexes,
		*iSFPageDrawPhasicEyeBlinks;
	GtkMenuItem
		*iSFPageFilter,
		*iSFPageSaveChannelAsSVG, *iSFPageSaveMontageAsSVG,
		*iSFPageExportSignal, *iSFPageUseThisScale,
		*iSFPageArtifactsDetect, *iSFPageArtifactsMarkFlat, *iSFPageArtifactsClear,
		*iSFPageHide,
		*iSFPageHidden,  // has a submenu
		*iSFPageSpaceEvenly,
		*iSFPageLocateSelection,
		*iSFPageAnnotationSeparator,
		*iSFPageAnnotationDelete,
		*iSFPageAnnotationEdit,
		*iSFPageAnnotationClearAll,
		*iSFPageAnnotationGotoPrev,
		*iSFPageAnnotationGotoNext,
		*iSFPageSelectionMarkArtifact, *iSFPageSelectionClearArtifact,
		*iSFPageSelectionFindPattern,
		*iSFPageSelectionAnnotate,
		*iSFPowerExportAll, *iSFPowerExportRange,
		*iSFPowerUseThisScale,
		*iSFScoreAssist, *iSFScoreImport, *iSFScoreExport, *iSFScoreClear,
		*iSFAcceptAndTakeNext;
	GtkSeparatorMenuItem
		*iSFPageProfilesSubmenuSeparator;

	// more important dialogs

	// less important dialogs
	GtkDialog
		*wSFAnnotationLabel,
		*wSFAnnotationSelector;
	GtkEntry
		*eSFAnnotationLabel;
	GtkRadioButton
		*eSFAnnotationTypePlain,
		*eSFAnnotationTypeSpindle,
		*eSFAnnotationTypeKComplex,
		*eSFAnnotationTypeBlink;
	GtkComboBox
		*eSFAnnotationSelectorWhich;

};

}
} // namespace agh::ui

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
