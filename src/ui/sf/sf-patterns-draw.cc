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
		      "width-request", da_thing_wd = width,
		      "height-request", da_thing_ht,
		      NULL);
	g_object_set( (GObject*)_p.swSFFDThing,
		      "width-request", min( width+5, 600),
		      "height-request", da_thing_ht + 30,
		      NULL);
}

void
aghui::SScoringFacility::SFindDialog::
set_field_da_width( int width)
{
	g_object_set( (GObject*)_p.daSFFDField,
		      "width-request", da_field_wd = width,
		      "height-request", da_field_ht,
		      NULL);
	g_object_set( (GObject*)_p.swSFFDField,
		      "width-request", min( width+5, 600),
		      "height-request", da_thing_ht + 30,
		      NULL);
}



void
aghui::SScoringFacility::SFindDialog::
draw_thing( cairo_t *cr)
{
	if ( thing.size() == 0 ) {
		set_thing_da_width( 200);
		aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "(no selection)");
		return;
	} else {
	}

      // ticks
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 9);
	double	seconds = (double)thing.size() / samplerate;
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

	size_t	run = pattern_size_essential();

      // snippet
	cairo_set_source_rgb( cr, 0., 0., 0.);
	cairo_set_line_width( cr, .8);
	aghui::cairo_draw_signal( cr, thing, 0, thing.size(),
				  da_thing_wd, 0, da_thing_ht/3, thing_display_scale);
	cairo_stroke( cr);

	// lines marking out context
	cairo_set_source_rgba( cr, 0.9, 0.9, 0.9, .5);
	cairo_set_line_width( cr, 1.);
	cairo_rectangle( cr, 0., 0., (float)context_before / thing.size() * da_thing_wd, da_thing_ht);
	cairo_rectangle( cr, (float)(context_before + run) / thing.size() * da_thing_wd, 0,
			 (float)(context_after) / thing.size() * da_thing_wd, da_thing_ht);
	cairo_fill( cr);
	cairo_stroke( cr);

	if ( draw_details ) {
		valarray<TFloat>
			env_u, env_l,
			course,
			dzcdf;
	      // envelope
		{
			if ( sigproc::envelope( {thing, samplerate}, Pp.env_tightness,
						1./samplerate,
						&env_l, &env_u) == 0 ) {
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Selection is too short");
				goto out;
			}

			_p._p.CwB[SExpDesignUI::TColour::sf_selection].set_source_rgba_contrasting( cr, .3);
			aghui::cairo_draw_signal( cr, env_u, 0, env_u.size(),
						  da_thing_wd, 0, da_thing_ht/2, thing_display_scale);
			aghui::cairo_draw_signal( cr, env_l, 0, env_l.size(),
						  da_thing_wd, 0, da_thing_ht/2, thing_display_scale, 1,
						  aghui::TDrawSignalDirection::backward, true);
			cairo_close_path( cr);
			cairo_fill( cr);
			cairo_stroke( cr);
		}

	      // target frequency
		{
			if ( Pp.bwf_ffrom >= Pp.bwf_fupto ) {
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Bad band-pass range");
				goto out;
			}
			course = exstrom::band_pass(
				thing, samplerate,
				Pp.bwf_ffrom, Pp.bwf_fupto, Pp.bwf_order, true);

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
			cairo_set_line_width( cr, 3.);
			aghui::cairo_draw_signal( cr, course, 0, course.size(),
						  da_thing_wd, 0, da_thing_ht/3, thing_display_scale);
			cairo_stroke( cr);
		}

	      // dzcdf
		{
			if ( samplerate < 10 ) {
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Samplerate is too low");
				goto out;
			}
			if ( Pp.dzcdf_step * 10 > pattern_length() ) { // require at least 10 dzcdf points
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Selection is too short");
				goto out;
			}

			dzcdf = sigproc::dzcdf( sigproc::SSignalRef<TFloat> {thing, samplerate},
						Pp.dzcdf_step, Pp.dzcdf_sigma, Pp.dzcdf_smooth);
			float	dzcdf_display_scale = da_thing_ht/4. / dzcdf.max();

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
			cairo_set_line_width( cr, 1.);
			aghui::cairo_draw_signal( cr, dzcdf, 0, dzcdf.size(),
						  da_thing_wd, 0, da_thing_ht/2-5, dzcdf_display_scale);
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
	_p._p.CwB[SExpDesignUI::TColour::sf_profile_psd].set_source_rgba( cr, .5);
	cairo_set_line_width( cr, 1.);
	cairo_move_to( cr, 0, da_field_ht/2);
	printf( "draw %d %d %g\n", da_field_wd, da_field_ht, field_display_scale);
	aghui::cairo_draw_signal(
		cr,
		field_channel->psd.course, 0, field_channel->psd.course.size(),
		da_field_wd, 0., da_field_ht/2, field_display_scale,
		1, TDrawSignalDirection::forward, true);
	cairo_line_to( cr, da_field_wd, da_field_ht/2);
	cairo_fill( cr);

	cairo_stroke( cr);
}

// eof
