// ;-*-C-*- *  Time-stamp: "2011-03-25 02:12:59 hmmr"
/*
 *       File name:  ui/scoring-facility.h
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
#include "../libagh/iface.h"

#if HAVE_CONFIG_H
#  include <config.h>
#endif


extern GtkWidget
	*wFilter,

	*wPattern,
	*eScoringFacCurrentPage,
	*bScoringFacShowFindDialog,

	*wPhaseDiff,
	*bScoringFacShowPhaseDiffDialog;


#define REDRAW_ALL \
	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed")



struct SChannelPresentation {
	const char
	          *name,
		  *type;
	TRecRef    rec_ref;

	float	  *signal_filtered,
		  *signal_original;
	size_t	   n_samples,
		   samplerate;

	float	   signal_display_scale;
	GtkWidget *da_page;
	gboolean   draw_original_signal,
		   draw_processed_signal;

	float     *envelope_upper,
		  *envelope_lower;
	gboolean   draw_envelope;

	float	  *signal_course;
	float	   bwf_cutoff;
	unsigned   bwf_order;
	gboolean   draw_course;

	float	  *signal_breadth;
	unsigned   env_tightness;

	float	  *signal_dzcdf;
	float	   dzcdf_step,
		   dzcdf_sigma;
	size_t	   dzcdf_smooth;
	gboolean   draw_dzcdf;

	struct SUnfazer
		  *unfazers;
	guint	   n_unfazers;

	float	   low_pass_cutoff,
		   high_pass_cutoff;
	unsigned   low_pass_order,
		   high_pass_order;


	float	   binsize;
	float	  *power;
	float	   from, upto;
	float	   power_display_scale;

	float	 **power_in_bands;
	size_t     n_bands;
	gshort	   focused_band,
		   uppermost_band;
	GtkWidget *da_power;
	gboolean   draw_bands;

	float	  *spectrum,  // per page, is volatile
		   spectrum_max;
	guint	   spectrum_upper_freq;
	gushort	   n_bins,
		   last_spectrum_bin;
	GtkWidget *da_spectrum;
	gboolean   draw_spectrum_absolute;

	size_t	  *artifacts;
	size_t     n_artifacts;
	gfloat	   dirty_percent;

	float     *emg_fabs_per_page;
	GtkWidget *da_emg_profile;
	gfloat     emg_scale;

	GtkWidget *expander,
		  *vbox,
		  *menu_item;

};

extern struct SChannelPresentation
	*HH;

extern TEDFRef
	__source_ref;

extern const struct SSubject *__our_j;
extern const char *__our_d, *__our_e;

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

#endif

// EOF