// ;-*-C++-*-
/*
 *       File name:  ui/managed-colour.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-28
 *
 *         Purpose:  SManagedColor
 *
 *         License:  GPL
 */


#ifndef _AGH_UI_MANAGED_COLOUR_H
#define _AGH_UI_MANAGED_COLOUR_H

#include <cstdlib>
#include <cmath>
#include <gtk/gtk.h>
#include <cairo/cairo.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace aghui {

inline const GdkColor*
contrasting_to( const GdkColor* c)
{
	static GdkColor cc;
	if ( c->red + c->green + c->blue < 65535*3/2 )
		cc.red = cc.green = cc.blue = 65535;
	else
		cc.red = cc.green = cc.blue = 0;
	return &cc;
}


struct SManagedColor {
	GdkRGBA clr;
	GtkColorButton* btn;

	SManagedColor& operator=( const SManagedColor&) = default;
	void acquire()
	{
		gtk_color_chooser_get_rgba( GTK_COLOR_CHOOSER (btn), &clr);
	}

	void set_source_rgb( cairo_t* cr) const
		{
			cairo_set_source_rgb( cr, clr.red, clr.green, clr.blue);
		}
	void set_source_rgba( cairo_t* cr, double alpha_override = NAN) const
		{
			cairo_set_source_rgba( cr, clr.red, clr.green, clr.blue,
					       isfinite(alpha_override) ? alpha_override : clr.alpha);
		}
	void pattern_add_color_stop_rgba( cairo_pattern_t* cp, double at, double alpha_override = NAN) const
		{
			cairo_pattern_add_color_stop_rgba( cp, at, clr.red, clr.green, clr.blue,
							   isfinite(alpha_override) ? alpha_override : clr.alpha);
		}
};

} // namespace aghui

#endif

// eof
