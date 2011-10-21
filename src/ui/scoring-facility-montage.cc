// ;-*-C++-*-
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




#include <cairo/cairo-svg.h>
#include <samplerate.h>

#include "misc.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;
using namespace aghui;


inline namespace {

	unsigned short __pagesize_ticks[] = {
		4, 5, 5, 3, 4, 6, 12, 24, 30
	};

}


void
aghui::SScoringFacility::SChannel::draw_signal( const valarray<TFloat>& signal,
						unsigned width, int vdisp, cairo_t *cr) const
{
	size_t	start = _p.cur_vpage_start() * samplerate(),
		end   = _p.cur_vpage_end()   * samplerate(),
		run = end - start,
		half_pad = run * _p.skirting_run_per1;
	if ( start == 0 ) {
		valarray<TFloat> padded (run + half_pad*2);
		padded[ slice(half_pad, run + half_pad, 1) ] = signal[ slice (0, run + half_pad, 1) ];
		::draw_signal( padded, 0, padded.size(),
			       width, vdisp, signal_display_scale, cr,
			       use_resample);

	} else if ( end > n_samples() ) {  // rather ensure more thorough padding
		valarray<TFloat> padded (run + half_pad*2);
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
aghui::SScoringFacility::SChannel::draw_page_static( cairo_t *cr,
						     int wd, int y0,
						     bool draw_marquee) const
{
	int	ptop = y0 - _p.interchannel_gap/2,
		pbot = ptop + _p.interchannel_gap;

      // marquee, goes first, not to obscure waveforms
	if ( draw_marquee // possibly undesired (such as when drawing for unfazer)
	     && overlap( selection_start_time, selection_end_time,
			 _p.cur_xvpage_start(), _p.cur_xvpage_end()) ) {
		double	pre = _p.skirting_run_per1 * _p.vpagesize(),
			ma = (selection_start_time - _p.cur_xvpage_start()) / _p.xvpagesize() * wd,
			me = (selection_end_time - _p.cur_xvpage_start()) / _p.xvpagesize() * wd;
		cairo_set_source_rgba( cr, .3, 1., .2, .4);
		cairo_rectangle( cr,
				 ma, ptop, me - ma, _p.interchannel_gap);
		cairo_fill( cr);
		cairo_stroke( cr);

	      // start/end timestamp
		cairo_set_font_size( cr, 10);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_source_rgb( cr, 1, .1, .1);

		cairo_text_extents_t extents;
		snprintf_buf( "%5.2fs",
			      selection_start_time - _p.cur_xvpage_start() - pre);
		cairo_text_extents( cr, __buf__, &extents);
		double ido = ma - 3 - extents.width;
		if ( ido < extents.width+3 )
			cairo_move_to( cr, ma+3, ptop + 30);
		else
			cairo_move_to( cr, ido, ptop + 12);
		cairo_show_text( cr, __buf__);

		if ( selection_end - selection_start > 5 ) {  // don't mark end if selection is too short
			snprintf_buf( "%5.2fs",
				      selection_end_time - _p.cur_xvpage_start() - pre);
			cairo_text_extents( cr, __buf__, &extents);
			ido = me + extents.width+3;
			if ( ido > wd )
				cairo_move_to( cr, me - 3 - extents.width, ptop + 30);
			else
				cairo_move_to( cr, me + 3, ptop + 12);
			cairo_show_text( cr, __buf__);

			snprintf_buf( "<-%4.2fs->", // "←%4.2fs→",
				      selection_end_time - selection_start_time);
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, ma+(me-ma)/2 - extents.width/2,
				       ptop + (extents.width < me-ma ? 12 : 30));
			cairo_show_text( cr, __buf__);
		}

		cairo_stroke( cr);
	}

      // zeroline
	if ( draw_zeroline ) {
		cairo_set_source_rgba( cr, .3, 1., .2, .4);
		cairo_move_to( cr, 0, y0);
		cairo_rel_line_to( cr, wd, 0);
		cairo_stroke( cr);
	}

      // waveform: signal_filtered
	bool one_signal_drawn = false;
	if ( draw_filtered_signal ) {
		// only show processed signal when done with unfazing
		cairo_set_line_width( cr, fine_line());
		cairo_set_source_rgb( cr, 0., 0., 0.); // bg is uniformly light shades

		draw_signal_filtered( wd, y0, cr);

		_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgb( cr);
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
			cairo_set_line_width( cr, fine_line() * .6);
			cairo_set_source_rgba( cr, 0., 0.3, 0., .4);
		} else {
			cairo_set_line_width( cr, fine_line());
			cairo_set_source_rgb( cr, 0., 0., 0.);
		}
		draw_signal_original( wd, y0, cr);

		_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgb( cr);
		cairo_move_to( cr, wd-120, y0 - 25);
		cairo_set_font_size( cr, 10);
		snprintf_buf( "orig");
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}

	size_t	half_pad = wd * _p.skirting_run_per1,
		ef = wd + 2*half_pad;

	int	half_pad_samples = _p.skirting_run_per1 * _p.vpagesize() * samplerate(),
		cvpa = _p.cur_vpage_start() * samplerate() - half_pad_samples,
		cvpe = _p.cur_vpage_end()   * samplerate() + half_pad_samples,
		evpz = cvpe - cvpa;
      // artifacts (changed bg)
	{
		auto& Aa = crecording.F()[name].artifacts;
		if ( not Aa.empty() ) {
			_p._p.CwB[SExpDesignUI::TColour::artifact].set_source_rgba( cr,  // do some gradients perhaps?
								.4);
			for ( auto A = Aa.begin(); A != Aa.end(); ++A ) {
				if ( overlap( (int)A->first, (int)A->second, cvpa, cvpe) ) {
					int	aa = (int)A->first - cvpa,
						ae = (int)A->second - cvpa;
					if ( aa < 0 )    aa = 0;
					if ( ae > evpz ) ae = evpz;
					cairo_rectangle( cr,
							 (float)(aa % evpz) / evpz * wd, ptop + _p.interchannel_gap * 1./3,
							 (float)(ae - aa) / evpz * wd,       _p.interchannel_gap * 1./3);
					cairo_fill( cr);
					cairo_stroke( cr);
				} else if ( (int)A->first > cvpe )  // no more artifacts up to and on current page
					break;
			}
			_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgb( cr);
			cairo_move_to( cr, ef-70, y0 + 15);
			cairo_set_font_size( cr, 8);
			snprintf_buf( "%4.2f %% dirty", percent_dirty);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}
	}

      // annotations
	{
		auto& Aa = crecording.F()[name].annotations;
		if ( not Aa.empty() ) {
			int on_this_page = 0;
			for ( auto A = Aa.begin(); A != Aa.end(); ++A ) {
				if ( overlap( (int)A->span.first, (int)A->span.second, cvpa, cvpe) ) {
					int disp = pbot - ++on_this_page * 5;
					cairo_pattern_t *cp = cairo_pattern_create_linear( 0., disp, 0., disp-30);
					_p._p.CwB[SExpDesignUI::TColour::annotations].pattern_add_color_stop_rgba( cp, 0.);
					_p._p.CwB[SExpDesignUI::TColour::annotations].pattern_add_color_stop_rgba( cp, .15, 0.1);
					_p._p.CwB[SExpDesignUI::TColour::annotations].pattern_add_color_stop_rgba( cp, 1., 0.);
					cairo_set_source( cr, cp);

					int	aa = (int)A->span.first - cvpa,
						ae = (int)A->span.second - cvpa;
					if ( aa < 0 )    aa = 0;
					if ( ae > evpz ) ae = evpz;
					cairo_rectangle( cr,
							 (float)(aa % evpz) / evpz * wd, disp-30,
							 (float)(ae - aa) / evpz * wd, 30);
					cairo_fill( cr);
					cairo_stroke( cr);
					cairo_pattern_destroy( cp);

					cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
					cairo_set_font_size( cr, 10);
					cairo_set_source_rgb( cr, 0., 0., 0.);
					cairo_move_to( cr, (float)(aa % evpz) / evpz * wd, disp - 4);
					cairo_show_text( cr, A->label.c_str());
				} else if ( (int)A->span.first > cvpe )  // no more artifacts up to and on current page
					break;
			}
		}
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

	_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgb( cr);

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
	{
		cairo_set_font_size( cr, 9);
		if ( have_low_pass() ) {
			snprintf_buf( "LP: %6.2f/%u", low_pass.cutoff, low_pass.order);
			cairo_move_to( cr, wd-100, y0 + 15);
			cairo_show_text( cr, __buf__);
		}
		if ( have_high_pass() ) {
			snprintf_buf( "HP: %6.2f/%u", high_pass.cutoff, high_pass.order);
			cairo_move_to( cr, wd-100, y0 + 24);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}

}




void
aghui::SScoringFacility::SChannel::draw_page( const char *fname, int width, int height) // to a file
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
aghui::SScoringFacility::SChannel::draw_page( cairo_t* cr)
{
	if ( hidden )
		return;

	draw_page_static( cr, _p.da_wd, zeroy, true);

	unsigned
		pbot = zeroy + _p.interchannel_gap / 2.,
		ptop = pbot - _p.interchannel_gap;
	bool	overlay = false;

       // PSD profile
	if ( draw_power and strcmp( type, "EEG") == 0 ) {
		overlay = true;

		cairo_set_line_width( cr, 1.);
		cairo_set_font_size( cr, 10);
		guint i;

		if ( draw_bands ) {
			// for ( TBand_underlying_type b = 0; b <= (TBand_underlying_type)Ch.uppermost_band; ++b ) {
			// the type conversions exactly as appearing above drive g++ to segfault
			// the same happens when (TBand_underlying_type) is replaced by (unsigned short)
			for ( agh::TBand b = agh::TBand::delta; b != uppermost_band; next(b) ) {
				auto& P = power_in_bands[(unsigned)b];
				_p._p.CwB[SExpDesignUI::band2colour(b)].set_source_rgba( cr, .5);
				cairo_move_to( cr, 0.,
					       - P[0] * power_display_scale + pbot);
				for ( i = 1; i < P.size(); ++i )
					cairo_line_to( cr, (double)i / P.size() * _p.da_wd,
						       - P[i] * power_display_scale + pbot);
				if ( b == focused_band ) {
					cairo_line_to( cr, _p.da_wd, pbot);
					cairo_line_to( cr,       0., pbot);
					cairo_fill( cr);
				}
				cairo_stroke( cr);

				if ( b == focused_band ) {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
					snprintf_buf( "%s %g–%g",
						      _p._p.FreqBandNames[(unsigned)b],
						      _p._p.freq_bands[(unsigned)b][0], _p._p.freq_bands[(unsigned)b][1]);
				} else {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
					snprintf_buf( "%s", _p._p.FreqBandNames[(unsigned)b]);
				}
				cairo_move_to( cr, _p.da_wd - 70,
					       ptop + (int)uppermost_band*12 - 12*(unsigned)b + 12);
				cairo_show_text( cr, __buf__);
				cairo_stroke( cr);
			}
		} else {
			_p._p.CwB[SExpDesignUI::TColour::power_sf].set_source_rgba( cr, .5);
			cairo_move_to( cr, 0, power[0]);
			for ( i = 0; i < power.size(); ++i )
				cairo_line_to( cr, (double)i / power.size() * _p.da_wd,
					       - power[i] * power_display_scale + pbot);
			cairo_line_to( cr, _p.da_wd, pbot);
			cairo_line_to( cr,       0., pbot);
			cairo_fill( cr);
			cairo_stroke( cr);

			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			snprintf_buf( "%g–%g Hz", from, upto);
			cairo_move_to( cr, _p.da_wd - 70, pbot - 15);
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
	}

      // EMG profile
	if ( draw_emg and strcmp( type, "EMG") == 0 ) {
		overlay = true;

		_p._p.CwB[SExpDesignUI::TColour::emg].set_source_rgba( cr, .7);
		cairo_set_line_width( cr, .3);
		double dps = (double)emg_profile.size() / _p.da_wd;
		cairo_move_to( cr, 0., pbot - EMGProfileHeight/2);
		size_t i = 0;
		for ( ; i < emg_profile.size(); ++i )
			cairo_line_to( cr, i / dps,
				       pbot - EMGProfileHeight/2 - emg_profile[i] * signal_display_scale/2);
		for ( ; i > 0; --i )
			cairo_line_to( cr, i / dps,
				       pbot - EMGProfileHeight/2 + emg_profile[i] * signal_display_scale/2);
		cairo_fill( cr);
		cairo_stroke( cr);
	}

	if ( overlay ) {
	      // hour ticks
		_p._p.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgba( cr, .5);
		cairo_set_line_width( cr, 1);
		cairo_set_font_size( cr, 10);
		float	hours4 = (float)n_samples() / samplerate() / 3600 * 4;
		for ( size_t i = 1; i < hours4; ++i ) {
			guint tick_pos = (float)i / hours4 * _p.da_wd;
			cairo_move_to( cr, tick_pos, pbot);
			cairo_rel_line_to( cr, 0, -((i%4 == 0) ? 20 : (i%2 == 0) ? 12 : 5));
			if ( i % 4 == 0 ) {
				snprintf_buf( "%2zuh", i/4);
				cairo_move_to( cr, tick_pos+5, pbot - 12);
				cairo_show_text( cr, __buf__);
			}
		}
		cairo_stroke( cr);

	      // cursor
		_p._p.CwB[SExpDesignUI::TColour::cursor].set_source_rgba( cr, .3);
		cairo_rectangle( cr,
				 (float) _p.cur_vpage() / _p.total_vpages() * _p.da_wd,  zeroy,
				 ceil( 1. / _p.total_vpages() * _p.da_wd), pbot - zeroy);
		cairo_fill( cr);
		cairo_stroke( cr);
	}

	_p.set_tooltip( SScoringFacility::TTipIdx::general);

      // crosshair (is drawn once in SScoringFacility::draw_montage), voltage at
	if ( _p.draw_crosshair ) {
		cairo_set_font_size( cr, 10);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		_p._p.CwB[SExpDesignUI::TColour::cursor].set_source_rgb( cr);
		if ( this == &_p.channels.front() )
			snprintf_buf( "%4.2f (%5.2fs)",
				      (draw_filtered_signal ? signal_filtered : signal_original)
				      [ (size_t)(_p.crosshair_at_time * samplerate()) ],
				      _p.crosshair_at_time - _p.cur_xvpage_start() - _p.skirting_run_per1 * _p.vpagesize());
		else
			snprintf_buf( "%4.2f",
				      (draw_filtered_signal ? signal_filtered : signal_original)
				      [ (size_t)(_p.crosshair_at_time * samplerate()) ]);

		cairo_move_to( cr, _p.crosshair_at+2, ptop + 22);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}

      // samples per pixel
	if ( _p.draw_spp ) {
		_p._p.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgb( cr);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size( cr, 8);
		cairo_move_to( cr, _p.da_wd-40, ptop + 11);
		snprintf_buf( "%4.2f spp", spp());
		cairo_show_text( cr, __buf__);
	}

	cairo_stroke( cr);
}




void
aghui::SScoringFacility::draw_montage( cairo_t* cr)
{
	double	true_frac = 1. - 1. / (1. + 2*skirting_run_per1);
	size_t	half_pad = da_wd * true_frac/2,
		ef = da_wd * (1. - true_frac);  // w + 10% = d

      // background, is now common to all channels
	using namespace agh;
	{
		float	ppart = (float)pagesize()/vpagesize();
		int	cp = cur_page();
		for ( int pp = cp-1; ; ++pp ) {
			SPage::TScore this_page_score = (pp < 0) ? SPage::TScore::none : SPage::char2score( hypnogram[pp]);
			_p.CwB[SExpDesignUI::score2colour(this_page_score)].set_source_rgba( cr, .2);
			float ppoff = ((float)pp * pagesize() - cur_vpage_start()) / vpagesize();
			if ( ppoff > 1.5 )
				break;
			cairo_rectangle( cr, half_pad + ppoff * ef,
					 0., ef * ppart, da_ht);
			cairo_fill( cr);
			cairo_stroke( cr);

			cairo_set_font_size( cr, 30);
			cairo_set_source_rgba( cr, 0., 0., 0., .1);
			cairo_move_to( cr, half_pad + ppoff * ef + ppart/2, da_ht-10);
			cairo_show_text( cr, SPage::score_name( this_page_score));
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
		_p.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgb( cr);
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

	// save/restore cairo contexts please

      // crosshair line
	if ( draw_crosshair ) {
		_p.CwB[SExpDesignUI::TColour::cursor].set_source_rgba( cr, .5);
		cairo_set_line_width( cr, .2);
		cairo_move_to( cr, crosshair_at, 0);
		cairo_rel_line_to( cr, 0, da_ht);
		cairo_stroke( cr);
	}
}




// eof

