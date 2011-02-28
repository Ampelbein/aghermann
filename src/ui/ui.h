// ;-*-C++-*- *  Time-stamp: "2011-02-24 09:43:43 hmmr"
/*
 *       File name:  ui/ui.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  globally accessible widgets, GTK-specific representations, and model storage
 *
 *         License:  GPL
 */


#ifndef _AGH_UI_H
#define _AGH_UI_H

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "../structures.h"
#include "../common.h"

#if HAVE_CONFIG_H
#  include <config.h>
#endif




extern GtkListStore *agh_mExpDesignList;

void progress_indicator( const char* current, size_t n, size_t i);
void do_rescan_tree();

gint	agh_ui_construct(void);
gint	agh_ui_construct_Measurements( GladeXML*);
void	agh_ui_destruct_Measurements(void);
gint	agh_ui_construct_Settings( GladeXML*);
gint	agh_ui_construct_ScoringFacility( GladeXML*);
void	agh_ui_destruct_ScoringFacility(void);
gint	agh_ui_construct_ScoringFacility_Filter( GladeXML*);
gint	agh_ui_construct_ScoringFacility_Patterns( GladeXML*);
gint	agh_ui_construct_ScoringFacility_PhaseDiff( GladeXML*);
gint	agh_ui_construct_Simulations( GladeXML*);
gint	agh_ui_construct_ModelRun( GladeXML*);
gint	agh_ui_construct_StatusBar( GladeXML*);
gint	agh_ui_construct_misc( GladeXML*);

void	agh_histfile_read();
void	agh_histfile_write();
gint	agh_ui_settings_load();
gint	agh_ui_settings_save();
gint	agh_ui_populate( int do_load);
void	agh_ui_depopulate( int do_save);

void	agh_populate_mSessions();
void	agh_populate_mChannels();


extern GtkTargetEntry target_list[];
extern guint n_targets;

void	agh_populate_cMeasurements();
gboolean       agh_prepare_scoring_facility( struct SSubject*, const char *d, const char *e);
gboolean       agh_prepare_modelrun_facility( TModelRef);

void	agh_populate_mSimulations( gboolean thoroghly);
void	agh_cleanup_mSimulations();


// //void __agh__disconnect_channels_combo();
void __agh__reconnect_channels_combo();
// //void __agh__disconnect_sessions_combo();
void __agh__reconnect_sessions_combo();
// void __agh_propagate_current_session_change();
// void __agh_propagate_current_channel_change();

// tree/list models
#define AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL 14
#define AGH_TV_SIMULATIONS_MODREF_COL 15
extern GtkListStore
	*agh_mFFTParamsWindowType,
	*agh_mFFTParamsPageSize,
	*agh_mScoringPageSize,
	*agh_mAfDampingWindowType;

extern GtkListStore
	*agh_mSessions,
	*agh_mEEGChannels,
	*agh_mAllChannels,
	*agh_mPatterns;
extern GtkTreeStore
	*agh_mSimulations;


// widgets
extern GtkWidget
	*wMainWindow,
	*wScoringFacility,
	*wPattern,
	*wModelRun,
	*wEDFFileDetails,
	*wScanLog,

	*lMsmtInfo,
	*cMeasurements,
	*tvSimulations,

	*eMsmtChannel,
	*eMsmtSession,
	*ePatternChannel,
	*ePhaseDiffChannelA,
	*ePhaseDiffChannelB,
	*eSimulationsChannel,
	*eSimulationsSession,

	*sbMainStatusBar,

	*eMsmtPSDFreqFrom,
	*eMsmtPSDFreqWidth,

	*lScanLog,

	*eBand[AGH_BAND__TOTAL][2];

void eMsmtSession_changed_cb();
void eMsmtChannel_changed_cb();
void eSimulationsSession_changed_cb();
void eSimulationsChannel_changed_cb();
extern gulong
	eMsmtSession_changed_cb_handler_id,
	eMsmtChannel_changed_cb_handler_id,
	ePatternChannel_changed_cb_handler_id,
	ePhaseDiffChannelA_changed_cb_handler_id,
	ePhaseDiffChannelB_changed_cb_handler_id;



extern guint
	agh_sb_context_id_General;
#define BUF_ON_STATUS_BAR \
	{ \
	gtk_statusbar_pop( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General); \
	gtk_statusbar_push( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General, __buf__); \
	while ( gtk_events_pending() )					\
	 	gtk_main_iteration(); \
	}



extern const guint
	AghFFTPageSizeValues[],
	AghDisplayPageSizeValues[];
extern guint
	AghFFTPageSizeCurrent,
	AghDisplayPageSizeItem;


guint SCOREID( gchar c);

extern gchar *AghScoreNames[];




// colours

const GdkColor* contrasting_to( GdkColor*);


extern GtkWidget
	*bColourPowerMT,
	*bColourTicksMT,
	*bColourLabelsMT;

enum {
	cPOWER_MT,
	cTICKS_MT,
	cBOUNDS,
	cLABELS_MT,
	cJINFO,
	cTOTAL_MT
};  // colours

extern GdkColor
	__fg0__[cTOTAL_MT],
	__bg0__[cTOTAL_MT];



extern GtkWidget
	*bColourNONE,
	*bColourNREM1,
	*bColourNREM2,
	*bColourNREM3,
	*bColourNREM4,
	*bColourREM,
	*bColourWake,
	*bColourPowerSF,
	*bColourEMG,
	*bColourHypnogram,
	*bColourArtifacts,
	*bColourOriginalSignal,
	*bColourFilteredSignal,
	*bColourTicksSF,
	*bColourLabelsSF,
	*bColourCursor,

	*bColourBandDelta,
	*bColourBandTheta,
	*bColourBandAlpha,
	*bColourBandBeta,
	*bColourBandGamma;

enum {
	cSIGNAL_SCORE_NONE,
	cSIGNAL_SCORE_NREM1,
	cSIGNAL_SCORE_NREM2,
	cSIGNAL_SCORE_NREM3,
	cSIGNAL_SCORE_NREM4,
	cSIGNAL_SCORE_REM,
	cSIGNAL_SCORE_WAKE,
	cSIGNAL_SCORE_MVT,

	cSIGNAL_UNFAZER,

	cARTIFACT,

	cLABELS_SF,
	cTICKS_SF,

	cPOWER_SF,
	cHYPNOGRAM,
	cHYPNOGRAM_SCORELINE,
	cCURSOR,

	cSPECTRUM,
	cSPECTRUM_AXES,
	cSPECTRUM_GRID,

	cEMG,

	cBAND_DELTA,
	cBAND_THETA,
	cBAND_ALPHA,
	cBAND_BETA,
	cBAND_GAMMA,

	cTOTAL_SF
};

extern GdkColor
	__fg1__[cTOTAL_SF],
	__bg1__[cTOTAL_SF];




extern GtkWidget
	*bColourSWA,
	*bColourSWASim,
	*bColourProcessS,
	*bColourPaperMR,
	*bColourTicksMR,
	*bColourLabelsMR;

enum {
	cSWA,
	cSWA_SIM,
	cPROCESS_S,
	cPAPER_MR,
	cLABELS_MR,
	cTICKS_MR,

	cTOTAL_MR
};  // colours

extern GdkColor
	__fg2__[cTOTAL_MR],
	__bg2__[cTOTAL_MR];




#endif

// EOF
