// ;-*-C++-*- *  Time-stamp: "2011-04-27 00:52:23 hmmr"
/*
 *       File name:  ui/scoring-facility-da_emg.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-27
 *
 *         Purpose:  scoring facility (EMG course)
 *
 *         License:  GPL
 */




#include <cairo/cairo-svg.h>

#include "misc.hh"
#include "ui.hh"
#include "settings.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {
namespace sf {


inline namespace {

}




} // namespace sf



// callbacks



extern "C" {



// -------------------- EMG profile

gboolean
daScoringFacEMGProfileView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)
	if ( !CH_IS_EXPANDED )
		return TRUE;

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	cairo_t *cr = gdk_cairo_create( wid->window);

	cairo_set_source_rgb( cr,
			      (double)__bg1__[cSIGNAL_SCORE_NONE].red/65536,
			      (double)__bg1__[cSIGNAL_SCORE_NONE].green/65536,
			      (double)__bg1__[cSIGNAL_SCORE_NONE].blue/65536);
	cairo_rectangle( cr, 0., 0., wd, ht);
	cairo_fill( cr);
	cairo_stroke( cr);

	guint i;

      // avg EMG
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cEMG].red/65536,
			      (double)__fg1__[cEMG].green/65536,
			      (double)__fg1__[cEMG].blue/65536);
	cairo_set_line_width( cr, .8);
	for ( i = 0; i < __total_pages; ++i ) {
		cairo_move_to( cr, (double)(i+.5) / __total_pages * wd,
			       AghSFDAEMGProfileHeight/2
			       - Ch->emg_fabs_per_page[i] * Ch->emg_scale);
		cairo_line_to( cr, (double)(i+.5) / __total_pages * wd,
			       AghSFDAEMGProfileHeight/2
			       + Ch->emg_fabs_per_page[i] * Ch->emg_scale);
	}
	cairo_stroke( cr);

      // hour ticks
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cTICKS_SF].red/65536,
			      (double)__fg1__[cTICKS_SF].green/65536,
			      (double)__fg1__[cTICKS_SF].blue/65536);
	cairo_set_line_width( cr, 1);

	cairo_set_font_size( cr, 7);
	float	hours = Ch->n_samples / Ch->samplerate / 3600;
	for ( i = 1; i < hours; ++i ) {
		guint tick_pos = (float)i / hours * wd;
		cairo_move_to( cr, tick_pos, 0);
		cairo_line_to( cr, tick_pos, 15);
		snprintf_buf( "%2uh", i);
		cairo_move_to( cr, tick_pos + 5, 9);
		cairo_show_text( cr, __buf__);
	}

      // cursor
	cairo_set_source_rgba( cr,
			       (double)__bg1__[cCURSOR].red/65536,
			       (double)__bg1__[cCURSOR].green/65536,
			       (double)__bg1__[cCURSOR].blue/65536,
			       .7);
	cairo_rectangle( cr,
			 (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
			 ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht-1);
	cairo_fill( cr);

	cairo_stroke( cr);
	cairo_destroy( cr);

	return TRUE;
#undef Ch
}








gboolean
daScoringFacEMGProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)

	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		if ( Ch->emg_scale < 2500 )
			Ch->emg_scale *= 1.3;
	    break;
	case GDK_SCROLL_DOWN:
		if ( Ch->emg_scale > .001 )
			Ch->emg_scale /= 1.3;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
#undef Ch
}







gboolean
daScoringFacEMGProfileView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	//SChannelPresentation *Ch = (SChannelPresentation*) userdata;

	gint wd, ht;
	gdk_drawable_get_size( wid->window, &wd, &ht);

	switch ( event->button ) {
	case 1:
		__cur_page = (event->x / wd) * __total_pages;
		__cur_page_app = P2AP (__cur_page);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	    break;
	case 2:
	    break;
	case 3:
	    break;
	}

	return TRUE;
}








} // extern "C"

} // namespace aghui


// eof

