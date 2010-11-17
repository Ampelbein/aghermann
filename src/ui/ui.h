// ;-*-C++-*- *  Time-stamp: "2010-11-16 23:25:50 hmmr"
/*
 *       File name:  ui.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-04-28
 *
 *         Purpose:  
 *
 *         License:  GPL
 */


#ifndef _AGH_UI_H
#define _AGH_UI_H

#include <gtk/gtk.h>
#include <glade/glade.h>

#if HAVE_CONFIG_H
#  include <config.h>
#endif

G_BEGIN_DECLS

extern int
	AghHi, AghHs,
	AghTi, AghTs,
	AghGi, AghGs,
	AghDi, AghDs,
	AghEi, AghEs;

extern char
	**AghDD,
	**AghHH,
	**AghTT,
	**AghGG,
	**AghEE;
#define AghD (AghDD ? (AghDi < AghDs && AghDi >= 0) ? AghDD[AghDi] : "no session" : "invalid session")
#define AghH (AghHH ? (AghHi < AghHs && AghHi >= 0) ? AghHH[AghHi] : "no channel" : "invalid channel")
#define AghT (AghTT ? (AghTi < AghTs && AghTi >= 0) ? AghTT[AghTi] : "no channel" : "invalid channel")
#define AghE (AghEE ? (AghEi < AghEs && AghEi >= 0) ? AghEE[AghEi] : "no episode" : "invalid episode")
#define AghG (AghGG ? (AghGi < AghGs && AghGi >= 0) ? AghGG[AghGi] : "no group"   : "invalid group")

extern const struct SSubject
	*AghJ;


extern gfloat
	AghPPuV2,
	AghGlitchMag;
extern guint
	AghSmoothover;

extern gfloat
	AghQuickViewFreqFrom,
	AghQuickViewFreqUpto;

extern gfloat
	AghSimOperatingRangeFrom,
	AghSimOperatingRangeUpto;

extern gboolean
	AghSimRunbatchIncludeAllChannels,
	AghSimRunbatchIncludeAllSessions,
	AghSimRunbatchIterateRanges;
// extern gint
// 	agh_sim_runbatch_redo_option;




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
void	agh_populate_mSimulations();

void	agh_populate_cMeasurements();

gboolean       agh_prepare_scoring_facility();
gboolean       agh_prepare_modelrun_facility();



// //void __agh__disconnect_channels_combo();
void __agh__reconnect_channels_combo();
// //void __agh__disconnect_sessions_combo();
void __agh__reconnect_sessions_combo();
// void __agh_propagate_current_session_change();
// void __agh_propagate_current_channel_change();

// tree/list models
extern GtkListStore	*agh_mSessions;
extern GtkListStore	*agh_mEEGChannels;
extern GtkListStore	*agh_mAllChannels;
extern GtkListStore	*agh_mSimulations;
#define AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL 12
extern GtkListStore	*agh_mFFTParamsWindowType;
extern GtkListStore	*agh_mFFTParamsPageSize;
extern GtkListStore	*agh_mScoringPageSize;
extern GtkListStore	*agh_mAfDampingWindowType;


// widgets
extern GtkWidget	*wMainWindow;
extern GtkWidget	*wScoringFacility;
extern GtkWidget	*wModelRun;
extern GtkWidget	*wEDFFileDetails;
extern GtkWidget	*wScanLog;

extern GtkWidget	*cMeasurements;
extern GtkWidget	*tvSimulations;

extern GtkWidget	*eMsmtChannel;
extern GtkWidget	*eMsmtSession;
extern GtkWidget	*eSimulationsChannel;
extern GtkWidget	*eSimulationsSession;

extern GtkWidget	*sbMainStatusBar;

extern GtkWidget	*eMsmtPSDFreqFrom;
extern GtkWidget	*eMsmtPSDFreqWidth;

extern GtkWidget	*lScanLog;

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
