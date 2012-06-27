// ;-*-C++-*-
/*
 *       File name:  ui/misc.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-09-03
 *
 *         Purpose:  misc general-purpose bits
 *
 *         License:  GPL
 */


#ifndef _AGH_UI_MISC_H
#define _AGH_UI_MISC_H

#include <cstdlib>
#include <cstring>
#include <string>
#include <valarray>
#include <itpp/base/mat.h>
#include <gtk/gtk.h>

#include "../common/misc.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace aghui {


#define snprintf_buf(...) snprintf( __buf__, AGH_BUF_SIZE-1, __VA_ARGS__)

void snprintf_buf_ts_d( double h);
void snprintf_buf_ts_h( double h);
void snprintf_buf_ts_m( double m);
void snprintf_buf_ts_s( double s);

void decompose_double( double value, float *mantissa, int *exponent);



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


inline void
gtk_flush()
{
	while ( gtk_events_pending() )
		gtk_main_iteration();
}

void pop_ok_message( GtkWindow *parent, const gchar*, ...);
gint pop_question( GtkWindow *parent, const gchar*, ...);
void set_cursor_busy( bool busy, GtkWidget *wid);

} // namespace aghui

#endif

// EOF
