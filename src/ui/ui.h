// ;-*-C++-*- *  Time-stamp: "2010-11-22 00:23:34 hmmr"
/*
 *       File name:  ui/ui.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

G_BEGIN_DECLS


extern int
	AghHi,
	AghTi,
	AghGi,
	AghDi,
	AghEi;
#define AghD (AghDD ? (AghDi < AghDs && AghDi >= 0) ? AghDD[AghDi] : "no session" : "invalid session")
#define AghH (AghHH ? (AghHi < AghHs && AghHi >= 0) ? AghHH[AghHi] : "no channel" : "invalid channel")
#define AghT (AghTT ? (AghTi < AghTs && AghTi >= 0) ? AghTT[AghTi] : "no channel" : "invalid channel")
#define AghE (AghEE ? (AghEi < AghEs && AghEi >= 0) ? AghEE[AghEi] : "no episode" : "invalid episode")
#define AghG (AghGG ? (AghGi < AghGs && AghGi >= 0) ? AghGG[AghGi] : "no group"   : "invalid group")

extern gfloat
	AghOperatingRangeFrom,
	AghOperatingRangeUpto;

extern const struct SSubject
	*AghJ;


extern gfloat
	AghPPuV2,
	AghGlitchMag;
extern guint
	AghSmoothover;

extern gboolean
	AghSimRunbatchIncludeAllChannels,
	AghSimRunbatchIncludeAllSessions,
	AghSimRunbatchIterateRanges;
// extern gint
// 	agh_sim_runbatch_redo_option;

const gchar* const agh_scoring_pagesize_values_s[9];
const gchar* const agh_fft_pagesize_values_s[5];
const gchar* const agh_fft_window_types_s[9];


extern GtkListStore *agh_mExpDesignList;

extern char*	AghLastExpdesignDir;
extern int	AghLastExpdesignDirNo;

void progress_indicator( const char* current, size_t n, size_t i);


gint	agh_ui_construct(void);
gint	agh_ui_construct_Measurements( GladeXML*);
void	agh_ui_destruct_Measurements(void);
gint	agh_ui_construct_Settings( GladeXML*);
gint	agh_ui_construct_ScoringFacility( GladeXML*);
void	agh_ui_destruct_ScoringFacility(void);
gint	agh_ui_construct_Simulations( GladeXML*);
gint	agh_ui_construct_SimulationParams( GladeXML*);
gint	agh_ui_construct_ModelRun( GladeXML*);
gint	agh_ui_construct_StatusBar( GladeXML*);
gint	agh_ui_construct_misc( GladeXML*);

void	agh_histfile_read();
void	agh_histfile_write();
gint	agh_ui_settings_load();
gint	agh_ui_settings_save();
gint	agh_ui_populate(void);
void	agh_ui_depopulate(void);

void	agh_populate_mSessions();
void	agh_populate_mChannels();


void	agh_populate_cMeasurements();
gboolean       agh_prepare_scoring_facility();
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
	*agh_mAllChannels;
extern GtkTreeStore
	*agh_mSimulations;


// widgets
extern GtkWidget
	*wMainWindow,
	*wScoringFacility,
	*wModelRun,
	*wEDFFileDetails,
	*wScanLog,

	*lMsmtInfo,
	*cMeasurements,
	*tvSimulations,

	*eMsmtChannel,
	*eMsmtSession,
	*eSimulationsChannel,
	*eSimulationsSession,

	*sbMainStatusBar,

	*eMsmtPSDFreqFrom,
	*eMsmtPSDFreqWidth,

	*lScanLog;

void eMsmtSession_changed_cb(void);
void eMsmtChannel_changed_cb(void);
void eSimulationsSession_changed_cb(void);
void eSimulationsChannel_changed_cb(void);
extern gulong
	eMsmtSession_changed_cb_handler_id,
	eMsmtChannel_changed_cb_handler_id,
	eSimulationsSession_changed_cb_handler_id,
	eSimulationsChannel_changed_cb_handler_id;



extern guint		agh_sb_context_id_General;


extern GdkVisual	*agh_visual;


extern guint
	AghFFTPageSizeValues[],
	AghFFTPageSizeCurrent;
extern guint
	AghDisplayPageSizeValues[],
	AghDisplayPageSizeItem;


guint SCOREID( gchar c);

extern gchar *AghScoreNames[];


G_END_DECLS

#endif

// EOF
