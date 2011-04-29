// ;-*-C++-*- *  Time-stamp: "2011-04-28 02:04:22 hmmr"
/*
 *       File name:  ui/scoring-facility-page.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-23
 *
 *         Purpose:  scoring facility (signal page)
 *
 *         License:  GPL
 */




#include <cassert>

#include <cairo/cairo-svg.h>
#include <samplerate.h>

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

SChannelPresentation
	*__unfazer_offending_channel;
float
	__unfazer_factor = 0.1;

}

void
SChannelPresentation::draw_page( cairo_t *cr, guint wd, guint ht,
				 bool draw_marquee)
{
	guint i;

	SManagedColor used_colour;
      // background
	TScore this_page_score = pagesize_is_right() ? agh::SPage::char2score( sf.hypnogram[__cur_page]) : TScore::none;
	if ( have_power() ) {
		cairo_pattern_t *cp = cairo_pattern_create_linear( 0., 0., 0., ht);
		CwB[TColour::score_none]	  .pattern_add_color_stop_rgba( cp, 0., 1.);
		CwB[score2colour(this_page_score)].pattern_add_color_stop_rgba( cp, .5, .5);
		CwB[TColour::score_none]	  .pattern_add_color_stop_rgba( cp, 1., 1.);
		cairo_set_source( cr, cp);
		cairo_rectangle( cr, 0., 0., wd, ht);
		cairo_fill( cr);
		cairo_stroke( cr);
		cairo_pattern_destroy( cp);

		used_colour = CwB[score2colour(this_page_score)];

	      // preceding and next score-coloured fade-in and -out
		if ( pagesize_is_right() ) {
			bool pattern_set = false;
			TScore neigh_page_score;
			cp = cairo_pattern_create_linear( 0., 0., wd, 0.);
			if ( __cur_page > 0 &&
			     (neigh_page_score = agh::SPage::char2score( sf.hypnogram[__cur_page-1])) != this_page_score &&
			     neigh_page_score != TScore::none ) {
				pattern_set = true;
				CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp,     0., .7);
				CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp, 50./wd, 0.);
			}
			if ( __cur_page < recording.n_pages()-1 &&
			     (neigh_page_score = agh::SPage::char2score( sf.hypnogram[__cur_page+1])) != this_page_score &&
			     neigh_page_score != TScore::none ) {
				pattern_set = true;
				CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp, 1. - 50./wd, 0.);
				CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp, 1.         , .7);
			}
			if ( pattern_set ) {
				cairo_set_source( cr, cp);
				cairo_rectangle( cr, 0., 0., wd, ht);
				cairo_fill( cr);
				cairo_stroke( cr);
			}
			cairo_pattern_destroy( cp);
		}
	} else {
		CwB[TColour::score_none].set_source_rgba( cr, .7);
		cairo_rectangle( cr, 0., 0., wd, ht);
		cairo_fill( cr);
		cairo_stroke( cr);

		used_colour = CwB[TColour::score_none];
	}


      // waveform: signal_filtered
	bool one_signal_drawn = false;
	if ( (Ch->draw_processed_signal && __select_state == 0) ||
	     (Ch->draw_processed_signal && Ch != __clicked_channel) ) {  // only show processed signal when done with unfazing
		cairo_set_line_width( cr, .5);
		cairo_set_source_rgb( cr, signal_r, signal_g, signal_b);
		__draw_signal( &Ch->signal_filtered[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       Ch->signal_display_scale,
			       wd, ht/2, cr, __use_resample);
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cLABELS_SF].red/65536,
				      (double)__fg1__[cLABELS_SF].green/65536,
				      (double)__fg1__[cLABELS_SF].blue/65536);
		cairo_move_to( cr, wd-120, 15);
		cairo_set_font_size( cr, 10);
		snprintf_buf( "filt");
		cairo_show_text( cr, __buf__);
		one_signal_drawn = TRUE;
		cairo_stroke( cr);
	}

      // waveform: signal_original
	if ( draw_original_signal ||
	     (__select_state == SEL_UNF_CHANNEL && Ch == __clicked_channel) ) {
		if ( one_signal_drawn ) {  // attenuate the other signal
			cairo_set_line_width( cr, .3);
			cairo_set_source_rgba( cr, signal_r, signal_g, signal_b, .4);
		} else {
			cairo_set_line_width( cr, .5);
			cairo_set_source_rgb( cr, signal_r, signal_g, signal_b);
		}
		__draw_signal( &Ch->signal_original[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       Ch->signal_display_scale,
			       wd, ht/2, cr, __use_resample);
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cLABELS_SF].red/65536,
				      (double)__fg1__[cLABELS_SF].green/65536,
				      (double)__fg1__[cLABELS_SF].blue/65536);
		cairo_move_to( cr, wd-120, 25);
		cairo_set_font_size( cr, 10);
		snprintf_buf( "orig");
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}


      // dzcdf
	if ( Ch->signal_dzcdf && Ch->draw_dzcdf ) {
		float	dzcdf_display_scale,
			avg = 0.;
		for ( size_t i = __cur_page_app * Ch->samplerate * APSZ; i < (__cur_page_app+1) * Ch->samplerate * APSZ; ++i )
			avg += Ch->signal_dzcdf[i];
		avg /= (Ch->samplerate * APSZ);
		dzcdf_display_scale = ht/3 / avg;

		cairo_set_source_rgba( cr, .1, .7, .2, .2);
		cairo_set_line_width( cr, 1.5);
		__draw_signal( &Ch->signal_dzcdf[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       dzcdf_display_scale,
			       wd, ht-5, cr, __use_resample);
		cairo_stroke( cr);

		cairo_rectangle( cr, 0, ht-10, wd, ht-9);
		cairo_stroke( cr);

		// scale
		cairo_set_source_rgb( cr, 0., 0., 0.);
		cairo_set_line_width( cr, 1.5);
		cairo_move_to( cr, 20, ht-10);
		cairo_line_to( cr, 20, ht-10 - dzcdf_display_scale);
		cairo_stroke( cr);
	}

      // envelope
	if ( Ch->signal_breadth && Ch->draw_envelope ) {
		cairo_set_source_rgba( cr, .9, .1, .1, .4);
		cairo_set_line_width( cr, .3);

		__draw_signal( &Ch->envelope_upper[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       Ch->signal_display_scale,
			       wd, ht/2, cr, __use_resample);
		__draw_signal( &Ch->envelope_lower[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       Ch->signal_display_scale,
			       wd, ht/2, cr, __use_resample);

		cairo_stroke( cr);
	}

      // artifacts (changed bg)
	if ( Ch->n_artifacts ) {
		size_t	lpp = APSZ * Ch->samplerate,
			cur_page_start_s =  __cur_page_app      * lpp,
			cur_page_end_s   = (__cur_page_app + 1) * lpp;
		for ( size_t a = 0; a < Ch->n_artifacts; ++a ) {
			if ( (Ch->artifacts[a*2  ] > cur_page_start_s && Ch->artifacts[a*2  ] < cur_page_end_s) ||
			     (Ch->artifacts[a*2+1] > cur_page_start_s && Ch->artifacts[a*2+1] < cur_page_end_s) ) {
				size_t	aa = (Ch->artifacts[a*2  ] < cur_page_start_s) ? cur_page_start_s : Ch->artifacts[a*2  ],
					az = (Ch->artifacts[a*2+1] > cur_page_end_s  ) ? cur_page_end_s   : Ch->artifacts[a*2+1];
				cairo_set_source_rgba( cr,  // do some gradients perhaps?
						       (double)__fg1__[cARTIFACT].red/65536,
						       (double)__fg1__[cARTIFACT].green/65536,
						       (double)__fg1__[cARTIFACT].blue/65536,
						       .5);
				cairo_rectangle( cr,
						 (float)( aa       % lpp) / lpp * wd, ht*1./3,
						 (float)((az - aa) % lpp) / lpp * wd, ht*1./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			} else if ( Ch->artifacts[a*2  ] <= cur_page_start_s && Ch->artifacts[a*2+1] >= cur_page_end_s ) {
				cairo_set_source_rgba( cr,  // flush solid (artifact covering all page)
						       (double)__fg1__[cARTIFACT].red/65536,
						       (double)__fg1__[cARTIFACT].green/65536,
						       (double)__fg1__[cARTIFACT].blue/65536,
						       .5);
				cairo_rectangle( cr,
						 0, ht*1./3, wd, ht*1./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			} else if ( Ch->artifacts[a*2] > cur_page_end_s )  // no more artifacts up to and on current page
				break;
		}
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cLABELS_SF].red/65536,
				      (double)__fg1__[cLABELS_SF].green/65536,
				      (double)__fg1__[cLABELS_SF].blue/65536);
		cairo_move_to( cr, wd-70, ht-15);
		cairo_set_font_size( cr, 8);
		snprintf_buf( "%4.2f %% dirty", Ch->dirty_percent);
		cairo_show_text( cr, __buf__);
	}


      // ticks
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cTICKS_SF].red/65536,
			      (double)__fg1__[cTICKS_SF].green/65536,
			      (double)__fg1__[cTICKS_SF].blue/65536);

	{
		cairo_set_font_size( cr, 9);
		cairo_set_line_width( cr, .3);
		for ( i = 0; i < __pagesize_ticks[__pagesize_item]; ++i ) {
			guint tick_pos = i * APSZ / __pagesize_ticks[__pagesize_item];
			cairo_move_to( cr, i * wd / __pagesize_ticks[__pagesize_item], 0);
			cairo_line_to( cr, i * wd / __pagesize_ticks[__pagesize_item], ht);

			cairo_move_to( cr, i * wd / __pagesize_ticks[__pagesize_item] + 5, ht-2);
			snprintf_buf_ts_s( tick_pos);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}


      // labels of all kinds
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cLABELS_SF].red/65536,
			      (double)__fg1__[cLABELS_SF].green/65536,
			      (double)__fg1__[cLABELS_SF].blue/65536);
      // unfazer info
	if ( Ch->n_unfazers ) {
		g_string_assign( __ss__, "Unf: ");
		for ( i = 0; i < Ch->n_unfazers; ++i ) {
			g_string_append_printf( __ss__, "%s: %5.3f%c",
						Ch->unfazers[i].channel, Ch->unfazers[i].factor,
						(i+1 == Ch->n_unfazers) ? ' ' : ';');
		}
		cairo_set_font_size( cr, 9);
		cairo_move_to( cr, 10, ht-4);
		cairo_show_text( cr, __ss__->str);
	}

      // uV scale
	{
		cairo_set_source_rgb( cr, 0., 0., 0.);
		guint dpuV = 1 * Ch->signal_display_scale;
		cairo_set_line_width( cr, 1.5);
		cairo_move_to( cr, 10, 10);
		cairo_line_to( cr, 10, 10 + dpuV);
		cairo_stroke( cr);
		cairo_set_font_size( cr, 9);
		cairo_move_to( cr, 15, 20);
		cairo_show_text( cr, "1 mV");
		cairo_stroke( cr);
	}

      // samples per pixel
	{
		cairo_set_font_size( cr, 8);
		snprintf_buf( "%4.2f spp", (float)Ch->samplerate * APSZ / wd);
		cairo_move_to( cr, wd-40, 15);
		cairo_show_text( cr, __buf__);
	}

      // filters
	cairo_set_font_size( cr, 9);
	if ( Ch->low_pass_cutoff > 0. ) {
		snprintf_buf( "LP: %g/%u", Ch->low_pass_cutoff, Ch->low_pass_order);
		cairo_move_to( cr, wd-100, 15);
		cairo_show_text( cr, __buf__);
	}
	if ( Ch->high_pass_cutoff > 0. ) {
		snprintf_buf( "HP: %g/%u", Ch->high_pass_cutoff, Ch->high_pass_order);
		cairo_move_to( cr, wd-100, 24);
		cairo_show_text( cr, __buf__);
	}
	cairo_stroke( cr);

      // marquee
	if ( draw_marquee ) {
		float vstart = (__marquee_start < __marquee_virtual_end) ? __marquee_start : __marquee_virtual_end,
			vend = (__marquee_start < __marquee_virtual_end) ? __marquee_virtual_end : __marquee_start;
		cairo_set_source_rgba( cr, .7, .7, .7, .3);
		cairo_rectangle( cr,
				 vstart, 0,
				 vend - vstart, ht);
		cairo_fill( cr);

	      // start/end timestamp
		cairo_set_font_size( cr, 9);
		cairo_set_source_rgb( cr, 1, .1, .11);

		cairo_text_extents_t extents;
		snprintf_buf( "%5.2fs", vstart/wd * APSZ);
		cairo_text_extents( cr, __buf__, &extents);
		double ido = vstart - 3 - extents.width;
		if ( ido < 0+extents.width+3 )
			cairo_move_to( cr, vstart+3, 30);
		else
			cairo_move_to( cr, ido, 12);
		cairo_show_text( cr, __buf__);

		if ( vend - vstart > 5 ) {  // don't mark end if selection is too short
			snprintf_buf( "%5.2fs", vend/wd * APSZ);
			cairo_text_extents( cr, __buf__, &extents);
			ido = vend+extents.width+3;
			if ( ido > wd )
				cairo_move_to( cr, vend-3-extents.width, 30);
			else
				cairo_move_to( cr, vend+3, 12);
			cairo_show_text( cr, __buf__);

			snprintf_buf( "←%4.2fs→", (vend-vstart)/wd * APSZ);
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, vstart+(vend-vstart)/2 - extents.width/2,
				       extents.width < vend-vstart ? 12 : 30);
			cairo_show_text( cr, __buf__);
		}
	}
}





