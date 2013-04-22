/*
 *       File name:  ui/sf/montage.cc
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
#include "d/patterns.hh"

using namespace std;

namespace {

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
	gtk_combo_box_set_model( eSFAnnotationSelectorWhich, NULL);
	gtk_list_store_clear( mSFAnnotationsAtCursor);
	GtkTreeIter iter;
	for ( auto &A : over_annotations ) {
		gtk_list_store_append( mSFAnnotationsAtCursor, &iter);
		gtk_list_store_set( mSFAnnotationsAtCursor, &iter,
				    0, A->label.c_str(),
				    -1);
	}
	gtk_combo_box_set_model( eSFAnnotationSelectorWhich, (GtkTreeModel*)mSFAnnotationsAtCursor);

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( wSFAnnotationSelector) ) {
		const char *selected_label = gtk_combo_box_get_active_id( eSFAnnotationSelectorWhich);
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
expand_by_factor( const double fac)
{
	for ( auto &H : channels ) {
		H.signal_display_scale *= fac;
		H.psd.display_scale *= fac;
		H.mc.display_scale *= fac;
		H.emg_display_scale *= fac;
		H.zeroy *= fac;
	}
	interchannel_gap *= fac;
	da_ht *= fac;

	gtk_widget_set_size_request(
		(GtkWidget*)daSFMontage,
		-1, da_ht);
}








void
aghui::SScoringFacility::SChannel::
draw_for_montage( const string& fname,
		  const int width, const int height) // to a file
{
	cairo_surface_t *cs = cairo_svg_surface_create( fname.c_str(), width, height);
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
	   const int wd, const float y0,
	   bool draw_marquee) const
{
	int	ptop = y0 - _p.interchannel_gap/2,
		pbot = ptop + _p.interchannel_gap;

      // zeroline
	if ( draw_zeroline ) {
		cairo_set_line_width( cr, fine_line());
		_p._p.CwB[SExpDesignUI::TColour::sf_ticks].set_source_rgba( cr);
		cairo_move_to( cr, 0, y0);
		cairo_rel_line_to( cr, wd, 0);
		cairo_stroke( cr);
	}

      // marquee, goes first, not to obscure waveforms
	if ( _p.mode != aghui::SScoringFacility::TMode::shuffling_channels
	     and draw_marquee // possibly undesired (such as when drawing for unfazer (what unfazer?))
	     && agh::alg::overlap(
		     selection_start_time, selection_end_time,
		     _p.cur_xvpage_start(), _p.cur_xvpage_end()) ) {

		double	pre = _p.skirting_run_per1 * _p.vpagesize(),
			ma = (selection_start_time - _p.cur_xvpage_start()) / _p.xvpagesize() * wd,
			me = (selection_end_time   - _p.cur_xvpage_start()) / _p.xvpagesize() * wd;
		{
			cairo_pattern_t *cp = cairo_pattern_create_linear( 0., ptop, 0., pbot);
			_p._p.CwB[SExpDesignUI::TColour::sf_selection].pattern_add_color_stop_rgba( cp, 0., .6);
			_p._p.CwB[SExpDesignUI::TColour::sf_selection].pattern_add_color_stop_rgba( cp, .2, .3);
			_p._p.CwB[SExpDesignUI::TColour::sf_selection].pattern_add_color_stop_rgba( cp, .8, .4);
			_p._p.CwB[SExpDesignUI::TColour::sf_selection].pattern_add_color_stop_rgba( cp, 1., .6);
			cairo_set_source( cr, cp);
			cairo_rectangle( cr,
					 ma, ptop, me - ma, _p.interchannel_gap);
			cairo_fill( cr);
			cairo_stroke( cr);
			cairo_pattern_destroy( cp);
		}

	      // start timestamp
		cairo_set_font_size( cr, 10);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		_p._p.CwB[SExpDesignUI::TColour::sf_cursor].set_source_rgba( cr);

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

		if ( (draw_selection_envelope || draw_selection_course || draw_selection_dzcdf) &&
		     selection_end_time - selection_start_time > .5 ) {
		      // signal properties
			const valarray<TFloat>
				selection {(draw_filtered_signal
					    ? signal_filtered
					    : signal_original)[ slice (selection_start,
								       selection_end - selection_start,
								       1) ]};
			if ( draw_selection_envelope ) {
				valarray<TFloat>
					env_u, env_l;
				if ( sigproc::envelope(
					     {selection, samplerate()},
					     pattern_params.env_scope,
					     1./samplerate(),
					     &env_l, &env_u) > 2 ) {

					cairo_set_source_rgba( cr, 1, 1, 1, .6);
					cairo_set_line_width( cr, 1);
					aghui::cairo_draw_signal(
						cr, env_u, 0, env_u.size(),
						me-ma, ma, y0, signal_display_scale);
					aghui::cairo_draw_signal(
						cr, env_l, 0, env_l.size(),
						me-ma, ma, y0, signal_display_scale,
						1, aghui::TDrawSignalDirection::backward, true);
					cairo_close_path( cr);
					cairo_fill( cr);
					cairo_stroke( cr);
				}
			}
			if ( draw_selection_course ) {
				valarray<TFloat>
					course
						= exstrom::band_pass(
							selection, samplerate(),
							pattern_params.bwf_ffrom,
							pattern_params.bwf_fupto,
							pattern_params.bwf_order, true);

				cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
				cairo_set_line_width( cr, 3.);
				aghui::cairo_draw_signal(
					cr, course, 0, course.size(),
					me-ma, ma, y0, signal_display_scale);
				cairo_stroke( cr);
			}
			if ( draw_selection_dzcdf ) {
				if ( samplerate() > 10 &&
				     pattern_params.dzcdf_step * 10 < selection_end_time - selection_start_time ) {
					valarray<TFloat>
						dzcdf = sigproc::dzcdf(
							sigproc::SSignalRef<TFloat> {selection, samplerate()},
							pattern_params.dzcdf_step,
							pattern_params.dzcdf_sigma,
							pattern_params.dzcdf_smooth);
					float	dzcdf_display_scale = (pbot-ptop)/2. / dzcdf.max();

					cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
					cairo_set_line_width( cr, 1.);
					aghui::cairo_draw_signal(
						cr, dzcdf, 0, dzcdf.size(),
						me-ma, ma, y0, dzcdf_display_scale);
					cairo_stroke( cr);
				}
			}
		}

	      // labels
		if ( selection_end_time - selection_start_time > .25 ) {  // don't mark end if selection is too short
			_p._p.CwB[SExpDesignUI::TColour::sf_cursor].set_source_rgba( cr);
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
			     selection_end_time - selection_start_time > 2. ) {

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

      // waveform: signal_filtered
	bool	need_filtered = (have_low_pass() or have_high_pass() or have_notch_filter()) or (not artifacts().empty()),
		one_signal_drawn = false;
	if ( draw_filtered_signal and need_filtered ) {
		cairo_set_line_width( cr, fine_line());
		cairo_set_source_rgb( cr, 0., 0., 0.); // bg is uniformly light shades

		draw_signal_filtered( wd, y0, cr);
		cairo_stroke( cr);

		if ( _p.mode == aghui::SScoringFacility::TMode::scoring ) {
			_p._p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgb( cr);
			cairo_move_to( cr, wd-88, y0 - 15);
			cairo_set_font_size( cr, 10);
			snprintf_buf( "filt");
			cairo_show_text( cr, __buf__);
			one_signal_drawn = true;
			cairo_stroke( cr);
		}
	}

      // waveform: signal_original
	if ( draw_original_signal or not one_signal_drawn ) {
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
			_p._p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgb( cr);
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

	double	half_pad = _p.pagesize() * _p.skirting_run_per1;

	double	cvpa = _p.cur_xvpage_start(),
		cvpe = _p.cur_xvpage_end(),
		evpz = cvpe - cvpa;
      // artifacts (changed bg)
	{
		auto& Aa = crecording.F().artifacts(name);
		if ( not Aa.obj.empty() ) {
			_p._p.CwB[SExpDesignUI::TColour::sf_artifact].set_source_rgba(
				cr, .4); // do some gradients perhaps?
			for ( auto &A : Aa() ) {
				if ( agh::alg::overlap(
					     A.a, A.z,
					     cvpa, cvpe) ) {
					double	aa = A.a - cvpa,
						ae = A.z - cvpa;
					if ( aa < 0.   ) aa = 0.;
					if ( ae > evpz ) ae = evpz;
					cairo_rectangle(
						cr,
						fmod(aa, evpz) / evpz * wd, ptop + _p.interchannel_gap * 1./3,
						(ae - aa)      / evpz * wd,        _p.interchannel_gap * 1./3);
					cairo_fill( cr);
					cairo_stroke( cr);
				} else if ( A.a > cvpe )  // no more artifacts up to and on current page
					break;
			}
			_p._p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgb( cr);
			cairo_move_to( cr, wd-70, y0 + 15);
			cairo_set_font_size( cr, 8);
			snprintf_buf( "%4.2f %% dirty", percent_dirty);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}
	}

      // annotations
	if ( _p.mode == aghui::SScoringFacility::TMode::scoring
	     and not annotations.empty() ) {
		double last_z = 0;
		int overlap_count = 0;
		for ( auto &A : annotations ) {
			if ( agh::alg::overlap( A.span.a, A.span.z, cvpa, cvpe) ) {
				double	aa = A.span.a - cvpa,
					ae = A.span.z - cvpa;
				agh::alg::ensure_within( aa, -half_pad, -half_pad + evpz);
				agh::alg::ensure_within( ae, -half_pad, -half_pad + evpz);

				auto	wa = fmod(aa, evpz) / evpz * wd,
					ww = (ae - aa) / evpz * wd;

				if ( A.type == sigfile::SAnnotation::TType::plain ) {
					int disp = ptop +
						((last_z > A.span.a)
						 ? ++overlap_count * 5
						 : (overlap_count = 0));
					last_z = A.span.z;
					cairo_pattern_t *cp = cairo_pattern_create_linear( 0., disp, 0., pbot);
					_p._p.CwB[SExpDesignUI::TColour::sf_annotations].pattern_add_color_stop_rgba( cp, 0., 1.);
					_p._p.CwB[SExpDesignUI::TColour::sf_annotations].pattern_add_color_stop_rgba( cp, .1, 0.3);
					_p._p.CwB[SExpDesignUI::TColour::sf_annotations].pattern_add_color_stop_rgba( cp, 1., 0.);
					cairo_set_source( cr, cp);

					cairo_rectangle( cr, wa, disp, ww, pbot-ptop);
					cairo_fill( cr);
					cairo_stroke( cr);
					cairo_pattern_destroy( cp);

					cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
					cairo_set_font_size( cr, 11);
					cairo_set_source_rgb( cr, 0., 0., 0.);
					cairo_move_to( cr, fmod(aa, evpz) / evpz * wd, disp + 12);
					cairo_show_text( cr, A.label.c_str());

				} else if ( A.type == sigfile::SAnnotation::TType::phasic_event_spindle
					    and draw_phasic_spindle ) {
					cairo_pattern_t *cp = cairo_pattern_create_linear( wa, 0., wa + ww, 0.);
					_p._p.CwB[SExpDesignUI::TColour::sf_phasic_spindle].pattern_add_color_stop_rgba( cp, 0., 0.);
					_p._p.CwB[SExpDesignUI::TColour::sf_phasic_spindle].pattern_add_color_stop_rgba( cp, .5, 0.3);
					_p._p.CwB[SExpDesignUI::TColour::sf_phasic_spindle].pattern_add_color_stop_rgba( cp, 1., 0.);
					cairo_set_source( cr, cp);

					cairo_rectangle( cr, wa, ptop, ww, pbot-ptop);
					cairo_fill( cr);
					cairo_stroke( cr);
					cairo_pattern_destroy( cp);

				} else if ( A.type == sigfile::SAnnotation::TType::phasic_event_K_complex
					    and draw_phasic_Kcomplex ) {
					cairo_pattern_t *cp = cairo_pattern_create_linear( 0., ptop, 0., pbot);
					_p._p.CwB[SExpDesignUI::TColour::sf_phasic_Kcomplex].pattern_add_color_stop_rgba( cp, 0., 0.);
					_p._p.CwB[SExpDesignUI::TColour::sf_phasic_Kcomplex].pattern_add_color_stop_rgba( cp, .5, 0.4);
					_p._p.CwB[SExpDesignUI::TColour::sf_phasic_Kcomplex].pattern_add_color_stop_rgba( cp, 1., 0.);
					cairo_set_source( cr, cp);

					cairo_rectangle( cr, wa, ptop, ww, pbot-ptop);
					cairo_fill( cr);
					cairo_stroke( cr);
					cairo_pattern_destroy( cp);

				} else if ( A.type == sigfile::SAnnotation::TType::eyeblink
					    and draw_phasic_eyeblink ) {
					cairo_pattern_t *cp = cairo_pattern_create_linear( 0., ptop, 0., pbot);
					_p._p.CwB[SExpDesignUI::TColour::sf_phasic_eyeblink].pattern_add_color_stop_rgba( cp, 0., 0.);
					_p._p.CwB[SExpDesignUI::TColour::sf_phasic_eyeblink].pattern_add_color_stop_rgba( cp, .5, 0.4);
					_p._p.CwB[SExpDesignUI::TColour::sf_phasic_eyeblink].pattern_add_color_stop_rgba( cp, 1., 0.);
					cairo_set_source( cr, cp);

					cairo_rectangle( cr, wa, ptop, ww, pbot-ptop);
					cairo_fill( cr);
					cairo_stroke( cr);
					cairo_pattern_destroy( cp);
				}

			} else if ( (int)A.span.a > cvpe )  // no more artifacts up to and on current page
				break;
		}
	}

      // labels of all kinds
      // channel id
	{
		int x = 15, y = y0 - 16;

		snprintf_buf( "[%s] %s", sigfile::SChannel::kemp_signal_types[type], name.c_str());
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

	_p._p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgb( cr);

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
aghui::SScoringFacility::
draw_montage( const string& fname) // to a file
{
	cairo_surface_t *cs = cairo_svg_surface_create( fname.c_str(), da_wd, da_ht);
	cairo_t *cr = cairo_create( cs);
	draw_montage( cr);
	cairo_destroy( cr);
	cairo_surface_destroy( cs);
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
				cairo_show_text( cr, channel_by_idx(ica_map[r].m).name.c_str());
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
draw_montage( cairo_t* cr)
{
	double	true_frac = 1. - 1. / (1. + 2*skirting_run_per1),
		half_pad = pagesize() * skirting_run_per1;
	size_t	ef = da_wd * (1. - true_frac);  // w + 10% = d

	using namespace sigfile;
	switch ( mode ) {
	case TMode::showing_ics:
		if ( ica_components.size() == 0 ) {
			aghui::cairo_put_banner(
				cr, da_wd, da_ht,
				"Now set up ICA parameters, then press [Compute ICs]");
		} else
			_draw_matrix_to_montage( cr, ica_components);
			// draw ignoring channels' zeroy
	    break;
	case TMode::separating:
		aghui::cairo_put_banner(
			cr, da_wd, da_ht,
			"Separating...");
	    break;
	case TMode::scoring:
      // background, is now common to all channels
	{
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
      // recording-wide annotations
	if ( not common_annotations.empty() ) {
		for ( auto &SA : common_annotations ) {
			auto &S = *SA.first;
			auto &A = *SA.second;
			double	cvpa = cur_xvpage_start(),
				cvpe = cur_xvpage_end(),
				evpz = cvpe - cvpa;
			double last_z = 0;
			int overlap_count = 0;
			if ( agh::alg::overlap( A.span.a, A.span.z, cvpa, cvpe) ) {
				double	aa = A.span.a - cvpa,
					ae = A.span.z - cvpa;
				agh::alg::ensure_within( aa, -half_pad, -half_pad + evpz);
				agh::alg::ensure_within( ae, -half_pad, -half_pad + evpz);

				auto	wa = fmod(aa, evpz) / evpz * da_wd,
					ww = (ae - aa) / evpz * da_wd;

				if ( A.type == sigfile::SAnnotation::TType::plain ) {
					int disp = 0 +
						((last_z > A.span.a)
						 ? ++overlap_count * 5
						 : (overlap_count = 0));
					last_z = A.span.z;

					_p.CwB[SExpDesignUI::TColour::sf_embedded_annotations].set_source_rgba( cr);

					cairo_set_line_width( cr, 2.5);
					cairo_rectangle( cr, wa, 0, ww, da_ht);
					cairo_fill( cr);
					cairo_stroke( cr);

					cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
					cairo_set_font_size( cr, 11);
					cairo_set_source_rgb( cr, 0., 0., 0.);
					cairo_move_to( cr, fmod(aa, evpz) / evpz * da_wd, da_ht - 12 - disp);
					cairo_show_text( cr, A.label.c_str());
				}
			}
		}
	}
	case TMode::showing_remixed:
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
			_p.CwB[SExpDesignUI::TColour::sf_ticks].set_source_rgba( cr);
			double tick_pos = (double)i * vpagesize() / PageTicks[pagesize_item];
			cairo_move_to( cr, half_pad + i * ef / PageTicks[pagesize_item], 0);
			cairo_rel_line_to( cr, 0, da_ht);

			_p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgba( cr);
			cairo_move_to( cr, half_pad + i * ef / PageTicks[pagesize_item] + 5, da_ht-2);
			snprintf_buf_ts_s( tick_pos);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}

	// save/restore cairo contexts please

      // crosshair line
	if ( draw_crosshair ) {
		_p.CwB[SExpDesignUI::TColour::sf_cursor].set_source_rgba( cr);
		cairo_set_line_width( cr, .2);
		cairo_move_to( cr, crosshair_at, 0);
		cairo_rel_line_to( cr, 0, da_ht);
		cairo_stroke( cr);
	}
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

