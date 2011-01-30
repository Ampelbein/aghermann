// ;-*-C++-*- *  Time-stamp: "2011-01-30 22:26:09 hmmr"
/*
 *       File name:  libexstrom/signal.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-26
 *
 *         Purpose:  various signal processing functions
 *
 *         License:  GPL
 */

#ifndef _SIGNAL_HH
#define _SIGNAL_HH

#include <cmath>
#include <vector>
#include <valarray>
#include <stdexcept>

#include "exstrom.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif

using namespace std;



namespace NSignal {



valarray<double>
interpolate_d( const vector<size_t>& xi,
	       size_t samplerate,
	       const valarray<double>& y,
	       double dt);

template <class T>
inline valarray<T>
interpolate( const vector<size_t>& xi,
	     size_t samplerate,
	     const valarray<T>& y,
	     double dx)
{
	valarray<double> in (y.size());
	for ( size_t i = 0; i < y.size(); ++i )
		in[i] = y[i];

	valarray<double> tmp = interpolate_d( xi, samplerate, in, dx);

	valarray<T> out (tmp.size());
	for ( size_t i = 0; i < tmp.size(); ++i )
		out[i] = tmp[i];
	return out;
}

template <>
inline valarray<double>
interpolate( const vector<size_t>& xi,
	     size_t samplerate,
	     const valarray<double>& y,
	     double dt)
{
	return interpolate( xi, samplerate, y, dt);
}



template <class T>
size_t
envelope( const valarray<T>& in,
	  size_t dh,  // tightness
	  size_t samplerate,
	  double dt,
	  valarray<T>& env_l,  // return interpolated
	  valarray<T>& env_u,
	  // optionally also return vector of points
	  vector<size_t> *envv_lp = NULL,
	  vector<size_t> *envv_up = NULL)
{
	size_t	i, j,
		n_samples = in.size();

	vector<size_t>
		envv_l,
		envv_u;

	for ( i = dh; i < n_samples-dh; ++i ) {
		for ( j = 1; j <= dh; ++j )
			if ( in[i-j] <= in[i] )  // [i] is not a local min
				goto inner_continue;
		for ( j = 1; j <= dh; ++j )
			if ( in[i+j] <= in[i] )  // [i] is not
				goto inner_continue;
		envv_l.push_back( i);
		continue;
	inner_continue:
		for ( j = 1; j <= dh; ++j )
			if ( in[i-j] >= in[i] )  // [i] is not a local max
				goto outer_continue;
		for ( j = 1; j <= dh; ++j )
			if ( in[i+j] >= in[i] )  // [i] is not
				goto outer_continue;
		envv_u.push_back( i);
	outer_continue:
		;
	}

	if ( envv_l.size() > 3 && envv_u.size() > 3 ) {
		env_l = interpolate( envv_l, samplerate, in, dt);
		env_u = interpolate( envv_u, samplerate, in, dt);
		if ( envv_lp )
			(*envv_lp) = envv_l;
		if ( envv_up )
			(*envv_up) = envv_u;
		return envv_u.size();
	} else
		return 0;
}







template <class T>
inline int
sign( const T& v)
{
	return v >= 0. ? 1 : -1;
}



template <class T>
valarray<float>
dzcdf( const valarray<T>& in,
       size_t samplerate,
       float dt,
       float sigma,
       size_t smooth)
{
	size_t i;

	valarray<T> in2 (0., in.size()),
		derivative (0., in.size());

      // smooth
	if ( smooth > 0 ) {
		for ( i = 0; i < smooth; ++i ) {
			in2[i] = in[i];
			in2[in.size()-i-1] = in[in.size()-i-1];
		}
		for ( i = smooth; i < in.size()-smooth; ++i ) {
			for ( size_t j = i - smooth; j <= i + smooth; ++j )
				in2[i] += in[j];
			in2[i] /= (smooth*2+1);
		}
	} else
		in2 = in;

      // get derivative
	for ( i = 1; i < in.size(); ++i )
		derivative[i-1] = in2[i] - in2[i-1];

      // collect zerocrossings
	vector<float> zerocrossings;
	for ( i = 1; i < in.size(); ++i )
		if ( sign( derivative[i-1]) != sign( derivative[i]) )
//			++out[lroundf( (float)i/samplerate/dt)];
			zerocrossings.push_back( (float)i/samplerate);
//	printf( "%zu zerocrossings %g/sec\n", zerocrossings.size(), zerocrossings.size()/((float)in.size()/samplerate));

      // prepare structures for interpolation
	size_t out_size = (float)in.size()/samplerate/dt;
	vector<size_t> xi (out_size);
	valarray<float> y (in.size());

      // calculate the bloody zcdf
	float	window = .5;
	float	t = 0., tdiff;
	size_t	I = 0, J;
	for ( i = 0; i < out_size; ++i ) {
		xi[i] = i * (dt * samplerate);
		for ( J = I; J > 0; --J ) {
			tdiff = zerocrossings[J] - t;
			if ( tdiff >  window/2. ) {
				continue;
			}
			if ( tdiff < -window/2. )
				break;
			y[ xi[i] ] += exp( -(tdiff*tdiff)/(sigma * sigma));
		}
		for ( J = I+1; J < zerocrossings.size(); ++J ) {
			tdiff = zerocrossings[J] - t;
			if ( tdiff < -window/2. ) {
				continue;
			}
			if ( tdiff >  window/2. )
				break;
			y[ xi[i] ] += exp( -(tdiff*tdiff)/(sigma * sigma));
		}
//		printf("J = %zu\n", J);
//		printf("t = %g [%zu] %g  J = %zu\n", t, xi[i], y[xi[i]], J);
		t += dt;
		I = J;
	}
	return interpolate( xi, samplerate, y, 1./samplerate);
}







template <class T>
class CPattern {
    private:
	CPattern();

