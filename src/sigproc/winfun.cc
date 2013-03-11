/*
 *       File name:  sigproc/winfun.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-17
 *
 *         Purpose:  windowing functions
 *
 *         License:  GPL
 */

#include <cmath>
#include "winfun.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

// must match those defined in glade
const char*
	sigproc::welch_window_type_names[sigproc::TWinType::TWinType_total] = {
	"Bartlett", "Blackman", "Blackman-Harris",
	"Hamming",  "Hanning",  "Parzen",
	"Square",   "Welch"
};


namespace {

#define TWOPI (M_PI*2)

// The following window functions have been taken from fft.c, part of WFDB package

/* See Oppenheim & Schafer, Digital Signal Processing, p. 241 (1st ed.) */
TFloat
__attribute__ ((const))
win_bartlett( size_t j, size_t n)
{
	TFloat a = 2.0/(n-1), w;
	if ( (w = j*a) > 1. )
		w = 2. - w;
	return w;
}

/* See Oppenheim & Schafer, Digital Signal Processing, p. 242 (1st ed.) */
TFloat
__attribute__ ((const))
win_blackman( size_t j, size_t n)
{
	TFloat a = TWOPI/(n-1), w;
	w = 0.42 - .5 * cos(a * j) + .08 * cos(2 * a * j);
	return w;
}

/* See Harris, F.J., "On the use of windows for harmonic analysis with the
   discrete Fourier transform", Proc. IEEE, Jan. 1978 */
TFloat
__attribute__ ((const))
win_blackman_harris( size_t j, size_t n)
{
	TFloat a = TWOPI/(n-1), w;
	w = 0.35875 - 0.48829 * cos(a * j) + 0.14128 * cos(2 * a * j) - 0.01168 * cos(3 * a * j);
	return w;
}

/* See Oppenheim & Schafer, Digital Signal Processing, p. 242 (1st ed.) */
TFloat
__attribute__ ((const))
win_hamming( size_t j, size_t n)
{
	TFloat a = TWOPI/(n-1), w;
	w = 0.54 - 0.46*cos(a*j);
	return w;
}

/* See Oppenheim & Schafer, Digital Signal Processing, p. 242 (1st ed.)
   The second edition of Numerical Recipes calls this the "Hann" window. */
TFloat
__attribute__ ((const))
win_hanning( size_t j, size_t n)
{
	TFloat a = TWOPI/(n-1), w;
	w = 0.5 - 0.5*cos(a*j);
	return w;
}

/* See Press, Flannery, Teukolsky, & Vetterling, Numerical Recipes in C,
   p. 442 (1st ed.) */
TFloat
__attribute__ ((const))
win_parzen( size_t j, size_t n)
{
	TFloat a = (n-1)/2.0, w;
	if ( (w = (j-a)/(a+1)) > 0.0 )
		w = 1 - w;
	else
		w = 1 + w;
	return w;
}

/* See any of the above references. */
TFloat
__attribute__ ((const))
win_square( size_t, size_t)
{
	return 1.0;
}

/* See Press, Flannery, Teukolsky, & Vetterling, Numerical Recipes in C,
   p. 442 (1st ed.) or p. 554 (2nd ed.) */
TFloat
__attribute__ ((const))
win_welch( size_t j, size_t n)
{
	TFloat a = (n-1)/2.0, w;
	w = (j-a)/(a+1);
	w = 1 - w*w;
	return w;
}

} // namespace

TFloat (*sigproc::winf[])(size_t, size_t) = {
	win_bartlett,
	win_blackman,
	win_blackman_harris,
	win_hamming,
	win_hanning,
	win_parzen,
	win_square,
	win_welch
};


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
