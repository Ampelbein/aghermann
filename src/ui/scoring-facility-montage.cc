// ;-*-C++-*- *  Time-stamp: "2011-06-15 02:09:23 hmmr"
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
SScoringFacility::SChannel::draw_page_static( cairo_t *cr,
					      int wd, int y0,
					      bool draw_marquee)
{
//	auto this_page_score = sf.pagesize_is_right() ? sf.cur_page_score() : TScore::none;
//	auto used_colour = CwB[score2colour(this_page_score)];
      // waveform: signal_filtered
	bool one_signal_drawn = false;
	if ( (draw_filtered_signal && sf.unfazer_mode == SScoringFacility::TUnfazerMode::none) ||
	     (draw_filtered_signal && this != sf.using_channel) ) {  // only show processed signal when done with unfazing
		cairo_set_line_width( cr, .5);
		// CwB[score2colour(this_page_score)].set_source_rgb( cr);
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
	if ( draw_original_signal ||
	     (sf.unfazer_mode == SScoringFacility::TUnfazerMode::channel_select && this == sf.using_channel) ) {
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
						 (float)( aa       % lpp) / lpp * sf.da_wd, y0 - settings::WidgetSize_SFPageHeight*1./3,
						 (float)((az - aa) % lpp) / lpp * sf.da_wd, settings::WidgetSize_SFPageHeight     * 2./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			} else if ( A->first <= cur_page_start_s && A->second >= cur_page_end_s ) {
				CwB[TColour::artifact].set_source_rgba( cr,  // flush solid (artifact covering all page)
									.5);
				cairo_rectangle( cr,
						 0, y0 - settings::WidgetSize_SFPageHeight   *1./3,
						 sf.da_wd, settings::WidgetSize_SFPageHeight *2./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			} else if ( A->first > cur_page_end_s )  // no more artifacts up to and on current page
				break;
		}
		CwB[TColour::labels_sf].set_source_rgb( cr);
		cairo_move_to( cr, sf.da_wd-70, y0 + settings::WidgetSize_SFPageHeight-15);
		cairo_set_font_size( cr, 8);
		snprintf_buf( "%4.2f %% dirty", percent_dirty);
		cairo_show_text( cr, __buf__);
	}

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
		cairo_move_to( cr, 10, y0 + 14);
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

      // marquee
	if ( draw_marquee && sf.selection_size() > 0 ) {
		float vstart = (sf.marquee_start < sf.marquee_virtual_end) ? sf.marquee_start : sf.marquee_virtual_end,
			vend = (sf.marquee_start < sf.marquee_virtual_end) ? sf.marquee_virtual_end : sf.marquee_start;
		cairo_set_source_rgba( cr, .7, .7, .7, .3);
//		printf( "vstart %f, vend %f ht %zu\n", vstart, vend, ht);
		cairo_rectangle( cr,
				 vstart, y0,
				 vend - vstart, y0 + settings::WidgetSize_SFPageHeight);
		cairo_fill( cr);

	      // start/end timestamp
		cairo_set_font_size( cr, 9);
		cairo_set_source_rgb( cr, 1, .1, .1);

		cairo_text_extents_t extents;
		snprintf_buf( "%5.2fs", vstart/wd * sf.vpagesize());
		cairo_text_extents( cr, __buf__, &extents);
		double ido = vstart - 3 - extents.width;
		if ( ido < 0+extents.width+3 )
			cairo_move_to( cr, vstart+3, y0 + 30);
		else
			cairo_move_to( cr, ido, y0 + 12);
		cairo_show_text( cr, __buf__);

		if ( vend - vstart > 5 ) {  // don't mark end if selection is too short
			snprintf_buf( "%5.2fs", vend/wd * sf.vpagesize());
			cairo_text_extents( cr, __buf__, &extents);
			ido = vend+extents.width+3;
			if ( ido > wd )
				cairo_move_to( cr, vend-3-extents.width, y0 + 30);
			else
				cairo_move_to( cr, vend+3, y0 + 12);
			cairo_show_text( cr, __buf__);

			snprintf_buf( "←%4.2fs→", (vend-vstart)/wd * sf.vpagesize());
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, vstart+(vend-vstart)/2 - extents.width/2,
				       y0 + (extents.width < vend-vstart ? 12 : 30));
			cairo_show_text( cr, __buf__);
		}
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
	draw_page_static( cr, sf.da_wd, zeroy,
			  sf.marking_in_channel == this);

      // power profile
	if ( draw_power and have_power() ) {
		// use lower half
		unsigned
			pbot = zeroy + settings::WidgetSize_SFPageHeight / 2.,
			ptop = zeroy;

		CwB[TColour::hypnogram].set_source_rgba( cr, .6);
//		cairo_rectangle( cr, 0., ptop, sf.da_wd, pbot);
		cairo_fill( cr);

		guint i;

	      // profile
		if ( draw_bands ) {
			cairo_set_line_width( cr, 1.);
			cairo_set_font_size( cr, 9);
			// for ( TBand_underlying_type b = 0; b <= (TBand_underlying_type)Ch.uppermost_band; ++b ) {
			// the type conversions exactly as appearing above drive g++ to segfault
			// the same happens when (TBand_underlying_type) is replaced by (unsigned short)
			for ( unsigned b = 0; b < (unsigned)uppermost_band; ++b ) {
				valarray<float> values = power_in_bands[b] * power_display_scale;
				CwB[(TColour)((int)TColour::band_delta + b)].set_source_rgba( cr, .5);
				cairo_move_to( cr, (0+.5) / sf.total_pages() * sf.da_wd,
					       - values[0] + pbot);
				for ( i = 1; i < sf.total_pages(); ++i )
					cairo_line_to( cr, (i+.5) / sf.total_pages() * sf.da_wd,
						       - values[i] + pbot);
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

			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			snprintf_buf( "%g–%g Hz", from, upto);
			cairo_move_to( cr, sf.da_wd - 70, ptop + 15);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);

		// scale
		cairo_set_source_rgba( cr, 0., 0., 0., .5);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_line_width( cr, 1.5);
		// cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT); // is default
		cairo_move_to( cr, 60, ptop + 10);
		cairo_line_to( cr, 60, ptop + 10 + power_display_scale * 1e6);
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
				::draw_signal( signal_original,
					       sf.cur_vpage_start() * samplerate(),
					       sf.cur_vpage_end() * samplerate(),
					       sf.da_wd, zeroy, signal_display_scale, cr,
					       true);

				// SRC_DATA samples;
				// float *s1, *s2;
				// samples.data_in = &signal_original[
				// 	(samples.input_frames = samplerate() * sf.vpagesize()) * sf.cur_vpage() ];
				// samples.data_out = s1 = (float*)malloc( (samples.output_frames = sf.da_wd) * sizeof(float));
				// samples.src_ratio = (double)samples.output_frames / samples.input_frames;
				// if ( src_simple( &samples, SRC_SINC_FASTEST, 1) )
				// 	;

				// samples.data_in = &sf.unfazer_offending_channel->signal_original[ samples.input_frames * sf.cur_vpage() ];
				// samples.data_out = s2 = (float*)malloc( samples.output_frames * sizeof(float));
				// if ( src_simple( &samples, SRC_LINEAR, 1) )
				// 	;

				// cairo_move_to( cr, 0,
				// 	       - (s1[0] - s2[0] * sf.unfazer_factor)
				// 	       * signal_display_scale
				// 	       + zeroy + settings::WidgetSize_SFPageHeight/2.);
				// for ( int i = 0; i < sf.da_wd; ++i ) {
				// 	cairo_line_to( cr, i,
				// 		       - (s1[i] - s2[i] * sf.unfazer_factor)
				// 		       * signal_display_scale
				// 		       + zeroy + settings::WidgetSize_SFPageHeight/2.);
				// }
				// cairo_stroke( cr);

				// free( (void*)s1);
				// free( (void*)s2);
				sf.set_tooltip( SScoringFacility::TTipIdx::unfazer);
			    break;
			case SScoringFacility::TUnfazerMode::none:
			    break;
			}

			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, (sf.da_wd - extents.width)/2., zeroy + settings::WidgetSize_SFPageHeight/2. - 30);
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
				cairo_move_to( cr, (sf.da_wd - extents.width)/2., zeroy + settings::WidgetSize_SFPageHeight/2. - 30);
				cairo_show_text( cr, __buf__);
			    break;
			case SScoringFacility::TUnfazerMode::none:
			    break;
			}
		}
	} else
		sf.set_tooltip( SScoringFacility::TTipIdx::general);

      // crosshair, voltage at
	if ( sf.draw_crosshair ) {
		cairo_set_font_size( cr, 9);
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
	}

      // samples per pixel
	{
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
//	SManagedColor used_colour;
      // background, is now common to all channels
	float	ppart = (float)pagesize()/vpagesize();
	int	cp = cur_page();
	for ( int pp = cp-1; ; ++pp ) {
		TScore this_page_score = (pp < 0) ? TScore::none : agh::SPage::char2score( hypnogram[pp]);
		CwB[score2colour(this_page_score)].set_source_rgba( cr, .2);
		float ppoff = ((float)pp * pagesize() - cur_vpage_start()) / vpagesize();
//		printf( "(%zu) pp = %d (%d) %f -| %f \n", cur_vpage_start(), pp, this_page_score, ppoff, ppart);
		if ( ppoff > 1.5 )
			break;
		cairo_rectangle( cr, ppoff * da_wd,
				 0., da_wd * ppart, da_ht);
		cairo_fill( cr);
		cairo_stroke( cr);
	}
//	printf( "\n");

//	used_colour = CwB[score2colour(this_page_score)];

      // // preceding and next score-coloured fade-in and -out
      // 	if ( pagesize_is_right() ) {
      // 		bool pattern_set = false;
      // 		TScore neigh_page_score;
      // 		cairo_pattern_t *cp = cairo_pattern_create_linear( 0., 0., da_wd, 0.);
      // 		if ( cur_page() > 0 &&
      // 		     (neigh_page_score = agh::SPage::char2score( hypnogram[cur_page()-1])) != this_page_score &&
      // 		     neigh_page_score != TScore::none ) {
      // 			pattern_set = true;
      // 			CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp,     0., .7);
      // 			CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp, 50./da_wd, 0.);
      // 		}
      // 		if ( cur_page() < total_pages()-1 &&
      // 		     (neigh_page_score = agh::SPage::char2score( hypnogram[cur_page()+1])) != this_page_score &&
      // 		     neigh_page_score != TScore::none ) {
      // 			pattern_set = true;
      // 			CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp, 1. - 50./da_wd, 0.);
      // 			CwB[score2colour(neigh_page_score)].pattern_add_color_stop_rgba( cp, 1.         , .7);
      // 		}
      // 		if ( pattern_set ) {
      // 			cairo_set_source( cr, cp);
      // 			cairo_rectangle( cr, 0., 0., da_wd, da_ht);
      // 			cairo_fill( cr);
      // 			cairo_stroke( cr);
      // 		}
      // 		cairo_pattern_destroy( cp);
      // 	}

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
		cairo_set_line_width( cr, .3);
		for ( size_t i = 0; i < __pagesize_ticks[pagesize_item]; ++i ) {
			guint tick_pos = i * vpagesize() / __pagesize_ticks[pagesize_item];
			cairo_move_to( cr, i * da_wd / __pagesize_ticks[pagesize_item], 0);
			cairo_line_to( cr, i * da_wd / __pagesize_ticks[pagesize_item], da_ht);

			cairo_move_to( cr, i * da_wd / __pagesize_ticks[pagesize_item] + 5, 12);
			snprintf_buf_ts_s( tick_pos);
			cairo_move_to( cr, i * da_wd / __pagesize_ticks[pagesize_item] + 5, da_ht-2);
			snprintf_buf_ts_s( tick_pos);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}

      // crosshair line
	if ( draw_crosshair ) {
		CwB[TColour::cursor].set_source_rgb( cr);
		cairo_move_to( cr, crosshair_at, 0);
		cairo_line_to( cr, crosshair_at, da_ht);
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
		auto& Ch = SF.channel_near( event->y);

		if ( SF.unfazer_mode != SScoringFacility::TUnfazerMode::none ) {
			switch ( SF.unfazer_mode ) {

			case SScoringFacility::TUnfazerMode::channel_select:
				if ( event->button == 1 )
					if ( &Ch != SF.using_channel ) {
						// using_channel is here the one affected
						SF.unfazer_offending_channel = &Ch;
						// get existing f value for this pair if any
						float f = SF.using_channel->ssignal().interferences[Ch.h()];  // don't forget to erase it if user cancels unfazer calibration
						SF.unfazer_factor = (f == 0.) ? 0.1 : f;
						SF.unfazer_mode = SScoringFacility::TUnfazerMode::calibrate;
					} else
						// cancel
						Ch.sf.unfazer_mode = SScoringFacility::TUnfazerMode::none;
				else // also cancel
					SF.unfazer_mode = SScoringFacility::TUnfazerMode::none;
				gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
			    break;

			case SScoringFacility::TUnfazerMode::calibrate:
				if ( event->button == 1 && SF.unfazer_offending_channel == &Ch ) {
					// confirm and apply
					SF.using_channel->ssignal().interferences[ SF.unfazer_offending_channel->h() ]
						= SF.unfazer_factor;

				} else if ( event->button == 2 && SF.unfazer_offending_channel == &Ch ) {
					// cancel

				} else if ( event->button == 2 && SF.using_channel == &Ch ) {
					// remove some unfazer(s)
					if ( event->state & GDK_CONTROL_MASK )
						// remove all unfazers on using_channel
						SF.using_channel->ssignal().interferences.clear();
					else
						// remove one currently being calibrated
						SF.using_channel->ssignal().interferences.erase( SF.unfazer_offending_channel->h());
					SF.using_channel->get_signal_filtered();
					if ( SF.using_channel->have_power() )
						SF.using_channel->get_power();
				}
				SF.unfazer_mode = SScoringFacility::TUnfazerMode::none;

				SF.using_channel->get_signal_filtered();
				if ( SF.using_channel->have_power() )
					SF.using_channel->get_power();

				SF.unfazer_offending_channel = NULL;
				gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
			    break;

			case SScoringFacility::TUnfazerMode::none:
			    break;
			}

		} else if ( Ch.have_power() && Ch.draw_power && event->y > Ch.zeroy ) {
			switch ( event->button ) {
			case 1:
				gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
							   (event->x / SF.da_wd) * SF.total_vpages() + 1);
				// will eventually call set_cur_vpage(), which will do redraw
				break;
			case 2:
				Ch.draw_bands = !Ch.draw_bands;
				gtk_widget_queue_draw( wid);
				break;
			case 3:
				SF.using_channel = &Ch;
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
					Ch.signal_display_scale = SF.sane_signal_display_scale;
				}
				gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
			    break;
			case 3:
				SF.using_channel = &Ch;  // no other way to mark this channel even though user may not intend to Unfaze
				// gtk_widget_set_sensitive( (GtkWidget*)Ch.sf.iSFPageShowDZCDF, Ch.have_sa_features());
				// gtk_widget_set_sensitive( (GtkWidget*)Ch.sf.iSFPageShowEnvelope, Ch.have_sa_features());
				gtk_menu_popup( SF.mSFPage,
						NULL, NULL, NULL, NULL, 3, event->time);
			    break;
			case 1:
				SF.marking_in_channel = &Ch;
				SF.marquee_start = SF.marquee_virtual_end = event->x;
				gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
			    break;
			}
		}
		return TRUE;
	}





	gboolean
	daScoringFacMontage_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		auto& Ch = SF.channel_near( event->y);

		if ( &Ch != SF.marking_in_channel )
			return TRUE;

		switch ( event->button ) {
		case 1:
			if ( SF.marquee_virtual_end != SF.marquee_start ) {
				SF.using_channel = &Ch;
				gtk_menu_popup( SF.mSFPageSelection,
						NULL, NULL, NULL, NULL, 3, event->time);
			}
		    break;
		case 3:
		    break;
		}

		SF.marking_in_channel = NULL;

		return TRUE;
	}








	gboolean
	daScoringFacMontage_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		auto& Ch = SF.channel_near( event->y);

		// update marquee boundaries
		if ( SF.marking_in_channel == &Ch && SF.unfazer_mode == SScoringFacility::TUnfazerMode::none )
			SF.marquee_virtual_end = (event->x > 0. ) ? event->x : 0;

		// update crosshair
		if ( SF.draw_crosshair ) {
			SF.crosshair_at = event->x;
			gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
		}

		return TRUE;
	}



	gboolean
	daScoringFacMontage_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		auto& Ch = SF.channel_near( event->y);

		if ( SF.unfazer_mode == SScoringFacility::TUnfazerMode::calibrate && &Ch == SF.using_channel ) {
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
			gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);

			return TRUE;
		}

		if ( Ch.have_power() && Ch.draw_power && event->y > Ch.zeroy ) {
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
					gtk_widget_queue_draw( wid);
					break;
				case GDK_SCROLL_UP:
					Ch.power_display_scale *= 1.1;
					gtk_widget_queue_draw( wid);
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
			for_each( SF.channels.begin(), SF.channels.end(),
				  [&] ( SScoringFacility::SChannel& H)
				  {
					  H.signal_display_scale = Ch.signal_display_scale;
				  });

		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
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
							(gboolean)SF.using_channel->draw_filtered_signal);
		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
	}


	void
	iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.using_channel->draw_filtered_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
		if ( !SF.using_channel->draw_filtered_signal && !SF.using_channel->draw_original_signal )
			gtk_check_menu_item_set_active( SF.iSFPageShowOriginal,
							(gboolean)(SF.using_channel->draw_original_signal));
		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
	}


	void
	iSFPageUseResample_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.using_channel->use_resample = (bool)gtk_check_menu_item_get_active( checkmenuitem);
		gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
	}

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

