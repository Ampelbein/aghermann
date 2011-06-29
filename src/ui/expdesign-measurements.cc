// ;-*-C++-*- *  Time-stamp: "2011-06-29 12:38:46 hmmr"
/*
 *       File name:  ui/expdesign-measurements.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI measurements view
 *
 *         License:  GPL
 */


#include <cstring>
#include <ctime>

#include <cairo.h>
#include <cairo-svg.h>

#include "misc.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

using namespace aghui;



bool
aghui::SSubjectPresentation::get_episode_from_timeline_click( unsigned along)
{
	try {
		auto& ee = csubject.measurements[*_AghDi].episodes;
		along -= __tl_left_margin;
		for ( auto e = ee.begin(); e != ee.end(); ++e )
			if ( along >= T2P(e->start_rel) && along <= T2P(e->end_rel) ) {
				episode_focused = e;
				return true;
			}
		episode_focused = ee.end();
		return false;
	} catch (...) {
		episode_focused = csubject.measurements[*_AghDi].episodes.end();
		return false;
	}
}

void
aghui::SSubjectPresentation::draw_timeline( const char *fname) const
{
#ifdef CAIRO_HAS_SVG_SURFACE
	cairo_surface_t *cs =
		cairo_svg_surface_create( fname,
					  __timeline_pixels + __tl_left_margin + __tl_right_margin,
					  settings::WidgetSize_MVTimelineHeight);
	cairo_t *cr = cairo_create( cs);
	draw_timeline( cr);
	cairo_destroy( cr);
	cairo_surface_destroy( cs);
#endif
}


