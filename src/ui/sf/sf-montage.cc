// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-montage.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-23
 *
 *         Purpose:  scoring facility: montage drawing area
 *
 *         License:  GPL
 */

#include <cairo/cairo-svg.h>

#include "common/lang.hh"
#include "ui/misc.hh"
#include "sf.hh"

using namespace std;
using namespace aghui;


inline namespace {

unsigned short PageTicks[] = {
	8, 10, 10, 5, 5, 6, 12, 24, 30
};

}


void
aghui::SScoringFacility::SChannel::
draw_signal( const valarray<TFloat>& signal,
	     size_t width, int vdisp, cairo_t *cr) const
{
	ssize_t	start  = _p.cur_vpage_start()  * samplerate(), // signed please
		end    = _p.cur_vpage_end()    * samplerate(),
		run    = end - start,
		half_pad = run * _p.skirting_run_per1;
	aghui::cairo_draw_signal( cr, signal,
				  start - half_pad,
				  end + half_pad,
				  width, 0, vdisp, signal_display_scale,
				  resample_signal ? max((unsigned short)1, (unsigned short)spp()) : 1);
}




void
aghui::SScoringFacility::
estimate_montage_height()
{
	da_ht = channels.size() * interchannel_gap;
}





struct SChHolder {
	aghui::SScoringFacility::SChannel* ch;
	SChHolder( aghui::SScoringFacility::SChannel& ini) : ch (&ini) {}
	bool operator<( const SChHolder& rv) const
		{
			return ch->zeroy < rv.ch->zeroy;
		}
};



sigfile::SAnnotation*
aghui::SScoringFacility::
interactively_choose_annotation() const
{
	// do some on-the-fly construcion
	gtk_combo_box_set_model( eAnnotationSelectorWhich, NULL);
	gtk_list_store_clear( mAnnotationsAtCursor);
	GtkTreeIter iter;
	for ( auto &A : over_annotations ) {
		gtk_list_store_append( mAnnotationsAtCursor, &iter);
		gtk_list_store_set( mAnnotationsAtCursor, &iter,
				    0, A->label.c_str(),
				    -1);
	}
	gtk_combo_box_set_model( eAnnotationSelectorWhich, (GtkTreeModel*)mAnnotationsAtCursor);

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( wAnnotationSelector) ) {
		const char *selected_label = gtk_combo_box_get_active_id( eAnnotationSelectorWhich);
		if ( selected_label == nullptr )
			return nullptr;
		for ( auto &A : over_annotations )
			if ( A->label == selected_label )
				return A;
	}
	return nullptr;
}





int
aghui::SScoringFacility::
find_free_space()
{
	vector<SChHolder> thomas;
	for ( SChannel& ch : channels )
		if ( not ch.hidden )
			thomas.push_back( {ch});
	sort( thomas.begin(), thomas.end());

	int	mean_gap,
		widest_gap = 0,
		widest_after = 0;
	int sum = 0;
	for ( auto ch = channels.begin(); ch != prev(channels.end()); ++ch ) {
		int gap = next(ch)->zeroy - ch->zeroy;
		sum += gap;
		if ( gap > widest_gap ) {
			widest_after = ch->zeroy;
			widest_gap = gap;
		}
	}
	mean_gap = sum / thomas.size()-1;
	if ( widest_gap > mean_gap * 1.5 )
		return widest_after + widest_gap / 2;
	else {
		gtk_widget_set_size_request( (GtkWidget*)daSFMontage,
					     -1, thomas.back().ch->zeroy + 42*2);
		return thomas.back().ch->zeroy + mean_gap;
	}
}

void
aghui::SScoringFacility::
space_evenly()
{
	vector<SChHolder> thomas;
	for ( auto& H : channels )
		if ( not H.hidden )
			thomas.push_back( H);
	if ( thomas.size() < 2 )
		return;

	interchannel_gap = da_ht / thomas.size();

	sort( thomas.begin(), thomas.end());
	size_t i = 0;
	for ( auto& T : thomas )
		T.ch->zeroy = interchannel_gap/2 + interchannel_gap * i++;

	// gtk_widget_set_size_request( (GtkWidget*)daSFMontage,
	// 			     -1, thomas.back().ch->zeroy + mean_gap/2);
}


