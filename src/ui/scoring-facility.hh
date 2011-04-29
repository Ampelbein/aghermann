// ;-*-C++-*- *  Time-stamp: "2011-04-29 04:08:28 hmmr"
/*
 *       File name:  ui/scoring-facility.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-14
 *
 *         Purpose:  scoring facility bits shared between scoring-facility{,-patterns}.c
 *
 *         License:  GPL
 */

#ifndef _AGH_SCORING_FACILITY_H
#define _AGH_SCORING_FACILITY_H

#include <cairo.h>
#include <gtk/gtk.h>

#include "libexstrom/exstrom.hh"
#include "libexstrom/signal.hh"


#if HAVE_CONFIG_H
#  include <config.h>
#endif


using namespace std;

namespace aghui {
namespace sf {


#define PSZ  settings::DisplayPageSizeValues[settings::DisplayPageSizeItem]
#define APSZ settings::DisplayPageSizeValues[__pagesize_item]



// structures

struct SScoringFacility;

struct SChannelPresentation {
	const char
		*name,
		*type;
	agh::CRecording&
		recording;

      // signal waveforms, cached here
	valarray<float>
		signal_filtered,
		signal_original;
      // filters
	struct SFilterInfo {
		float	cutoff;
		unsigned
			order;
	};
	SFilterInfo
		low_pass,
		high_pass;

	size_t n_samples() const
		{
			return signal_filtered.size();
		}
	size_t samplerate() const
		{
			return recording.F().samplerate(name);
		}

	float	signal_display_scale;

	bool	draw_original_signal:1,
		draw_processed_signal:1,
		draw_envelope:1,
		draw_course:1,
		draw_dzcdf:1,
		draw_bands:1,
		draw_spectrum_absolute:1,
		use_resample:1;

      // artifacts
	float calculate_dirty_percent();
	float	percent_dirty;

      // signal features
	struct SSFLowPassCourse {
		float	cutoff;
		unsigned
			order;
		valarray<float>
			data;
		float& operator[]( size_t i)
			{
				return data[i];
			}
		SSFLowPassCourse( float _cutoff, unsigned _order, const valarray<float>& signal,
				  unsigned samplerate)
		      : cutoff (_cutoff), order (_order),
			data (exstrom::low_pass( signal, samplerate, cutoff, order, true))
			{}
		SSFLowPassCourse() = default;
	};
	SSFLowPassCourse
		signal_lowpass;

	struct SSFEnvelope {
		unsigned
			tightness;
		valarray<float>
			upper,
			lower;
		float breadth( size_t i) const
			{
				return upper[i] - lower[i];
			}
		SSFEnvelope( unsigned _tightness,
			     const valarray<float>& data_in, unsigned samplerate)
		      : tightness (_tightness)
			{
				sigproc::envelope( data_in, tightness, samplerate,
						   1./samplerate,
						   lower, upper); // don't need anchor points, nor their count
			}
		SSFEnvelope() = default;
	};
	SSFEnvelope
		signal_breadth;

	struct SSFDzcdf {
		float	step,
			sigma;
		unsigned
			smooth;
		valarray<float>
			data;
		float& operator[]( size_t i)
			{
				return data[i];
			}
		SSFDzcdf( float _step, float _sigma, unsigned _smooth,
			  const valarray<float>& data_in, unsigned samplerate)
		      : step (_step), sigma (_sigma), smooth (_smooth),
			data (sigproc::dzcdf( data_in, samplerate,
					      step, sigma, smooth))
			{}
		SSFDzcdf() = default;
	};
	SSFDzcdf
		signal_dzcdf;

      // power courses
	valarray<float>
		power; // can possibly live outside in core, no?
	float	from, upto;
	float	power_display_scale;
	bool have_power() const
		{
			return power.size() > 0;
		}

	array<valarray<float>, (size_t)TBand::_total>
		power_in_bands;
	TBand	focused_band,
		uppermost_band;

      // spectrum
	valarray<float>
		spectrum;  // per page, is volatile
	float	spectrum_upper_freq;
	unsigned
		n_bins,
		last_spectrum_bin;

      // unsorted
	valarray<float>
		emg_fabs_per_page;
	float	emg_scale;

	GtkExpander
		*expander;
	GtkVBox
		*vbox;
	// GtkMenuItem
	// 	*menu_item;
	GtkDrawingArea
		*da_page,
		*da_power,
		*da_spectrum,
		*da_emg_profile;

      // draw entire page
	void draw_page( const char *fname, int width, int height); // to a file
	void draw_page(); // to own da_page

