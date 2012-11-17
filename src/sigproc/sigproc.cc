// ;-*-C++-*-
/*
 *       File name:  sigproc/sigproc.cc
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


template void sigproc::smooth( valarray<TFloat>&, size_t);
template void sigproc::normalize( valarray<TFloat>&);
template valarray<TFloat> sigproc::derivative( const valarray<TFloat>&);
template size_t sigproc::envelope( const valarray<TFloat>&, size_t, size_t, double, valarray<TFloat>&, valarray<TFloat>&, vector<size_t>*, vector<size_t>*);
template valarray<TFloat> sigproc::dzcdf( const valarray<TFloat>&, size_t, float, float, size_t);
template sigproc::CPattern<TFloat>::CPattern( const valarray<TFloat>&, size_t, size_t, size_t, const SPatternParamPack&, float, float, float);
template size_t sigproc::CPattern<TFloat>::find( const valarray<TFloat>&, const valarray<TFloat>&, const valarray<TFloat>&, ssize_t, int);
template size_t sigproc::CPattern<TFloat>::find( const valarray<TFloat>&, ssize_t, int);
template double sigproc::sig_diff( const valarray<TFloat>&, const valarray<TFloat>&, int);
template double sigproc::phase_diff( const valarray<TFloat>&, const valarray<TFloat>&, size_t, size_t, size_t, float, float, unsigned, size_t);


valarray<float>
sigproc::
resample_f( const valarray<float>& signal,
	    size_t start, size_t end,
	    size_t to_size,
	    int alg)
{
	if ( unlikely (end <= start) || (end > signal.size()) )
		throw runtime_error ("bad args for resample");

	valarray<float>	resampled (to_size);
	SRC_DATA S;
	S.data_in       = const_cast<float*>(&signal[start]);
	S.input_frames  = end - start;
	S.output_frames = to_size;
	S.data_out      = &resampled[0];
	S.src_ratio     = (double)S.output_frames / S.input_frames;

	src_simple( &S, alg, 1);

	return resampled;
}

valarray<double>
sigproc::
resample( const valarray<double>& signal,
	  size_t start, size_t end,
	  size_t to_size,
	  int alg)
{
	valarray<float> tmp1 (end - start);
	for ( size_t i = start; i < end; ++i )
		tmp1[i] = signal[i];

	valarray<float> tmp2
		= sigproc::resample_f( tmp1, 0, end-start, to_size, alg);

	valarray<double> ret (end-start);
	for ( size_t i = 0; i < end-start; ++i )
		ret[i] = tmp2[i];

	return ret;
}







valarray<double>
sigproc::
interpolate_d( const vector<size_t>& xi,
	       unsigned samplerate,
	       const valarray<double>& y,
	       float dt)
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


valarray<float>
sigproc::
interpolate( const vector<size_t>& xi,
	     unsigned samplerate,
	     const valarray<float>& y,
	     float dx)
{
	valarray<double> in (y.size());
	for ( size_t i = 0; i < y.size(); ++i )
		in[i] = y[i];

	valarray<double> tmp = sigproc::interpolate_d( xi, samplerate, in, dx);

	valarray<float> out (tmp.size());
	for ( size_t i = 0; i < tmp.size(); ++i )
		out[i] = tmp[i];
	return out;
}

// eof
