// ;-*-C++-*- *  Time-stamp: "2011-01-06 00:42:07 hmmr"
/*
 *       File name:  libagh/signal.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
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


using namespace std;

namespace NSignal {

valarray<double>
interpolate_d( const vector<size_t>& xi,
	       size_t samplerate,
	       const valarray<double>& y,
	       double dt)
{
	size_t i;
	valarray<double>
		x_known (xi.size()),
		y_known (xi.size());
	for ( i = 0; i < xi.size(); ++i ) {
		x_known[i] = (double)xi[i] / samplerate;
		y_known[i] = y[ xi[i] ];
	}

	gsl_spline *spline = gsl_spline_alloc( gsl_interp_akima, xi.size());;
	gsl_interp_accel *acc = gsl_interp_accel_alloc();

	gsl_spline_init( spline, &x_known[0], &y_known[0], xi.size());

	double	dx = dt * samplerate,
		x;
	size_t	n_signif   = (*xi.rbegin() - *xi. begin()) / dx,
		pad_before = (*xi. begin() - 0           ) / dx;
		//pad_after  = (    y.size() - *xi.rbegin()) / dx;
	// valarray<double> out (pad_before + n_signif + pad_after);
	valarray<double> out ((size_t)(y.size() / dx));
	for ( i = pad_before, x = *xi.begin(); i < pad_before + n_signif; ++i, x += dx )
		out[i] = gsl_spline_eval( spline, x / samplerate, acc);

	gsl_interp_accel_free( acc);
	gsl_spline_free( spline);

	return out;
}

} // namespace NSignal

// eof