void
SChannelPresentation::draw_page()
{
	GtkWindow *window = gtk_widget_get_window( GTK_WIDGET (da_page));
	cairo_t *cr = gdk_cairo_create( window);

	int	ht = gdk_window_get_height( window),
		wd = gdk_window_get_width( window);

	draw_page( cr, wd, ht,
		   __marking_in_widget == da_page);

	cairo_set_line_width( cr, .3);

      // unfazer
	if ( __select_state ) {
		cairo_text_extents_t extents;
		cairo_set_font_size( cr, 15);
		if ( Ch == __clicked_channel ) {
			switch ( __select_state ) {
			case SEL_UNF_CHANNEL:
				snprintf_buf( "Unfaze this channel from...");
				gtk_widget_set_tooltip_markup( lScoringFacHint, __tooltips[AGH_TIP_UNFAZER]);
			    break;
			case SEL_UNF_CALIBRATE:
				snprintf_buf( "Unfaze this channel from %s",
					      __unfazer_offending_channel->name);
				// show the signal being set up for unfazer live
				SRC_DATA samples;
				float *s1, *s2;
				samples.data_in = &Ch->signal_original[ (samples.input_frames = Ch->samplerate * APSZ) * __cur_page_app ];
				samples.data_out = s1 = (float*)malloc( (samples.output_frames = wd) * sizeof(float));
				samples.src_ratio = (double)samples.output_frames / samples.input_frames;
				if ( src_simple( &samples, SRC_SINC_FASTEST, 1) )
					;

				samples.data_in = &__unfazer_offending_channel->signal_original[ samples.input_frames * __cur_page_app ];
				samples.data_out = s2 = (float*)malloc( samples.output_frames * sizeof(float));
				if ( src_simple( &samples, SRC_LINEAR, 1) )
					;

				cairo_move_to( cr, 0,
					       - (s1[0] - s2[0] * __unfazer_factor)
					       * Ch->signal_display_scale
					       + ht/2);
				for ( size_t i = 0; i < wd; ++i ) {
					cairo_line_to( cr, i,
						       - (s1[i] - s2[i] * __unfazer_factor)
						       * Ch->signal_display_scale
						       + ht/2);
				}
				cairo_stroke( cr);

				free( (void*)s1);
				free( (void*)s2);
				gtk_widget_set_tooltip_markup( lScoringFacHint, __tooltips[AGH_TIP_UNFAZER]);
			    break;
			}

			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, wd/2 - extents.width/2, ht-30);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);

		} else if ( Ch == __unfazer_offending_channel ) {
			switch ( __select_state ) {
			case SEL_UNF_CHANNEL:
			    break;
			case SEL_UNF_CALIBRATE:
				snprintf_buf( "Calibrating unfaze factor: %4.2f",
					      __unfazer_factor);
				cairo_text_extents( cr, __buf__, &extents);
				cairo_move_to( cr, wd/2 - extents.width/2, ht-30);
				cairo_show_text( cr, __buf__);
			    break;
			}
		}
	} else
		gtk_widget_set_tooltip_markup( lScoringFacHint, __tooltips[AGH_TIP_GENERAL]);

      // crosshair
	if ( __draw_crosshair ) {
		cairo_set_font_size( cr, 9);
		cairo_set_source_rgb( cr,
				      (double)__bg1__[cCURSOR].red/65536,
				      (double)__bg1__[cCURSOR].green/65536,
				      (double)__bg1__[cCURSOR].blue/65536);

		float t = (float)__crosshair_at/wd * APSZ;
		cairo_move_to( cr, __crosshair_at, 0);
		cairo_line_to( cr, __crosshair_at, ht);
		snprintf_buf( "(%5.2fs) %4.2f",
			      t,
			      (Ch->draw_processed_signal ? Ch->signal_filtered : Ch->signal_original)
			      [ (size_t)((__cur_page_app*APSZ + t) * Ch->samplerate) ]);
		cairo_move_to( cr, __crosshair_at+2, 12);
		cairo_show_text( cr, __buf__);
	}

	cairo_stroke( cr);
	cairo_destroy( cr);

}


