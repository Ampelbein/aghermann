// ;-*-C++-*-
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;
using namespace aghui;

const char* const
	aghui::SExpDesignUI::mannotations_column_names[] = {
	"Recording", "Pages", "Channel", "Label"
};


bool
aghui::SExpDesignUI::SSubjectPresentation::get_episode_from_timeline_click( unsigned along)
{
	try {
		auto& ee = csubject.measurements[*_p._p._AghDi].episodes;
		along -= tl_left_margin();
		for ( auto e = ee.begin(); e != ee.end(); ++e )
			if ( along >= _p._p.T2P(e->start_rel) && along <= _p._p.T2P(e->end_rel) ) {
				using_episode = e;
				return true;
			}
		using_episode = ee.end();
		return false;
	} catch (...) {
		using_episode = csubject.measurements[*_p._p._AghDi].episodes.end();
		return false;
	}
}

void
aghui::SExpDesignUI::SSubjectPresentation::draw_timeline( const char *fname) const
{
#ifdef CAIRO_HAS_SVG_SURFACE
	cairo_surface_t *cs =
		cairo_svg_surface_create( fname,
					  timeline_width() + tl_left_margin() + tl_right_margin(),
					  timeline_height());
	cairo_t *cr = cairo_create( cs);
	draw_timeline( cr);
	cairo_destroy( cr);
	cairo_surface_destroy( cs);
#endif
}


void
aghui::SExpDesignUI::SSubjectPresentation::draw_timeline( cairo_t *cr) const
{
      // draw subject name, gender and age
	cairo_move_to( cr, 2, 28);
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size( cr, 9);
	snprintf_buf( "%s %u y.o.",
		      csubject.gender == agh::CSubject::TGender::female
		      ? "F"
		      : (csubject.gender == agh::CSubject::TGender::male
			 ? "M" : "??"),
		      csubject.age);
	cairo_show_text( cr, __buf__);

	cairo_move_to( cr, 2, 12);
	cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 12);
	cairo_show_text( cr, csubject.name());

	if ( cscourse == NULL ) {
		cairo_stroke( cr);
		cairo_move_to( cr, 50, timeline_height()/2+9);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 18);
		cairo_set_source_rgba( cr, 0., 0., 0., .13);
		cairo_show_text( cr, "(no episodes)");
		return;
	}

      // draw day and night
	{
		cairo_pattern_t *cp = cairo_pattern_create_linear( tl_left_margin(), 0., timeline_width() - tl_right_margin(), 0.);
		struct tm clock_time;
		memcpy( &clock_time, localtime( &_p._p.timeline_start), sizeof(clock_time));
		clock_time.tm_hour = 4;
		clock_time.tm_min = clock_time.tm_sec = 0;
		time_t	dawn = mktime( &clock_time),
			t;
		bool up = true;
		for ( t = dawn; t < timeline_end(); t += 3600 * 12, up = !up )
			if ( t > timeline_start() ) {
				//printf( "part %lg %d\n", (double)T2P(t) / __timeline_pixels, up);
				cairo_pattern_add_color_stop_rgb( cp, (double)_p._p.T2P(t) / timeline_width(), up?.5:.8, up?.4:.8, 1.);
			}
		cairo_set_source( cr, cp);
		cairo_rectangle( cr, tl_left_margin(), 0., tl_left_margin() + timeline_width(), timeline_height());
		cairo_fill( cr);
		cairo_pattern_destroy( cp);
	}
	cairo_stroke( cr);

	struct tm tl_start_fixed_tm;
	memcpy( &tl_start_fixed_tm, localtime( &_p._p.timeline_start), sizeof(struct tm));
	// determine the latest full hour before timeline_start
	tl_start_fixed_tm.tm_min = 0;
	time_t tl_start_fixed = mktime( &tl_start_fixed_tm);

      // SWA
	if ( cscourse == NULL )
		return;

	auto& ee = csubject.measurements[*_p._p._AghDi].episodes;
