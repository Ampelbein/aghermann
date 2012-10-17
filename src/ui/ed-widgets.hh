// ;-*-C++-*-
/*
 *       File name:  ui/ed-widgets.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-06
 *
 *         Purpose:  GTK widgets class for SExpDesignUI
 *
 *         License:  GPL
 */

#ifndef _AGHUI_EXPDESIGN_WIDGETS_H
#define _AGHUI_EXPDESIGN_WIDGETS_H

#include <map>
#include <gtk/gtk.h>
#include "../libsigfile/page.hh" // for various enums
#include "../libsigfile/psd.hh"
#include "../model/achermann-tunable.hh"
#include "ui.hh"  // for SManagedColor

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;

namespace aghui {

// ui structures everything is public, mainly to give access to the
// bulk of extern "C" GTK callbacks

struct SExpDesignUIWidgets {

	SExpDesignUIWidgets ();
       ~SExpDesignUIWidgets ();

	GtkBuilder
		*builder;

      // storage
	GtkListStore
		*mSessions,
		*mEEGChannels,
		*mAllChannels,
		*mGlobalADProfiles;
	GtkTreeStore
		*mGlobalAnnotations,
		*mSimulations;

	gulong 	wMainWindow_delete_event_cb_handler_id,
		eMsmtSession_changed_cb_handler_id,
		eMsmtChannel_changed_cb_handler_id;

	GtkListStore
		*mScoringPageSize,
		*mFFTParamsPageSize,
		*mFFTParamsBinSize,
		*mFFTParamsWindowType,
		*mMsmtProfileType;
	static const auto
		msimulations_visibility_switch_col = 14,
		msimulations_modref_col = msimulations_visibility_switch_col + 1;
	static const auto
		mannotations_visibility_switch_col = 4,
		mannotations_ref_col = mannotations_visibility_switch_col + 1;
	static const char* const mannotations_column_names[];

      // misc
	PangoFontDescription
		*monofont;

      // main toplevel
	GtkWindow
		*wMainWindow;
	void
	set_wMainWindow_interactive( bool indeed, bool flush = true);

	// tabs
	GtkNotebook
		*tTaskSelector,
		*tDesign, *tSimulations,
		*tSettings;
	GtkLabel
		*lTaskSelector1, *lTaskSelector2,
		*lSettings;
      // 1. Measurements
	GtkMenu
		*iiMainMenu;
	GtkMenuItem
		*iExpRefresh, *iExpPurgeComputed, *iExpAnnotations, *iExpClose, *iExpQuit,
		*iExpBasicSADetectUltradianCycles,
		*iExpGloballyDetectArtifacts,
		*iMontageSetDefaults,
		*iMontageResetAll,
		*iMontageNotchNone, *iMontageNotch50Hz, *iMontageNotch60Hz,
		*iHelpAbout,
		*iHelpUsage;

	// profile mode & parameters
	GtkComboBox
		*eMsmtProfileType;
	GtkToggleButton
		*eMsmtProfileAutoscale;
	GtkScaleButton
		*eMsmtProfileSmooth;
	GtkBox	*cMsmtProfileParams1,
		*cMsmtProfileParams2;
	GtkSpinButton
		*eMsmtOpFreqFrom,
		*eMsmtOpFreqWidth;
	GtkAdjustment
		*jMsmtOpFreqFrom,
		*jMsmtOpFreqWidth;
	GtkBox	*cMsmtMainToolbar,
		*cMsmtProfileParamsContainer;
	GtkLabel
		*lMsmtPSDInfo,
		*lMsmtMCInfo;

	// view selectors
	GtkComboBox
		*eMsmtChannel,
		*eMsmtSession;

	// main area
	GtkVBox
		*cMeasurements;
	GtkStatusbar
		*sbMainStatusBar;
	guint	sbMainContextIdGeneral;
	// menus
	GtkMenu
		*iiSubjectTimeline;
	GtkMenuItem
		*iSubjectTimelineScore,
		*iSubjectTimelineDetectUltradianCycle,
		*iSubjectTimelineSubjectInfo,
		*iSubjectTimelineEDFInfo,
		*iSubjectTimelineSaveAsSVG,
		*iSubjectTimelineBrowse,
		*iSubjectTimelineResetMontage;

	// settings
	GtkSpinButton
		*eUltradianCycleDetectionAccuracy;
	GtkComboBox
		*eFFTParamsWindowType,
		*eFFTParamsPageSize,
		*eFFTParamsBinSize,
		*eArtifDampenWindowType;
	GtkListStore
		*mNotchFilter;
	GtkEntry
		*eScoreCode[(size_t)sigfile::SPage::TScore::_total];
	GtkSpinButton
		*eArtifDampenFactor,

		*eMCParamBandWidth,
		*eMCParamIIRBackpolate,
		*eMCParamMCGain,

		*eDAMsmtPPH,
		*eDAMsmtTLHeight,
		*eDAPageHeight,
		*eDAHypnogramHeight,
		*eDAEMGHeight;
	GtkAdjustment
		*jFreqFrom,
		*jFreqWidth;
	GtkSpinButton
		*eBand[(size_t)sigfile::TBand::_total][2];
	GtkEntry
		*eBrowseCommand;

	GtkButton
		*bMainCloseThatSF;

