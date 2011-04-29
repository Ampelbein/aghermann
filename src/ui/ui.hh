// ;-*-C++-*- *  Time-stamp: "2011-04-26 01:46:16 hmmr"
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
#include <cairo/cairo.h>


#include "../structures.hh"
#include "../libagh/enums.hh"
#include "misc.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

namespace aghui {

using namespace std;


struct SGeometry {
	int x, y, w, h;
};
extern SGeometry
	GeometryMain,
	GeometryScoringFac,
	GeometryModRunFac;


extern GdkVisual
	*__visual;



void progress_indicator( const char* current, size_t n, size_t i);
void do_rescan_tree();

int	construct();

int	populate( bool do_load);
void	depopulate( bool do_save);


extern GtkTargetEntry target_list[];
extern size_t n_targets;



namespace misc {
	int	construct( GtkBuilder*);
}
namespace settings {
	int	construct( GtkBuilder*);
	int	load();
	int	save();
}

namespace msmtview {
	void	populate(void);
	int	construct( GtkBuilder*);
	void	destruct();
	namespace dnd {
		int	construct( GtkBuilder*);
		void	destruct();
	}
}

namespace sf {
	bool	prepare( const CSubject&);
	int	construct( GtkBuilder*);
	void	destruct();
	namespace filter {
		int	construct( GtkBuilder*);
	}
	namespace patterns {
		int	construct( GtkBuilder*);
	}
	namespace phasediff {
		int	construct( GtkBuilder*);
	}
}

namespace simview {
	int	construct( GtkBuilder*);
	void	populate( bool thoroughly);
	void	cleanup();
}
namespace mf {
	int	construct( GtkBuilder*);
	bool	prepare( CModelRun*);
}

namespace misc {
	int	construct( GtkBuilder*);
}

namespace sb {
	int	construct( GtkBuilder*);
	void	histfile_read();
	void	histfile_write();

	extern GtkListStore *mExpDesignList;
}


void	populate_mSessions();
void	populate_mChannels();

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
extern GtkWindow
	*wMainWindow,
	*wScoringFacility,
	*wModelRun;
extern GtkDialog
	*wEDFFileDetails,
	*wScanLog,
	*wPattern;

extern GtkButton
	*bExpChange;
extern GtkLabel
	*lMsmtInfo,
	*lSimulationsChannel,
	*lSimulationsSession;
extern GtkVBox
	*cMeasurements;
extern GtkTreeView
	*tvSimulations;

extern GtkComboBox
	*eMsmtChannel,
	*eMsmtSession,
	*ePatternChannel,
	*ePhaseDiffChannelA,
	*ePhaseDiffChannelB;

extern GtkSpinButton
	*eMsmtPSDFreqFrom,
	*eMsmtPSDFreqWidth,
	*eBand[(size_t)TBand::_total][2];

extern GtkStatusbar
	*sbMainStatusBar;

extern GtkTextView
	*lScanLog;

extern guint
	sbContextIdGeneral;

inline void
buf_on_status_bar()
{
	gtk_statusbar_pop( sbMainStatusBar, sbContextIdGeneral);
	gtk_statusbar_push( sbMainStatusBar, sbContextIdGeneral, __buf__);
	while ( gtk_events_pending() )
	 	gtk_main_iteration();
}

extern "C" {
	void eMsmtSession_changed_cb();
	void eMsmtChannel_changed_cb();
	void eSimulationsSession_changed_cb();
	void eSimulationsChannel_changed_cb();
}

namespace msmt {
	extern gulong
		eMsmtSession_changed_cb_handler_id,
		eMsmtChannel_changed_cb_handler_id;
}
namespace sf {
	extern gulong
		ePatternChannel_changed_cb_handler_id,
		ePhaseDiffChannelA_changed_cb_handler_id,
		ePhaseDiffChannelB_changed_cb_handler_id;
}



namespace settings {
	extern const unsigned short
		FFTPageSizeValues[],
		DisplayPageSizeValues[];
	extern unsigned short
		FFTPageSizeCurrent,
		DisplayPageSizeItem;
}

extern const char* const
	fft_window_types_s[(size_t)TFFTWinType::_total];

extern const char* const
	scoring_pagesize_values_s[9];
extern const char* const
	fft_pagesize_values_s[5];




// colours

const GdkColor* contrasting_to( const GdkColor*);



struct SManagedColor {
	GdkColor clr;
	guint16	alpha;
	GtkColorButton* btn;

	SManagedColor& operator=( const SManagedColor&) = default;
	void acquire()
	{
		gtk_color_button_get_color( btn, &clr);
		alpha = gtk_color_button_get_use_alpha( btn) ? gtk_color_button_get_alpha( btn) : 65535;
	}

	void set_source_rgb( cairo_t* cr) const
		{
			cairo_set_source_rgb( cr,
					      (double)clr.red/65536,
					      (double)clr.green/65536,
					      (double)clr.blue/65536);
		}
	void set_source_rgba( cairo_t* cr, double alpha_override = NAN) const
		{
			cairo_set_source_rgba( cr,
					       (double)clr.red/65536,
					       (double)clr.green/65536,
					       (double)clr.blue/65536,
					       isfinite(alpha_override) ? alpha_override : (double)alpha/65536);
		}
	void pattern_add_color_stop_rgba( cairo_pattern_t* cp, double at, double alpha_override = NAN) const
		{
			cairo_pattern_add_color_stop_rgba( cp, at,
							   (double)clr.red/65536,
							   (double)clr.green/65536,
							   (double)clr.blue/65536,
							   isfinite(alpha_override) ? alpha_override : (double)alpha/65536);
		}
};


typedef unsigned TColour_underlying_type;
enum class TColour : TColour_underlying_type {
	power_mt,
	ticks_mt,
	bounds,
	labels_mt,
	jinfo,

	score_none,
	score_nrem1,
	score_nrem2,
	score_nrem3,
	score_nrem4,
	score_rem,
	score_wake,
	score_mvt,

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

extern unordered_map<TColour, SManagedColor>
	CwB;

inline TColour
score2colour( TScore s)
{
	return (TColour)((unsigned)s + (unsigned)TColour::score_none);
}

}

#endif

// EOF