void
aghui::SScoringFacility::
expand_by_factor( double fac)
{
	for ( auto &H : channels ) {
		H.signal_display_scale *= fac;
		H.psd.display_scale *= fac;
		H.mc.display_scale *= fac;
		H.zeroy *= fac;
	}
	interchannel_gap *= fac;
	da_ht *= fac;

	gtk_widget_set_size_request( (GtkWidget*)daSFMontage,
				     -1, da_ht);
}















void
aghui::SScoringFacility::SChannel::
draw_for_montage( const char *fname, int width, int height) // to a file
{
	cairo_surface_t *cs = cairo_svg_surface_create( fname, width, height);
	cairo_t *cr = cairo_create( cs);

	draw_page( cr, width, height/2, false); // or maybe *with* selection?
	draw_overlays( cr, width, height/2);

	cairo_destroy( cr);
	cairo_surface_destroy( cs);
}

void
aghui::SScoringFacility::SChannel::
draw_for_montage( cairo_t* cr)
{
	if ( !hidden ) {
		draw_page( cr, _p.da_wd, zeroy, true);
		draw_overlays( cr, _p.da_wd, zeroy);
	}
}



void
aghui::SScoringFacility::SChannel::
draw_page( cairo_t *cr,
	   int wd, float y0,
	   bool draw_marquee) const
{
	int	ptop = y0 - _p.interchannel_gap/2,
		pbot = ptop + _p.interchannel_gap;

      // marquee, goes first, not to obscure waveforms
	if ( draw_marquee // possibly undesired (such as when drawing for unfazer (what unfazer?))
	     && agh::alg::overlap(
		     selection_start_time, selection_end_time,
		     _p.cur_xvpage_start(), _p.cur_xvpage_end()) ) {
		double	pre = _p.skirting_run_per1 * _p.vpagesize(),
			ma = (selection_start_time - _p.cur_xvpage_start()) / _p.xvpagesize() * wd,
			me = (selection_end_time   - _p.cur_xvpage_start()) / _p.xvpagesize() * wd;
		_p._p.CwB[SExpDesignUI::TColour::selection].set_source_rgba( cr);
		cairo_rectangle( cr,
				 ma, ptop, me - ma, _p.interchannel_gap);
		cairo_fill( cr);
		cairo_stroke( cr);

	      // start timestamp
		cairo_set_font_size( cr, 10);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		_p._p.CwB[SExpDesignUI::TColour::cursor].set_source_rgba( cr);

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
		cairo_stroke( cr);

		if ( selection_end - selection_start > 5 ) {  // don't mark end if selection is too short
		      // signal properties
			auto& Pp = _p.find_dialog.params;
			if ( draw_selection_envelope ) {
				valarray<TFloat>
					selection {(draw_filtered_signal
						    ? signal_filtered
						    : signal_original)[ slice (selection_start,
									       selection_end - selection_start,
									       1) ]};
				valarray<TFloat>
					env_u, env_l;
				if ( sigproc::envelope(
					     selection,
					     Pp.env_tightness, samplerate(),
					     1./samplerate(),
					     env_l, env_u) != 0 ) {
					cairo_set_source_rgba( cr, 1, 1, 1, .6);
					cairo_set_line_width( cr, 1);
					aghui::cairo_draw_signal(
						cr, env_u, 0, env_u.size(),
						me-ma, ma, y0, signal_display_scale);
					aghui::cairo_draw_signal(
						cr, env_l, 0, env_l.size(),
						me-ma, ma, y0, signal_display_scale, 1, aghui::TDrawSignalDirection::Backward, true);
					cairo_close_path( cr);
					cairo_fill( cr);
					cairo_stroke( cr);
				}
			}
			if ( draw_selection_course ) {
				valarray<TFloat>
					selection {(draw_filtered_signal
						    ? signal_filtered
						    : signal_original)[ slice (selection_start,
									       selection_end - selection_start,
									       1) ]};
				valarray<TFloat>
					course
					= exstrom::low_pass(
						selection, samplerate(),
						Pp.bwf_cutoff, Pp.bwf_order, true);

				cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
				cairo_set_line_width( cr, 3.);
				aghui::cairo_draw_signal(
					cr, course, 0, course.size(),
					me-ma, ma, y0, signal_display_scale);
				cairo_stroke( cr);
			}
			if ( draw_selection_dzcdf ) {
				valarray<TFloat>
					selection {(draw_filtered_signal
						    ? signal_filtered
						    : signal_original)[ slice (selection_start,
									       selection_end - selection_start,
									       1) ]};
				if ( samplerate() > 10 &&
				     Pp.dzcdf_step * 10 < selection_end_time - selection_start_time ) {
					valarray<TFloat>
						dzcdf = sigproc::dzcdf(
							selection, samplerate(),
							Pp.dzcdf_step,
							Pp.dzcdf_sigma,
							Pp.dzcdf_smooth);
					float	dzcdf_display_scale = (pbot-ptop)/2. / dzcdf.max();

					cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
					cairo_set_line_width( cr, 1.);
					aghui::cairo_draw_signal(
						cr, dzcdf, 0, dzcdf.size(),
						me-ma, ma, y0, dzcdf_display_scale);
					cairo_stroke( cr);
				}
			}

		      // labels
			_p._p.CwB[SExpDesignUI::TColour::cursor].set_source_rgba( cr);
			snprintf_buf( "%5.2fs",
				      selection_end_time - _p.cur_xvpage_start() - pre);
			cairo_text_extents( cr, __buf__, &extents);
			ido = me + extents.width+3;
			if ( ido > wd )
				cairo_move_to( cr, me - 3 - extents.width, ptop + 30);
			else
				cairo_move_to( cr, me + 3, ptop + 12);
			cairo_show_text( cr, __buf__);

			snprintf_buf( "< %4.2fs >", // "←%4.2fs→",
				      selection_end_time - selection_start_time);
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, ma+(me-ma)/2 - extents.width/2,
				       ptop + (extents.width < me-ma ? 12 : 30));
			cairo_show_text( cr, __buf__);

			// MC metrics
			if ( _p.mode != SScoringFacility::TMode::marking &&
			     type == sigfile::SChannel::TType::eeg &&
			     selection_end_time - selection_start_time > 1. ) {
				cairo_set_font_size( cr, 12);
				cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
				snprintf_buf( "%4.2f / %4.2f",
					      selection_SS, selection_SU);
				cairo_text_extents( cr, __buf__, &extents);
				cairo_move_to( cr, ma+(me-ma)/2 - extents.width/2,
					       pbot - (extents.width < me-ma ? 12 : 30));
				cairo_show_text( cr, __buf__);
			}
			cairo_stroke( cr);
		}
	}

      // zeroline
	if ( draw_zeroline ) {
		cairo_set_line_width( cr, fine_line());
		_p._p.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgba( cr);
		cairo_move_to( cr, 0, y0);
		cairo_rel_line_to( cr, wd, 0);
		cairo_stroke( cr);
	}

      // waveform: signal_filtered
	bool one_signal_drawn = false;
	if ( draw_filtered_signal ) {
		cairo_set_line_width( cr, fine_line());
		cairo_set_source_rgb( cr, 0., 0., 0.); // bg is uniformly light shades

		draw_signal_filtered( wd, y0, cr);
		cairo_stroke( cr);

		if ( _p.mode == aghui::SScoringFacility::TMode::scoring ) {
			_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgb( cr);
			cairo_move_to( cr, wd-88, y0 - 15);
			cairo_set_font_size( cr, 10);
			snprintf_buf( "filt");
			cairo_show_text( cr, __buf__);
			one_signal_drawn = true;
			cairo_stroke( cr);
		}
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
		cairo_stroke( cr);

		if ( _p.mode == aghui::SScoringFacility::TMode::scoring ) {
			_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgb( cr);
			cairo_move_to( cr, wd-88, y0 - 25);
			cairo_set_font_size( cr, 10);
			snprintf_buf( "orig");
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}
	}

      // waveform: signal_reconstituted
	if ( _p.mode == aghui::SScoringFacility::TMode::showing_remixed &&
	     signal_reconstituted.size() != 0 ) {
		if ( apply_reconstituted ) {
			cairo_set_line_width( cr, fine_line() * 1.3);
			cairo_set_source_rgba( cr, .7, 0., .6, 1); // red
		} else {
			cairo_set_line_width( cr, fine_line() * 1.3 * 2);
			cairo_set_source_rgba( cr, 1., .2, 0., .4);
		}

		draw_signal_reconstituted( wd, y0, cr);
		cairo_stroke( cr);

		if ( apply_reconstituted ) {
			cairo_move_to( cr, 120, y0 + 35);
			cairo_set_source_rgba( cr, 1., 0., 0., .4);
			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_font_size( cr, 28);
			cairo_show_text( cr, "APPLY");
			cairo_stroke( cr);
		}
	}

	size_t	half_pad = wd * _p.skirting_run_per1,
		ef = wd + 2*half_pad;

	int	half_pad_samples = _p.skirting_run_per1 * _p.vpagesize() * samplerate(),
		cvpa = _p.cur_vpage_start() * samplerate() - half_pad_samples,
		cvpe = _p.cur_vpage_end()   * samplerate() + half_pad_samples,
		evpz = cvpe - cvpa;
      // artifacts (changed bg)
	{
		auto& Aa = crecording.F().artifacts(name);
		if ( not Aa.obj.empty() ) {
			_p._p.CwB[SExpDesignUI::TColour::artifact].set_source_rgba( cr,  // do some gradients perhaps?
										    .4);
			for ( auto &A : Aa() ) {
				if ( agh::alg::overlap(
					     (int)A.a, (int)A.z,
					     cvpa, cvpe) ) {
					int	aa = (int)A.a - cvpa,
						ae = (int)A.z - cvpa;
					if ( aa < 0 )    aa = 0;
					if ( ae > evpz ) ae = evpz;
					cairo_rectangle( cr,
							 (float)(aa % evpz) / evpz * wd, ptop + _p.interchannel_gap * 1./3,
							 (float)(ae - aa) / evpz * wd,          _p.interchannel_gap * 1./3);
					cairo_fill( cr);
					cairo_stroke( cr);
				} else if ( (int)A.a > cvpe )  // no more artifacts up to and on current page
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
		auto& Aa = crecording.F().annotations(name);
		if ( not Aa.empty() ) {
			int on_this_page = 0;
			for ( auto &A : Aa ) {
				if ( agh::alg::overlap( (int)A.span.a, (int)A.span.z, cvpa, cvpe) ) {
					int disp = ptop + ++on_this_page * 5;
					cairo_pattern_t *cp = cairo_pattern_create_linear( 0., disp, 0., pbot);
					_p._p.CwB[SExpDesignUI::TColour::annotations].pattern_add_color_stop_rgba( cp, 0., 1.);
					_p._p.CwB[SExpDesignUI::TColour::annotations].pattern_add_color_stop_rgba( cp, .1, 0.3);
					_p._p.CwB[SExpDesignUI::TColour::annotations].pattern_add_color_stop_rgba( cp, 1., 0.);
					cairo_set_source( cr, cp);

					int	aa = (int)A.span.a - cvpa,
						ae = (int)A.span.z - cvpa;
					if ( aa < 0 )    aa = 0;
					if ( ae > evpz ) ae = evpz;
					cairo_rectangle( cr,
							 (float)(aa % evpz) / evpz * wd, disp,
							 (float)(ae - aa) / evpz * wd, pbot-ptop);
					cairo_fill( cr);
					cairo_stroke( cr);
					cairo_pattern_destroy( cp);

					cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
					cairo_set_font_size( cr, 11);
					cairo_set_source_rgb( cr, 0., 0., 0.);
					cairo_move_to( cr, (float)(aa % evpz) / evpz * wd, disp + 12);
					cairo_show_text( cr, A.label.c_str());
				} else if ( (int)A.span.a > cvpe )  // no more artifacts up to and on current page
					break;
			}
		}
	}

      // labels of all kinds
      // channel id
	{
		int x = 15, y = y0 - 16;

		snprintf_buf( "[%s] %s", sigfile::SChannel::kemp_signal_types[type], name);
		cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 14);

		cairo_set_source_rgba( cr, 1., 1., 1., 0.6);
		cairo_move_to( cr, x+1, y+1);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);

		cairo_set_source_rgba( cr, 0., 0., 0., 0.7);
		cairo_move_to( cr, x, y);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}

	_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgb( cr);

       // uV scale
	{
		cairo_set_font_size( cr, 10);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

		cairo_set_source_rgb( cr, 0., 0., 0.);
		cairo_set_line_width( cr, 1.5);
		double dpuf =
			agh::alg::sensible_scale_reduction_factor(
				1 * signal_display_scale, _p.interchannel_gap * .75);
		int x = 10;
		cairo_move_to( cr, x, ptop + 5);
		cairo_rel_line_to( cr, 0, dpuf * signal_display_scale);
		cairo_stroke( cr);

		cairo_set_font_size( cr, 9);
		cairo_move_to( cr, x + 5, ptop + 20);
		snprintf_buf( "%g mV", dpuf);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}

       // applied filters legend
	{
		cairo_set_font_size( cr, 9);
		if ( have_low_pass() ) {
			snprintf_buf( "LP: %6.2f/%u", filters.low_pass_cutoff, filters.low_pass_order);
			cairo_move_to( cr, wd-100, y0 + 15);
			cairo_show_text( cr, __buf__);
		}
		if ( have_high_pass() ) {
			snprintf_buf( "HP: %6.2f/%u", filters.high_pass_cutoff, filters.high_pass_order);
			cairo_move_to( cr, wd-100, y0 + 24);
			cairo_show_text( cr, __buf__);
		}
		if ( have_notch_filter() ) {
			static const char *nfs[] = { "", "50 Hz", "60 Hz" };
			snprintf_buf( "-v-: %s", nfs[(int)filters.notch_filter]);
			cairo_move_to( cr, wd-100, y0 + 33);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}
}


