// ;-*-C++-*- *  Time-stamp: "2011-04-21 03:39:42 hmmr"
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#define REDRAW_ALL \
	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed")


using namespace std;

namespace aghui {

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




struct SChannelPresentation {
	const char
		*name,
		*type;
	agh::CRecording&
		recording;

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
		draw_spectrum_absolute:1;

      // artifacts
	float dirty_percent() const
		{
			size_t total = 0; // in samples
			auto& af = recording.F()[name].artifacts;
			for_each( af.begin(), af.end(),
				  [&total] ( const agh::CEDFFile::SSignal::TRegion& r)
				  { total += r.second - r.first; });
			return (float)total / n_samples();
		}

      // signal features
	struct SSFLowPassCourse {
		valarray<float>
			data;
		float	cutoff;
		unsigned
			order;
		float& operator[]( size_t i)
			{
				return data[i];
			}
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
	};
	SSFDzcdf
		signal_dzcdf;

      // power courses
	float	binsize;
	valarray<float>
		power;
	float	from, upto;
	float	power_display_scale;

	array<valarray<float>, (size_t)TBand::_total>
		power_in_bands;
	TBand_underlying_type
		focused_band,
		uppermost_band;

      // spectrum
	valarray<float>
		spectrum;  // per page, is volatile
	float	spectrum_max,
		spectrum_upper_freq;
	unsigned
		n_bins,
		last_spectrum_bin;

      // unsorted
	float     *emg_fabs_per_page;
	GtkWidget *da_emg_profile;
	gfloat     emg_scale;


	GtkWidget *expander,
		  *vbox,
		  *menu_item;
	GtkDrawingArea
		*da_page,
		*da_power,
		*da_spectrum;

	SChannelPresentation( agh::CRecording& r)
	      : recording (r)
		{
		}
};

extern SChannelPresentation
	*HH;

extern size_t
	__n_all_channels,
	__n_eeg_channels,
	__total_pages,
	__cur_page_app;

extern guint
	__pagesize_item;

#define PSZ  AghDisplayPageSizeValues[AghDisplayPageSizeItem]
#define APSZ AghDisplayPageSizeValues[__pagesize_item]
#define PS_IS_RIGHT (AghDisplayPageSizeItem==__pagesize_item)

#define P2AP(p)  (guint)((p) * (float)PSZ / APSZ)
#define AP2P(p)  (guint)((p) * (float)APSZ / PSZ)



extern GtkWidget *__marking_in_widget;
extern double __marquee_start, __marquee_virtual_end;


extern struct SChannelPresentation
	*__clicked_channel;

extern struct SSignalPatternPrimer
	__pattern;

extern size_t
	__pattern_ia, __pattern_iz;
extern size_t
	__pattern_wd;

float	__calibrate_display_scale( const float *signal, size_t over, float fit);
void	__draw_signal( float *signal, size_t n_samples, float scale,
		       unsigned width, unsigned vdisp,
		       cairo_t *cr, gboolean use_resample);
void	__enumerate_patterns_to_combo();
size_t	__marquee_to_az();
void	__mark_region_as_pattern();

} // namespace sf
} // namespace aghui

#endif

// EOF
