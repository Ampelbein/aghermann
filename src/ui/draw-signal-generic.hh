// ;-*-C++-*-
/*
 *       File name:  ui/draw-signal-generic.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-09
 *
 *         Purpose:  generic draw_signal to a cairo_t
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_DRAW_SIGNAL_GENERIC_H
#define _AGH_UI_DRAW_SIGNAL_GENERIC_H

#include <valarray>
#include <cairo/cairo.h>
#include <itpp/base/mat.h>

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

// draw arbitrary region
void draw_signal( const valarray<float>& signal,
		  size_t start, size_t end,
		  unsigned, int, float display_scale,
		  cairo_t*, bool do_resample);
// libresample strictly uses float

inline void
draw_signal( const valarray<double>& signal,
	     size_t start, size_t end,
	     unsigned width, int vdisp, float display_scale,
	     cairo_t *cr, bool use_resample)
{
	valarray<float> tmp (signal.size());
	for ( size_t i = 0; i < signal.size(); ++i )
		tmp[i] = signal[i];
	draw_signal( tmp, start, end, width, vdisp, display_scale, cr, use_resample);
}


// draw from itpp::Mat
void draw_signal( const itpp::Mat<float>& signal, int row,
		  size_t start, size_t end,
		  unsigned, int, float display_scale,
		  cairo_t*, bool do_resample);
// libresample strictly uses float

inline void
draw_signal( const itpp::Mat<double>& signal, int row,
	     size_t start, size_t end,
	     unsigned width, int vdisp, float display_scale,
	     cairo_t *cr, bool use_resample)
{
	valarray<float> tmp (end - start); // avoid copying other rows, cols
	for ( size_t c = 0; c < (end-start); ++c )
		tmp[c] = signal(row, start + c);
	draw_signal( tmp, 0, end-start, width, vdisp, display_scale, cr, use_resample);
}

#endif

// eof
