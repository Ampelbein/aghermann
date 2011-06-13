// ;-*-C++-*- *  Time-stamp: "2011-06-13 21:49:18 hmmr"
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

#include <map>
#include <array>

#include <gtk/gtk.h>
#include <cairo/cairo.h>


#include "../structures.hh"
#include "../libagh/enums.hh"
#include "misc.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;


namespace aghui {



struct SGeometry {
	int x, y, w, h;
};
extern SGeometry
	GeometryMain;


extern GdkVisual
	*__visual;

extern GtkBuilder
	*__builder;



void do_rescan_tree();

int	construct_once();

int	populate( bool do_load);
void	depopulate( bool do_save);


extern GtkWindow
	*wMainWindow;
extern GtkDialog
	*wEDFFileDetails,
	*wScanLog;

extern GtkButton
	*bExpChange;


extern GtkListStore
	*mSessions,
	*mEEGChannels,
	*mAllChannels;
void	populate_mSessions();
void	populate_mChannels();
void __reconnect_channels_combo();
void __reconnect_sessions_combo();



// extern "C" {
// 	void eMsmtSession_changed_cb();
// 	void eMsmtChannel_changed_cb();
// }
extern GtkLabel
	*lMsmtInfo,
	*lSimulationsChannel,
	*lSimulationsSession;
extern GtkVBox
	*cMeasurements;
extern GtkComboBox
	*eMsmtChannel,
	*eMsmtSession;

extern GtkSpinButton
	*eMsmtPSDFreqFrom,
	*eMsmtPSDFreqWidth;

extern GtkTreeView
	*tvSimulations;

extern GtkTextView
	*lScanLog;


// sb has no header file on its own, yet
extern GtkStatusbar
	*sbMainStatusBar;
namespace sb {
	int	construct_once();

	void do_rescan_tree();
	void progress_indicator( const char*, size_t n, size_t i);

	extern guint
		sbContextIdGeneral;
	void buf_on_status_bar();
}


extern GtkDialog
	*wExpDesignChooser;
namespace expdselect {
	int	construct_once();

	void	read_histfile();
	void	write_histfile();

	extern string
		hist_filename;
}








extern const array<unsigned, 4>
	FFTPageSizeValues;
extern const array<unsigned, 8>
	DisplayPageSizeValues;
// extern unsigned short
// 	FFTPageSizeItem,
// 	DisplayPageSizeItem;

// extern const char* const
// 	scoring_pagesize_values_s[9];
// extern const char* const
// 	fft_pagesize_values_s[5];



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

extern map<TColour, SManagedColor>
	CwB;

inline TColour
score2colour( TScore s)
{
	return (TColour)((unsigned)s + (unsigned)TColour::score_none);
}

}

#endif

// EOF