      // draw signal to a cairo_t canvas
	void draw_signal_original( unsigned width, int vdisp, cairo_t *cr)
		{
			draw_signal( signal_original, width, vdisp, cr);
		}
	void draw_signal_filtered( unsigned width, int vdisp, cairo_t *cr)
		{
			draw_signal( signal_filtered, width, vdisp, cr);
		}

      // draw arbitrary region
	void draw_signal( const valarray<float>& signal, size_t start, size_t end,
			  unsigned, int, cairo_t*);

	void mark_region_as_artifact( size_t start, size_t end, bool do_mark);

      // convenience shortcuts
	void get_signal_original()
		{
			signal_original  = recording.F().get_signal_original<const char*, float>( name);
		}
	void get_signal_filtered()
		{
			signal_filtered  = recording.F().get_signal_filtered<const char*, float>( name);
		}
	void get_power()
		{
			power = recording.power_course<float>( from, upto);
		}
	void get_spectrum( size_t p)
		{
			spectrum = recording.power_spectrum<float>( p);
		}
	void get_power_in_bands()
		{
			for ( size_t b = 0; b < (size_t)uppermost_band; ++b )
				power_in_bands[b] =
					recording.power_course<float>( settings::FreqBands[b][0],
								       settings::FreqBands[b][1]);
		}

      // ctor, dtor
	SChannelPresentation( agh::CRecording& r, SScoringFacility&);
       ~SChannelPresentation();

    private:
	SScoringFacility &sf;

	void draw_page( cairo_t*, int wd, int ht, bool draw_marquee);
	void draw_signal( const valarray<float>& signal,
			  unsigned width, int vdisp, cairo_t *cr);
	float* _resample_buffer;
	size_t _resample_buffer_size;

	static float calibrate_display_scale( const valarray<float>&, size_t over, float fit);
};




struct SScoringFacility {
	list<SChannelPresentation>
		channels;

	time_t start_time() const
		{
			return channels.front().recording.F().start_time;
		}

	vector<char>
		hypnogram;
	size_t total_pages() const
		{
//			return channels.front().recording.F().n_pages();
			return hypnogram.size();
		}
	float	scored_percent,
		scored_percent_nrem,
		scored_percent_rem,
		scored_percent_wake;

	void calculate_scored_percent()
		{
			scored_percent = channels.front().recording.F().percent_scored(
				&scored_percent_nrem,
				&scored_percent_rem,
				&scored_percent_wake);
		}

	SScoringFacility( agh::CSubject&, const string& d, const string& e);
       ~SScoringFacility();

	size_t	n_eeg_channels;

	void queue_redraw_all() const;

	bool	draw_crosshair:1,
		draw_power:1;
};


extern SScoringFacility
	*SF;

extern SChannelPresentation
	*__clicked_channel;

extern GtkWidget *__marking_in_widget;
extern double __marquee_start, __marquee_virtual_end;




//extern sigproc::CPattern
//	__pattern;

extern size_t
	__pattern_ia, __pattern_iz,
	__pattern_wd;
size_t	marquee_to_az(); // sets __pattern_i{a,z}

extern size_t
	__cur_page_app;
extern size_t
	__pagesize_item;

inline bool
pagesize_is_right()
{
	return settings::DisplayPageSizeItem == __pagesize_item;
}

inline size_t
p2ap( size_t p) // page to apparent_page
{
	return (size_t)((p) * (float)PSZ / APSZ);
}

inline size_t
ap2p( size_t p)
{
	return (size_t)((p) * (float)APSZ / PSZ);
}

} // namespace sf


extern GtkDialog
	*wFilter,
	*wPattern,
	*wPhaseDiff;
extern GtkSpinButton
	*eScoringFacCurrentPage;
extern GtkToggleButton
	*bScoringFacShowFindDialog,
	*bScoringFacShowPhaseDiffDialog;



namespace sf {
	inline void
	redraw_all()
	{
		g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
	}
}

// forward declarations of callbacks
extern "C" {

	gboolean daScoringFacPageView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
	gboolean daScoringFacPageView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacPageView_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacPageView_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
	gboolean daScoringFacPageView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	gboolean daScoringFacPSDProfileView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
	gboolean daScoringFacPSDProfileView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacPSDProfileView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	gboolean daScoringFacEMGProfileView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
	gboolean daScoringFacEMGProfileView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacEMGProfileView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	gboolean daScoringFacSpectrumView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
	gboolean daScoringFacSpectrumView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacSpectrumView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	void iSFPageSelectionInspectMany_activate_cb( GtkMenuItem*, gpointer);
}

} // namespace aghui

#endif

// eof
