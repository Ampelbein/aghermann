// ;-*-C-*- *  Time-stamp: "2011-01-14 03:08:31 hmmr"
/*
 *       File name:  ui/scoring-facility.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
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
	*wPattern,
	*eScoringFacCurrentPage,
	*bScoringFacShowFindDialog;

#define REDRAW_ALL \
	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed")


typedef struct {
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

	float	   binsize;
	GArray	  *power;
	float	   from, upto;
	float	   power_display_scale;

	GArray	  *power_in_bands;
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

	GArray    *emg_fabs_per_page;
	GtkWidget *da_emg_profile;
	gfloat     emg_scale;

	GtkWidget *expander,
		  *vbox,
		  *menu_item;

} SChannelPresentation;

extern GArray	*HH;

size_t __n_visible;


extern struct SSubject *__our_j;
extern const char *__our_d, *__our_e;

extern size_t
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


extern SChannelPresentation
	*__clicked_channel;

extern struct SSignalPatternPrimer
	__pattern;

extern size_t
	__pattern_ia, __pattern_iz;
extern size_t
	__pattern_wd;

void	__draw_signal( float *signal, size_t n_samples, float scale,
		       unsigned width, unsigned vdisp,
		       cairo_t *cr, gboolean use_resample);
void	__enumerate_patterns_to_combo();
size_t	__marquee_to_az();
void	__mark_region_as_pattern();

#endif

// EOF