void
SChannelPresentation::draw_page( const char *fname,
				 int width, int height)
{
#ifdef CAIRO_HAS_SVG_SURFACE
	cairo_surface_t *cs = cairo_svg_surface_create( fname, width, height);
	cairo_t *cr = cairo_create( cs);

	__draw_page( cr, Ch, width, height, FALSE);

	cairo_destroy( cr);
	cairo_surface_destroy( cs);
#endif

}




} // namespace sf



// callbacks



extern "C" {



// -------------------- Page

	gboolean
	daScoringFacPageView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
	{
		SChannelPresentation& Ch = *(SChannelPresentation*)userdata;
		if ( Ch.n_samples() == 0 || !gtk_expander_get_expanded( Ch.expander) )
			return TRUE;

		Ch.draw_page();

		return TRUE;
	}



	gboolean
	daScoringFacPageView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	{
		SChannelPresentation& Ch = *(SChannelPresentation*)userdata;

		int	ht = gdk_window_get_height( window),
			wd = gdk_window_get_width( window);

		guint h;

		switch ( __select_state ) {

		case SEL_UNF_CHANNEL:
			if ( event->button == 1 )
				if ( strcmp( Ch->name, __clicked_channel->name) != 0 ) {
					__unfazer_offending_channel = Ch;
					float f = agh_edf_get_unfazer_factor( __source_ref,
									      __clicked_channel->name,
									      __unfazer_offending_channel->name);
					__unfazer_factor = ( isnan(f) ) ? 0. : f;
					__select_state = SEL_UNF_CALIBRATE;
				} else {
					__select_state = 0;
				}
			else
				__select_state = 0;
			REDRAW_ALL;
			break;

		case SEL_UNF_CALIBRATE:
			if ( event->button == 1 && __clicked_channel == Ch ) {
				agh_edf_add_or_mod_unfazer( __source_ref,  // apply
							    __clicked_channel->name,
							    __unfazer_offending_channel->name,
							    __unfazer_factor);
				Ch->n_unfazers = agh_edf_get_unfazers( __source_ref,
								       Ch->name,
								       &Ch->unfazers);
				agh_msmt_get_signal_filtered_as_float( Ch->rec_ref,
								       &Ch->signal_filtered, NULL, NULL);
				if ( Ch->power )
					agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
											    Ch->from, Ch->upto,
											    Ch->power);
				__select_state = 0;

			} else if ( event->button == 2 )
				if ( event->state & GDK_CONTROL_MASK ) { // remove unfazer
					agh_edf_remove_unfazer( __source_ref,
								__clicked_channel->name,
								__unfazer_offending_channel->name);
					Ch->n_unfazers = agh_edf_get_unfazers( __source_ref,
									       Ch->name,
									       &Ch->unfazers);
					agh_msmt_get_signal_filtered_as_float( Ch->rec_ref,
									       &Ch->signal_filtered, NULL, NULL);
					if ( Ch->power )
						agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
												    Ch->from, Ch->upto,
												    Ch->power);
					__select_state = 0;
				} else
					__unfazer_factor = 0.;
			else
				__select_state = 0;

			REDRAW_ALL;
			break;

		case 0:
			switch ( event->button ) {
			case 2:
				if ( event->state & GDK_CONTROL_MASK )
					for ( h = 0; h < __n_all_channels; ++h )
						HH[h].signal_display_scale = __sane_signal_display_scale;
				else
					Ch->signal_display_scale = __sane_signal_display_scale;
				REDRAW_ALL;
				break;
			case 3:
				__clicked_channel = Ch;  // no other way to mark this channel even though user may not select Unfaze
				gtk_widget_set_sensitive( iSFPageShowDZCDF, Ch->signal_dzcdf != NULL);
				gtk_widget_set_sensitive( iSFPageShowEnvelope, Ch->signal_breadth != NULL);
				gtk_menu_popup( GTK_MENU (mSFPage),
						NULL, NULL, NULL, NULL, 3, event->time);
				break;
			case 1:
				__marking_in_widget = wid;
				__marquee_start = __marquee_virtual_end = event->x;
				gtk_widget_queue_draw( wid);
				break;
			}
		}

		return TRUE;

	}





	gboolean
	daScoringFacPageView_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	{
		SChannelPresentation& Ch = *(SChannelPresentation*)userdata;

		gint wd;
		gdk_drawable_get_size( wid->window, &wd, NULL);

		if ( wid != __marking_in_widget )
			return TRUE;

		switch ( event->button ) {
		case 1:
			if ( __marquee_virtual_end != __marquee_start ) {
				__clicked_channel = Ch;
				gtk_menu_popup( GTK_MENU (mSFPageSelection),
						NULL, NULL, NULL, NULL, 3, event->time);
			}
			break;
		case 3:
			break;
		}

		__marking_in_widget = NULL;

		return TRUE;
	}








	gboolean
	daScoringFacPageView_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
	{
		SChannelPresentation& Ch = *(SChannelPresentation*)userdata;
		int wd = gdk_window_get_width( window);

		// update marquee boundaries
		if ( __marking_in_widget == wid && __select_state == 0 )
			__marquee_virtual_end = (event->x > 0. ) ? event->x : 0;

		// update crosshair
		if ( __draw_crosshair ) {
			__crosshair_at = event->x;
			for ( guint h = 0; h < __n_all_channels; ++h ) {
				if ( gtk_expander_get_expanded( GTK_EXPANDER (HH[h].expander)) && HH[h].da_page )
					gtk_widget_queue_draw( HH[h].da_page);
			}
		} else if ( __marking_in_widget == wid )
			gtk_widget_queue_draw( wid);

		return TRUE;
	}



	gboolean
	daScoringFacPageView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	{
#define Ch ((struct SChannelPresentation*) userdata)

		if ( __select_state == SEL_UNF_CALIBRATE && Ch == __clicked_channel ) {
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				if ( fabs(__unfazer_factor) > .2 )
					__unfazer_factor -= .1;
				else
					__unfazer_factor -= .02;
				break;
			case GDK_SCROLL_UP:
				if ( fabs(__unfazer_factor) > .2 )
					__unfazer_factor += .1;
				else
					__unfazer_factor += .02;
				break;
			default:
				break;
			}
			gtk_widget_queue_draw( __clicked_channel->da_page);
			gtk_widget_queue_draw( __unfazer_offending_channel->da_page);
			return TRUE;
		}

		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			Ch->signal_display_scale /= 1.1;
			break;
		case GDK_SCROLL_UP:
			Ch->signal_display_scale *= 1.1;
			break;
		default:
			break;
		}

		if ( event->state & GDK_CONTROL_MASK ) {
			for ( guint h = 0; h < __n_all_channels; ++h )
				HH[h].signal_display_scale =
					Ch->signal_display_scale;
			gtk_widget_queue_draw( cScoringFacPageViews);
		} else
			gtk_widget_queue_draw( wid);

		return TRUE;
