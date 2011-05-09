// ;-*-C++-*- *  Time-stamp: "2011-05-09 23:55:27 hmmr"
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

#ifndef _AGH_DRAW_SIGNAL_GENERIC
#define _AGH_DRAW_SIGNAL_GENERIC

#include <valarray>
#include <cairo/cairo.h>


// draw arbitrary region
void draw_signal( const std::valarray<float>& signal,
		  size_t start, size_t end,
		  unsigned, int, float display_scale,
		  cairo_t*, bool do_resample);


#endif

// eof