void
aghui::SScoringFacility::SChannel::
draw_overlays( cairo_t* cr,
	       int wd, float zeroy) const
{
	float	pbot = zeroy + _p.interchannel_gap / 2.2,
		ptop = zeroy - _p.interchannel_gap / 2.2;
	bool	overlay = false;

       // PSD profile
	if ( _p.mode == TMode::scoring and
	     draw_psd and type == sigfile::SChannel::TType::eeg ) {
		overlay = true;

		cairo_set_line_width( cr, 1.);
		cairo_set_font_size( cr, 10);
		guint i;

		if ( draw_bands ) {
			for ( size_t b = sigfile::TBand::delta; b <= psd.uppermost_band; ++b ) {
				auto& P = psd.course_in_bands[b];
				_p._p.CwB[SExpDesignUI::band2colour((sigfile::TBand)b)].set_source_rgba( cr, .5);
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
			_p._p.CwB[SExpDesignUI::TColour::profile_psd_sf].set_source_rgba( cr, .5);
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

		if ( draw_spectrum and _p.pagesize_is_right() ) {
			guint	gx = 120,
				gy = ptop + 25,
				gh = min( 60.f, pbot - ptop - 5),
				gw  = 80;
			size_t	m;

			cairo_set_source_rgba( cr, 1., 1., 1., .8);
			cairo_rectangle( cr, gx-15, gy-5, gw+15+5, gh+5+5);
			cairo_fill( cr);

			// grid lines
			_p._p.CwB[SExpDesignUI::TColour::spectrum_grid].set_source_rgba( cr, .1);
			cairo_set_line_width( cr, .3);
			for ( size_t i = 1; i < last_spectrum_bin; ++i ) {
				cairo_move_to( cr, gx + (float)i/last_spectrum_bin * gw, gy);
				cairo_rel_line_to( cr, 0, gh);
			}
			cairo_stroke( cr);

			// spectrum
			_p._p.CwB[SExpDesignUI::TColour::spectrum].set_source_rgba( cr, .8);
			cairo_set_line_width( cr, 2);
			float factor = psd.display_scale / crecording.binsize;
			cairo_move_to( cr,
				       gx, gy + gh - (2 + spectrum[0] * factor));
			for ( m = 1; m < last_spectrum_bin; ++m ) {
				cairo_line_to( cr,
					       gx + (float)m / last_spectrum_bin * gw,
					       gy + gh - spectrum[m] * factor);
			}
			cairo_stroke( cr);

			// axes
			_p._p.CwB[SExpDesignUI::TColour::spectrum_axes].set_source_rgba( cr, .5);
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
			_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgba( cr);
			cairo_set_font_size( cr, 8);

			snprintf_buf( "%g Hz",
				      last_spectrum_bin * crecording.binsize);
//				      draw_spectrum_absolute ? 'A' : 'R');
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr,
				       gx + gw - extents.width - 5,
				       gy + 4);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}
	}

	if ( _p.mode == TMode::scoring and
	     draw_mc and type == sigfile::SChannel::TType::eeg ) {
		overlay = true;

		cairo_set_line_width( cr, 1.);
		cairo_set_font_size( cr, 10);
		guint i;

		_p._p.CwB[SExpDesignUI::TColour::profile_mc_sf].set_source_rgba( cr, .5);
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
		snprintf_buf( "%g–%g Hz",
			      crecording.freq_from + (mc.bin  ) * crecording.bandwidth,
			      crecording.freq_from + (mc.bin+1) * crecording.bandwidth);
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

      // EMG profile
	if ( _p.mode == TMode::scoring and draw_emg and
	     type == sigfile::SChannel::TType::emg ) {
		overlay = true;

		_p._p.CwB[SExpDesignUI::TColour::emg].set_source_rgba( cr, .7);
		cairo_set_line_width( cr, .3);
		double dps = (double)emg_profile.size() / _p.da_wd;
		cairo_move_to( cr, 0., pbot - EMGProfileHeight/2);
		size_t i = 0;
		for ( ; i < emg_profile.size(); ++i )
			cairo_line_to( cr, i / dps,
				       pbot - EMGProfileHeight/2 - emg_profile[i] * signal_display_scale/2);
		for ( --i; i > 0; --i )
			cairo_line_to( cr, i / dps,
				       pbot - EMGProfileHeight/2 + emg_profile[i] * signal_display_scale/2);
		cairo_fill( cr);
		cairo_stroke( cr);
	}

	if ( overlay )
		_p._draw_hour_ticks( cr, zeroy, pbot);

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
	if ( _p.mode == TMode::scoring and resample_signal ) {
		_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgb( cr);
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
_draw_hour_ticks( cairo_t *cr, int htop, int hbot, bool with_cursor)
{
	cairo_set_line_width( cr, 1);
	cairo_set_font_size( cr, 10);
	float	hours4 = (float)total_pages() * pagesize() / 3600 * 4;
	for ( size_t i = 1; i < hours4; ++i ) {
		guint tick_pos = (float)i / hours4 * da_wd;
		_p.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgba( cr);
		cairo_move_to( cr, tick_pos, hbot);
		cairo_rel_line_to( cr, 0, -((i%4 == 0) ? 20 : (i%2 == 0) ? 12 : 5));
		if ( i % 4 == 0 ) {
			snprintf_buf( "%2zuh", i/4);
			_p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgba( cr);
			cairo_move_to( cr, tick_pos+5, hbot - 2);
			cairo_show_text( cr, __buf__);
		}
	}
	cairo_stroke( cr);

	if ( with_cursor ) {
		_p.CwB[SExpDesignUI::TColour::cursor].set_source_rgba( cr, .3);
		cairo_rectangle( cr,
				 (double)cur_vpage() / total_vpages() * da_wd, htop,
				 max( .5, 1. / total_vpages() * da_wd), hbot - htop);
		cairo_fill( cr);
		cairo_stroke( cr);
	}
}


template <class T>
void
aghui::SScoringFacility::
_draw_matrix_to_montage( cairo_t *cr, const itpp::Mat<T>& mat)
{
	int gap = da_ht/mat.rows();
	int our_y = gap/2;

      // labels, per mode and mark
	cairo_set_line_width( cr, 16);
	cairo_set_source_rgba( cr, .7, .2, .1, .2);
	for ( int r = 0; r < mat.rows(); ++r ) {
		if ( ica_map[r].m == -1 )
			switch ( remix_mode ) {
			case TICARemixMode::map:
				cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
				cairo_set_font_size( cr, 20);
				cairo_move_to( cr, 30, our_y-10);
				cairo_set_source_rgba( cr, .2, .6, .2, .45);
				cairo_show_text( cr, "(not mapped)");
			    break;
			default:
			    break;
			}
		else
			switch ( remix_mode ) {
			case TICARemixMode::map:
				cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
				cairo_set_font_size( cr, 28);
				cairo_move_to( cr, 30, our_y-10);
				cairo_set_source_rgba( cr, .3, .1, .2, .65);
				cairo_show_text( cr, channel_by_idx(ica_map[r].m).name);
			    break;
			default:
				cairo_move_to( cr, da_wd * .06, our_y - gap/2.5);
				cairo_line_to( cr, da_wd * .94, our_y + gap/2.5);
				cairo_move_to( cr, da_wd * .06, our_y + gap/2.5);
				cairo_line_to( cr, da_wd * .94, our_y - gap/2.5);
			    break;
			}
		cairo_stroke( cr);
		our_y += gap;
	}

      // waveform
	auto sr = channels.front().samplerate();  // ica wouldn't start if samplerates were different between any two channels
	auto our_display_scale = channels.front().signal_display_scale;

	cairo_set_line_width( cr, .6);
	our_y = gap/2;
	for ( int r = 0; r < mat.rows(); ++r ) {
		if ( ica_map[r].m != -1 )
			cairo_set_source_rgba( cr, 0, 0, 0, .6);
		else
			cairo_set_source_rgba( cr, 0., 0., .3, 1.);
		size_t  start = cur_vpage_start() * sr,
			end   = cur_vpage_end()   * sr,
			run   = end - start,
			half_pad = run * skirting_run_per1;

		aghui::cairo_draw_signal( cr, mat, r,
					  start - half_pad,
					  end + half_pad,
					  da_wd, 0, our_y, our_display_scale);
		cairo_stroke( cr);
		our_y += gap;
	}
}

void
aghui::SScoringFacility::
draw_montage( const char *fname) // to a file
{
	cairo_surface_t *cs = cairo_svg_surface_create( fname, da_wd, da_ht);
	cairo_t *cr = cairo_create( cs);
	draw_montage( cr);
	cairo_destroy( cr);
	cairo_surface_destroy( cs);
}

void
aghui::SScoringFacility::
draw_montage( cairo_t* cr)
{
	double	true_frac = 1. - 1. / (1. + 2*skirting_run_per1);
	size_t	half_pad = da_wd * true_frac/2,
		ef = da_wd * (1. - true_frac);  // w + 10% = d

      // background, is now common to all channels
	using namespace sigfile;
	if ( mode == TMode::scoring ) {
		double	ppart = (double)pagesize()/vpagesize();
		int	cp = cur_page();
		for ( int pp = cp-1; ; ++pp ) {
			double ppoff = ((double)pp * pagesize() - cur_vpage_start()) / vpagesize();
			if ( ppoff > 1.5 )
				break;

			SPage::TScore this_page_score = (pp < 0) ? SPage::TScore::none : SPage::char2score( hypnogram[pp]);
			_p.CwB[SExpDesignUI::score2colour(this_page_score)].set_source_rgba( cr, .2);
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

	switch ( mode ) {
	case TMode::showing_ics:
		if ( ica_components.size() == 0 ) {
			::cairo_put_banner( cr, da_wd, da_ht,
					    "Now set up ICA parameters, then press [Compute ICs]");
		} else
			_draw_matrix_to_montage( cr, ica_components);
			// draw ignoring channels' zeroy
	    break;
	case TMode::separating:
		::cairo_put_banner( cr, da_wd, da_ht,
				    "Separating...");
	    break;
	case TMode::showing_remixed:
	case TMode::scoring:
	default:
	      // draw individual signal pages (let SChannel::draw_page_static draw the appropriate signal)
		for ( auto &H : channels )
			H.draw_for_montage( cr);
	    break;
	}

      // ticks
	{
		cairo_set_font_size( cr, 9);
		cairo_set_line_width( cr, .2);
		for ( size_t i = 0; i <= PageTicks[pagesize_item]; ++i ) {
			_p.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgba( cr);
			double tick_pos = (double)i * vpagesize() / PageTicks[pagesize_item];
			cairo_move_to( cr, half_pad + i * ef / PageTicks[pagesize_item], 0);
			cairo_rel_line_to( cr, 0, da_ht);

			_p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgba( cr);
			cairo_move_to( cr, half_pad + i * ef / PageTicks[pagesize_item] + 5, da_ht-2);
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