//	printf( "csubject %s ", csubject.name());

	// boundaries, with scored percentage bars
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size( cr, 11);
	for ( auto e = ee.begin(); e != ee.end(); ++e ) {
		unsigned
			e_pixel_start = _p._p.T2P( e->start_rel),
			e_pixel_end   = _p._p.T2P( e->end_rel),
			e_pixels = e_pixel_end - e_pixel_start;

		// episode start timestamp
		cairo_move_to( cr, tl_left_margin() + e_pixel_start + 2, 12);
		cairo_set_source_rgb( cr, 1., 1., 1.);
		strftime( __buf__, 79, "%F %T",
			  localtime( &e->start_time()));
		g_string_printf( __ss__, "%s | %s",
				 __buf__, e->name());
		cairo_show_text( cr, __ss__->str);
		cairo_stroke( cr);

		// highlight
		if ( is_focused && using_episode == e ) {
			cairo_set_line_width( cr, .2);
			cairo_set_source_rgba( cr, 1., 1., 1., .5);
			cairo_rectangle( cr,
					 tl_left_margin() + e_pixel_start, 0,
					 e_pixels, timeline_height());
			cairo_fill( cr);
			cairo_stroke( cr);
		}

		// percentage bar graph
		float pc_scored, pc_nrem, pc_rem, pc_wake;
		pc_scored = e->sources.front().percent_scored( &pc_nrem, &pc_rem, &pc_wake);
		pc_scored *= e_pixels / 100.;
		pc_nrem   *= e_pixels / 100.;
		pc_rem    *= e_pixels / 100.;
		pc_wake   *= e_pixels / 100.;

		cairo_set_line_width( cr, 4);

		cairo_set_source_rgb( cr, 0., .1, .9);
		cairo_move_to( cr, tl_left_margin() + e_pixel_start + 2, timeline_height()-5);
		cairo_rel_line_to( cr, pc_nrem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, .9, .0, .5);
		cairo_move_to( cr, tl_left_margin() + e_pixel_start + 2 + pc_nrem, timeline_height()-5);
		cairo_rel_line_to( cr, pc_rem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, 0., .9, .1);
		cairo_move_to( cr, tl_left_margin() + e_pixel_start + 2 + pc_nrem + pc_rem, timeline_height()-5);
		cairo_rel_line_to( cr, pc_wake, 0);
		cairo_stroke( cr);

		cairo_set_line_width( cr, 10);
		cairo_set_source_rgba( cr, 1., 1., 1., .5);
		cairo_move_to( cr, tl_left_margin() + e_pixel_start + 2, timeline_height()-5);
		cairo_rel_line_to( cr, pc_scored, 0);
		cairo_stroke( cr);
	}

      // power
	unsigned
		j_tl_pixel_start = _p._p.T2P( ee.front().start_rel),
		j_tl_pixel_end   = _p._p.T2P( ee.back().end_rel),
		j_tl_pixels = j_tl_pixel_end - j_tl_pixel_start;

	_p._p.CwB[TColour::power_mt].set_source_rgb( cr);
	cairo_set_line_width( cr, .3);
	cairo_move_to( cr, tl_left_margin() + j_tl_pixel_start, timeline_height()-12);
	for ( size_t i = 0; i < cscourse->timeline().size(); ++i )
		// if ( i %10 == 0 )
		// 	printf( "[%zu] %g %g\n", i, (*cscourse)[i].SWA, PPuV2);
		cairo_line_to( cr,
			        tl_left_margin() + j_tl_pixel_start + ((float)i)/cscourse->timeline().size() * j_tl_pixels,
			       -(*cscourse)[i].SWA * _p._p.ppuv2 + timeline_height()-12);
	cairo_line_to( cr, j_tl_pixel_start + tl_left_margin() + j_tl_pixels, timeline_height()-12);
	cairo_fill( cr);
	cairo_stroke( cr);

      // ticks
	if ( is_focused ) {
		cairo_set_line_width( cr, .5);
		_p._p.CwB[TColour::ticks_mt].set_source_rgb( cr);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		unsigned clock_d0 = localtime(&tl_start_fixed)->tm_mday;
		for ( time_t t = tl_start_fixed; t <= timeline_end(); t += 3600 ) {
			size_t x = _p._p.T2P(t);
			unsigned
				clock_h  = localtime(&t)->tm_hour,
				clock_d  = localtime(&t)->tm_mday;
			if ( clock_h % 6 == 0 ) {
				cairo_set_font_size( cr, (clock_h % 24 == 0) ? 10 : 8);
				cairo_move_to( cr, tl_left_margin() + x, ( clock_h % 24 == 0 ) ? 0 : (timeline_height() - 16));
				cairo_line_to( cr, tl_left_margin() + x, timeline_height() - 10);

				snprintf_buf_ts_h( (clock_d - clock_d0) * 24 + clock_h);
				cairo_text_extents_t extents;
				cairo_text_extents( cr, __buf__, &extents);
				cairo_move_to( cr, tl_left_margin() + x - extents.width/2, timeline_height()-1);
				cairo_show_text( cr, __buf__);

			} else {
				cairo_move_to( cr, tl_left_margin() + x, timeline_height() - 14);
				cairo_line_to( cr, tl_left_margin() + x, timeline_height() - 7);
			}
		}
		cairo_stroke( cr);
	}
}

// eof