      // 2. Simulations
	GtkTreeView
		*tvSimulations;
	GtkMenuItem
		*iSimulationsRunBatch,
		*iSimulationsRunClearAll,
		*iSimulationsReportGenerate;
	GtkLabel
		*lSimulationsProfile,
		*lSimulationsChannel,
		*lSimulationsSession;

	// settings
	GtkSpinButton
		*eCtlParamAnnlNTries,		*eCtlParamAnnlItersFixedT,
		*eCtlParamAnnlStepSize,		*eCtlParamAnnlBoltzmannk,
		*eCtlParamAnnlTInitialMantissa,	*eCtlParamAnnlTInitialExponent,
		*eCtlParamAnnlDampingMu,	*eCtlParamAnnlTMinMantissa,
		*eCtlParamAnnlTMinExponent,	*eCtlParamNSWAPpBeforeSimStart,
		*eCtlParamReqScoredPercent;
	GtkCheckButton
		*eCtlParamDBAmendment1,		*eCtlParamDBAmendment2,
		*eCtlParamAZAmendment1,		*eCtlParamAZAmendment2;
	GtkLabel
		*lCtlParamDBAmendment1,		*lCtlParamDBAmendment2,
		*lCtlParamAZAmendment1,		*lCtlParamAZAmendment2;

	GtkRadioButton
		*eCtlParamScoreUnscoredAsWake;

	GtkSpinButton
		*eTunable[agh::ach::TTunable::_basic_tunables][4];
	GtkAdjustment
		*jTunable[agh::ach::TTunable::_basic_tunables][4];
	GtkButton
		*bSimParamRevertTunables;

      // other toplevels
	// about
	GtkDialog
		*wAbout;
	GtkNotebook
		*cAboutTabs;

	// scan log
	GtkDialog
		*wScanLog;
	GtkTextView
		*tScanLog;

	// edf header
	GtkDialog
		*wEDFFileDetails;
	GtkTextView
		*lEDFFileDetailsReport;
	GtkTextBuffer
		*tEDFFileDetailsReport;

	// edf dnd import
	GtkDialog
		*wEdfImport;
	GtkComboBox
		*eEdfImportGroup,
		*eEdfImportSession,
		*eEdfImportEpisode;
	GtkEntry
		*eEdfImportGroupEntry,
		*eEdfImportSessionEntry,
		*eEdfImportEpisodeEntry;
	GtkLabel
		*lEdfImportSubject,
		*lEdfImportCaption;
	GtkTextView
		*lEdfImportFileInfo;
	GtkButton
		*bEdfImportAdmit,
		*bEdfImportEdfhed,
		*bEdfImportAttachCopy,
		*bEdfImportAttachMove;

	// annotations
	GtkDialog
		*wGlobalAnnotations;
	GtkTreeView
		*tvGlobalAnnotations;

	// subject details
	GtkDialog
		*wSubjectDetails;
	GtkEntry
		*eSubjectDetailsName,
		*eSubjectDetailsComment;
	GtkSpinButton
		*eSubjectDetailsAge;
	GtkRadioButton
		*eSubjectDetailsGenderMale,
		*eSubjectDetailsGenderFemale;

	// batch setup
	GtkDialog
		*wBatchSetup;
	GtkEntry
		*eBatchSetupSubjects,
		*eBatchSetupSessions,
		*eBatchSetupChannels;
	GtkSpinButton
		*eBatchSetupRangeFrom,
		*eBatchSetupRangeWidth,
		*eBatchSetupRangeInc,
		*eBatchSetupRangeSteps;

	// montage defaults
	GtkDialog
		*wMontageDefaults;
	GtkEntry
		*eMontageDefaultsChannelList;
	GtkCheckButton
		*eMontageDefaultsShowPSD,
		*eMontageDefaultsShowPSDSpectrum,
		*eMontageDefaultsShowMC,
		*eMontageDefaultsShowEMG,
		*eMontageDefaultsOverride;

	// global artifact dtection
	GtkDialog
		*wGlobalArtifactDetection;
	GtkComboBox
		*eGlobalADProfiles;

      // colours
	enum TColour {
		night,		day,

		power_mt,	ticks_mt,	bounds,
		labels_mt,	jinfo,

		score_none,	score_nrem1,	score_nrem2,
		score_nrem3,	score_nrem4,	score_rem,
		score_wake,
		score_invalid,  // has no color chooser

		artifact,
		annotations,
		selection,

		labels_sf,	ticks_sf,	profile_psd_sf,	profile_mc_sf,
		hypnogram,	hypnogram_scoreline,
		cursor,

		spectrum,	spectrum_axes,	spectrum_grid,

		emg,

		band_delta,	band_theta,	band_alpha,
		band_beta,	band_gamma,

		swa,		swa_sim,	process_s,
		paper_mr,
		labels_mr,
		ticks_mr,
	};
	map<TColour, SManagedColor>
		CwB;

	static TColour
	score2colour( sigfile::SPage::TScore s)
		{
			return (TColour)((unsigned)s + (unsigned)TColour::score_none);
		}
	static TColour
	band2colour( sigfile::TBand b)
		{
			return (TColour)((unsigned)b + (unsigned)TColour::band_delta);
		}
};

} // namespace aghui

#endif // _AGHUI_EXPDESIGN_WIDGETS_H

// eof
