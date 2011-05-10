// ;-*-C++-*- *  Time-stamp: "2011-05-11 01:16:43 hmmr"
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
#include "settings.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {
namespace sf {


inline namespace {

	unsigned short __pagesize_ticks[] = {
		5, 5, 3, 4, 6, 12, 24, 30
	};

}

void
SScoringFacility::SChannel::draw_page( cairo_t *cr, int wd, int ht,
				       bool draw_marquee)
{
	SManagedColor used_colour;
      // background
	TScore this_page_score = sf.pagesize_is_right() ? sf.cur_page_score() : TScore::none;
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
		if ( sf.pagesize_is_right() ) {
			bool pattern_set = false;
			TScore neigh_page_score;
			cp = cairo_pattern_create_linear( 0., 0., wd, 0.);
			if ( sf.cur_page() > 0 &&
			     (neigh_page_score = agh::SPage::char2score( sf.hypnogram[sf.cur_page()-1])) != this_page_score &&
			     neigh_page_score != TScore::none ) {
				pattern_set = true;
				CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp,     0., .7);
				CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp, 50./wd, 0.);
			}
			if ( sf.cur_page() < recording.n_pages()-1 &&
			     (neigh_page_score = agh::SPage::char2score( sf.hypnogram[sf.cur_page()+1])) != this_page_score &&
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
	if ( (draw_processed_signal && sf.unfazer_mode == SScoringFacility::TUnfazerMode::none) ||
	     (draw_processed_signal && this != sf.using_channel) ) {  // only show processed signal when done with unfazing
		cairo_set_line_width( cr, .5);
		used_colour.set_source_rgb( cr);
		draw_signal_filtered( wd, ht/2, cr);

		CwB[TColour::labels_sf].set_source_rgb( cr);
		cairo_move_to( cr, wd-120, 15);
		cairo_set_font_size( cr, 10);
		snprintf_buf( "filt");
		cairo_show_text( cr, __buf__);
		one_signal_drawn = true;
		cairo_stroke( cr);
	}

      // waveform: signal_original
	if ( draw_original_signal ||
	     (sf.unfazer_mode == SScoringFacility::TUnfazerMode::channel_select && this == sf.using_channel) ) {
		if ( one_signal_drawn ) {  // attenuate the other signal
			cairo_set_line_width( cr, .3);
			used_colour.set_source_rgba( cr, .4);
		} else {
			cairo_set_line_width( cr, .5);
			used_colour.set_source_rgb( cr);
		}
		draw_signal_original( wd, ht/2, cr);

		CwB[TColour::labels_sf].set_source_rgb( cr);
		cairo_move_to( cr, wd-120, 25);
		cairo_set_font_size( cr, 10);
		snprintf_buf( "orig");
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}


      // // dzcdf
      // 	if ( draw_dzcdf && signal_dzcdf.data.size() > 0 ) {
      // 		float	dzcdf_display_scale,
      // 			avg = valarray<float> (signal_dzcdf.data[ slice (sf.cur_vpage_start() * samplerate(),
      // 									 sf.pagesize() * samplerate(), 1) ]) . sum()
      // 				/ sf.pagesize() * samplerate();
      // 		dzcdf_display_scale = (float)ht/3 / avg;

      // 		cairo_set_source_rgba( cr, .1, .7, .2, .2);
      // 		cairo_set_line_width( cr, 1.5);
      // 		::draw_signal( signal_dzcdf.data,
      // 			       sf.cur_vpage_start() * samplerate(), sf.cur_vpage_end() * samplerate(),
      // 			       wd, ht-5, dzcdf_display_scale, cr, use_resample);
      // 		cairo_stroke( cr);

      // 		cairo_rectangle( cr, 0, ht-10, wd, ht-9);
      // 		cairo_stroke( cr);

      // 		// scale
      // 		cairo_set_source_rgb( cr, 0., 0., 0.);
      // 		cairo_set_line_width( cr, 1.5);
      // 		cairo_move_to( cr, 20, ht-10);
      // 		cairo_line_to( cr, 20, ht-10 - dzcdf_display_scale);
      // 		cairo_stroke( cr);
      // 	}

      // // envelope
      // 	if ( draw_envelope && signal_envelope.upper.size() ) {
      // 		cairo_set_source_rgba( cr, .9, .1, .1, .4);
      // 		cairo_set_line_width( cr, .3);

      // 		::draw_signal( signal_breadth.upper,
      // 			       sf.cur_vpage_start() * samplerate(), sf.cur_vpage_end() * samplerate(),
      // 			       wd, ht/2, signal_display_scale, cr, use_resample);
      // 		::draw_signal( signal_breadth.lower,
      // 			       sf.cur_vpage_start() * samplerate(), sf.cur_vpage_end() * samplerate(),
      // 			       wd, ht/2, signal_display_scale, cr, use_resample);

      // 		cairo_stroke( cr);
      // 	}

      // artifacts (changed bg)
	auto& Aa = recording.F()[name].artifacts;
	if ( not Aa.empty() ) {
		size_t	lpp = sf.vpagesize() * samplerate(),
			cur_page_start_s = sf.cur_vpage_start() * samplerate(),
			cur_page_end_s   = sf.cur_vpage_end()   * samplerate();
		for ( auto A = Aa.begin(); A != Aa.end(); ++A ) {
			if ( (A->first  > cur_page_start_s && A->first  < cur_page_end_s) ||
			     (A->second > cur_page_start_s && A->second < cur_page_end_s) ) {
				size_t	aa = (A->first  < cur_page_start_s) ? cur_page_start_s : A->first,
					az = (A->second > cur_page_end_s  ) ? cur_page_end_s   : A->second;
				CwB[TColour::artifact].set_source_rgba( cr,  // do some gradients perhaps?
									.5);
				cairo_rectangle( cr,
						 (float)( aa       % lpp) / lpp * wd, ht*1./3,
						 (float)((az - aa) % lpp) / lpp * wd, ht*1./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			} else if ( A->first <= cur_page_start_s && A->second >= cur_page_end_s ) {
				CwB[TColour::artifact].set_source_rgba( cr,  // flush solid (artifact covering all page)
									.5);
				cairo_rectangle( cr,
						 0, ht*1./3, wd, ht*1./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			} else if ( A->first > cur_page_end_s )  // no more artifacts up to and on current page
				break;
		}
		CwB[TColour::labels_sf].set_source_rgb( cr);
		cairo_move_to( cr, wd-70, ht-15);
		cairo_set_font_size( cr, 8);
		snprintf_buf( "%4.2f %% dirty", percent_dirty);
		cairo_show_text( cr, __buf__);
	}


      // ticks
	CwB[TColour::ticks_sf].set_source_rgb( cr);

	{
		cairo_set_font_size( cr, 9);
		cairo_set_line_width( cr, .3);
		for ( size_t i = 0; i < __pagesize_ticks[sf.pagesize_item]; ++i ) {
			guint tick_pos = i * sf.vpagesize() / __pagesize_ticks[sf.pagesize_item];
			cairo_move_to( cr, i * wd / __pagesize_ticks[sf.pagesize_item], 0);
			cairo_line_to( cr, i * wd / __pagesize_ticks[sf.pagesize_item], ht);

			cairo_move_to( cr, i * wd / __pagesize_ticks[sf.pagesize_item] + 5, ht-2);
			snprintf_buf_ts_s( tick_pos);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}


      // labels of all kinds
	CwB[TColour::labels_sf].set_source_rgb( cr);
      // unfazer info
	auto& Uu = recording.F()[name].interferences;
	if ( not Uu.empty() ) {
		g_string_assign( __ss__, "Unf: ");
		for ( auto U = Uu.begin(); U != Uu.end(); ++U ) {
			g_string_append_printf( __ss__, "%s: %5.3f%c",
						recording.F()[U->first].channel.c_str(), U->second,
						(next(U) == Uu.end()) ? ' ' : ';');
		}
		cairo_set_font_size( cr, 9);
		cairo_move_to( cr, 10, ht-4);
		cairo_show_text( cr, __ss__->str);
	}

      // uV scale
	{
		cairo_set_source_rgb( cr, 0., 0., 0.);
		guint dpuV = 1 * signal_display_scale;
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
		snprintf_buf( "%4.2f spp", (float)samplerate() * sf.vpagesize() / wd);
		cairo_move_to( cr, wd-40, 15);
		cairo_show_text( cr, __buf__);
	}

      // filters
	cairo_set_font_size( cr, 9);
	if ( low_pass.cutoff > 0. ) {
		snprintf_buf( "LP: %g/%u", low_pass.cutoff, low_pass.order);
		cairo_move_to( cr, wd-100, 15);
		cairo_show_text( cr, __buf__);
	}
	if ( high_pass.cutoff > 0. ) {
		snprintf_buf( "HP: %g/%u", high_pass.cutoff, high_pass.order);
		cairo_move_to( cr, wd-100, 24);
		cairo_show_text( cr, __buf__);
	}
	cairo_stroke( cr);

      // marquee
	if ( draw_marquee && sf.selection_size() > 0 ) {
		float vstart = (sf.marquee_start < sf.marquee_virtual_end) ? sf.marquee_start : sf.marquee_virtual_end,
			vend = (sf.marquee_start < sf.marquee_virtual_end) ? sf.marquee_virtual_end : sf.marquee_start;
		cairo_set_source_rgba( cr, .7, .7, .7, .3);
		cairo_rectangle( cr,
				 vstart, 0,
				 vend - vstart, ht);
		cairo_fill( cr);

	      // start/end timestamp
		cairo_set_font_size( cr, 9);
		cairo_set_source_rgb( cr, 1, .1, .11);

		cairo_text_extents_t extents;
		snprintf_buf( "%5.2fs", vstart/wd * sf.vpagesize());
		cairo_text_extents( cr, __buf__, &extents);
		double ido = vstart - 3 - extents.width;
		if ( ido < 0+extents.width+3 )
			cairo_move_to( cr, vstart+3, 30);
		else
			cairo_move_to( cr, ido, 12);
		cairo_show_text( cr, __buf__);

		if ( vend - vstart > 5 ) {  // don't mark end if selection is too short
			snprintf_buf( "%5.2fs", vend/wd * sf.vpagesize());
			cairo_text_extents( cr, __buf__, &extents);
			ido = vend+extents.width+3;
			if ( ido > wd )
				cairo_move_to( cr, vend-3-extents.width, 30);
			else
				cairo_move_to( cr, vend+3, 12);
			cairo_show_text( cr, __buf__);

			snprintf_buf( "←%4.2fs→", (vend-vstart)/wd * sf.vpagesize());
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, vstart+(vend-vstart)/2 - extents.width/2,
				       extents.width < vend-vstart ? 12 : 30);
			cairo_show_text( cr, __buf__);
		}
	}
}





void
SScoringFacility::SChannel::draw_page()
{
//	using SScoringFacility;

	cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( (GtkWidget*)da_page));

	draw_page( cr, da_page_wd, da_page_ht,
		   sf.marking_in_widget == da_page);

	cairo_set_line_width( cr, .3);

      // unfazer
	if ( sf.unfazer_mode != TUnfazerMode::none ) {
		cairo_text_extents_t extents;
		cairo_set_font_size( cr, 15);
		if ( this == sf.using_channel ) {
			switch ( sf.unfazer_mode ) {
			case SScoringFacility::TUnfazerMode::channel_select:
				snprintf_buf( "Unfaze this channel from...");
				sf.set_tooltip( SScoringFacility::TTipIdx::unfazer);
			    break;
			case SScoringFacility::TUnfazerMode::calibrate:
				snprintf_buf( "Unfaze this channel from %s",
					      sf.unfazer_offending_channel->name);
				// show the signal being set up for unfazer live
				SRC_DATA samples;
				float *s1, *s2;
				samples.data_in = &signal_original[ (samples.input_frames = samplerate() * sf.vpagesize()) * sf.cur_vpage() ];
				samples.data_out = s1 = (float*)malloc( (samples.output_frames = da_page_wd) * sizeof(float));
				samples.src_ratio = (double)samples.output_frames / samples.input_frames;
				if ( src_simple( &samples, SRC_SINC_FASTEST, 1) )
					;

				samples.data_in = &sf.unfazer_offending_channel->signal_original[ samples.input_frames * sf.cur_vpage() ];
				samples.data_out = s2 = (float*)malloc( samples.output_frames * sizeof(float));
				if ( src_simple( &samples, SRC_LINEAR, 1) )
					;

				cairo_move_to( cr, 0,
					       - (s1[0] - s2[0] * sf.unfazer_factor)
					       * signal_display_scale
					       + da_page_ht/2);
				for ( int i = 0; i < da_page_wd; ++i ) {
					cairo_line_to( cr, i,
						       - (s1[i] - s2[i] * sf.unfazer_factor)
						       * signal_display_scale
						       + da_page_ht/2);
				}
				cairo_stroke( cr);

				free( (void*)s1);
				free( (void*)s2);
				sf.set_tooltip( SScoringFacility::TTipIdx::unfazer);
			    break;
			case SScoringFacility::TUnfazerMode::none:
			    break;
			}

			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, da_page_wd/2 - extents.width/2, da_page_ht-30);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);

		} else if ( this == sf.unfazer_offending_channel ) {
			switch ( sf.unfazer_mode ) {
			case SScoringFacility::TUnfazerMode::channel_select:
			    break;
			case SScoringFacility::TUnfazerMode::calibrate:
				snprintf_buf( "Calibrating unfaze factor: %4.2f",
					      sf.unfazer_factor);
				cairo_text_extents( cr, __buf__, &extents);
				cairo_move_to( cr, da_page_wd/2 - extents.width/2, da_page_ht-30);
				cairo_show_text( cr, __buf__);
			    break;
			case SScoringFacility::TUnfazerMode::none:
			    break;
			}
		}
	} else
		sf.set_tooltip( SScoringFacility::TTipIdx::general);

