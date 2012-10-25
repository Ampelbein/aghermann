// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-widgets.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-06
 *
 *         Purpose:  scoring facility widgets class
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_SCORING_FACILITY_WIDGETS_H
#define _AGH_UI_SCORING_FACILITY_WIDGETS_H

#include <gtk/gtk.h>
#include "ui/forward-decls.hh"

namespace aghui {

struct SScoringFacilityWidgets {

	// we load and construct own widget set (wScoringFacility and all its contents)
	// ourself, for every SScoringFacility instance being created, so
	// construct_widgets below takes an arg
	GtkBuilder *builder;
	SScoringFacilityWidgets (SExpDesignUI&);
       ~SScoringFacilityWidgets ();

	// storage
	GtkListStore
		*mScoringPageSize,
		*mAnnotationsAtCursor;

	// window
	GtkWindow
		*wScoringFacility;
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
		*jPageNo;
	GtkLabel
		*lSFTotalPages;
	GtkBox
		*cSFScoringModeContainer,
		*cSFICAModeContainer;
	// 1. scoring mode
	GtkButton  // acting label
		*eSFCurrentPos;
	GtkButton
		*bSFBack, *bSFForward,
		*bScoreClear, *bScoreNREM1, *bScoreNREM2, *bScoreNREM3, *bScoreNREM4,
		*bScoreREM, *bScoreWake,
		*bScoreGotoPrevUnscored, *bScoreGotoNextUnscored,
		*bScoreGotoPrevArtifact, *bScoreGotoNextArtifact;
	GtkToggleButton
		*bSFDrawCrosshair,
		*bSFShowFindDialog, *bSFShowPhaseDiffDialog;
	GtkButton
	//*bSFResetMontage,
		*bSFRunICA;
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

	// common controls (contd)
	GtkMenuToolButton
		*bSFAccept;
	GtkMenu
		*iiSFAccept;

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
		*mSFPage,
		*mSFPageSelection,
		*mSFPageAnnotation,
		*mSFPageHidden,
		*mSFPower,
		*mSFScore,
		*mSFICAPage;
	GtkCheckMenuItem
		*iSFPageShowOriginal, *iSFPageShowProcessed,
		*iSFPageUseResample, *iSFPageDrawZeroline,
		*iSFPageDrawPSDProfile,
		*iSFPageDrawPSDSpectrum,
		*iSFPageDrawMCProfile,
		*iSFPageDrawEMGProfile,
		*iSFPowerDrawBands,
		*iSFPowerSmooth,
		*iSFPowerAutoscale,
		*iSFPageSelectionDrawCourse,
		*iSFPageSelectionDrawEnvelope,
		*iSFPageSelectionDrawDzxdf;
	GtkMenuItem
		*iSFPageFilter,
		*iSFPageSaveChannelAsSVG, *iSFPageSaveMontageAsSVG,
		*iSFPageExportSignal, *iSFPageUseThisScale,
		*iSFPageDetectArtifacts, *iSFPageClearArtifacts, *iSFPageHide,
		*iSFPageHidden,  // has a submenu
		*iSFPageSpaceEvenly,
		*iSFPageLocateSelection,
		*iSFPageAnnotationSeparator,
		*iSFPageAnnotationDelete,
		*iSFPageAnnotationEdit,
		*iSFPageSelectionMarkArtifact, *iSFPageSelectionClearArtifact,
		*iSFPageSelectionFindPattern,
		*iSFPageSelectionAnnotate,
		*iSFPowerExportAll, *iSFPowerExportRange,
		*iSFPowerUseThisScale,
		*iSFScoreAssist, *iSFScoreImport, *iSFScoreExport, *iSFScoreClear,
		*iSFAcceptAndTakeNext;
	GtkSeparatorMenuItem
		*iSFPageProfileItemsSeparator;

	// more important dialogs
	// find/patterns dialog
	GtkListStore
		*mPatterns;
	GtkDialog
		*wPattern;
	GtkComboBox
		*ePatternChannel,
		*ePatternList;
	GtkScrolledWindow
		*vpPatternSelection;
	GtkDrawingArea
		*daPatternSelection;
	GtkButton
		*bPatternFindNext, *bPatternFindPrevious,
		*bPatternSave, *bPatternDiscard;
	GtkSpinButton
		*ePatternEnvTightness, *ePatternFilterCutoff,
		*ePatternFilterOrder, *ePatternDZCDFStep,
		*ePatternDZCDFSigma, *ePatternDZCDFSmooth,
		*ePatternParameterA, *ePatternParameterB,
		*ePatternParameterC;
	GtkHBox
		*cPatternLabelBox;
	GtkLabel
		*lPatternSimilarity;
	GtkDialog
		*wPatternName;
	GtkEntry
		*ePatternNameName;
	GtkCheckButton
		*ePatternNameSaveGlobally;
	gulong	ePatternChannel_changed_cb_handler_id,
		ePatternList_changed_cb_handler_id;

	// filters dialog
	GtkDialog
		*wFilters;
	GtkLabel
		*lFilterCaption;
	GtkSpinButton
		*eFilterLowPassCutoff, *eFilterHighPassCutoff,
		*eFilterLowPassOrder, *eFilterHighPassOrder;
	GtkComboBox
		*eFilterNotchFilter;
	GtkListStore
		*mFilterNotchFilter;
	GtkButton
		*bFilterOK;

	// phasediff dialog
	GtkDialog
		*wSFPD;
	GtkComboBox
		*eSFPDChannelA, *eSFPDChannelB;
	GtkDrawingArea
		*daSFPD;
	GtkSpinButton
		*eSFPDFreqFrom,
		*eSFPDBandwidth;
	GtkScaleButton
		*eSFPDSmooth;
	gulong
		eSFPDChannelA_changed_cb_handler_id,
		eSFPDChannelB_changed_cb_handler_id;

	// artifact detection dialog
	GtkDialog
		*wSFArtifactDetection;
	GtkListStore
		*mSFADProfiles;
	GtkComboBox
		*eSFADProfiles;
	gulong	eSFADProfiles_changed_cb_handler_id;
	GtkButton
		*bSFADProfileSave,
		*bSFADProfileDelete;
	GtkSpinButton
		*eSFADUpperThr,
		*eSFADLowerThr,
		*eSFADScope,
		*eSFADF0,
		*eSFADFc,
		*eSFADBandwidth,
		*eSFADMCGain,
		*eSFADBackpolate,
		*eSFADEValue,
		*eSFADHistRangeMin,
		*eSFADHistRangeMax,
		*eSFADHistBins,
		*eSFADSmoothSide;
	GtkCheckButton
		*eSFADEstimateE,
		*eSFADSingleChannelPreview;
	GtkRadioButton
		*eSFADUseThisRange,
		*eSFADUseComputedRange;
	GtkTable
		*cSFADWhenEstimateEOn,
		*cSFADWhenEstimateEOff;
	GtkLabel
		*lSFADInfo,
		*lSFADDirtyPercent;
	GtkToggleButton
		*bSFADPreview;
	GtkButton
		*bSFADApply,
		*bSFADCancel;
	GtkDialog
		*wSFADSaveProfileName;
	GtkEntry
		*eSFADSaveProfileNameName;

	// less important dialogs
	GtkDialog
		*wAnnotationLabel,
		*wAnnotationSelector;
	GtkEntry
		*eAnnotationLabel;
	GtkComboBox
		*eAnnotationSelectorWhich;

};

} // namespace aghui

#endif

// eof
