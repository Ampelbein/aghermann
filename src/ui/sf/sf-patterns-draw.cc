// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-patterns-draw.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-16
 *
 *         Purpose:  scoring facility patterns (drawing functions)
 *
 *         License:  GPL
 */

#include "ui/misc.hh"
#include "sf.hh"

using namespace std;


void
aghui::SScoringFacility::SFindDialog::
set_thing_da_width( int width)
{
	g_object_set( (GObject*)_p.daSFFDThing,
		      "width-request", da_thing_wd = max( width+5, 600),
		      "height-request", da_thing_ht,
		      NULL);
}

void
aghui::SScoringFacility::SFindDialog::
set_field_da_width( int width)
{
	g_object_set( (GObject*)_p.daSFFDField,
		      "width-request", da_field_wd = max( width+5, 600),
		      "height-request", da_field_ht,
		      NULL);
}



void
aghui::SScoringFacility::SFindDialog::
draw_thing( cairo_t *cr)
{
	if ( not Q or Q->thing.size() == 0 ) {
		aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "(no selection)");
		return;
	}

      // ticks
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 9);
	double	seconds = (double)Q->thing.size() / Q->samplerate;
	for ( size_t i8 = 0; (float)i8 / 8 < seconds; ++i8 ) {
		_p._p.CwB[SExpDesignUI::TColour::sf_ticks].set_source_rgba( cr);
		cairo_set_line_width( cr, (i8%8 == 0) ? 1. : (i8%4 == 0) ? .6 : .3);
		guint x = (float)i8/8 / seconds * da_thing_wd;
		cairo_move_to( cr, x, 0);
		cairo_rel_line_to( cr, 0, da_thing_ht);
		cairo_stroke( cr);

		if ( i8 % 8 == 0 ) {
			_p._p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgba( cr);
			cairo_move_to( cr, x + 5, da_thing_ht-2);
			snprintf_buf( "%g", (float)i8/8);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}
	}

	size_t	run = Q->pattern_size_essential();

      // thing
	int	zeroline = da_thing_ht/2;
	cairo_set_source_rgb( cr, 0., 0., 0.);
	cairo_set_line_width( cr, .8);
	aghui::cairo_draw_signal( cr, Q->thing, 0, Q->thing.size(),
				  da_thing_wd, 0, zeroline,
				  thing_display_scale);
	cairo_stroke( cr);

	// lines marking out context
	cairo_set_source_rgba( cr, 0.9, 0.9, 0.9, .5);
	cairo_set_line_width( cr, 1.);
	cairo_rectangle( cr, 0., 0., (float)Q->context_before / Q->thing.size() * da_thing_wd, da_thing_ht);
	cairo_rectangle( cr, (float)(Q->context_before + run) / Q->thing.size() * da_thing_wd, 0,
			 (float)(Q->context_after) / Q->thing.size() * da_thing_wd, da_thing_ht);
	cairo_fill( cr);
	cairo_stroke( cr);

	if ( draw_details ) {
		valarray<TFloat>
			env_u, env_l,
			target_freq,
			dzcdf;
	      // envelope
		{
			if ( sigproc::envelope( {Q->thing, Q->samplerate}, Q->Pp.env_scope,
						1./Q->samplerate,
						&env_l, &env_u) == 0 ) {
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Selection is too short");
				goto out;
			}

			_p._p.CwB[SExpDesignUI::TColour::sf_selection].set_source_rgba_contrasting( cr, .3);
			aghui::cairo_draw_signal( cr, env_u, 0, env_u.size(),
						  da_thing_wd, 0, zeroline, thing_display_scale);
			aghui::cairo_draw_signal( cr, env_l, 0, env_l.size(),
						  da_thing_wd, 0, zeroline, thing_display_scale,
						  1, aghui::TDrawSignalDirection::backward, true);
			cairo_close_path( cr);
			cairo_fill( cr);
			cairo_stroke( cr);
		}
	      // target frequency
		{
			if ( Q->Pp.bwf_ffrom >= Q->Pp.bwf_fupto ) {
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Bad band-pass range");
				goto out;
			}
			target_freq = exstrom::band_pass(
				Q->thing, Q->samplerate,
				Q->Pp.bwf_ffrom, Q->Pp.bwf_fupto, Q->Pp.bwf_order, true);

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
			cairo_set_line_width( cr, 3.);
			aghui::cairo_draw_signal( cr, target_freq, 0, target_freq.size(),
						  da_thing_wd, 0, zeroline, thing_display_scale);
			cairo_stroke( cr);
		}

	      // dzcdf
		{
			if ( Q->samplerate < 10 ) {
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Samplerate is too low");
				goto out;
			}
			if ( Q->Pp.dzcdf_step * 10 > Q->pattern_length() ) { // require at least 10 dzcdf points
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Selection is too short");
				goto out;
			}

			dzcdf = sigproc::dzcdf( sigproc::SSignalRef<TFloat> {Q->thing, Q->samplerate},
						Q->Pp.dzcdf_step, Q->Pp.dzcdf_sigma, Q->Pp.dzcdf_smooth);
			float	dzcdf_display_scale = da_thing_ht/4. / dzcdf.max();

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
			cairo_set_line_width( cr, 1.);
			aghui::cairo_draw_signal( cr, dzcdf, 0, dzcdf.size(),
						  da_thing_wd, 0, zeroline, dzcdf_display_scale);
			cairo_stroke( cr);
		}
	}