      // crosshair
	if ( sf.draw_crosshair ) {
		cairo_set_font_size( cr, 9);
		CwB[TColour::cursor].set_source_rgb( cr);

		float t = (float)sf.crosshair_at/da_page_wd * sf.vpagesize();
		cairo_move_to( cr, sf.crosshair_at, 0);
		cairo_line_to( cr, sf.crosshair_at, da_page_ht);
		snprintf_buf( "(%5.2fs) %4.2f",
			      t,
			      (draw_processed_signal ? signal_filtered : signal_original)
			      [ (size_t)((sf.cur_vpage_start() + t) * samplerate()) ]);
		cairo_move_to( cr, sf.crosshair_at+2, 12);
		cairo_show_text( cr, __buf__);
	}

	cairo_stroke( cr);
	cairo_destroy( cr);

}






} // namespace sf



// callbacks




using namespace aghui;
using namespace aghui::sf;

extern "C" {



// -------------------- Page

	gboolean
	daScoringFacPageView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
	{
		SScoringFacility::SChannel& Ch = *(SScoringFacility::SChannel*)userdata;
		if ( Ch.n_samples() == 0 || !gtk_expander_get_expanded( Ch.expander) )
			return TRUE;

		Ch.draw_page();

		return TRUE;
	}



	gboolean
	daScoringFacPageView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	{
		auto& Ch = *(SScoringFacility::SChannel*)userdata;

		switch ( Ch.sf.unfazer_mode ) {

		case SScoringFacility::TUnfazerMode::channel_select:
			if ( event->button == 1 )
				if ( &Ch != Ch.sf.using_channel ) {
					// using_channel is here the one affected
					Ch.sf.unfazer_offending_channel = &Ch;
					// get existing f value for this pair if any
					float f = Ch.sf.using_channel->ssignal().interferences[Ch.h()];  // don't forget to erase it if user cancels unfazer calibration
					Ch.sf.unfazer_factor = (f == 0.) ? 0.1 : f;
					Ch.sf.unfazer_mode = SScoringFacility::TUnfazerMode::calibrate;
				} else
					// cancel
					Ch.sf.unfazer_mode = SScoringFacility::TUnfazerMode::none;
			else // also cancel
				Ch.sf.unfazer_mode = SScoringFacility::TUnfazerMode::none;
			Ch.draw_page();
			Ch.sf.using_channel->draw_page();
			break;

		case SScoringFacility::TUnfazerMode::calibrate:
			if ( event->button == 1 && Ch.sf.unfazer_offending_channel == &Ch ) {
				// confirm and apply
				Ch.sf.using_channel->ssignal().interferences[ Ch.sf.unfazer_offending_channel->h() ]
					= Ch.sf.unfazer_factor;

			} else if ( event->button == 2 && Ch.sf.unfazer_offending_channel == &Ch ) {
				// cancel

			} else if ( event->button == 2 && Ch.sf.using_channel == &Ch ) {
				// remove some unfazer(s)
				if ( event->state & GDK_CONTROL_MASK )
					// remove all unfazers on using_channel
					Ch.sf.using_channel->ssignal().interferences.clear();
				else
					// remove one currently being calibrated
					Ch.sf.using_channel->ssignal().interferences.erase( Ch.sf.unfazer_offending_channel->h());
				Ch.sf.using_channel->get_signal_filtered();
				if ( Ch.sf.using_channel->have_power() )
					Ch.sf.using_channel->get_power();
			}
			Ch.sf.unfazer_mode = SScoringFacility::TUnfazerMode::none;

			Ch.sf.using_channel->get_signal_filtered();
			if ( Ch.sf.using_channel->have_power() )
				Ch.sf.using_channel->get_power();

			Ch.sf.using_channel->draw_page();
			Ch.sf.unfazer_offending_channel->draw_page();

			Ch.sf.unfazer_offending_channel = NULL;
		      break;

		case SScoringFacility::TUnfazerMode::none:
			switch ( event->button ) {
			case 2:
				if ( event->state & GDK_CONTROL_MASK )
					for ( auto H = Ch.sf.channels.begin(); H != Ch.sf.channels.end(); ++H ) {
						H->signal_display_scale = Ch.sf.sane_signal_display_scale;
						H->draw_page();
					}
				else {
					Ch.signal_display_scale = Ch.sf.sane_signal_display_scale;
					Ch.draw_page();
				}
			    break;
			case 3:
				Ch.sf.using_channel = &Ch;  // no other way to mark this channel even though user may not intend to Unfaze
				// gtk_widget_set_sensitive( (GtkWidget*)Ch.sf.iSFPageShowDZCDF, Ch.have_sa_features());
				// gtk_widget_set_sensitive( (GtkWidget*)Ch.sf.iSFPageShowEnvelope, Ch.have_sa_features());
				gtk_menu_popup( Ch.sf.mSFPage,
						NULL, NULL, NULL, NULL, 3, event->time);
			    break;
			case 1:
				Ch.sf.marking_in_widget = (GtkDrawingArea*)wid;
				Ch.sf.marquee_start = Ch.sf.marquee_virtual_end = event->x;
				Ch.draw_page();
			    break;
			}
		}

		return TRUE;
	}





	gboolean
	daScoringFacPageView_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	{
		auto& Ch = *(SScoringFacility::SChannel*)userdata;

		if ( wid != (GtkWidget*)Ch.sf.marking_in_widget )
			return TRUE;

		switch ( event->button ) {
		case 1:
			if ( Ch.sf.marquee_virtual_end != Ch.sf.marquee_start ) {
				Ch.sf.using_channel = &Ch;
				gtk_menu_popup( Ch.sf.mSFPageSelection,
						NULL, NULL, NULL, NULL, 3, event->time);
			}
			break;
		case 3:
			break;
		}

		Ch.sf.marking_in_widget = NULL;

		return TRUE;
	}








	gboolean
	daScoringFacPageView_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
	{
		auto& Ch = *(SScoringFacility::SChannel*)userdata;

		// update marquee boundaries
		if ( (GtkWidget*)Ch.sf.marking_in_widget == wid && Ch.sf.unfazer_mode == SScoringFacility::TUnfazerMode::none )
			Ch.sf.marquee_virtual_end = (event->x > 0. ) ? event->x : 0;

		// update crosshair
		if ( Ch.sf.draw_crosshair ) {
			Ch.sf.crosshair_at = event->x;
			for_each ( Ch.sf.channels.begin(), Ch.sf.channels.end(),
				   [] (SScoringFacility::SChannel& H)
				   {
					   H.draw_page();
				   });
		} else if ( (GtkWidget*)Ch.sf.marking_in_widget == wid )
			Ch.draw_page();

		return TRUE;
	}



	gboolean
	daScoringFacPageView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	{
		auto& Ch = *(SScoringFacility::SChannel*)userdata;

		if ( Ch.sf.unfazer_mode == SScoringFacility::TUnfazerMode::calibrate && &Ch == Ch.sf.using_channel ) {
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				if ( fabs( Ch.sf.unfazer_factor) > .2 )
					Ch.sf.unfazer_factor -= .1;
				else
					Ch.sf.unfazer_factor -= .02;
				break;
			case GDK_SCROLL_UP:
				if ( fabs( Ch.sf.unfazer_factor) > .2 )
					Ch.sf.unfazer_factor += .1;
				else
					Ch.sf.unfazer_factor += .02;
				break;
			default:
				break;
			}
			Ch.sf.using_channel->draw_page();
			Ch.sf.unfazer_offending_channel->draw_page();

			return TRUE;
		}

		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			Ch.signal_display_scale /= 1.1;
		      break;
		case GDK_SCROLL_UP:
			Ch.signal_display_scale *= 1.1;
		      break;
		default:
		      break;
		}

		if ( event->state & GDK_CONTROL_MASK )
			for_each( Ch.sf.channels.begin(), Ch.sf.channels.end(),
				  [&] ( SScoringFacility::SChannel& H)
				  {
					  H.signal_display_scale = Ch.signal_display_scale;
					  H.draw_page();
				  });
		else
			Ch.draw_page();

		return TRUE;
	}






