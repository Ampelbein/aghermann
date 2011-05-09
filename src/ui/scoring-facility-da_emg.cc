// ;-*-C++-*- *  Time-stamp: "2011-05-06 15:31:18 hmmr"
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


using namespace aghui;
using namespace aghui::sf;

extern "C" {



// -------------------- EMG profile

gboolean
daScoringFacEMGProfileView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	auto& Ch = *(SScoringFacility::SChannel*)userdata;

	if ( !Ch.is_expanded() )
		return TRUE;

	cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( wid));

	CwB[TColour::score_none].set_source_rgb( cr);
	cairo_rectangle( cr, 0., 0., Ch.da_emg_profile_wd, Ch.da_emg_profile_ht);
	cairo_fill( cr);
	cairo_stroke( cr);

      // avg EMG
	CwB[TColour::emg].set_source_rgb( cr);
	cairo_set_line_width( cr, .8);
	for ( size_t i = 0; i < Ch.sf.total_pages(); ++i ) {
		cairo_move_to( cr, (double)(i+.5) / Ch.sf.total_pages() * Ch.da_emg_profile_wd,
			       settings::SFDAEMGProfileHeight/2
			       - Ch.emg_fabs_per_page[i] * Ch.emg_scale);
		cairo_line_to( cr, (double)(i+.5) / Ch.sf.total_pages() * Ch.da_emg_profile_wd,
			       settings::SFDAEMGProfileHeight/2
			       + Ch.emg_fabs_per_page[i] * Ch.emg_scale);
	}
	cairo_stroke( cr);

      // hour ticks
	CwB[TColour::ticks_sf].set_source_rgb( cr);
	cairo_set_line_width( cr, 1);

	cairo_set_font_size( cr, 7);
	float	hours = (float)Ch.recording.length_in_seconds() / 3600;
	for ( int i = 1; i < hours; ++i ) {
		guint tick_pos = (float)i / hours * Ch.da_emg_profile_wd;
		cairo_move_to( cr, tick_pos, 0);
		cairo_line_to( cr, tick_pos, 15);
		snprintf_buf( "%2uh", i);
		cairo_move_to( cr, tick_pos + 5, 9);
		cairo_show_text( cr, __buf__);
	}

      // cursor
	CwB[TColour::cursor].set_source_rgba( cr, .7);
	cairo_rectangle( cr,
			 (float) Ch.sf.cur_vpage() / Ch.sf.total_vpages() * Ch.da_emg_profile_wd,  0,
			 ceil( 1. / Ch.sf.total_pages() * Ch.da_emg_profile_wd), Ch.da_emg_profile_ht - 1);
	cairo_fill( cr);

	cairo_stroke( cr);
	cairo_destroy( cr);

	return TRUE;
}






gboolean
daScoringFacEMGProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	auto& Ch = *(SScoringFacility::SChannel*)userdata;

	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		if ( Ch.emg_scale < 2500 )
			Ch.emg_scale *= 1.1;
	    break;
	case GDK_SCROLL_DOWN:
		if ( Ch.emg_scale > .001 )
			Ch.emg_scale /= 1.1;
	    break;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}







gboolean
daScoringFacEMGProfileView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	auto& Ch = *(SScoringFacility::SChannel*) userdata;

	switch ( event->button ) {
	case 1:
		gtk_spin_button_set_value( Ch.sf.eScoringFacCurrentPage,
					   (event->x / Ch.da_emg_profile_wd) * Ch.sf.total_vpages() + 1);
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

