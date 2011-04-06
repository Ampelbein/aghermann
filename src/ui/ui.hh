// ;-*-C++-*- *  Time-stamp: "2011-04-06 02:41:14 hmmr"
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

#include <unordered_map>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "../structures.hh"
#include "../libagh/enums.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

namespace aghui {

using namespace std;
using namespace agh;


struct SGeometry {
	int x, y, w, h;
};
extern SGeometry
	GeometryMain,
	GeometryScoringFac,
	GeometryModRunFac;


extern GdkVisual
	*__visual;
extern GdkColormap
	*__cmap;



extern GtkListStore *mExpDesignList;

void progress_indicator( const char* current, size_t n, size_t i);
void do_rescan_tree();

int	construct();
int	construct_Measurements( const GladeXML*);
void	destruct_Measurements();
int	construct_Settings( const GladeXML*);
int	construct_ScoringFacility( const GladeXML*);
void	destruct_ScoringFacility();
int	construct_ScoringFacility_Filter( const GladeXML*);
int	construct_ScoringFacility_Patterns( const GladeXML*);
int	construct_ScoringFacility_PhaseDiff( const GladeXML*);
int	construct_Simulations( const GladeXML*);
int	construct_ModelRun( const GladeXML*);
int	construct_StatusBar( const GladeXML*);
int	construct_misc( const GladeXML*);

void	histfile_read();
void	histfile_write();
int	load_settings();
int	save_settings();
int	populate( bool do_load);
void	depopulate( bool do_save);

void	populate_mSessions();
void	populate_mChannels();


extern GtkTargetEntry target_list[];
extern guint n_targets;

void	populate_cMeasurements(void);
bool	prepare_scoring_facility( const CSubject*,
				  const char *d,
				  const char *e);
bool	prepare_modelrun_facility( CModelRun*);

void	populate_mSimulations( bool thoroughly);
void	cleanup_mSimulations();


void __reconnect_channels_combo();
void __reconnect_sessions_combo();

// tree/list models
#define AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL 14
#define AGH_TV_SIMULATIONS_MODREF_COL 15
extern GtkListStore
	*mFFTParamsWindowType,
	*mFFTParamsPageSize,
	*mScoringPageSize,
	*mAfDampingWindowType;

extern GtkListStore
	*mSessions,
	*mEEGChannels,
	*mAllChannels,
	*mPatterns;
extern GtkTreeStore
	*mSimulations;


// widgets
extern GtkWidget
	*wMainWindow,
	*wScoringFacility,
	*wPattern,
	*wModelRun,
	*wEDFFileDetails,
	*wScanLog,

	*bExpChange,
	*lMsmtInfo,
	*cMeasurements,
	*tvSimulations,

	*eMsmtChannel,
	*eMsmtSession,
	*ePatternChannel,
	*ePhaseDiffChannelA,
	*ePhaseDiffChannelB,
	*lSimulationsChannel,
	*lSimulationsSession,

	*sbMainStatusBar,

	*eMsmtPSDFreqFrom,
	*eMsmtPSDFreqWidth,

	*lScanLog,

	*eBand[(size_t)TBand::_total][2];

extern "C" {
	void eMsmtSession_changed_cb();
	void eMsmtChannel_changed_cb();
	void eSimulationsSession_changed_cb();
	void eSimulationsChannel_changed_cb();
}

extern gulong
	eMsmtSession_changed_cb_handler_id,
	eMsmtChannel_changed_cb_handler_id,
	ePatternChannel_changed_cb_handler_id,
	ePhaseDiffChannelA_changed_cb_handler_id,
	ePhaseDiffChannelB_changed_cb_handler_id;



extern guint
	sb_context_id_General;



extern const unsigned short
	FFTPageSizeValues[],
	DisplayPageSizeValues[];
extern unsigned short
	FFTPageSizeCurrent,
	DisplayPageSizeItem;





// colours

const GdkColor* contrasting_to( const GdkColor*);



struct SManagedColor {
	GdkColor fg, bg;
	GtkWidget btn;
};

extern unordered_map<TColour, SManagedColor>
	CwB;

enum class TColour {
	power_mt,
	ticks_mt,
	bounds,
	labels_mt,
	jinfo,

	signal_score_none,
	signal_score_nrem1,
	signal_score_nrem2,
	signal_score_nrem3,
	signal_score_nrem4,
	signal_score_rem,
	signal_score_wake,
	signal_score_mvt,

	signal_unfazer,

	artifact,

	labels_sf,
	ticks_sf,

	power_sf,
	hypnogram,
	hypnogram_scoreline,
	cursor,

	spectrum,
	spectrum_axes,
	spectrum_grid,

	emg,

	band_delta,
	band_theta,
	band_alpha,
	band_beta,
	band_gamma,

	swa,
	swa_sim,
	process_s,
	paper_mr,
	labels_mr,
	ticks_mr,
};



#endif

// EOF
