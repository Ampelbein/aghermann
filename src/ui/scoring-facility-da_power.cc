// ;-*-C++-*- *  Time-stamp: "2011-05-30 10:44:08 hmmr"
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


	gboolean
	daScoringFacPSDProfileView_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
	{
		auto& Ch = *(SScoringFacility::SChannel*)userdata;
		if ( !Ch.is_expanded() )
			return TRUE;

//		cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( wid));

		CwB[TColour::hypnogram].set_source_rgb( cr);
		cairo_rectangle( cr, 0., 0., Ch.da_page_wd, Ch.da_page_ht);
		cairo_fill( cr);

		guint i;

	      // profile
		if ( Ch.draw_bands ) {
			cairo_set_line_width( cr, 1.);
			cairo_set_font_size( cr, 9);
			// for ( TBand_underlying_type b = 0; b <= (TBand_underlying_type)Ch.uppermost_band; ++b ) {
			// the type conversions exactly as appearing above drive g++ to segfault
			// the same happens when (TBand_underlying_type) is replaced by (unsigned short)
			for ( unsigned b = 0; b <= (unsigned)Ch.uppermost_band; ++b ) {
				CwB[(TColour)((int)TColour::band_delta + b)].set_source_rgb( cr);
				cairo_move_to( cr, .5 / Ch.sf.total_pages() * Ch.da_power_wd,
					       - Ch.power_in_bands[b][0] * Ch.power_display_scale + Ch.da_power_ht);
				for ( i = 1; i < Ch.sf.total_pages(); ++i )
					cairo_line_to( cr, (float)(i+.5) / Ch.sf.total_pages() * Ch.da_power_wd,
						       - Ch.power_in_bands[b][i] * Ch.power_display_scale + Ch.da_power_ht);
				if ( (TBand)b == Ch.focused_band ) {
					cairo_line_to( cr, Ch.da_power_wd, Ch.da_power_ht);
					cairo_line_to( cr, 0., Ch.da_power_ht);
					cairo_fill( cr);
				}
				cairo_stroke( cr);

				if ( (TBand)b == Ch.focused_band ) {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
					snprintf_buf( "%s %g–%g",
						      settings::FreqBandNames[b],
						      settings::FreqBands[b][0], settings::FreqBands[b][1]);
				} else {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
					snprintf_buf( "%s", settings::FreqBandNames[b]);
				}
				cairo_move_to( cr, Ch.da_power_wd - 70, (int)Ch.uppermost_band*12 - 12*b + 12);
				cairo_show_text( cr, __buf__);
			}

		} else {
			CwB[TColour::power_sf].set_source_rgb( cr);
			cairo_move_to( cr, .5 / Ch.sf.total_pages() * Ch.da_power_wd, Ch.power[0]);
			for ( i = 0; i < Ch.sf.total_pages(); ++i )
				cairo_line_to( cr, (double)(i+.5) / Ch.sf.total_pages() * Ch.da_power_wd,
					       - Ch.power[i] * Ch.power_display_scale + Ch.da_power_ht);
			cairo_line_to( cr, Ch.da_power_wd, Ch.da_power_ht);
			cairo_line_to( cr, 0., Ch.da_power_ht);
			cairo_fill( cr);

			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			snprintf_buf( "%g–%g Hz", Ch.from, Ch.upto);
			cairo_move_to( cr, Ch.da_power_wd-50, 15);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);

		// scale
		cairo_set_source_rgb( cr, 0., 0., 0.);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_line_width( cr, 1.5);
		// cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT); // is default
		cairo_move_to( cr, 10, 10);
		cairo_line_to( cr, 10, 10 + Ch.power_display_scale * 1e6);
		cairo_stroke( cr);

		cairo_move_to( cr, 15, 20);
		cairo_show_text( cr, "1 µV²/Hz");
		cairo_stroke( cr);

		// hour ticks
		CwB[TColour::ticks_sf].set_source_rgb( cr);
		cairo_set_line_width( cr, 1);
		cairo_set_font_size( cr, 10);
		float	hours4 = (float)Ch.n_samples() / Ch.samplerate() / 3600 * 4;
		for ( i = 1; i < hours4; ++i ) {
			guint tick_pos = (float)i / hours4 * Ch.da_power_wd;
			cairo_move_to( cr, tick_pos, 0);
			cairo_line_to( cr, tick_pos, (i%4 == 0) ? 20 : (i%2 == 0) ? 12 : 5);
			if ( i % 4 == 0 ) {
				snprintf_buf( "%2uh", i/4);
				cairo_move_to( cr, tick_pos+5, 12);
				cairo_show_text( cr, __buf__);
			}
		}
		cairo_stroke( cr);

		// cursor
		CwB[TColour::cursor].set_source_rgba( cr, .5);
		cairo_rectangle( cr,
				 (float) Ch.sf.cur_vpage() / Ch.sf.total_vpages() * Ch.da_page_wd,  0,
				 ceil( 1. / Ch.sf.total_vpages() * Ch.da_page_wd), Ch.da_page_ht-1);
		cairo_fill( cr);

		cairo_stroke( cr);
//		cairo_destroy( cr);

		return TRUE;
	}









	gboolean
	daScoringFacPSDProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	{
		auto& Ch = *(SScoringFacility::SChannel*)userdata;

		if ( event->state & GDK_SHIFT_MASK ) {
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				if ( Ch.draw_bands ) {
					if ( Ch.focused_band != TBand::delta )
						prev( Ch.focused_band);
				} else
					if ( Ch.from > 0 ) {
						Ch.from = Ch.from - .5;
						Ch.upto = Ch.upto - .5;
						Ch.get_power();
					}
				break;
			case GDK_SCROLL_UP:
				if ( Ch.draw_bands ) {
					if ( Ch.focused_band != Ch.uppermost_band )
						next( Ch.focused_band);
				} else
					if ( Ch.upto < 18. ) {
						Ch.from = Ch.from + .5;
						Ch.upto = Ch.upto + .5;
						Ch.get_power();
					}
				break;
			case GDK_SCROLL_LEFT:
			case GDK_SCROLL_RIGHT:
				break;
			}

		} else
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				Ch.power_display_scale /= 1.1;
				gtk_widget_queue_draw( (GtkWidget*)Ch.da_power);
			    break;
			case GDK_SCROLL_UP:
				Ch.power_display_scale *= 1.1;
				gtk_widget_queue_draw( (GtkWidget*)Ch.da_power);
			    break;
			case GDK_SCROLL_LEFT:
				if ( Ch.sf.cur_vpage() > 0 )
					gtk_spin_button_set_value( Ch.sf.eScoringFacCurrentPage,
								   Ch.sf.cur_vpage() - 1);
			    break;
			case GDK_SCROLL_RIGHT:
				if ( Ch.sf.cur_vpage() < Ch.sf.total_vpages() )
					gtk_spin_button_set_value( Ch.sf.eScoringFacCurrentPage,
								   Ch.sf.cur_vpage() + 1);
			    break;
			}

		return TRUE;
	}









	gboolean
	daScoringFacPSDProfileView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	{
		auto& Ch = *(SScoringFacility::SChannel*)userdata;

		switch ( event->button ) {
		case 1:
			gtk_spin_button_set_value( Ch.sf.eScoringFacCurrentPage,
						   (event->x / Ch.da_power_wd) * Ch.sf.total_vpages() + 1);
			// will eventually call set_cur_vpage(), which will do redraw
		    break;
		case 2:
			Ch.draw_bands = !Ch.draw_bands;
			gtk_widget_queue_draw( wid);
		    break;
		case 3:
			Ch.sf.using_channel = &Ch;
			gtk_menu_popup( Ch.sf.mSFPower,
					NULL, NULL, NULL, NULL, 3, event->time);
		    break;
		}

		return TRUE;
	}






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

	void
	iSFPowerExportRange_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;

		string fname_base = SF.using_channel->recording.fname_base();
		snprintf_buf( "%s_%g-%g.tsv",
			      fname_base.c_str(), SF.using_channel->from, SF.using_channel->upto);
		SF.using_channel->recording.export_tsv( SF.using_channel->from, SF.using_channel->upto,
							__buf__);
		gtk_statusbar_pop( SF.sbSF, sb::sbContextIdGeneral);
		snprintf_buf( "Wrote %s_%g-%g.tsv",
			      fname_base.c_str(), SF.using_channel->from, SF.using_channel->upto);
		gtk_statusbar_push( SF.sbSF, sb::sbContextIdGeneral, __buf__);
	}

	void
	iSFPowerExportAll_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;

		string fname_base = SF.using_channel->recording.fname_base();
		snprintf_buf( "%s_%g-%g.tsv",
			      fname_base.c_str(), SF.using_channel->from, SF.using_channel->upto);
		SF.using_channel->recording.export_tsv( __buf__);
		gtk_statusbar_pop( SF.sbSF, sb::sbContextIdGeneral);
		snprintf_buf( "Wrote %s_%g-%g.tsv",
			      fname_base.c_str(), SF.using_channel->from, SF.using_channel->upto);
		gtk_statusbar_push( SF.sbSF, sb::sbContextIdGeneral, __buf__);
	}

	void
	iSFPowerUseThisScale_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;

		SF.sane_power_display_scale = SF.using_channel->power_display_scale;
		for_each( SF.channels.begin(), SF.channels.end(),
			  [&] ( SScoringFacility::SChannel& H)
			  {
				  H.power_display_scale = SF.sane_power_display_scale;
			  });
		SF.queue_redraw_all();
	}





} // extern "C"

} // namespace aghui


// eof

