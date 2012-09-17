// ;-*-C++-*-
/*
 *       File name:  ui/globals.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose: collected global variables for use after
 *                  gtk_main(), and __buf__ and __ss__ strings
 *
 *         License:  GPL
 */


#ifndef _AGHUI_GLOBALS_H
#define _AGHUI_GLOBALS_H

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


// convenience assign-once vars
extern GdkDevice
	*__client_pointer__;

// quick tmp storage
#define AGH_BUF_SIZE (1024*5)
extern char
	__buf__[AGH_BUF_SIZE];
extern GString
	*__ss__;


// project-specific 'misc.hh'
int prepare_for_expdesign();




#define snprintf_buf(...) snprintf( __buf__, AGH_BUF_SIZE-1, __VA_ARGS__)

void snprintf_buf_ts_d( double h);
void snprintf_buf_ts_h( double h);
void snprintf_buf_ts_m( double m);
void snprintf_buf_ts_s( double s);

void decompose_double( double value, float *mantissa, int *exponent);



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





#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)				\
	(A = (Type*)(gtk_builder_get_object( B, #A)))


// our files in share/aghermann/data
#define AGH_UI_MAIN_GLADE "ui/main.glade"
#define AGH_UI_DIALOGS_GLADE "ui/dialogs.glade"
#define AGH_UI_SESSION_CHOOSER_GLADE "ui/session-chooser.glade"
#define AGH_UI_SF_GLADE "ui/sf.glade"
#define AGH_UI_MF_GLADE "ui/mf.glade"

#define AGH_BG_IMAGE_FNAME "ui/idle-bg.svg"


} // namespace aghui

#endif

// eof
