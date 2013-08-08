/*
 *       File name:  aghermann/ui/ui.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose: collected global variables for use after
 *                  gtk_main(), and __buf__ and __ss__ strings
 *
 *         License:  GPL
 */


#ifndef AGH_AGHERMANN_UI_UI_H_
#define AGH_AGHERMANN_UI_UI_H_

#include <cstdlib>
#include <cstring>
#include <string>
#include <valarray>
#include <itpp/base/mat.h>
#include <gtk/gtk.h>

#include "common/lang.hh"
#include "aghermann/ui/globals.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


namespace agh {
namespace ui {


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


enum class TGtkRefreshMode {
	gtk, gdk
};

inline void
gtk_flush()
{
	while ( gtk_events_pending() )
		gtk_main_iteration();
}

void gtk_combo_box_set_model_properly( GtkComboBox*, GtkListStore*);
void gtk_cell_layout_set_renderer( GtkComboBox*);



enum class TDrawSignalDirection { forward, backward };
enum class TDrawSignalPathOption { yes, no };

void
cairo_draw_signal( cairo_t*,
		   const valarray<TFloat>&,
		   ssize_t start, ssize_t end,
		   size_t da_wd, float hdisp, float vdisp, float display_scale,
		   unsigned short decimate = 1,
		   TDrawSignalDirection direction = TDrawSignalDirection::forward,
		   TDrawSignalPathOption continue_path = TDrawSignalPathOption::yes);
void
cairo_draw_envelope( cairo_t*,
		     const valarray<TFloat>&,
		     ssize_t start, ssize_t end,
		     size_t da_wd, float hdisp, float vdisp, float display_scale);

inline void
cairo_draw_signal( cairo_t *cr,
		   const itpp::Mat<double>& signal, int row,
		   ssize_t start, ssize_t end,
		   size_t width, double hdisp, double vdisp, float display_scale,
		   unsigned short decimate = 1,
		   TDrawSignalDirection direction = TDrawSignalDirection::forward,
		   TDrawSignalPathOption continue_path = TDrawSignalPathOption::yes)
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



void pop_ok_message( GtkWindow *parent, const char* primary, const gchar*, ...) __attribute__ ((format (printf, 3, 4)));
gint pop_question( GtkWindow *parent, const char* primary, const char*, ...) __attribute__ ((format (printf, 3, 4)));
void set_cursor_busy( bool busy, GtkWidget *wid);




#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)			\
	(A = (Type*)(gtk_builder_get_object( B, #A)))

#define G_CONNECT_1(W, A)						\
	g_signal_connect(W, #A, (GCallback)W ## _ ## A ## _cb, this)
#define G_CONNECT_2(W, A1, A2)						\
	g_signal_connect(W, #A1 "-" #A2, (GCallback)W ## _ ## A1 ## _ ## A2 ## _cb, this)
#define G_CONNECT_3(W, A1, A2, A3)					\
	g_signal_connect(W, #A1 "-" #A2 "-" #A3, (GCallback)W ## _ ## A1 ## _ ## A2 ## _ ## A3 ## _cb, this)

}
} // namespace agh::ui

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