#undef Ch
	}






// ------ menu callbacks

// -- Page
	void
	mSFPage_show_cb( GtkWidget *widget, gpointer user_data)
	{
		gtk_check_menu_item_set_active( iSFPageShowOriginal,
						(gboolean)__clicked_channel->draw_original_signal);
		gtk_check_menu_item_set_active( iSFPageShowProcessed,
						(gboolean)__clicked_channel->draw_processed_signal);
		gtk_check_menu_item_set_active( iSFPageShowDZCDF,
						(gboolean)__clicked_channel->draw_dzcdf);
		gtk_check_menu_item_set_active( iSFPageShowEnvelope,
						(gboolean)__clicked_channel->draw_envelope);
	}


	void
	iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
	{
		__clicked_channel->draw_original_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	      // prevent both being switched off
		if ( !__clicked_channel->draw_original_signal && !__clicked_channel->draw_processed_signal )
			gtk_check_menu_item_set_active( iSFPageShowProcessed,
							(gboolean)(__clicked_channel->draw_processed_signal = true));
		gtk_widget_queue_draw( (GtkWidget*)__clicked_channel->da_page);
	}


	void
	iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
	{
		__clicked_channel->draw_processed_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
		if ( !__clicked_channel->draw_processed_signal && !__clicked_channel->draw_original_signal )
			gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (iSFPageShowOriginal),
							(gboolean)(__clicked_channel->draw_original_signal = true));
		gtk_widget_queue_draw( (GtkWidget*)__clicked_channel->da_page);
	}


	void
	iSFPageShowDZCDF_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
	{
		__clicked_channel->draw_dzcdf = (bool)gtk_check_menu_item_get_active( checkmenuitem);
		gtk_widget_queue_draw( (GtkWidget*)__clicked_channel->da_page);
	}

	void
	iSFPageShowEnvelope_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
	{
		__clicked_channel->draw_envelope = (bool)gtk_check_menu_item_get_active( checkmenuitem);
		gtk_widget_queue_draw( (GtkWidget*)__clicked_channel->da_page);
	}



	void
	iSFPageClearArtifacts_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		if ( pop_question( wScoringFacility,
				   "All marked artifacts will be lost in this channel.  Continue?") != GTK_RESPONSE_YES )
			return;

		__clicked_channel->recording.F()[__clicked_channel->name].artifacts.clear();
		__clicked_channel->get_signal_filtered();

		if ( __clicked_channel->have_power() ) {
			__clicked_channel->get_power();
			__clicked_channel->get_power_in_bands();

			gtk_widget_queue_draw( (GtkWidget*)__clicked_channel->da_power);
		}

		gtk_widget_queue_draw( (GtkWidget*)__clicked_channel->da_page);
	}


	void
	iSFPageUnfazer_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		__unfazer_mode = TUnfazerMode::channel_select;

		SF->queue_redraw_all();
	}



	void
	iSFPageSaveAs_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		int	ht = gdk_window_get_height( gtk_widget_get_window( (GtkWidget*)__clicked_channel->da_page)),
			wd = gdk_window_get_width( gtk_widget_get_window( (GtkWidget*)__clicked_channel->da_page));

		string j_dir = AghCC->subject_dir( __clicked_channel->recording.subject());
		snprintf_buf( "%s/%s/%s-p%zu@%u.svg", j_dir.c_str(), AghD(), AghT(), __cur_page_app, APSZ);
		UNIQUE_CHARP(fname);
		fname = g_strdup( __buf__);

		__clicked_channel->draw_page( fname, wd, ht);
	}


	void
	iSFPageExportSignal_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		string fname_base = __clicked_channel->recording.fname_base();
		snprintf_buf( "%s-orig.tsv", fname_base.c_str());
		__clicked_channel->recording.F().export_original( __clicked_channel->name, __buf__);
		snprintf_buf( "%s-filt.tsv", fname_base.c_str());
		__clicked_channel->recording.F().export_filtered( __clicked_channel->name, __buf__);
		snprintf_buf( "Wrote %s-{filt,orig}.tsv", fname_base.c_str());
		gtk_statusbar_pop( sbSF, sbContextIdGeneral);
		gtk_statusbar_push( sbSF, sbContextIdGeneral, __buf__);
	}



	void
	iSFPageUseThisScale_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		__sane_signal_display_scale = __clicked_channel->signal_display_scale;
		for ( auto H = SF->channels.begin(); H != SF->channels.end(); ++H ) {
			H->signal_display_scale = __sane_signal_display_scale;
			gtk_widget_queue_draw( (GtkWidget*)H->da_page);
		}
	}





} // extern "C"

} // namespace aghui


// eof

