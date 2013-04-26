/*
 *       File name:  ui/sf/sf-montage-overlays.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-04
 *
 *         Purpose:  scoring facility: montage drawing area (profile overlays)
 *
 *         License:  GPL
 */

#include <cairo/cairo-svg.h>

#include "common/lang.hh"
#include "ui/misc.hh"
#include "sf.hh"

using namespace std;


void
aghui::SScoringFacility::SChannel::
draw_overlays( cairo_t* cr,
	       const int wd, const float zeroy) const
{
	if ( _p.mode != TMode::scoring )
		return;

	float	pbot = zeroy + _p.interchannel_gap / 2.2,
		ptop = zeroy - _p.interchannel_gap / 2.2;
	bool	overlay = false;

       // PSD profile
	if ( draw_psd and type == sigfile::SChannel::TType::eeg ) {
		overlay = true;

		cairo_set_line_width( cr, 1.);
		cairo_set_font_size( cr, 10);
		guint i;

		if ( draw_psd_bands ) {
			for ( size_t b = metrics::psd::TBand::delta; b <= psd.uppermost_band; ++b ) {
				auto& P = psd.course_in_bands[b];
				_p._p.CwB[SExpDesignUI::band2colour((metrics::psd::TBand)b)].set_source_rgba( cr, .5);
				double zero = 0.5 / P.size() * _p.da_wd;
				cairo_move_to( cr, zero,
					       - P[0] * psd.display_scale + pbot);
				for ( i = 1; i < P.size(); ++i )
					cairo_line_to( cr, ((double)i+0.5) / P.size() * _p.da_wd,
						       - P[i] * psd.display_scale + pbot);
				if ( b == psd.focused_band ) {
					cairo_line_to( cr, _p.da_wd, pbot);
					cairo_line_to( cr,     zero, pbot);
					cairo_fill( cr);
				}
				cairo_stroke( cr);

				if ( b == psd.focused_band ) {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
					snprintf_buf( "%s %g–%g",
						      _p._p.FreqBandNames[(unsigned)b],
						      _p._p.freq_bands[(unsigned)b][0], _p._p.freq_bands[(unsigned)b][1]);
				} else {
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
					snprintf_buf( "%s", _p._p.FreqBandNames[(unsigned)b]);
				}
				cairo_move_to( cr, _p.da_wd - 170,
					       ptop + psd.uppermost_band*12 - 12*(unsigned)b + 12);
				cairo_show_text( cr, __buf__);
				cairo_stroke( cr);
			}
		} else {
			_p._p.CwB[SExpDesignUI::TColour::sf_profile_psd].set_source_rgba( cr, .5);
			double zero = 0.5 / psd.course.size() * _p.da_wd;
			cairo_move_to( cr, zero,
				       psd.course[0]);
			for ( i = 0; i < psd.course.size(); ++i )
				cairo_line_to( cr, ((double)i+.5) / psd.course.size() * _p.da_wd,
					       - psd.course[i] * psd.display_scale + pbot);
			cairo_line_to( cr, _p.da_wd, pbot);
			cairo_line_to( cr,     zero, pbot);
			cairo_fill( cr);
			cairo_stroke( cr);

			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			snprintf_buf( "%g–%g Hz", psd.from, psd.upto);
			cairo_move_to( cr, _p.da_wd - 170, pbot - 15);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}

	      // scale
		{
			cairo_set_source_rgb( cr, 0., 0., 0.);
			cairo_set_line_width( cr, 1.5);
			double dpuf =
				agh::alg::sensible_scale_reduction_factor(
					1e6 * psd.display_scale, _p.interchannel_gap/2);
			int x = 30;
			cairo_move_to( cr, x, pbot - 5);
			cairo_rel_line_to( cr, 0, -dpuf * (1e6 * psd.display_scale));
			cairo_stroke( cr);

			cairo_set_font_size( cr, 9);
			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_move_to( cr, x + 5, pbot - 20);
			snprintf_buf( "%g uV2", dpuf);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}

		if ( draw_spectrum and _p.pagesize_is_right()
		     and _p.cur_page() < crecording.full_pages() ) {
			guint	gx = 120,
				gy = ptop + 25,
				gh = min( 60.f, pbot - ptop - 5),
				gw  = 80;
			size_t	m;

			cairo_set_source_rgba( cr, 1., 1., 1., .8);
			cairo_rectangle( cr, gx-15, gy-5, gw+15+5, gh+5+5);
			cairo_fill( cr);

			// grid lines
			_p._p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgba( cr, .1);
			cairo_set_line_width( cr, .3);
			for ( size_t i = 1; i < last_spectrum_bin; ++i ) {
				cairo_move_to( cr, gx + (float)i/last_spectrum_bin * gw, gy);
				cairo_rel_line_to( cr, 0, gh);
			}
			cairo_stroke( cr);

			// spectrum
			_p._p.CwB[SExpDesignUI::TColour::sf_ticks].set_source_rgba( cr, .8);
			cairo_set_line_width( cr, 2);
			float factor = psd.display_scale / crecording.psd_profile.Pp.binsize;
			cairo_move_to( cr,
				       gx, gy + gh - (2 + spectrum[0] * factor));
			for ( m = 1; m < last_spectrum_bin; ++m ) {
				cairo_line_to( cr,
					       gx + (float)m / last_spectrum_bin * gw,
					       gy + gh - spectrum[m] * factor);
			}
			cairo_stroke( cr);

			// axes
			_p._p.CwB[SExpDesignUI::TColour::sf_ticks].set_source_rgba( cr, .5);
			cairo_set_line_width( cr, .5);
			cairo_move_to( cr, gx, gy);
			cairo_rel_line_to( cr, 0, gh);
			cairo_rel_line_to( cr, gw, 0);

			// y ticks
			m = 0;
			while ( (++m * 1e6) < gh / factor ) {
				cairo_move_to( cr, gx-3,  gy + gh - (2 + (float)m*1e6 * factor));
				cairo_rel_line_to( cr, 3, 0);
			}
			cairo_stroke( cr);

			// labels
			cairo_text_extents_t extents;
			_p._p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgba( cr);
			cairo_set_font_size( cr, 8);

			snprintf_buf( "%g Hz",
				      last_spectrum_bin * crecording.psd_profile.Pp.binsize);
//				      draw_spectrum_absolute ? 'A' : 'R');
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr,
				       gx + gw - extents.width - 5,
				       gy + 4);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}
	}

	if ( draw_mc and type == sigfile::SChannel::TType::eeg ) {
		overlay = true;

		cairo_set_line_width( cr, 1.);
		cairo_set_font_size( cr, 10);
		guint i;

		_p._p.CwB[SExpDesignUI::TColour::sf_profile_mc].set_source_rgba( cr, .5);
		double zero = 0.5 / mc.course.size() * _p.da_wd;
		cairo_move_to( cr, zero,
			       mc.course[0]);
		for ( i = 0; i < mc.course.size(); ++i )
			cairo_line_to( cr, ((double)i+.5) / mc.course.size() * _p.da_wd,
				       - mc.course[i] * mc.display_scale + pbot);
		cairo_line_to( cr, _p.da_wd, pbot);
		cairo_line_to( cr,     zero, pbot);
		cairo_fill( cr);
		cairo_stroke( cr);

		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		snprintf_buf( "f0:%g", mc.f0);
		cairo_move_to( cr, _p.da_wd - 70, pbot - 30);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);

	      // scale
		{
			cairo_set_source_rgb( cr, 0., 0., 0.);
			cairo_set_line_width( cr, 1.5);
			double dpuf =
				agh::alg::sensible_scale_reduction_factor(
					mc.display_scale, _p.interchannel_gap/2);
			int x = 80;
			cairo_move_to( cr, x, pbot - 5);
			cairo_rel_line_to( cr, 0, -dpuf * mc.display_scale);
			cairo_stroke( cr);

			cairo_set_font_size( cr, 9);
			//cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_move_to( cr, x + 5, pbot - 20);
			snprintf_buf( "%g a.u.", dpuf);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}

	}

	if ( draw_swu and type == sigfile::SChannel::TType::eeg ) {
		overlay = true;

		cairo_set_line_width( cr, 1.);
		cairo_set_font_size( cr, 10);
		guint i;

		_p._p.CwB[SExpDesignUI::TColour::sf_profile_swu].set_source_rgba( cr, .5);
		double zero = 0.5 / swu.course.size() * _p.da_wd;
		cairo_move_to( cr, zero,
			       swu.course[0]);
		for ( i = 0; i < swu.course.size(); ++i )
			cairo_line_to( cr, ((double)i+.5) / swu.course.size() * _p.da_wd,
				       - swu.course[i] * swu.display_scale + pbot);
		cairo_line_to( cr, _p.da_wd, pbot);
		cairo_line_to( cr,     zero, pbot);
		cairo_fill( cr);
		cairo_stroke( cr);

	      // scale
		{
			cairo_set_source_rgb( cr, 0., 0., 0.);
			cairo_set_line_width( cr, 1.5);
			double dpuf =
				agh::alg::sensible_scale_reduction_factor(
					swu.display_scale, _p.interchannel_gap/2);
			int x = 140;
			cairo_move_to( cr, x, pbot - 5);
			cairo_rel_line_to( cr, 0, -dpuf * swu.display_scale);
			cairo_stroke( cr);

			cairo_set_font_size( cr, 9);
			//cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_move_to( cr, x + 5, pbot - 20);
			snprintf_buf( "%g a.u.", dpuf);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}

	}

      // phasic events
	if ( draw_phasic_spindle ) {
		_p._p.CwB[SExpDesignUI::TColour::sf_phasic_spindle].set_source_rgba( cr);
		cairo_set_line_width( cr, 1);
		for ( auto& A : annotations )
			if ( A.type == sigfile::SAnnotation::TType::phasic_event_spindle ) {
				auto x = (double)(A.span.z + A.span.a)/2 / samplerate()
					/ ((double)_p.total_pages() * _p.pagesize());
				cairo_move_to( cr, x * _p.da_wd - 2,  pbot - 8);
				cairo_rel_line_to( cr,  2, -5);
				cairo_rel_line_to( cr,  2,  5);
				cairo_rel_line_to( cr, -2,  5);
				cairo_rel_line_to( cr, -2, -5);
				cairo_close_path( cr);
				cairo_fill( cr);
				cairo_stroke( cr);
			}
	}
	if ( draw_phasic_Kcomplex ) {
		_p._p.CwB[SExpDesignUI::TColour::sf_phasic_Kcomplex].set_source_rgba( cr);
		cairo_set_line_width( cr, 8);
		for ( auto& A : annotations )
			if ( A.type == sigfile::SAnnotation::TType::phasic_event_K_complex ) {
				auto x = (float)(A.span.z + A.span.a)/2 / samplerate()
					/ ((float)_p.total_pages() * _p.pagesize());
				cairo_move_to( cr, x * _p.da_wd - 1, pbot - ptop - 8);
				cairo_rel_line_to( cr, 2, 0);
				cairo_stroke( cr);
			}
	}

      // EMG profile
	if ( draw_emg and
	     type == sigfile::SChannel::TType::emg ) {
		overlay = true;

		cairo_pattern_t *cp = cairo_pattern_create_linear( 0., pbot-EMGProfileHeight, 0., pbot);
		_p._p.CwB[SExpDesignUI::TColour::sf_emg].pattern_add_color_stop_rgba( cp, 0., 1.);
		_p._p.CwB[SExpDesignUI::TColour::sf_emg].pattern_add_color_stop_rgba( cp, .5, 0.6);
		_p._p.CwB[SExpDesignUI::TColour::sf_emg].pattern_add_color_stop_rgba( cp, 1., 1.);
		cairo_set_source( cr, cp);

		cairo_set_line_width( cr, .3);
		aghui::cairo_draw_envelope(
			cr,
			raw_profile, 0, raw_profile.size(),
			_p.da_wd, 0., pbot - EMGProfileHeight/2, signal_display_scale/2.); // half-signal scale, looks ok?
		cairo_stroke( cr);
		cairo_pattern_destroy( cp);
	}

	if ( overlay )
		_p._draw_hour_ticks( cr, zeroy, pbot);

      // crosshair (is drawn once in SScoringFacility::draw_montage), voltage at
	if ( _p.draw_crosshair and
	     _p.crosshair_at_time >= 0. and
	     _p.crosshair_at_time * samplerate() < n_samples() ) {
		cairo_set_font_size( cr, 10);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		_p._p.CwB[SExpDesignUI::TColour::sf_cursor].set_source_rgb( cr);
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
	if ( resample_signal ) {
		_p._p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgb( cr);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 8);
		cairo_move_to( cr, _p.da_wd-40, ptop + 11);
		snprintf_buf( "%4.2f spp", spp());
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}
}






