// ;-*-C++-*- *  Time-stamp: "2011-07-07 14:29:15 hmmr"
/*
 *       File name:  ui/scoring-facility-da_power.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-23
 *
 *         Purpose:  scoring facility (power course)
 *
 *         License:  GPL
 */




#include <cassert>

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


// -------------------- PSD profile



// ------------- Spectrum

	gboolean
	daScoringFacSpectrumView_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
	{
		auto& Ch = *(SScoringFacility::SChannel*)userdata;

		if ( !Ch.sf.pagesize_is_right() )
			return TRUE;

		if ( !Ch.is_expanded() )
			return TRUE;

//		cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( wid));

		CwB[TColour::score_none].set_source_rgb( cr);
		cairo_rectangle( cr, 0., 0., Ch.da_spectrum_wd, Ch.da_spectrum_ht);
		cairo_fill( cr);

		guint	graph_height = settings::SFDAPowerProfileHeight - 4,
			graph_width  = settings::SFDASpectrumWidth - 14;

		// grid lines
		CwB[TColour::spectrum_grid].set_source_rgba( cr, .2);
		cairo_set_line_width( cr, .3);
		for ( size_t i = 1; i < Ch.last_spectrum_bin; ++i ) {
			cairo_move_to( cr, 12 + (float)i/Ch.last_spectrum_bin * graph_width, settings::SFDAPowerProfileHeight - 2);
			cairo_line_to( cr, 12 + (float)i/Ch.last_spectrum_bin * graph_width, 2);
		}
		cairo_stroke( cr);

		// spectrum
		CwB[TColour::spectrum].set_source_rgb( cr);
		cairo_set_line_width( cr, 1);
		size_t m;
		float factor = Ch.draw_spectrum_absolute ? 1./Ch.power_display_scale : Ch.spectrum_upper_freq/graph_height;
		printf( "Ch.draw_spectrum_absolute %d  Ch.power_display_scale %g Ch.spectrum_upper_freq %g graph_height %d\n",
			Ch.draw_spectrum_absolute, Ch.power_display_scale, Ch.spectrum_upper_freq, graph_height);
		cairo_move_to( cr,
			       12, settings::SFDAPowerProfileHeight - (2 + Ch.spectrum[0] / factor));
		for ( m = 1; m < Ch.last_spectrum_bin; ++m ) {
			cairo_line_to( cr,
				       12 + (float)(graph_width) / (Ch.last_spectrum_bin) * m,
				       settings::SFDAPowerProfileHeight
				       - (2 + Ch.spectrum[m] / factor));
		}
		cairo_stroke( cr);

		// axes
		CwB[TColour::spectrum_axes].set_source_rgb( cr);
		cairo_set_line_width( cr, .5);
		cairo_move_to( cr, 12, 2);
		cairo_line_to( cr, 12, settings::SFDAPowerProfileHeight - 2);
		cairo_line_to( cr, graph_width - 2, settings::SFDAPowerProfileHeight - 2);

		// x ticks
		m = 0;
		while ( (++m * 1e6) < graph_height * factor ) {
//			printf( "mvto %g fac %g\n", settings::SFDAPowerProfileHeight - (2 + (float)m*1e6 / factor), factor);
			cairo_move_to( cr, 6,  settings::SFDAPowerProfileHeight - (2 + (float)m*1e6 / factor));
			cairo_line_to( cr, 12, settings::SFDAPowerProfileHeight - (2 + (float)m*1e6 / factor));
		}
		cairo_stroke( cr);

		// labels
		cairo_text_extents_t extents;
		CwB[TColour::labels_sf].set_source_rgb( cr);
		cairo_set_font_size( cr, 8);

		snprintf_buf( "%g Hz", Ch.last_spectrum_bin * Ch.recording.binsize());
		cairo_text_extents( cr, __buf__, &extents);
		cairo_move_to( cr,
			       settings::SFDASpectrumWidth - extents.width - 2,
			       settings::SFDAPowerProfileHeight - 2 - extents.height - 2);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);

		snprintf_buf( "%c", Ch.draw_spectrum_absolute ? 'A' : 'R');
		cairo_move_to( cr, settings::SFDASpectrumWidth - extents.width - 3, 9);
		cairo_show_text( cr, __buf__);

		cairo_stroke( cr);
		//	cairo_destroy( cr);

		return TRUE;
	}





	gboolean
	daScoringFacSpectrumView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
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
	daScoringFacSpectrumView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
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

