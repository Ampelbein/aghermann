// ;-*-C++-*- *  Time-stamp: "2011-04-19 01:54:23 hmmr"
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
#include <glade/glade.h>


#include "../structures.hh"
#include "../libagh/enums.hh"

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
extern guint n_targets;



namespace misc {
	int	construct( GladeXML*);
}
namespace settings {
	int	construct( GladeXML*);
	int	load();
	int	save();
}

namespace msmtview {
	void	populate(void);
	int	construct( GladeXML*);
	void	destruct();
	namespace dnd {
		int	construct( GladeXML*);
		void	destruct();
	}
}

namespace sf {
	bool	prepare( const CSubject&);
	int	construct( GladeXML*);
	void	destruct();
	namespace filter {
		int	construct( GladeXML*);
	}
	namespace patterns {
		int	construct( GladeXML*);
	}
	namespace phasediff {
		int	construct( GladeXML*);
	}
}

namespace simview {
	int	construct( GladeXML*);
	void	populate( bool thoroughly);
	void	cleanup();
}
namespace mf {
	int	construct( GladeXML*);
	bool	prepare( CModelRun*);
}

namespace misc {
	int	construct( GladeXML*);
}

namespace sb {
	int	construct( GladeXML*);
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


extern guint
	sbContextIdGeneral;



extern const unsigned short
	FFTPageSizeValues[],
	DisplayPageSizeValues[];
extern unsigned short
	FFTPageSizeCurrent,
	DisplayPageSizeItem;





// colours

const GdkColor* contrasting_to( const GdkColor*);



struct SManagedColor {
	GdkColor clr;
	GtkColorButton* btn;

	void acquire()
	{
		gtk_color_button_get_color( btn, &clr);
	}

	void set_source_rgb( cairo_t* cr) const
		{
			cairo_set_source_rgb( cr,
					      (double)clr.red/65536,
					      (double)clr.green/65536,
					      (double)clr.blue/65536);
		}
};

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

extern unordered_map<TColour, SManagedColor>
	CwB;

}

#endif

// EOF