void
aghui::SScoringFacility::
_draw_hour_ticks( cairo_t *cr,
		  const int htop, const int hbot,
		  const bool with_cursor)
{
	cairo_set_line_width( cr, 1);
	cairo_set_font_size( cr, 10);
	float	hours4 = (float)total_pages() * pagesize() / 3600 * 4;
	for ( size_t i = 1; i < hours4; ++i ) {
		guint tick_pos = (float)i / hours4 * da_wd;
		_p.CwB[SExpDesignUI::TColour::sf_ticks].set_source_rgba( cr);
		cairo_move_to( cr, tick_pos, hbot);
		cairo_rel_line_to( cr, 0, -((i%4 == 0) ? 20 : (i%2 == 0) ? 12 : 5));
		if ( i % 4 == 0 ) {
			snprintf_buf( "%2zuh", i/4);
			_p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgba( cr);
			cairo_move_to( cr, tick_pos+5, hbot - 2);
			cairo_show_text( cr, __buf__);
		}
	}
	cairo_stroke( cr);

	if ( with_cursor ) {
		_p.CwB[SExpDesignUI::TColour::sf_cursor].set_source_rgba( cr);
		cairo_rectangle( cr,
				 (double)cur_vpage() / total_vpages() * da_wd, htop,
				 max( .5, 1. / total_vpages() * da_wd), hbot - htop);
		cairo_fill( cr);
		cairo_stroke( cr);
	}
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