// ------ menu callbacks

// -- Page
	void
	mSFPage_show_cb( GtkWidget *widget, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		gtk_check_menu_item_set_active( SF.iSFPageShowOriginal,
						(gboolean)SF.using_channel->draw_original_signal);
		gtk_check_menu_item_set_active( SF.iSFPageShowProcessed,
						(gboolean)SF.using_channel->draw_processed_signal);
		// gtk_check_menu_item_set_active( SF.iSFPageShowDZCDF,
		// 				(gboolean)SF.using_channel->draw_dzcdf);
		// gtk_check_menu_item_set_active( SF.iSFPageShowEnvelope,
		// 				(gboolean)SF.using_channel->draw_envelope);
	}


	void
	iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.using_channel->draw_original_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	      // prevent both being switched off
		if ( !SF.using_channel->draw_original_signal && !SF.using_channel->draw_processed_signal )
			gtk_check_menu_item_set_active( SF.iSFPageShowProcessed,
							(gboolean)SF.using_channel->draw_processed_signal);
		SF.using_channel->draw_page();
	}


	void
	iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.using_channel->draw_processed_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
		if ( !SF.using_channel->draw_processed_signal && !SF.using_channel->draw_original_signal )
			gtk_check_menu_item_set_active( SF.iSFPageShowOriginal,
							(gboolean)(SF.using_channel->draw_original_signal));
		SF.using_channel->draw_page();
	}


	// void
	// iSFPageShowDZCDF_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
	// {
	// 	auto& SF = *(SScoringFacility*)userdata;
	// 	SF.using_channel->draw_dzcdf = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	// 	SF.using_channel->draw_page();
	// }

	// void
	// iSFPageShowEnvelope_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
	// {
	// 	auto& SF = *(SScoringFacility*)userdata;
	// 	SF.using_channel->draw_envelope = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	// 	SF.using_channel->draw_page();
	// }



	void
	iSFPageClearArtifacts_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( pop_question( SF.wScoringFacility,
				   "All marked artifacts will be lost in this channel.  Continue?") != GTK_RESPONSE_YES )
			return;

		SF.using_channel->ssignal().artifacts.clear();
		SF.using_channel->get_signal_filtered();

		if ( SF.using_channel->have_power() ) {
			SF.using_channel->get_power();
			SF.using_channel->get_power_in_bands();
			gtk_widget_queue_draw( (GtkWidget*)SF.using_channel->da_power);
		}

		SF.using_channel->draw_page();
	}


	void
	iSFPageUnfazer_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.unfazer_mode = SScoringFacility::TUnfazerMode::channel_select;
		SF.using_channel->draw_page();
	}



	void
	iSFPageSaveAs_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		string j_dir = AghCC->subject_dir( SF.using_channel->recording.subject());
		snprintf_buf( "%s/%s/%s-p%zu@%zu.svg", j_dir.c_str(), AghD(), AghT(), SF.cur_vpage(), SF.vpagesize());
		UNIQUE_CHARP(fname);
		fname = g_strdup( __buf__);

		SF.using_channel->draw_page( fname, SF.using_channel->da_page_wd, SF.using_channel->da_page_ht);
	}


	void
	iSFPageExportSignal_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		auto& r = SF.using_channel->recording;
		string fname_base = r.fname_base();
		snprintf_buf( "%s-orig.tsv", fname_base.c_str());
		r.F().export_original( SF.using_channel->name, __buf__);
		snprintf_buf( "%s-filt.tsv", fname_base.c_str());
		r.F().export_filtered( SF.using_channel->name, __buf__);
		snprintf_buf( "Wrote %s-{filt,orig}.tsv", fname_base.c_str());
		gtk_statusbar_pop( SF.sbSF, sbContextIdGeneral);
		gtk_statusbar_push( SF.sbSF, sbContextIdGeneral, __buf__);
	}



	void
	iSFPageUseThisScale_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.sane_signal_display_scale = SF.using_channel->signal_display_scale;
		for_each( SF.channels.begin(), SF.channels.end(),
			  [&] ( SScoringFacility::SChannel& H)
			  {
				  H.signal_display_scale = SF.sane_signal_display_scale;
				  H.draw_page();
			  });
	}





} // extern "C"

} // namespace aghui


// eof

