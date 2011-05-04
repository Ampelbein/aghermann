// ;-*-C++-*- *  Time-stamp: "2011-05-04 03:28:46 hmmr"
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
	daScoringFacPSDProfileView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
	{
		auto& Ch = *(SScoringFacility::SChannel*)userdata;
		if ( !Ch.is_expanded() )
			return TRUE;

		cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( wid));

		CwB[TColour::hypnogram].set_source_rgb( cr);
		cairo_rectangle( cr, 0., 0., Ch.da_page_wd, Ch.da_page_ht);
		cairo_fill( cr);

		guint i;

		// profile
		if ( Ch.draw_bands ) {
			cairo_set_line_width( cr, 1.);
			cairo_set_font_size( cr, 9);
			for ( TBand_underlying_type b = 0; b <= (TBand_underlying_type)Ch.uppermost_band; ++b ) {
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

				if ( (TBand)b == Ch->focused_band ) {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
					snprintf_buf( "%s %g–%g",
						      AghFreqBandsNames[b],
						      AghFreqBands[b][0], AghFreqBands[b][1]);
				} else {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
					snprintf_buf( "%s", AghFreqBandsNames[b]);
				}
				cairo_move_to( cr, wd - 70, Ch->uppermost_band*12 - 12*b + 12);
				cairo_show_text( cr, __buf__);
			}

		} else {
			cairo_set_source_rgb( cr,
					      (double)__fg1__[cPOWER_SF].red/65536,
					      (double)__fg1__[cPOWER_SF].green/65536,
					      (double)__fg1__[cPOWER_SF].blue/65536);
			cairo_move_to( cr, .5 / __total_pages * wd, Ch->power[0]);
			for ( i = 0; i < __total_pages; ++i )
				cairo_line_to( cr, (double)(i+.5) / __total_pages * wd,
					       - Ch->power[i] * Ch->power_display_scale + ht);
			cairo_line_to( cr, wd, ht);
			cairo_line_to( cr, 0., ht);
			cairo_fill( cr);

			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			snprintf_buf( "%g–%g Hz", Ch->from, Ch->upto);
			cairo_move_to( cr, wd-50, 15);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);

		// scale
		cairo_set_source_rgb( cr, 0., 0., 0.);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_line_width( cr, 1.5);
		// cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT); // is default
		cairo_move_to( cr, 10, 10);
		cairo_line_to( cr, 10, 10 + Ch->power_display_scale * 1e6);
		cairo_stroke( cr);

		cairo_move_to( cr, 15, 20);
		cairo_show_text( cr, "1 µV²/Hz");
		cairo_stroke( cr);

		// hour ticks
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cTICKS_SF].red/65536,
				      (double)__fg1__[cTICKS_SF].green/65536,
				      (double)__fg1__[cTICKS_SF].blue/65536);
		cairo_set_line_width( cr, 1);
		cairo_set_font_size( cr, 10);
		float	hours4 = (float)Ch->n_samples / Ch->samplerate / 3600 * 4;
		for ( i = 1; i < hours4; ++i ) {
			guint tick_pos = (float)i / hours4 * wd;
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
		cairo_set_source_rgba( cr,
				       (double)__bg1__[cCURSOR].red/65536,
				       (double)__bg1__[cCURSOR].green/65536,
				       (double)__bg1__[cCURSOR].blue/65536,
				       .5);
		cairo_rectangle( cr,
				 (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
				 ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht-1);
		cairo_fill( cr);

		cairo_stroke( cr);
		cairo_destroy( cr);

		return TRUE;
#undef Ch
	}









	void bScoringFacForward_clicked_cb();
	void bScoringFacBack_clicked_cb();

	gboolean
	daScoringFacPSDProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	{
#define Ch ((struct SChannelPresentation*) userdata)

		if ( event->state & GDK_SHIFT_MASK ) {
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				if ( Ch->draw_bands ) {
					if ( Ch->focused_band > 0 )
						--Ch->focused_band;
				} else
					if ( Ch->from > 0 ) {
						Ch->from--, Ch->upto--;
						agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
												    Ch->from, Ch->upto,
												    Ch->power);
					}
				break;
			case GDK_SCROLL_UP:
				if ( Ch->draw_bands ) {
					if ( Ch->focused_band < Ch->uppermost_band )
						++Ch->focused_band;
				} else
					if ( Ch->upto < 10 ) {
						Ch->from++, Ch->upto++;
						agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
												    Ch->from, Ch->upto,
												    Ch->power);
					}
				break;
			case GDK_SCROLL_LEFT:
			case GDK_SCROLL_RIGHT:
				break;
			}

		} else {
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				Ch->power_display_scale /= 1.1;
				break;
			case GDK_SCROLL_UP:
				Ch->power_display_scale *= 1.1;
				break;
			case GDK_SCROLL_LEFT:
				bScoringFacBack_clicked_cb();
				break;
			case GDK_SCROLL_RIGHT:
				bScoringFacForward_clicked_cb();
				break;
			}

			__queue_redraw_all();
		}

		gtk_widget_queue_draw( wid);

		return TRUE;
