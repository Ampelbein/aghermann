// ;-*-C++-*- *  Time-stamp: "2011-06-22 03:01:40 hmmr"
/*
 *       File name:  ui/scoring-facility-montage.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-23
 *
 *         Purpose:  scoring facility: montage drawing area
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
SScoringFacility::SChannel::draw_signal( const valarray<float>& signal,
					 unsigned width, int vdisp, cairo_t *cr) const
{
	size_t	start = sf.cur_vpage_start() * samplerate(),
		end   = sf.cur_vpage_end()   * samplerate(),
		run = end - start,
		half_pad = run * sf.skirting_run_per1;
	if ( start == 0 ) {
		valarray<float> padded (run + half_pad*2);
		padded[ slice(half_pad, run + half_pad, 1) ] = signal[ slice (0, run + half_pad, 1) ];
		::draw_signal( padded, 0, padded.size(),
			       width, vdisp, signal_display_scale, cr,
			       use_resample);

	} else if ( end > n_samples() ) {  // rather ensure more thorough padding
		valarray<float> padded (run + half_pad*2);
		size_t remainder = n_samples() - start;
		padded[ slice(0, 1, remainder) ] = signal[ slice (start-half_pad, 1, remainder) ];
		::draw_signal( padded, 0, padded.size(),
			       width, vdisp, signal_display_scale, cr,
			       use_resample);

	} else {
		::draw_signal( signal,
			       start - half_pad,
			       end + half_pad,
			       width, vdisp, signal_display_scale, cr,
			       use_resample);
	}
}



void
SScoringFacility::SChannel::draw_page_static( cairo_t *cr,
					      int wd, int y0,
					      bool draw_marquee)
{
      // waveform: signal_filtered
	bool one_signal_drawn = false;
	if ( draw_filtered_signal ) {
		// only show processed signal when done with unfazing
		cairo_set_line_width( cr, .5);
		cairo_set_source_rgb( cr, 0., 0., 0.); // bg is uniformly light shades

		draw_signal_filtered( wd, y0, cr);

		CwB[TColour::labels_sf].set_source_rgb( cr);
		cairo_move_to( cr, wd-120, y0 - 15);
		cairo_set_font_size( cr, 10);
		snprintf_buf( "filt");
		cairo_show_text( cr, __buf__);
		one_signal_drawn = true;
		cairo_stroke( cr);
	}

      // waveform: signal_original
	if ( draw_original_signal ) {
		if ( one_signal_drawn ) {  // attenuate the other signal
			cairo_set_line_width( cr, .3);
			cairo_set_source_rgba( cr, 0., 0.3, 0., .4);
		} else {
			cairo_set_line_width( cr, .5);
			cairo_set_source_rgb( cr, 0., 0., 0.);
		}
		draw_signal_original( wd, y0, cr);

		CwB[TColour::labels_sf].set_source_rgb( cr);
		cairo_move_to( cr, wd-120, 25);
		cairo_set_font_size( cr, 10);
		snprintf_buf( "orig");
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}

	int ptop = y0 - settings::WidgetSize_SFPageHeight/2;

	size_t	half_pad = wd * sf.skirting_run_per1,
		ef = wd + 2*half_pad;

	int	half_pad_samples = sf.skirting_run_per1 * sf.vpagesize() * samplerate(),
		cvpa = sf.cur_vpage_start() * samplerate() - half_pad_samples,
		cvpe = sf.cur_vpage_end()   * samplerate() + half_pad_samples,
		evpz = cvpe - cvpa;
      // artifacts (changed bg)
	auto& Aa = recording.F()[name].artifacts;
	if ( not Aa.empty() && sf.unfazer_mode == TUnfazerMode::none ) {
		CwB[TColour::artifact].set_source_rgba( cr,  // do some gradients perhaps?
							.4);
		for ( auto A = Aa.begin(); A != Aa.end(); ++A ) {
			if ( overlap( (int)A->first, (int)A->second, cvpa, cvpe) ) {
				int	aa = (int)A->first - cvpa,
					ae = (int)A->second - cvpa;
				if ( aa < 0 )    aa = 0;
				if ( ae > evpz ) ae = evpz;
				cairo_rectangle( cr,
						 (float)(aa % evpz) / evpz * wd, ptop + settings::WidgetSize_SFPageHeight * 1./3,
						 (float)(ae - aa) / evpz * wd,       settings::WidgetSize_SFPageHeight * 1./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			} else if ( (int)A->first > cvpe )  // no more artifacts up to and on current page
				break;
		}
		CwB[TColour::labels_sf].set_source_rgb( cr);
		cairo_move_to( cr, ef-70, y0 + 15);
		cairo_set_font_size( cr, 8);
		snprintf_buf( "%4.2f %% dirty", percent_dirty);
		cairo_show_text( cr, __buf__);
	}

      // marquee
	if ( draw_marquee // possibly undesired (such as when drawing for unfazer)
	     && sf.unfazer_mode == TUnfazerMode::none && overlap( selection_start_time, selection_end_time,
			 sf.cur_xvpage_start(), sf.cur_xvpage_end()) ) {
		double	pre = sf.skirting_run_per1 * sf.vpagesize(),
			ma = (selection_start_time - sf.cur_xvpage_start()) / sf.xvpagesize() * wd,
			me = (selection_end_time - sf.cur_xvpage_start()) / sf.xvpagesize() * wd;
		cairo_set_source_rgba( cr, .7, .7, .7, .3);
		cairo_rectangle( cr,
				 ma, ptop, me - ma, settings::WidgetSize_SFPageHeight);
		cairo_fill( cr);
		cairo_stroke( cr);

	      // start/end timestamp
		cairo_set_font_size( cr, 10);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_source_rgb( cr, 1, .1, .1);

		cairo_text_extents_t extents;
		snprintf_buf( "%5.2fs",
			      selection_start_time - sf.cur_xvpage_start() - pre);
		cairo_text_extents( cr, __buf__, &extents);
		double ido = ma - 3 - extents.width;
		if ( ido < extents.width+3 )
			cairo_move_to( cr, ma+3, ptop + 30);
		else
			cairo_move_to( cr, ido, ptop + 12);
		cairo_show_text( cr, __buf__);

		if ( selection_end - selection_start > 5 ) {  // don't mark end if selection is too short
			snprintf_buf( "%5.2fs",
				      selection_end_time - sf.cur_xvpage_start() - pre);
			cairo_text_extents( cr, __buf__, &extents);
			ido = me + extents.width+3;
			if ( ido > wd )
				cairo_move_to( cr, me - 3 - extents.width, ptop + 30);
			else
				cairo_move_to( cr, me + 3, ptop + 12);
			cairo_show_text( cr, __buf__);

			snprintf_buf( "←%4.2fs→", selection_end_time - selection_start_time);
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, ma+(me-ma)/2 - extents.width/2,
				       ptop + (extents.width < me-ma ? 12 : 30));
			cairo_show_text( cr, __buf__);
		}

		cairo_stroke( cr);
	}

      // labels of all kinds
      // channel id
	{
		gchar *h_escaped = g_markup_escape_text( name, -1);
		snprintf_buf( "[%s] %s", type, h_escaped);
		g_free( h_escaped);

		cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 10);
		cairo_set_source_rgb( cr, 0., 0., 0.);
		cairo_move_to( cr, 10, y0 - 14);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}

	CwB[TColour::labels_sf].set_source_rgb( cr);
       // unfazer info
	auto& Uu = recording.F()[name].interferences;
	if ( not Uu.empty() ) {
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
		g_string_assign( __ss__, "Unf: ");
		for ( auto U = Uu.begin(); U != Uu.end(); ++U ) {
			g_string_append_printf( __ss__, "%s: %5.3f%c",
						recording.F()[U->first].channel.c_str(), U->second,
						(next(U) == Uu.end()) ? ' ' : ';');
		}
		cairo_set_font_size( cr, 9);
		cairo_move_to( cr, 70, y0 - 14);
		cairo_show_text( cr, __ss__->str);
	}

       // uV scale
	{
		cairo_set_source_rgb( cr, 0., 0., 0.);
		guint dpuV = 1 * signal_display_scale;
		cairo_set_line_width( cr, 1.5);
		cairo_move_to( cr, 10, y0 + 10);
		cairo_line_to( cr, 10, y0 + 10 + dpuV);
		cairo_stroke( cr);
		cairo_set_font_size( cr, 9);
		cairo_move_to( cr, 15, y0 + 20);
		cairo_show_text( cr, "1 mV");
		cairo_stroke( cr);
	}

       // filters
	if ( sf.unfazer_mode != TUnfazerMode::none ) {
		cairo_set_font_size( cr, 9);
		if ( low_pass.cutoff > 0. ) {
			snprintf_buf( "LP: %g/%u", low_pass.cutoff, low_pass.order);
			cairo_move_to( cr, wd-100, y0 + 15);
			cairo_show_text( cr, __buf__);
		}
		if ( high_pass.cutoff > 0. ) {
			snprintf_buf( "HP: %g/%u", high_pass.cutoff, high_pass.order);
			cairo_move_to( cr, wd-100, y0 + 24);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}

}




void
SScoringFacility::SChannel::draw_page( const char *fname, int width, int height) // to a file
{
#ifdef CAIRO_HAS_SVG_SURFACE
	 cairo_surface_t *cs = cairo_svg_surface_create( fname, width, height);
	 cairo_t *cr = cairo_create( cs);
	 draw_page_static( cr, width, height/2, false);
	 cairo_destroy( cr);
	 cairo_surface_destroy( cs);
#endif
}


void
SScoringFacility::SChannel::draw_page( cairo_t* cr)
{
	if ( sf.unfazer_mode != SScoringFacility::TUnfazerMode::calibrate || this == sf.unfazer_offending_channel )
		draw_page_static( cr, sf.da_wd, zeroy, true);

	unsigned
		pbot = zeroy + settings::WidgetSize_SFPageHeight / 2.;
       // power profile
	if ( draw_power and have_power() and sf.unfazer_mode == TUnfazerMode::none ) {
		 // use lower half
		unsigned
			pbot = zeroy + settings::WidgetSize_SFPageHeight / 2.,
			ptop = zeroy;

//		 CwB[TColour::hypnogram].set_source_rgba( cr, .6);
//		cairo_rectangle( cr, 0., ptop, sf.da_wd, pbot);
//		 cairo_fill( cr);

		cairo_set_line_width( cr, 1.);
		cairo_set_font_size( cr, 9);
		guint i;

		if ( draw_bands ) {
			// for ( TBand_underlying_type b = 0; b <= (TBand_underlying_type)Ch.uppermost_band; ++b ) {
			// the type conversions exactly as appearing above drive g++ to segfault
			// the same happens when (TBand_underlying_type) is replaced by (unsigned short)
			for ( unsigned b = 0; b < (unsigned)uppermost_band; ++b ) {
				CwB[(TColour)((int)TColour::band_delta + b)].set_source_rgba( cr, .5);
				cairo_move_to( cr, (0+.5) / sf.total_pages() * sf.da_wd,
					       - power_in_bands[b][0] * power_display_scale + pbot);
				for ( i = 1; i < sf.total_pages(); ++i )
					cairo_line_to( cr, (i+.5) / sf.total_pages() * sf.da_wd,
						       - power_in_bands[b][i] * power_display_scale + pbot);
				if ( (TBand)b == focused_band ) {
					cairo_line_to( cr, sf.da_wd, pbot);
					cairo_line_to( cr,       0., pbot);
					cairo_fill( cr);
				}
				cairo_stroke( cr);

				if ( (TBand)b == focused_band ) {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
					snprintf_buf( "%s %g–%g",
						      settings::FreqBandNames[b],
						      settings::FreqBands[b][0], settings::FreqBands[b][1]);
				} else {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
					snprintf_buf( "%s", settings::FreqBandNames[b]);
				}
				cairo_move_to( cr, sf.da_wd - 70,
					       ptop + (int)uppermost_band*12 - 12*b + 12);
				cairo_show_text( cr, __buf__);
				cairo_stroke( cr);
			}
		} else {
			CwB[TColour::power_sf].set_source_rgba( cr, .5);
			cairo_move_to( cr, (0+.5) / sf.total_pages() * sf.da_wd, power[0]);
			for ( i = 0; i < sf.total_pages(); ++i )
				cairo_line_to( cr, (i+.5) / sf.total_pages() * sf.da_wd,
					       - power[i] * power_display_scale + pbot);
			cairo_line_to( cr, sf.da_wd, pbot);
			cairo_line_to( cr,       0., pbot);
			cairo_fill( cr);
			cairo_stroke( cr);

			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			snprintf_buf( "%g–%g Hz", from, upto);
			cairo_move_to( cr, sf.da_wd - 70, ptop + 15);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}

		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 9);

	      // scale
		cairo_set_source_rgba( cr, 0., 0., 0., .5);
		cairo_set_line_width( cr, 1.5);
		// cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT); // is default
		cairo_move_to( cr, 60, ptop + 10);
		cairo_rel_line_to( cr, 0, power_display_scale * 1e6);
		cairo_stroke( cr);

		cairo_move_to( cr, 65, ptop + 20);
		cairo_show_text( cr, "1 µV²/Hz");
		cairo_stroke( cr);

	      // hour ticks
		CwB[TColour::ticks_sf].set_source_rgba( cr, .5);
		cairo_set_line_width( cr, 1);
		cairo_set_font_size( cr, 10);
		float	hours4 = (float)n_samples() / samplerate() / 3600 * 4;
		for ( i = 1; i < hours4; ++i ) {
			guint tick_pos = (float)i / hours4 * sf.da_wd;
			cairo_move_to( cr, tick_pos, pbot);
			cairo_rel_line_to( cr, 0, -((i%4 == 0) ? 20 : (i%2 == 0) ? 12 : 5));
			if ( i % 4 == 0 ) {
				snprintf_buf( "%2uh", i/4);
				cairo_move_to( cr, tick_pos+5, pbot - 12);
				cairo_show_text( cr, __buf__);
			}
		}
		cairo_stroke( cr);

	      // cursor
		CwB[TColour::cursor].set_source_rgba( cr, .3);
		cairo_rectangle( cr,
				 (float) sf.cur_vpage() / sf.total_vpages() * sf.da_wd,  ptop,
				 ceil( 1. / sf.total_vpages() * sf.da_wd), pbot - ptop);
		cairo_fill( cr);
		cairo_stroke( cr);
	}

      // unfazer
	if ( sf.unfazer_mode != TUnfazerMode::none ) {
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_text_extents_t extents;

		if ( this == sf.unfazer_affected_channel ) {
			switch ( sf.unfazer_mode ) {
			case SScoringFacility::TUnfazerMode::channel_select:
				cairo_set_source_rgba( cr, 0., 0., 0., .7);
				cairo_set_font_size( cr, 13);
				snprintf_buf( "Unfaze this channel from...");
				sf.set_tooltip( SScoringFacility::TTipIdx::unfazer);
			    break;
			case SScoringFacility::TUnfazerMode::calibrate:
				cairo_set_source_rgba( cr, 0., 0., 0., .5);
				cairo_set_font_size( cr, 13);
				snprintf_buf( "Unfaze this channel from %s",
					      sf.unfazer_offending_channel->name);
				// show the signal being set up for unfazer live
				{
					auto page = slice( sf.cur_vpage_start(), sf.vpagesize() * samplerate(), 1);
					auto being_unfazed = valarray<float> (
						signal_filtered[ page ]);
					cairo_set_line_width( cr, .5);
					cairo_set_source_rgba( cr, 0., 0., 0., .3);
					::draw_signal( being_unfazed,
						       0, being_unfazed.size(),
						       sf.da_wd, zeroy, signal_display_scale, cr,
						       true);
					cairo_stroke( cr);

					being_unfazed -=
						valarray<float> (sf.unfazer_offending_channel->signal_filtered[ page ]) * sf.unfazer_factor;

					cairo_set_line_width( cr, .3);
					cairo_set_source_rgba( cr, 0., 0., 0., 1.);
					::draw_signal( being_unfazed,
						       0, being_unfazed.size(),
						       sf.da_wd, zeroy, signal_display_scale, cr,
						       true);
					cairo_stroke( cr);
				}
				sf.set_tooltip( SScoringFacility::TTipIdx::unfazer);
			    break;
			default:
			    break;
			}

			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, (sf.da_wd - extents.width)/2., pbot - 30);
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
				cairo_move_to( cr, (sf.da_wd - extents.width)/2., pbot - 30);
				cairo_show_text( cr, __buf__);
				cairo_stroke( cr);
			    break;
			default:
			    break;
			}
		}
	} else
		sf.set_tooltip( SScoringFacility::TTipIdx::general);

      // crosshair, voltage at
	if ( sf.draw_crosshair && sf.unfazer_mode == TUnfazerMode::none ) {
		cairo_set_font_size( cr, 9);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		CwB[TColour::cursor].set_source_rgb( cr);
		float t = (float)sf.crosshair_at/sf.da_wd * sf.vpagesize();
		if ( this == &sf.channels.front() )
			snprintf_buf( "%4.2f (%5.2fs)",
				      (draw_filtered_signal ? signal_filtered : signal_original)
				      [ (size_t)((sf.cur_vpage_start() + t) * samplerate()) ],
				      t);
		else
			snprintf_buf( "%4.2f",
				      (draw_filtered_signal ? signal_filtered : signal_original)
				      [ (size_t)((sf.cur_vpage_start() + t) * samplerate()) ]);

		cairo_move_to( cr, sf.crosshair_at+2, zeroy + 12);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}

      // samples per pixel
	if ( sf.draw_spp ) {
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size( cr, 8);
		snprintf_buf( "%4.2f spp", (float)samplerate() * sf.vpagesize() / sf.da_wd);
		cairo_move_to( cr, sf.da_wd-40, 15);
		cairo_show_text( cr, __buf__);
	}

	cairo_stroke( cr);
}




void
SScoringFacility::draw_montage( cairo_t* cr)
{
	double	true_frac = 1. - 1. / (1. + 2*skirting_run_per1);
	size_t	half_pad = da_wd * true_frac/2,
		ef = da_wd * (1. - true_frac);  // w + 10% = d

      // background, is now common to all channels
	if ( unfazer_mode == TUnfazerMode::none ) {
		float	ppart = (float)pagesize()/vpagesize();
		int	cp = cur_page();
		for ( int pp = cp-1; ; ++pp ) {
			TScore this_page_score = (pp < 0) ? TScore::none : agh::SPage::char2score( hypnogram[pp]);
			CwB[score2colour(this_page_score)].set_source_rgba( cr, .2);
			float ppoff = ((float)pp * pagesize() - cur_vpage_start()) / vpagesize();
			if ( ppoff > 1.5 )
				break;
			cairo_rectangle( cr, half_pad + ppoff * ef,
					 0., ef * ppart, da_ht);
			cairo_fill( cr);
			cairo_stroke( cr);
		}
	}

      // draw individual signal pages
	for_each( channels.begin(), channels.end(),
		  [&cr] ( SChannel& h)
		  {
			  h.draw_page( cr);
		  });

      // ticks
	{
		CwB[TColour::ticks_sf].set_source_rgb( cr);
		cairo_set_font_size( cr, 9);
		cairo_set_line_width( cr, .2);
		for ( size_t i = 0; i <= __pagesize_ticks[pagesize_item]; ++i ) {
			unsigned tick_pos = i * vpagesize() / __pagesize_ticks[pagesize_item];
			cairo_move_to( cr, half_pad + i * ef / __pagesize_ticks[pagesize_item], 0);
			cairo_rel_line_to( cr, 0, da_ht);

			cairo_move_to( cr, half_pad + i * ef / __pagesize_ticks[pagesize_item] + 5, 12);
			snprintf_buf_ts_s( tick_pos);
			cairo_move_to( cr, half_pad + i * ef / __pagesize_ticks[pagesize_item] + 5, da_ht-2);
			snprintf_buf_ts_s( tick_pos);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}

      // crosshair line
	if ( draw_crosshair ) {
		CwB[TColour::cursor].set_source_rgba( cr, .5);
		cairo_set_line_width( cr, .2);
		cairo_move_to( cr, crosshair_at, 0);
		cairo_rel_line_to( cr, 0, da_ht);
		cairo_stroke( cr);
	}
}


} // namespace sf



// callbacks




using namespace aghui;
using namespace aghui::sf;

extern "C" {

	 gboolean
	 daScoringFacMontage_configure_event_cb( GtkWidget *widget,
						 GdkEventConfigure *event,
						 gpointer userdata)
	 {
		 if ( event->type == GDK_CONFIGURE ) {
			 auto& SF = *(SScoringFacility*)userdata;
			 SF.da_wd = event->width;
			 // don't care about height: it's our own calculation
		 }
		 return FALSE;
	 }




// -------------------- Page

	 gboolean
	 daScoringFacMontage_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
	 {
		 auto& SF = *(SScoringFacility*)userdata;
		 SF.draw_montage( cr);
		 return TRUE;
	 }

	 gboolean
	 daScoringFacMontage_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	 {
		 auto& SF = *(SScoringFacility*)userdata;
		 auto Ch = SF.using_channel = SF.channel_near( event->y);

		 if ( SF.unfazer_mode != SScoringFacility::TUnfazerMode::none ) {
			 switch ( SF.unfazer_mode ) {

			 case SScoringFacility::TUnfazerMode::channel_select:
				 if ( event->button == 1 )
					 if ( Ch != SF.unfazer_affected_channel ) {
						 // using_channel is here the one affected
						 SF.unfazer_offending_channel = Ch;
						 // get existing f value for this pair if any
						 float f = SF.unfazer_affected_channel->ssignal().interferences[Ch->h()];  // don't forget to erase it if user cancels unfazer calibration
						 SF.unfazer_factor = (f == 0.) ? 0.1 : f;
						 SF.unfazer_mode = SScoringFacility::TUnfazerMode::calibrate;
					 } else
						 // cancel
						 Ch->sf.unfazer_mode = SScoringFacility::TUnfazerMode::none;
				 else // also cancel
					 SF.unfazer_mode = SScoringFacility::TUnfazerMode::none;
				 gtk_widget_queue_draw( wid);
			     break;

			 case SScoringFacility::TUnfazerMode::calibrate:
				 if ( Ch == SF.unfazer_offending_channel ) {
					 if ( event->button == 1 ) {
						 // confirm and apply
						 SF.unfazer_affected_channel -> ssignal().interferences[ SF.unfazer_offending_channel->h() ]
							 = SF.unfazer_factor;
						 SF.unfazer_affected_channel->get_signal_filtered();
						 if ( SF.unfazer_affected_channel->have_power() )
							 SF.unfazer_affected_channel->get_power();

					 } else if ( event->button == 3 ) {
						 // cancel
						 ;
					 } else if ( event->button == 2 ) {
						 // remove some unfazer(s)
						 if ( event->state & GDK_CONTROL_MASK )
							 // remove all unfazers on using_channel
							 SF.unfazer_affected_channel->ssignal().interferences.clear();
						 else
							 // remove one currently being calibrated
							 SF.unfazer_affected_channel->ssignal().interferences.erase( SF.unfazer_offending_channel->h());
						 SF.unfazer_affected_channel->get_signal_filtered();
						 if ( SF.unfazer_affected_channel->have_power() )
							 SF.unfazer_affected_channel->get_power();
					 }
					 SF.unfazer_mode = SScoringFacility::TUnfazerMode::none;
					 SF.unfazer_offending_channel = SF.unfazer_affected_channel = NULL;
					 gtk_widget_queue_draw( wid);
				 }
			     break;

			 case SScoringFacility::TUnfazerMode::none:
			     break;
			 }

		 } else if ( Ch->have_power() && Ch->draw_power && event->y > Ch->zeroy ) {
			 switch ( event->button ) {
			 case 1:
				 gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
							    (event->x / SF.da_wd) * SF.total_vpages() + 1);
				 // will eventually call set_cur_vpage(), which will do redraw
				 break;
			 case 2:
				 Ch->draw_bands = !Ch->draw_bands;
				 gtk_widget_queue_draw( wid);
				 break;
			 case 3:
				 gtk_menu_popup( SF.mSFPower,
						 NULL, NULL, NULL, NULL, 3, event->time);
				 break;
			 }

		 } else {
			 switch ( event->button ) {
			 case 2:
				 if ( event->state & GDK_CONTROL_MASK )
					 for ( auto H = SF.channels.begin(); H != SF.channels.end(); ++H ) {
						 H->signal_display_scale = SF.sane_signal_display_scale;
					 }
				 else {
					 Ch->signal_display_scale = SF.sane_signal_display_scale;
				 }
				 gtk_widget_queue_draw( wid);
			     break;
			 case 3:
			 {
				 double cpos = SF.time_at_click( event->x);
				 gtk_menu_popup( overlap( Ch->selection_start_time, Ch->selection_end_time,
							  cpos, cpos) ? SF.mSFPageSelection : SF.mSFPage,
						 NULL, NULL, NULL, NULL, 3, event->time);
			 }
			     break;
			 case 1:
				 SF.marking_now = true;
				 Ch->marquee_mstart = Ch->marquee_mend = event->x;
				 gtk_widget_queue_draw( wid);
			     break;
			 }
		 }
		 return TRUE;
	 }





	 gboolean
	 daScoringFacMontage_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
	 {
		 auto& SF = *(SScoringFacility*)userdata;
		 if ( SF.channel_near( event->y) != SF.using_channel ) // user has dragged too much vertically
			 return TRUE;

		 // update marquee boundaries
		 if ( SF.marking_now ) {
			 SF.using_channel->marquee_mend = event->x;
			 SF.using_channel->marquee_to_selection(); // to be sure, also do it on button_release
			 if ( event->state & GDK_SHIFT_MASK )
				 for_each( SF.channels.begin(), SF.channels.end(),
					   [&] (SScoringFacility::SChannel& H)
					   {
						   H.marquee_mstart = SF.using_channel->marquee_mstart;
						   H.marquee_mend = event->x;
						   H.marquee_to_selection(); // to be sure, also do it on button_release
					   });
			 gtk_widget_queue_draw( wid);

		 } else if ( SF.draw_crosshair ) {
			 SF.crosshair_at = event->x;
			 gtk_widget_queue_draw( wid);
		 }

		 return TRUE;
	 }



	 gboolean
	 daScoringFacMontage_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	 {
		 auto& SF = *(SScoringFacility*)userdata;
		 // auto Ch = SF.channel_near( event->y); // rather think using_channel as set in button_press_cb

		 if ( SF.channel_near( event->y) != SF.using_channel ) // user has dragged too much vertically
			 return TRUE;

		 switch ( event->button ) {
		 case 1:
			 if ( fabs(SF.using_channel->marquee_mstart - SF.using_channel->marquee_mend) > 5 )
				 gtk_menu_popup( SF.mSFPageSelection,
						 NULL, NULL, NULL, NULL, 3, event->time);
			 else
				 SF.using_channel->marquee_to_selection();
			 SF.marking_now = false;
			 gtk_widget_queue_draw( wid);
		     break;
		 case 3:
		     break;
		 }

		 return TRUE;
	 }







	 gboolean
	 daScoringFacMontage_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	 {
		 auto& SF = *(SScoringFacility*)userdata;
		 auto Ch = SF.using_channel = SF.channel_near( event->y);

		 if ( SF.unfazer_mode == SScoringFacility::TUnfazerMode::calibrate && Ch == SF.unfazer_offending_channel ) {
			 switch ( event->direction ) {
			 case GDK_SCROLL_DOWN:
				 if ( fabs( SF.unfazer_factor) > .2 )
					 SF.unfazer_factor -= .1;
				 else
					 SF.unfazer_factor -= .02;
			     break;
			 case GDK_SCROLL_UP:
				 if ( fabs( SF.unfazer_factor) > .2 )
					 SF.unfazer_factor += .1;
				 else
					 SF.unfazer_factor += .02;
			     break;
			 default:
			     break;
			 }
			 gtk_widget_queue_draw( wid);

			 return TRUE;
		 }

		 if ( Ch->have_power() && Ch->draw_power && event->y > Ch->zeroy ) {
			 if ( event->state & GDK_SHIFT_MASK ) {
				 switch ( event->direction ) {
				 case GDK_SCROLL_DOWN:
					 if ( Ch->draw_bands ) {
						 if ( Ch->focused_band != TBand::delta )
							 prev( Ch->focused_band);
					 } else
						 if ( Ch->from > 0 ) {
							 Ch->from = Ch->from - .5;
							 Ch->upto = Ch->upto - .5;
							 Ch->get_power();
						 }
					 break;
				 case GDK_SCROLL_UP:
					 if ( Ch->draw_bands ) {
						 if ( Ch->focused_band != Ch->uppermost_band )
							 next( Ch->focused_band);
					 } else
						 if ( Ch->upto < 18. ) {
							 Ch->from = Ch->from + .5;
							 Ch->upto = Ch->upto + .5;
							 Ch->get_power();
						 }
					 break;
				 case GDK_SCROLL_LEFT:
				 case GDK_SCROLL_RIGHT:
					 break;
				 }

			 } else
				 switch ( event->direction ) {
				 case GDK_SCROLL_DOWN:
					 Ch->power_display_scale /= 1.1;
					 gtk_widget_queue_draw( wid);
					 break;
				 case GDK_SCROLL_UP:
					 Ch->power_display_scale *= 1.1;
					 gtk_widget_queue_draw( wid);
					 break;
				 case GDK_SCROLL_LEFT:
					 if ( SF.cur_vpage() > 0 )
						 gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
									    SF.cur_vpage() - 1);
					 break;
				 case GDK_SCROLL_RIGHT:
					 if ( SF.cur_vpage() < SF.total_vpages() )
						 gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
									    SF.cur_vpage() + 1);
					 break;
				 }

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

		 if ( event->state & GDK_CONTROL_MASK )
			 for_each( SF.channels.begin(), SF.channels.end(),
				   [&] ( SScoringFacility::SChannel& H)
				   {
					   H.signal_display_scale = Ch->signal_display_scale;
				   });

		 gtk_widget_queue_draw( wid);
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
						(gboolean)SF.using_channel->draw_filtered_signal);
		gtk_check_menu_item_set_active( SF.iSFPageUseResample,
						(gboolean)SF.using_channel->use_resample);
		// gtk_check_menu_item_set_active( SF.iSFPageShowEnvelope,
		// 				(gboolean)SF.using_channel->draw_envelope);
	}


	void
	iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.using_channel->draw_original_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
		// prevent both being switched off
		if ( !SF.using_channel->draw_original_signal && !SF.using_channel->draw_filtered_signal )
			gtk_check_menu_item_set_active( SF.iSFPageShowProcessed,
							(gboolean)(SF.using_channel->draw_filtered_signal = true));
		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
	}


	void
	iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.using_channel->draw_filtered_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
		if ( !SF.using_channel->draw_filtered_signal && !SF.using_channel->draw_original_signal )
			gtk_check_menu_item_set_active( SF.iSFPageShowOriginal,
							(gboolean)(SF.using_channel->draw_original_signal = true));
		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
	}


	void
	iSFPageUseResample_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.using_channel->use_resample = (bool)gtk_check_menu_item_get_active( checkmenuitem);
		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
	}




	void
	iSFPageClearArtifacts_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( GTK_RESPONSE_YES != pop_question(
			     SF.wScoringFacility,
			     "All marked artifacts will be lost in this channel.  Continue?") )
			return;

		SF.using_channel->ssignal().artifacts.clear();
		SF.using_channel->get_signal_filtered();

		if ( SF.using_channel->have_power() ) {
			SF.using_channel->get_power();
			SF.using_channel->get_power_in_bands();
		}

		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
	}


	void
	iSFPageUnfazer_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.unfazer_affected_channel = SF.using_channel;
		SF.unfazer_mode = SScoringFacility::TUnfazerMode::channel_select;
		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
	}



	void
	iSFPageSaveAs_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		string j_dir = AghCC->subject_dir( SF.using_channel->recording.subject());
		snprintf_buf( "%s/%s/%s-p%zu@%zu.svg", j_dir.c_str(), AghD(), AghT(), SF.cur_vpage(), SF.vpagesize());
		UNIQUE_CHARP(fname);
		fname = g_strdup( __buf__);

		SF.using_channel->draw_page( fname, SF.da_wd, settings::WidgetSize_SFPageHeight);
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
		gtk_statusbar_pop( SF.sbSF, sb::sbContextIdGeneral);
		gtk_statusbar_push( SF.sbSF, sb::sbContextIdGeneral, __buf__);
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
			  });
		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
	}


      // page selection
	void
	iSFPageSelectionMarkArtifact_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( SF.using_channel->selection_size() > 5 )
			SF.using_channel->mark_region_as_artifact( true);
	}

	void
	iSFPageSelectionClearArtifact_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( SF.using_channel->selection_size() > 5 )
			SF.using_channel->mark_region_as_artifact( false);
	}

	void
	iSFPageSelectionFindPattern_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( SF.using_channel->selection_size() > 5 )
			SF.using_channel->mark_region_as_pattern();
	}




     // power
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

