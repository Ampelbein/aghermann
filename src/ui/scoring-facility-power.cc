// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-power.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-23
 *
 *         Purpose:  scoring facility power spectrum overlay
 *
 *         License:  GPL
 */




#include <cassert>

#include <cairo/cairo-svg.h>

#include "misc.hh"
#include "ui.hh"
#include "scoring-facility.hh"

using namespace std;

namespace aghui {


inline namespace {

}




// callbacks


using namespace aghui;

extern "C" {


// -------------------- PSD profile



// ------------- Spectrum

gboolean
daSFSpectrumView_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
{
	auto& Ch = *(SScoringFacility::SChannel*)userdata;

	if ( !Ch._p.pagesize_is_right() )
		return TRUE;

	if ( !Ch.is_expanded() )
		return TRUE;

//		cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( wid));


	return TRUE;
}





gboolean
daSFSpectrumView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	auto& Ch = *(SScoringFacility::SChannel*)userdata;

	switch ( event->button ) {
	case 1:
		break;
	case 2:
		Ch.draw_spectrum_absolute = !Ch.draw_spectrum_absolute;
		gtk_widget_queue_draw( wid);
	    break;
	case 3:
//		gtk_menu_popup( GTK_MENU (mSFSpectrum),  // nothing useful
//				NULL, NULL, NULL, NULL, 3, event->time);
		break;
	}

	return TRUE;
}


gboolean
daSFSpectrumView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	auto& Ch = *(SScoringFacility::SChannel*)userdata;

	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		if ( event->state & GDK_SHIFT_MASK ) {
			if ( Ch.last_spectrum_bin > 4 )
				Ch.last_spectrum_bin -= 1;
		} else
			Ch.power_display_scale /= 1.1;
	    break;
	case GDK_SCROLL_UP:
		if ( event->state & GDK_SHIFT_MASK ) {
			if ( Ch.last_spectrum_bin < Ch.n_bins )
				Ch.last_spectrum_bin += 1;
		} else
			Ch.power_display_scale *= 1.1;
	    break;
	default:
	    break;
	}

	return TRUE;
}


// -- menu items





} // extern "C"

} // namespace aghui


// eof

