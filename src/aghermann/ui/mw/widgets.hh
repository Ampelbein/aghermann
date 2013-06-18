/*
 *       File name:  aghermann/ui/mw/widgets.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-06
 *
 *         Purpose:  GTK widgets class for SExpDesignUI
 *
 *         License:  GPL
 */

#ifndef AGH_AGHERMANN_UI_MW_WIDGETS_H_
#define AGH_AGHERMANN_UI_MW_WIDGETS_H_

#include <map>
#include <gtk/gtk.h>
#include "libsigfile/page.hh" // for various enums
#include "libmetrics/bands.hh"
#include "aghermann/model/achermann-tunable.hh"
#include "aghermann/ui/ui.hh"  // for SManagedColor

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
	// dynamic
	GtkListStore
		*mSessions,
		*mEEGChannels,
		*mAllChannels,
		*mGlobalADProfiles;
	GtkTreeStore
		*mGlobalAnnotations,
		*mSimulations;

	// static
	GtkListStore
		*mScoringPageSize,
		*mFFTParamsPageSize,
		*mFFTParamsBinSize,
		*mFFTParamsWindowType,
		*mFFTParamsPlanType,
		*mMsmtProfileType,
		*mGlobalFiltersNotchFilter;
	static const auto
		msimulations_visibility_switch_col = 14,
		msimulations_modref_col = msimulations_visibility_switch_col + 1;
	static const auto
		mannotations_visibility_switch_col = 5,
		mannotations_ref_col = mannotations_visibility_switch_col + 1;

      // misc
	gulong 	wMainWindow_delete_event_cb_handler_id,
		eMsmtSession_changed_cb_handler_id,
		eMsmtChannel_changed_cb_handler_id;

	PangoFontDescription
		*monofont;

      // main toplevel
	GtkWindow
		*wMainWindow;
	void
	set_wMainWindow_interactive( bool indeed, bool flush = true);
	void
	set_controls_for_empty_experiment( bool indeed, bool flush = true);

	// tabs
	GtkNotebook
		*tTaskSelector,
		*tDesign, *tSimulations,
		*tSettings;
	GtkLabel
		*lTaskSelector1, *lTaskSelector2,
		*lSettings;
      // 1. Measurements
	GtkMenuItem
		*iiMainMenu,
		*iExpRefresh, *iExpPurgeComputed, *iExpAnnotations,
		*iExpClose, *iExpQuit,
		*iExpBasicSADetectUltradianCycles,
		*iiExpGlobalOperations,
		*iExpGloballyDetectArtifacts,
		*iExpGloballySetFilters,
		*iMontageSetDefaults,
		*iHelpAbout,
		*iHelpUsage;
	GtkRadioMenuItem
		*iExpSubjectSortName,
		*iExpSubjectSortAge,
		*iExpSubjectSortAdmissionDate,
		*iExpSubjectSortAvgPower;
	GtkCheckMenuItem
		*iExpSubjectSortAscending,
		*iExpSubjectSortSegregate;


	// profile mode & parameters
	GtkBox	*cMsmtTopArea,
		*cMsmtMainToolbar,
		*cMsmtProfileParamsContainer;
	GtkComboBox
		*eMsmtProfileType;
	GtkBox	*cMsmtProfileParamsPSD,
		*cMsmtProfileParamsSWU,
		*cMsmtProfileParamsMC;
	GtkSpinButton
		*eMsmtProfileParamsPSDFreqFrom,
		*eMsmtProfileParamsPSDFreqWidth,
		*eMsmtProfileParamsSWUF0,
		*eMsmtProfileParamsMCF0;
	GtkAdjustment
		*jMsmtProfileParamsPSDFreqFrom,
		*jMsmtProfileParamsPSDFreqWidth,
		*jMsmtProfileParamsSWUF0,
		*jMsmtProfileParamsMCF0;

	GtkLabel
		*lMsmtProfilePSDExtra,
		*lMsmtProfileSWUExtra,
		*lMsmtProfileMCExtra;

	GtkScaleButton
		*eMsmtProfileSmooth;
	GtkToggleButton
		*eMsmtProfileAutoscale;
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
		*iSubjectTimelineEDFInfo,
		*iSubjectTimelineSaveAsSVG,
		*iSubjectTimelineBrowse,
		*iSubjectTimelineResetMontage;

	// settings
	GtkSpinButton
		*eSMPMaxThreads;
	GtkSpinButton
		*eUltradianCycleDetectionAccuracy;
	GtkCheckButton
		*eScanTreeStrict,
		*eScanTreeSuppressReport;
	GtkComboBox
		*eFFTParamsWindowType,
		*eFFTParamsPageSize,
		*eFFTParamsBinSize,
		*eFFTParamsPlanType,
		*eArtifDampenWindowType;
	GtkEntry
		*eScoreCode[sigfile::SPage::TScore_total];
	GtkSpinButton
		*eArtifDampenFactor,
		*eMCParamBandWidth,
		*eMCParamIIRBackpolate,
		*eMCParamMCGain,
		*eMCParamFreqInc,
		*eMCParamNBins,
		*eSWUParamMinUpswingDuration,
		*eDAMsmtPPH,
		*eDAMsmtTLHeight,
		*eDAPageHeight,
		*eDAHypnogramHeight,
		*eDAEMGHeight,
		*eScrollSpeedFactor;
	GtkAdjustment
		*jFreqFrom,
		*jFreqWidth;
	GtkSpinButton
		*eBand[metrics::TBand_total][2];
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
		*eCtlParamAZAmendment1;
	GtkLabel
		*lCtlParamDBAmendment1,		*lCtlParamDBAmendment2,
		*lCtlParamAZAmendment1;

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
	GtkLabel
		*lAboutVersion;

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
	GtkCheckButton
		*eGlobalAnnotationsShowPhasicEvents;

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

	// // montage defaults
	// GtkDialog
	// 	*wMontageDefaults;
	// GtkEntry
	// 	*eMontageDefaultsChannelList;
	// GtkCheckButton
	// 	*eMontageDefaultsShowPSD,
	// 	*eMontageDefaultsShowPSDSpectrum,
	// 	*eMontageDefaultsShowMC,
	// 	*eMontageDefaultsShowEMG;

	// global artifact detection
	GtkDialog
		*wGlobalArtifactDetection;
	GtkComboBox
		*eGlobalADProfiles;
	GtkCheckButton
		*eGlobalADKeepExisting;
	GtkLabel
		*lGlobalADHint;
	GtkButton
		*bGlobalADOK;

	// global filters
	GtkDialog
		*wGlobalFilters;
	GtkSpinButton
		*eGlobalFiltersLowPassCutoff, *eGlobalFiltersHighPassCutoff,
		*eGlobalFiltersLowPassOrder, *eGlobalFiltersHighPassOrder;
	GtkComboBox
		*eGlobalFiltersNotchFilter;

      // colours
	enum TColour {
		mw_night,	mw_day,
		mw_profile,	mw_ticks,	mw_bounds,
		mw_labels,

		sf_artifact,
		sf_annotations,
		sf_embedded_annotations,
		sf_selection,
		sf_profile_psd,	sf_profile_mc, sf_profile_swu,
		sf_phasic_spindle, sf_phasic_Kcomplex, sf_phasic_eyeblink,
		sf_hypnogram,
		sf_cursor,
		sf_emg,
		sf_labels,	sf_ticks,

		mf_swa,		mf_swa_sim,	mf_process_s,
		mf_paper,
		mf_labels,
		mf_ticks,

		band_delta,	band_theta,	band_alpha,
		band_beta,	band_gamma,

		score_none,	score_nrem1,	score_nrem2,
		score_nrem3,	score_nrem4,	score_rem,
		score_wake,
		score_invalid,  // has no color chooser
	};
	map<TColour, SManagedColor>
		CwB;

	static TColour
	score2colour( sigfile::SPage::TScore s)
		{
			return (TColour)((unsigned)s + (unsigned)TColour::score_none);
		}
	static TColour
	band2colour( metrics::TBand b)
		{
			return (TColour)((unsigned)b + (unsigned)TColour::band_delta);
		}
};

} // namespace aghui

#endif // _AGHUI_EXPDESIGN_WIDGETS_H

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