#undef Ch
	}









	gboolean
	daScoringFacPSDProfileView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	{
#define Ch ((struct SChannelPresentation*) userdata)

		gint wd, ht;
		gdk_drawable_get_size( wid->window, &wd, &ht);

		switch ( event->button ) {
		case 1:
			__cur_page = (event->x / wd) * __total_pages;
			__cur_page_app = P2AP (__cur_page);
			gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
			break;
		case 2:
			Ch->draw_bands = !Ch->draw_bands;
			gtk_widget_queue_draw( wid);
			break;
		case 3:
			__clicked_channel = Ch;
			gtk_menu_popup( GTK_MENU (mSFPower),
					NULL, NULL, NULL, NULL, 3, event->time);
			break;
		}

		return TRUE;
#undef Ch
	}






// ------------- Spectrum

	gboolean
	daScoringFacSpectrumView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
	{
#define Ch ((struct SChannelPresentation*) userdata)

		if ( !PS_IS_RIGHT )
			return TRUE;

		if ( !CH_IS_EXPANDED )
			return TRUE;

		gint wd, ht;
		gdk_drawable_get_size( wid->window, &wd, &ht);

		cairo_t *cr = gdk_cairo_create( wid->window);

		cairo_set_source_rgb( cr,
				      (double)__bg1__[cSIGNAL_SCORE_NONE].red/65536,
				      (double)__bg1__[cSIGNAL_SCORE_NONE].green/65536,
				      (double)__bg1__[cSIGNAL_SCORE_NONE].blue/65536);
		cairo_rectangle( cr, 0., 0., wd, ht);
		cairo_fill( cr);

		guint	graph_height = AghSFDAPowerProfileHeight - 4,
			graph_width  = AghSFDASpectrumWidth - 14;

		// grid lines
		cairo_set_source_rgba( cr,
				       (double)__fg1__[cSPECTRUM_GRID].red/65536,
				       (double)__fg1__[cSPECTRUM_GRID].green/65536,
				       (double)__fg1__[cSPECTRUM_GRID].blue/65536,
				       .2);
		cairo_set_line_width( cr, .3);
		for ( gushort i = 1; i < Ch->last_spectrum_bin; ++i ) {
			cairo_move_to( cr, 12 + (float)i/Ch->last_spectrum_bin * graph_width, AghSFDAPowerProfileHeight - 2);
			cairo_line_to( cr, 12 + (float)i/Ch->last_spectrum_bin * graph_width, 2);
		}
		cairo_stroke( cr);

		// spectrum
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cSPECTRUM].red/65536,
				      (double)__fg1__[cSPECTRUM].green/65536,
				      (double)__fg1__[cSPECTRUM].blue/65536);
		cairo_set_line_width( cr, 1);
		guint m;
		gfloat	factor = Ch->draw_spectrum_absolute ? 1/Ch->power_display_scale : Ch->spectrum_max/graph_height;
		cairo_move_to( cr,
			       12, AghSFDAPowerProfileHeight - (2 + Ch->spectrum[0] / factor));
		for ( m = 1; m < Ch->last_spectrum_bin; ++m ) {
			cairo_line_to( cr,
				       12 + (float)(graph_width) / (Ch->last_spectrum_bin) * m,
				       AghSFDAPowerProfileHeight
				       - (2 + Ch->spectrum[m] / factor));
		}
		cairo_stroke( cr);

		// axes
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cSPECTRUM_AXES].red/65536,
				      (double)__fg1__[cSPECTRUM_AXES].green/65536,
				      (double)__fg1__[cSPECTRUM_AXES].blue/65536);
		cairo_set_line_width( cr, .5);
		cairo_move_to( cr, 12, 2);
		cairo_line_to( cr, 12, AghSFDAPowerProfileHeight - 2);
		cairo_line_to( cr, graph_width - 2, AghSFDAPowerProfileHeight - 2);

		// x ticks
		m = 0;
		while ( (++m * 1e6) < graph_height * factor ) {
			cairo_move_to( cr, 6,  AghSFDAPowerProfileHeight - (2 + (float)m*1e6 / factor));
			cairo_line_to( cr, 12, AghSFDAPowerProfileHeight - (2 + (float)m*1e6 / factor));
		}
		cairo_stroke( cr);

		// labels
		cairo_text_extents_t extents;
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cLABELS_SF].red/65536,
				      (double)__fg1__[cLABELS_SF].green/65536,
				      (double)__fg1__[cLABELS_SF].blue/65536);
		cairo_set_font_size( cr, 8);

		snprintf_buf( "%g Hz", Ch->last_spectrum_bin * Ch->binsize);
		cairo_text_extents( cr, __buf__, &extents);
		cairo_move_to( cr,
			       AghSFDASpectrumWidth - extents.width - 2,
			       AghSFDAPowerProfileHeight - 2 - extents.height - 2);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);

		snprintf_buf( "%c", Ch->draw_spectrum_absolute ? 'A' : 'R');
		cairo_move_to( cr, AghSFDASpectrumWidth - extents.width - 3, 9);
		cairo_show_text( cr, __buf__);

		cairo_stroke( cr);
		cairo_destroy( cr);

		return TRUE;
