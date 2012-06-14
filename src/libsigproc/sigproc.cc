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