    public:
	size_t	context_before,
		context_after,
		samplerate;

      // the complete pattern signature is made of:
      // (a) course of the mean (low-freq component);
      // (b) instantaneous frequency at fine intervals;
      // (c) signal breadth at given tightness.

      // data for individual constituents of the pattern:
        // Butterworth low-pass filter
	size_t	bwf_order;
	float	bwf_cutoff;
	bool	bwf_scale;

        // ZC density function fields
	float 	dzcdf_step,
		dzcdf_sigma;
	size_t	dzcdf_smooth;

        // envelope
	size_t	env_tightness;

	valarray<T>
		course,
		breadth;
	valarray<float>
		dzcd;

	float	a, b, c;

	size_t size_with_context() const
		{
			return course.size();
		}
	size_t size_essential() const
		{
			return size_with_context() - context_before - context_after;
		}

	CPattern( const valarray<T>& pattern,
		  size_t _context_before, size_t _context_after,
		  size_t _samplerate,
		  size_t _order, float _cutoff, bool _scale,
		  size_t _tightness,
		  float _step, float _sigma, size_t _smooth,
		  float _a, float _b, float _c)
	      : context_before (_context_before), context_after (_context_after),
		samplerate (_samplerate),
		bwf_order (_order), bwf_cutoff (_cutoff), bwf_scale (_scale),
		dzcdf_step (_step), dzcdf_sigma (_sigma), dzcdf_smooth (_smooth),
		env_tightness (_tightness),
		a (_a), b (_b), c (_c),
		match_a (NAN), match_b (NAN), match_c (NAN)
		{
			if ( context_before + context_after >= pattern.size() )
				throw invalid_argument ("pattern.size too small");
			course = NExstrom::low_pass( pattern, samplerate,
						     bwf_cutoff, bwf_order, bwf_scale);

			valarray<T> env_u, env_l;
			envelope( pattern, env_tightness, samplerate,
				  1./samplerate,
				  env_l, env_u);
			breadth.resize( env_u.size());
			breadth = env_u - env_l;

			dzcd = dzcdf( pattern, samplerate,
				      dzcdf_step, dzcdf_sigma, dzcdf_smooth);
		}
	size_t find( const valarray<T>&    course,
		     const valarray<T>&   breadth,
		     const valarray<float>& dzcdf,
		     ssize_t start,
		     int inc);
	size_t find( const valarray<T>& signal,
		     ssize_t start,
		     int inc);
	// resulting
	float	match_a,
		match_b,
		match_c;
};



template <class T>
size_t
CPattern<T>::find( const valarray<T>&     fcourse,
		   const valarray<T>&     fbreadth,
		   const valarray<float>& fdzcd,
		   ssize_t start,
		   int inc)
{
	if ( inc == 0 || inc > (ssize_t)fcourse.size() ) {
		fprintf( stderr, "CSignalPattern::find(): bad search increment: %d\n", inc);
		return (size_t)-1;
	}

	T	diff_course,
		diff_breadth,
		diff_dzcd;

	// printf( "course.size = %zu, fcourse.size = %zu, start = %zu\n",
	// 	course.size(), fcourse.size(), start);
	ssize_t	iz = (inc > 0) ? fcourse.size() - size_with_context() : 0;
	size_t	essential_part = size_essential();
	// bool	looking_further = false;
	// T	ax, bx, cx;
	for ( ssize_t i = start; (inc > 0) ? i < iz : i > iz; i += inc ) {
		diff_course = diff_breadth = diff_dzcd = 0.;
		for ( size_t j = 0; j < essential_part; ++j ) {
			diff_course  += fdim( course [context_before + j], fcourse [i+j]);
			diff_breadth += fdim( breadth[context_before + j], fbreadth[i+j]);
			diff_dzcd    += fdim( dzcd   [context_before + j], fdzcd   [i+j]);
		}

		diff_course  /= essential_part;
		diff_breadth /= essential_part;
		diff_dzcd    /= essential_part;

		// if ( i % 250 == 0 ) printf( "at %zu diff_course = %g,\tdiff_breadth = %g\t diff_dzcdf = %g\n", i, diff_course, diff_breadth, diff_dzcd);
		if ( diff_course < a && diff_breadth < b && diff_dzcd < c ) {
			// if ( !looking_further ) {
			// 	looking_further = true;
			match_a = diff_course, match_b = diff_breadth, match_c = diff_dzcd;
			return i;
		}
	}

	return (size_t)-1;
}


template <class T>
size_t
CPattern<T>::find( const valarray<T>& signal,
		   ssize_t start,
		   int inc)
{
      // low-pass signal being searched, too
	valarray<float> fcourse =
		NExstrom::low_pass( signal, samplerate,
				    bwf_cutoff, bwf_order, bwf_scale);

      // prepare for comparison by other criteria:
	// signal envelope and breadth
	valarray<T> env_u, env_l;
	envelope( signal, env_tightness, samplerate,
			 1./samplerate, env_u, env_l);
	valarray<T> fbreadth (env_u.size());
	fbreadth = env_u - env_l;

	// dzcdf
	valarray<float> fdzcd =
		dzcdf( signal, samplerate,
		       dzcdf_step, dzcdf_sigma, dzcdf_smooth);

	return find( fcourse, fbreadth, fdzcd,
		     start, inc);
}





template <class T>
inline double
sig_diff( const valarray<T>& a, const valarray<T>& b,
	  int d)
{
	double diff = 0.;
	if ( d > 0 )
		for ( size_t i =  d; i < a.size(); ++i )
			diff += fdim( a[i - d], b[i]);
	else
		for ( size_t i = -d; i < a.size(); ++i )
			diff += fdim( a[i], b[i + d]);
	return diff;
}

template <class T>
double
phase_diff( const valarray<T>& sig1,
	    const valarray<T>& sig2,
	    size_t samplerate,
	    size_t sa, size_t sz,
	    float fa, float fz,
	    unsigned order,
	    size_t scope)
{
	if ( order == 0 )
		throw invalid_argument ("NExstrom::phase_diff(): order == 0");
      // bandpass sig1 and sig2
	valarray<T>
		sig1p = NExstrom::band_pass( valarray<T> (&sig1[sa], sz - sa), samplerate, fa, fz, order, true),
		sig2p = NExstrom::band_pass( valarray<T> (&sig2[sa], sz - sa), samplerate, fa, fz, order, true);

      // slide one against the other a little
	double	diff = INFINITY, old_diff, diff_min = INFINITY;
	int	dist, dist_min = 0;
	// go east
	dist = 0;
	do {
		old_diff = diff;
		if ( (diff = sig_diff( sig1p, sig2p, dist)) < diff_min )
			diff_min = diff, dist_min = dist;
	} while ( -(dist--) < (int)scope && old_diff > diff );  // proceed until the first minimum
	// and west
	dist = 0, old_diff = INFINITY;
	do {
		old_diff = diff;
		if ( (diff = sig_diff( sig1p, sig2p, dist)) < diff_min )
			diff_min = diff, dist_min = dist;
	} while (  (dist++) < (int)scope && old_diff > diff );

	return (double)dist_min / samplerate;
}


}

#endif

// eof