out:
	;
}

void
aghui::SScoringFacility::SFindDialog::
draw_field( cairo_t *cr)
{
      // field
	_p._p.CwB[SExpDesignUI::TColour::sf_profile_psd].set_source_rgba( cr, .5);
	cairo_set_line_width( cr, 1.);
	cairo_move_to( cr, 0, da_field_ht/2);
	auto& profile = field_channel->which_profile( field_profile_type);
	aghui::cairo_draw_signal(
		cr,
		profile, 0, profile.size(),
		da_field_wd, 0., da_field_ht/2,
		field_display_scale,
		1, TDrawSignalDirection::forward, true);
	cairo_line_to( cr, da_field_wd, da_field_ht/2);
	cairo_fill( cr);
	cairo_stroke( cr);

      // occurrences
	if ( occurrences.size() ) {
		cairo_set_line_width( cr, 1.);
		for ( size_t o = 0; o < occurrences.size(); ++o ) {
			auto x = (double)occurrences[o]/diff_line.size() * da_field_wd;
			if ( o == highlighted_occurrence )
				cairo_set_source_rgba( cr, .1, .3, .5, 1.);
			else
				cairo_set_source_rgba( cr, .1, .3, .5, .4);

			cairo_rectangle(
				cr,
				x - 1, da_field_ht * .75 - 5,
				2, 10);
			cairo_fill( cr);
			cairo_stroke( cr);
		}
	} else
		aghui::cairo_put_banner(
			cr, da_field_wd, da_field_ht / .75, "Nothing found");

      // diff line with degree of criteria attainment
	cairo_set_line_width( cr, .5);
	valarray<TFloat> tmp (diff_line.size());

#define KEKE(R,G,B,N)		 \
	cairo_set_source_rgba( cr, R, G, B, 1.); \
	for ( size_t i = 0; i < diff_line.size(); ++i ) tmp[i] = get<N>(diff_line[i]); \
	aghui::cairo_draw_signal( cr, tmp, 0, tmp.size(), da_field_wd, 0., da_field_ht-20, get<N>(criteria) / 20); \
	cairo_stroke( cr);

	// FAFA;
	// KEKE(.1, .5, .8, 0);
	// FAFA;
	// KEKE(.5, .1, .8, 1);
	// FAFA;
	// KEKE(.1, .8, .5, 2);
	// FAFA;
	// KEKE(.5, .8, .1, 3);
#undef KEKE
}

// eof
