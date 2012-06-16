// ;-*-C++-*-
/*
 *       File name:  libexstrom/sigproc.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-12-26
 *
 *         Purpose:  various standalone signal processing functions
 *
 *         License:  GPL
 */

#include <vector>
#include <valarray>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>

#include "sigproc.hh"

using namespace std;



template valarray<TFloat>& sigproc::smooth( valarray<TFloat>&, size_t);
template valarray<TFloat> sigproc::interpolate( const vector<size_t>&, size_t, const valarray<TFloat>&, double);
template size_t sigproc::envelope( const valarray<TFloat>&, size_t, size_t, double, valarray<TFloat>&, valarray<TFloat>&, vector<size_t>*, vector<size_t>*);
template valarray<TFloat> sigproc::dzcdf( const valarray<TFloat>&, size_t, float, float, size_t);
template sigproc::CPattern<TFloat>::CPattern( const valarray<TFloat>&, size_t, size_t, size_t, size_t, float, bool, size_t, float, float, size_t, float, float, float);
template size_t sigproc::CPattern<TFloat>::find( const valarray<TFloat>&, const valarray<TFloat>&, const valarray<TFloat>&, ssize_t, int);
template size_t sigproc::CPattern<TFloat>::find( const valarray<TFloat>&, ssize_t, int);
template double sigproc::sig_diff( const valarray<TFloat>&, const valarray<TFloat>&, int);
template double sigproc::phase_diff( const valarray<TFloat>&, const valarray<TFloat>&, size_t, size_t, size_t, float, float, unsigned, size_t);





valarray<double>
sigproc::interpolate_d( const vector<size_t>& xi,
			size_t samplerate,
			const valarray<double>& y,
			double dt)
{
//	if ( 1. / samplerate > dt )
//		return y;

	size_t i;
	valarray<double>
		x_known (xi.size()),
		y_known (xi.size());
	for ( i = 0; i < xi.size(); ++i ) {
		x_known[i] = (double)xi[i] / samplerate;
		y_known[i] = y[ xi[i] ];
	}

	gsl_spline *spline = gsl_spline_alloc( gsl_interp_akima, xi.size());
	gsl_interp_accel *acc = gsl_interp_accel_alloc();

	gsl_spline_init( spline, &x_known[0], &y_known[0], xi.size());

	double	t;
	size_t	pad = (1./samplerate / dt) // this I understand
			/ 2;                // this, I don't
	valarray<double>
		out (ceilf((x_known[x_known.size()-1] - x_known[0] + 1./samplerate) / dt) + 1);
	for ( i = pad, t = x_known[0]; t < x_known[x_known.size()-1]; ++i, t += dt )
		out[i] = gsl_spline_eval( spline, t, acc);

	gsl_interp_accel_free( acc);
	gsl_spline_free( spline);

	return out;
}




// window functions

// The following window functions have been taken from fft.c, part of WFDB package

#define TWOPI (M_PI*2)

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



// extern template
// valarray<TFloat>&
// sigproc::
// smooth( valarray<TFloat>& a, size_t side);


// extern template
// valarray<TFloat>
// sigproc::
// interpolate( const vector<size_t>& xi,
// 	     size_t samplerate,
// 	     const valarray<TFloat>& y,
// 	     double dx);

// eof
