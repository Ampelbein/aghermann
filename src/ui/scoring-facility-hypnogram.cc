// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-hypnogram.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-27
 *
 *         Purpose:  scoring facility (hypnogram)
 *
 *         License:  GPL
 */




#include <cairo/cairo.h>

#include "scoring-facility.hh"

using namespace std;


inline namespace {

unsigned short __score_hypn_depth[8] = {
	0, 20, 23, 30, 33, 5, 10, 1
};

}


void
aghui::SScoringFacility::
draw_hypnogram( cairo_t *cr)
{
      // bg
	_p.CwB[SExpDesignUI::TColour::hypnogram].set_source_rgb( cr);
	cairo_rectangle( cr, 0., 0., da_wd, HypnogramHeight);
	cairo_fill( cr);
	cairo_stroke( cr);

	if ( alt_hypnogram ) {
		for ( size_t i = 0; i < total_pages(); ++i ) {
			auto s = sigfile::SPage::char2score( hypnogram[i]);
			if ( s != sigfile::SPage::TScore::none ) {
				_p.CwB[SExpDesignUI::score2colour(s)].set_source_rgba( cr, .4);
				cairo_rectangle( cr,
						 (float)i/total_pages() * da_wd, 0,
						 1./total_pages() * da_wd, da_ht);
				cairo_fill( cr);
				cairo_stroke( cr);
			}
		}
	} else {
		_p.CwB[SExpDesignUI::TColour::hypnogram_scoreline].set_source_rgba( cr, .5);
		cairo_set_line_width( cr, .4);
		for ( size_t i = 1; i < (size_t)sigfile::SPage::TScore::_total; ++i ) {
			cairo_move_to( cr, 0,     __score_hypn_depth[i]);
			cairo_line_to( cr, da_wd, __score_hypn_depth[i]);
		}
		cairo_stroke( cr);

	      // scores
		_p.CwB[SExpDesignUI::TColour::hypnogram_scoreline].set_source_rgba( cr, 1.);
		cairo_set_line_width( cr, 3.);
		// these lines can be discontinuous
		for ( size_t i = 0; i < total_pages(); ++i ) {
			char c = hypnogram[i];
			if ( c != sigfile::SPage::score_code( sigfile::SPage::TScore::none) ) {
				int y = __score_hypn_depth[ (size_t)sigfile::SPage::char2score(c) ];
				cairo_move_to( cr, (float)i/total_pages() * da_wd, y);
				cairo_rel_line_to( cr, 1./total_pages() * da_wd, 0);
			}
		}
		cairo_stroke( cr);
	}

      // extra: annotations
	{
		_p.CwB[SExpDesignUI::TColour::annotations].set_source_rgba( cr, .6);
		cairo_set_line_width( cr, 18.);

		auto total_seconds = total_pages() * pagesize();
		for ( auto &H : channels ) {
			size_t this_sr = H.samplerate();
			for ( auto &A : H.annotations ) {
				cairo_move_to( cr, (double)A.span.first / this_sr / total_seconds * da_wd, 4);
				cairo_line_to( cr, (double)A.span.second / this_sr / total_seconds * da_wd, 4);
			}
		}
		cairo_stroke( cr);
	}

      // extra: artifacts
	{
		_p.CwB[SExpDesignUI::TColour::artifact].set_source_rgba( cr, .6);
		cairo_set_line_width( cr, 12.);

		auto total_seconds = total_pages() * pagesize();
		for ( auto &H : channels ) {
			size_t this_sr = H.samplerate();
			for ( auto &A : H.artifacts() ) {
				cairo_move_to( cr, (double)A.first / this_sr / total_seconds * da_wd, 12);
				cairo_line_to( cr, (double)A.second / this_sr / total_seconds * da_wd, 12);
			}
		}
		cairo_stroke( cr);
	}

      // hour ticks
	_draw_hour_ticks( cr, 0, HypnogramHeight);
}



// eof