void
aghui::SSubjectPresentation::draw_timeline( cairo_t *cr) const
{
	// draw subject name
	cairo_move_to( cr, 2, 15);
	cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 11);
	cairo_show_text( cr, csubject.name());

	if ( cscourse == NULL ) {
		cairo_stroke( cr);
		cairo_move_to( cr, 50, settings::WidgetSize_MVTimelineHeight/2+9);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 18);
		cairo_set_source_rgba( cr, 0., 0., 0., .13);
		cairo_show_text( cr, "(no episodes)");
		return;
	}

	// draw day and night
	{
		cairo_pattern_t *cp = cairo_pattern_create_linear( __tl_left_margin, 0., __timeline_pixels-__tl_right_margin, 0.);
		struct tm clock_time;
		memcpy( &clock_time, localtime( &__timeline_start), sizeof(clock_time));
		clock_time.tm_hour = 4;
		clock_time.tm_min = clock_time.tm_sec = 0;
		time_t	dawn = mktime( &clock_time),
			t;
		gboolean up = TRUE;
		for ( t = dawn; t < __timeline_end; t += 3600 * 12, up = !up )
			if ( t > __timeline_start ) {
				//printf( "part %lg %d\n", (double)T2P(t) / __timeline_pixels, up);
				cairo_pattern_add_color_stop_rgb( cp, (double)T2P(t) / __timeline_pixels, up?.5:.8, up?.4:.8, 1.);
			}
		cairo_set_source( cr, cp);
		cairo_rectangle( cr, __tl_left_margin, 0., __tl_left_margin+__timeline_pixels, settings::WidgetSize_MVTimelineHeight);
		cairo_fill( cr);
		cairo_pattern_destroy( cp);
	}

	struct tm tl_start_fixed_tm;
	memcpy( &tl_start_fixed_tm, localtime( &__timeline_start), sizeof(struct tm));
	// determine the latest full hour before __timeline_start
	tl_start_fixed_tm.tm_min = 0;
	time_t tl_start_fixed = mktime( &tl_start_fixed_tm);

      // SWA
	if ( cscourse == NULL ) {
		cairo_stroke( cr);
		return;
	}

	auto& ee = csubject.measurements[*_AghDi].episodes;

	// boundaries, with scored percentage bars
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size( cr, 11);
	for ( auto e = ee.begin(); e != ee.end(); ++e ) {
		unsigned
			e_pixel_start = T2P( e->start_rel),
			e_pixel_end   = T2P( e->end_rel),
			e_pixels = e_pixel_end - e_pixel_start;

		// episode start timestamp
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2, 12);
		cairo_set_source_rgb( cr, 1., 1., 1.);
		strftime( __buf__, 79, "%F %T",
			  localtime( &e->start_time()));
		g_string_printf( __ss__, "%s | %s",
				 __buf__, e->name());
		cairo_show_text( cr, __ss__->str);
		cairo_stroke( cr);

		// highlight
		if ( is_focused && episode_focused == e ) {
			cairo_set_line_width( cr, .2);
			cairo_set_source_rgba( cr, 1., 1., 1., .5);
			cairo_rectangle( cr,
					 __tl_left_margin + e_pixel_start, 0,
					 e_pixels, settings::WidgetSize_MVTimelineHeight);
			cairo_fill( cr);
			cairo_stroke( cr);
		}

		// percentage bar graph
		float pc_scored, pc_nrem, pc_rem, pc_wake;
		pc_scored = e->sources.front().percent_scored( &pc_nrem, &pc_rem, &pc_wake);

		pc_scored *= e_pixels / 100;
		pc_nrem   *= e_pixels / 100;
		pc_rem    *= e_pixels / 100;
		pc_wake   *= e_pixels / 100;

		cairo_set_line_width( cr, 4);

		cairo_set_source_rgb( cr, 0., .1, .9);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2, settings::WidgetSize_MVTimelineHeight-5);
		cairo_rel_line_to( cr, pc_nrem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, .9, .0, .5);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2 + pc_nrem, settings::WidgetSize_MVTimelineHeight-5);
		cairo_rel_line_to( cr, pc_rem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, 0., .9, .1);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2 + pc_nrem + pc_rem, settings::WidgetSize_MVTimelineHeight-5);
		cairo_rel_line_to( cr, pc_wake, 0);
		cairo_stroke( cr);

		cairo_set_line_width( cr, 10);
		cairo_set_source_rgba( cr, 1., 1., 1., .5);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2, settings::WidgetSize_MVTimelineHeight-5);
		cairo_rel_line_to( cr, pc_scored, 0);
		cairo_stroke( cr);
	}

      // power
	unsigned
		j_tl_pixel_start = T2P( ee.front().start_rel),
		j_tl_pixel_end   = T2P( ee.back().end_rel),
		j_tl_pixels = j_tl_pixel_end - j_tl_pixel_start;

	CwB[TColour::power_mt].set_source_rgb( cr);
	cairo_set_line_width( cr, .3);
	cairo_move_to( cr, __tl_left_margin + j_tl_pixel_start, settings::WidgetSize_MVTimelineHeight-12);
	for ( size_t i = 0; i < cscourse->timeline().size(); ++i )
		// if ( i %10 == 0 )
		// 	printf( "[%zu] %g %g\n", i, (*cscourse)[i].SWA, PPuV2);
		cairo_line_to( cr,
			        __tl_left_margin + j_tl_pixel_start + ((float)i)/cscourse->timeline().size() * j_tl_pixels,
			       -(*cscourse)[i].SWA * PPuV2 + settings::WidgetSize_MVTimelineHeight-12);
	cairo_line_to( cr, j_tl_pixel_start + __tl_left_margin + j_tl_pixels, settings::WidgetSize_MVTimelineHeight-12);
	cairo_fill( cr);
	cairo_stroke( cr);

      // ticks
	if ( is_focused ) {
		cairo_set_line_width( cr, .5);
		CwB[TColour::ticks_mt].set_source_rgb( cr);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 8);
		unsigned clock_d0 = localtime(&tl_start_fixed)->tm_mday;
		for ( time_t t = tl_start_fixed; t <= __timeline_end; t += 3600 ) {
			size_t x = T2P(t);
			unsigned
				clock_h  = localtime(&t)->tm_hour,
				clock_d  = localtime(&t)->tm_mday;
			if ( clock_h % 6 == 0 ) {
				cairo_move_to( cr, __tl_left_margin + x, ( clock_h % 24 == 0 ) ? 0 : (settings::WidgetSize_MVTimelineHeight - 16));
				cairo_line_to( cr, __tl_left_margin + x, settings::WidgetSize_MVTimelineHeight - 10);

				snprintf_buf_ts_h( (clock_d - clock_d0) * 24 + clock_h);
				cairo_text_extents_t extents;
				cairo_text_extents( cr, __buf__, &extents);
				cairo_move_to( cr, __tl_left_margin + x - extents.width/2, settings::WidgetSize_MVTimelineHeight-1);
				cairo_show_text( cr, __buf__);

			} else {
				cairo_move_to( cr, __tl_left_margin + x, settings::WidgetSize_MVTimelineHeight - 14);
				cairo_line_to( cr, __tl_left_margin + x, settings::WidgetSize_MVTimelineHeight - 7);
			}
		}
		cairo_stroke( cr);
	}
}



