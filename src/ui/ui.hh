// ;-*-C++-*-
/*
 *       File name:  ui/ui.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose: collected global variables for use after
 *                  gtk_main(), and __buf__ and __ss__ strings
 *
 *         License:  GPL
 */


#ifndef _AGHUI_UI_H
#define _AGHUI_UI_H

#include <cstdlib>
#include <cstring>
#include <string>
#include <valarray>
#include <itpp/base/mat.h>
#include <gtk/gtk.h>
#include "../common/lang.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


namespace aghui {


struct SGeometry {
	int x, y, w, h;
	SGeometry()
	      : x (-1), y (-1), w (-1), h (-1)
		{}
	SGeometry( int x_, int y_, int w_, int h_)
	      : x (x_), y (y_), w (w_), h (h_)
		{}
	bool is_valid() const
		{
			return	(x > 0) && (x < 3000) &&
				(y > 0) && (y < 3000) &&
				(w > 1) && (w < 50000) &&
				(h > 1) && (h < 50000);
		}
};


inline GdkColor
contrasting_to( const GdkColor* c)
{
	GdkColor cc;
	if ( c->red + c->green + c->blue < 65535*3/2 )
		cc.red = cc.green = cc.blue = 65535;
	else
		cc.red = cc.green = cc.blue = 0;
	return cc;
}


struct SManagedColor {
	GdkRGBA clr;
	GtkColorButton* btn;

	SManagedColor& operator=( const SManagedColor&) = default;
	void acquire()
		{
			gtk_color_chooser_get_rgba( (GtkColorChooser*)btn, &clr);
		}

	void set_source_rgb( cairo_t* cr) const
		{
			cairo_set_source_rgb( cr, clr.red, clr.green, clr.blue);
		}
	void set_source_rgba( cairo_t* cr, double alpha_override = NAN) const
		{
			cairo_set_source_rgba(
				cr, clr.red, clr.green, clr.blue,
				isfinite(alpha_override) ? alpha_override : clr.alpha);
		}
	void set_source_rgb_contrasting( cairo_t* cr) const
		{
			cairo_set_source_rgb(
				cr, 1-clr.red, 1-clr.green, 1-clr.blue);
		}
	void set_source_rgba_contrasting( cairo_t* cr, double alpha_override = NAN) const
		{
			cairo_set_source_rgba(
				cr, 1-clr.red, 1-clr.green, 1-clr.blue,
				isfinite(alpha_override) ? alpha_override : clr.alpha);
		}

	void pattern_add_color_stop_rgba( cairo_pattern_t* cp, double at, double alpha_override = NAN) const
		{
			cairo_pattern_add_color_stop_rgba(
				cp, at, clr.red, clr.green, clr.blue,
				isfinite(alpha_override) ? alpha_override : clr.alpha);
		}
};



inline void
gtk_flush()
{
	while ( gtk_events_pending() )
		gtk_main_iteration();
}

enum TDrawSignalDirection { Forward, Backward };
void
cairo_draw_signal( cairo_t *cr,
		   const valarray<TFloat>& signal,
		   ssize_t start, ssize_t end,
		   size_t da_wd, double hdisp, double vdisp, float display_scale,
		   unsigned short decimate = 1,
		   TDrawSignalDirection direction = TDrawSignalDirection::Forward,
		   bool continue_path = false);

inline void
cairo_draw_signal( cairo_t *cr,
		   const itpp::Mat<double>& signal, int row,
		   ssize_t start, ssize_t end,
		   size_t width, double hdisp, double vdisp, float display_scale,
		   unsigned short decimate = 1,
		   TDrawSignalDirection direction = TDrawSignalDirection::Forward,
		   bool continue_path = false)
{
	valarray<TFloat> tmp (end - start); // avoid copying other rows, cols
	for ( ssize_t c = 0; c < (end-start); ++c )
		if ( likely (start + c > 0 && start + c < (ssize_t)signal.size()) )
			tmp[c] = signal(row, start + c);
	cairo_draw_signal( cr,
			   tmp, 0, end-start,
			   width, hdisp, vdisp, display_scale,
			   decimate,
			   direction,
			   continue_path);
}


void
cairo_put_banner( cairo_t *cr,
		  float wd, float ht,
		  const char *text,
		  float font_size = 18,
		  float r = .1, float g = .1, float b = .1, float a = .3);



void pop_ok_message( GtkWindow *parent, const gchar*, ...);
gint pop_question( GtkWindow *parent, const gchar*, ...);
void set_cursor_busy( bool busy, GtkWidget *wid);


class SBusyBlock {
	DELETE_DEFAULT_METHODS (SBusyBlock);
    public:
	SBusyBlock (GtkWidget* w_)
	      : w (w_)
		{
			lock();
		}
	// poor ubuntu people
	// SBusyBlock (GtkWindow* w)
	//       : SBusyBlock ((GtkWidget*)w)
	// 	{}
	// SBusyBlock (GtkDialog* w)
	//       : SBusyBlock ((GtkWidget*)w)
	// 	{}
	SBusyBlock (GtkWindow* w_)
	      : w ((GtkWidget*)w_)
		{
			lock();
		}
	SBusyBlock (GtkDialog* w_)
	      : w ((GtkWidget*)w_)
		{
			lock();
		}

       ~SBusyBlock ()
		{
			set_cursor_busy( false, w);
			gtk_widget_set_sensitive( w, TRUE);
			gtk_flush();
		}
    private:
	GtkWidget *w;
	void lock()
		{
			gtk_widget_set_sensitive( w, FALSE);
			set_cursor_busy( true, w);
			gtk_flush();
		}
};




template <typename Tw, typename Tv>
class SUIVar {
	DELETE_DEFAULT_METHODS (SUIVar);

    private:
	Tw	*w;
	Tv&	v;

    public:
	SUIVar (Tw w_, Tv& v_)
	      : w (w_), v (v_)
		{}
	void down();
	void up();
};


template <>
inline void
SUIVar<GtkSpinButton, double>::up()
{
	gtk_spin_button_set_value( w, v);
}

template <>
inline void
SUIVar<GtkSpinButton, double>::down()
{
	v = gtk_spin_button_get_value( w);
}


template <>
inline void
SUIVar<GtkCheckButton, bool>::up()
{
	gtk_toggle_button_set_active( (GtkToggleButton*)w, v);
}

template <>
inline void
SUIVar<GtkCheckButton, bool>::down()
{
	v = gtk_toggle_button_get_active( (GtkToggleButton*)w);
}


template <>
inline void
SUIVar<GtkEntry, string>::up()
{
	gtk_entry_set_text( w, v.c_str());
}

template <>
inline void
SUIVar<GtkEntry, string>::down()
{
	const char *tmp = gtk_entry_get_text( w);
	v.assign(tmp);
	g_free( (void*)tmp);
}





#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)			\
	(A = (Type*)(gtk_builder_get_object( B, #A)))


} // namespace aghui

#endif

// eof
