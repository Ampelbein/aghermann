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
		along -= tl_left_margin();
		for ( auto& E : csubject.measurements[*_p._p._AghDi].episodes )
			if ( along >= _p._p.T2P(E.start_rel) && along <= _p._p.T2P(E.end_rel) ) {
				using_episode = &E;
				return true;
			}
		using_episode = nullptr;
		return false;
	} catch (...) {
		using_episode = nullptr;
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
		      agh::CSubject::gender_sign( csubject.gender),
		      csubject.age);
	cairo_show_text( cr, __buf__);

	cairo_move_to( cr, 2, 12);
	cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 12);
	cairo_show_text( cr, csubject.name());
	cairo_stroke( cr);

	if ( cscourse == nullptr || cscourse->mm_list().empty() ) {
		cairo_move_to( cr, 50, timeline_height()/2+9);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size( cr, 18);
		cairo_set_source_rgba( cr, 0., 0., 0., .13);
		cairo_show_text( cr, "(no episodes)");
		cairo_stroke( cr);
		return;
	}

      // day and night
	{
		cairo_pattern_t *cp = cairo_pattern_create_linear( tl_left_margin(), 0., timeline_width() - tl_right_margin(), 0.);
		struct tm clock_time;
		memcpy( &clock_time, localtime( &_p._p.timeline_start), sizeof(clock_time));
		clock_time.tm_hour = 4;
		clock_time.tm_min = clock_time.tm_sec = 0;
		time_t	dawn = mktime( &clock_time),
			t;
		bool day = false;
		for ( t = dawn; t < timeline_end(); t += 3600 * 12, day = !day )
			if ( t > timeline_start() )
				_p._p.CwB[day ? TColour::day : TColour::night].
					pattern_add_color_stop_rgba( cp, (double)_p._p.T2P(t) / timeline_width());
		cairo_set_source( cr, cp);
		cairo_rectangle( cr, tl_left_margin(), 0., tl_left_margin() + timeline_width(), timeline_height());
		cairo_fill( cr);
		cairo_stroke( cr);
		cairo_pattern_destroy( cp);
	}

	struct tm tl_start_fixed_tm;
	memcpy( &tl_start_fixed_tm, localtime( &_p._p.timeline_start), sizeof(struct tm));
	// determine the latest full hour before timeline_start
	tl_start_fixed_tm.tm_min = 0;
	time_t tl_start_fixed = mktime( &tl_start_fixed_tm);

      // profile
	auto& episodes = csubject.measurements[*_p._p._AghDi].episodes;
	// boundaries, with scored percentage bars
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size( cr, 11);
	for ( auto& E : episodes ) {
		unsigned
			e_pixel_start = _p._p.T2P( E.start_rel),
			e_pixel_end   = _p._p.T2P( E.end_rel),
			e_pixels = e_pixel_end - e_pixel_start;

		// episode start timestamp
		cairo_move_to( cr, tl_left_margin() + e_pixel_start + 2, 12);
		cairo_set_source_rgb( cr, 1., 1., 1.);
		strftime( __buf__, 79, "%F %T",
			  localtime( &E.start_time()));
		g_string_printf( __ss__, "%s | %s",
				 __buf__, E.name());
		cairo_show_text( cr, __ss__->str);
		cairo_stroke( cr);

		// highlight
		if ( is_focused && using_episode == &E ) {
			const auto fuzz = 10;
			cairo_pattern_t *cp =
				cairo_pattern_create_linear(
					tl_left_margin() + e_pixel_start - fuzz, 0,
					tl_left_margin() + e_pixel_start + e_pixels + fuzz, 0);
			cairo_pattern_add_color_stop_rgba( cp, 0.,					1., 1., 1., 0.);
			cairo_pattern_add_color_stop_rgba( cp, 0. + (double)fuzz/(e_pixels + fuzz*2),	1., 1., 1., .3);
			cairo_pattern_add_color_stop_rgba( cp, 1. - (double)fuzz/(e_pixels + fuzz*2),	1., 1., 1., .3);
			cairo_pattern_add_color_stop_rgba( cp, 1.,					1., 1., 1., 0.);

			cairo_set_line_width( cr, .2);
			cairo_set_source( cr, cp);
			cairo_rectangle( cr,
					 tl_left_margin() + e_pixel_start - fuzz, 0,
					 e_pixels + fuzz*2, timeline_height());
			cairo_fill( cr);
			cairo_stroke( cr);
			cairo_pattern_destroy( cp);
		}

		// percentage bar graph
		float pc_scored, pc_nrem, pc_rem, pc_wake;
		pc_scored = E.sources.front().percent_scored( &pc_nrem, &pc_rem, &pc_wake);
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

      // profile
	unsigned
		j_tl_pixel_start = _p._p.T2P( episodes.front().start_rel),
		j_tl_pixel_end   = _p._p.T2P( episodes.back().end_rel),
		j_tl_pixels = j_tl_pixel_end - j_tl_pixel_start;
	auto&	scale = (_p._p.display_profile_type == sigfile::TMetricType::Psd) ? _p._p.profile_scale_psd : _p._p.profile_scale_mc;

	_p._p.CwB[TColour::power_mt].set_source_rgb( cr);
	cairo_set_line_width( cr, .3);
	cairo_move_to( cr, tl_left_margin() + j_tl_pixel_start, timeline_height()-12);
	for ( size_t i = 0; i < cscourse->timeline().size(); ++i )
		cairo_line_to( cr,
			       tl_left_margin() + j_tl_pixel_start + ((float)i)/cscourse->timeline().size() * j_tl_pixels,
			       -(*cscourse)[i].metric * scale + timeline_height()-12);
	cairo_line_to( cr, j_tl_pixel_start + tl_left_margin() + j_tl_pixels, timeline_height()-12);
	cairo_fill( cr);
	cairo_stroke( cr);

      // ticks
	if ( is_focused ) {
		cairo_set_line_width( cr, .5);
		_p._p.CwB[TColour::ticks_mt].set_source_rgb( cr);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		unsigned clock_d0 = localtime(&tl_start_fixed)->tm_mday + 1;
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