#undef Ch
	}





	gboolean
	daScoringFacSpectrumView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	{
#define Ch ((struct SChannelPresentation*) userdata)

		switch ( event->button ) {
		case 1:
			break;
		case 2:
			Ch->draw_spectrum_absolute = !Ch->draw_spectrum_absolute;
			gtk_widget_queue_draw( wid);
			break;
		case 3:
//		gtk_menu_popup( GTK_MENU (mSFSpectrum),  // nothing useful
//				NULL, NULL, NULL, NULL, 3, event->time);
			break;
		}

		return TRUE;
#undef Ch
	}


	gboolean
	daScoringFacSpectrumView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	{
#define Ch ((struct SChannelPresentation*) userdata)

		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			if ( event->state & GDK_SHIFT_MASK ) {
				if ( Ch->last_spectrum_bin > 4 )
					Ch->last_spectrum_bin -= 1;
			} else
				Ch->power_display_scale /= 1.1;
			break;
		case GDK_SCROLL_UP:
			if ( event->state & GDK_SHIFT_MASK ) {
				if ( Ch->last_spectrum_bin < Ch->n_bins )
					Ch->last_spectrum_bin += 1;
			} else
				Ch->power_display_scale *= 1.1;
		default:
			break;
		}

		__queue_redraw_all();

		return TRUE;
#undef Ch
	}




// -- Power

	void
	iSFPowerExportRange_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		string fname_base = __clicked_channel->recording.fname_base();
		snprintf_buf( "%s_%g-%g.tsv",
			      fname_base.c_str(), __clicked_channel->from, __clicked_channel->upto);
		__clicked_channel->recording.export_tsv( __clicked_channel->from, __clicked_channel->upto,
							 __buf__);
		gtk_statusbar_pop( sbSF, sbContextIdGeneral);
		snprintf_buf( "Wrote %s_%g-%g.tsv",
			      fname_base.c_str(), __clicked_channel->from, __clicked_channel->upto);
		gtk_statusbar_push( sbSF, sbContextIdGeneral, __buf__);
	}

	void
	iSFPowerExportAll_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		string fname_base = __clicked_channel->recording.fname_base();
		snprintf_buf( "%s_%g-%g.tsv",
			      fname_base.c_str(), __clicked_channel->from, __clicked_channel->upto);
		__clicked_channel->recording.export_tsv( __buf__);
		gtk_statusbar_pop( sbSF, sbContextIdGeneral);
		snprintf_buf( "Wrote %s_%g-%g.tsv",
			      fname_base.c_str(), __clicked_channel->from, __clicked_channel->upto);
		gtk_statusbar_push( sbSF, sbContextIdGeneral, __buf__);
	}

	void
	iSFPowerUseThisScale_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		__sane_power_display_scale = __clicked_channel->power_display_scale;
		for ( auto H = SF->channels.begin(); H != SF->channels.end(); ++H )
			H->power_display_scale = __sane_power_display_scale;
		SF->queue_redraw_all();
	}





} // extern "C"

} // namespace aghui


// eof

