// ;-*-C++-*- *  Time-stamp: "2011-01-16 02:14:27 hmmr"
/*
 *       File name:  signal.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-12-26
 *
 *         Purpose:  various standalone signal processing functions
 *
 *         License:  GPL
 */

#ifndef _AGH_SIGNAL_H
#define _AGH_SIGNAL_H

#include <vector>
#include <valarray>
#include <stdexcept>

#if HAVE_CONFIG_H
#  include "config.h"
#endif

using namespace std;

namespace NSignal {

// the following exogenous functions are those found in liir.c from exstrom.com
// code was C++ified by me;
// original blurb from liir.c is:
/*
 *                            COPYRIGHT
 *
 *  liir - Recursive digital filter functions
 *  Copyright (C) 2007 Exstrom Laboratories LLC
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  A copy of the GNU General Public License is available on the internet at:
 *
 *  http://www.gnu.org/copyleft/gpl.html
 *
 *  or you can write to:
 *
 *  The Free Software Foundation, Inc.
 *  675 Mass Ave
 *  Cambridge, MA 02139, USA
 *
 *  You can contact Exstrom Laboratories LLC via Email at:
 *
 *  stefan(AT)exstrom.com
 *
 *  or you can write to:
 *
 *  Exstrom Laboratories LLC
 *  P.O. Box 7651
 *  Longmont, CO 80501, USA
 *
 */

// --- end blurb

/**********************************************************************
  binomial_mult - multiplies a series of binomials together and returns
  the coefficients of the resulting polynomial.

  The multiplication has the following form:

  (x+p[0])*(x+p[1])*...*(x+p[n-1])

  The p[i] coefficients are assumed to be complex and are passed to the
  function as a pointer to an array of doubles of length 2n.

  The resulting polynomial has the following form:

  x^n + a[0]*x^n-1 + a[1]*x^n-2 + ... +a[n-2]*x + a[n-1]

  The a[i] coefficients can in general be complex but should in most
  cases turn out to be real. The a[i] coefficients are returned by the
  function as a pointer to an array of doubles of length 2n. Storage
  for the array is allocated by the function and should be freed by the
  calling program when no longer needed.

  Function arguments:

  n  -  The number of binomials to multiply
  p  -  Pointer to an array of doubles where p[2i] (i=0...n-1) is
	assumed to be the real part of the coefficient of the ith binomial
	and p[2i+1] is assumed to be the imaginary part. The overall size
	of the array is then 2n.
*/

template <class T>
valarray<T>
binomial_mult( int n, const valarray<T>& p)
{
    int i, j;
    valarray<T> a (2*n);

    for( i = 0; i < n; ++i ) {
	for( j = i; j > 0; --j ) {
	    a[2*j  ] += p[2*i] * a[2*(j-1)  ] - p[2*i+1] * a[2*(j-1)+1];
	    a[2*j+1] += p[2*i] * a[2*(j-1)+1] + p[2*i+1] * a[2*(j-1)  ];
	}
	a[0] += p[2*i  ];
	a[1] += p[2*i+1];
    }
    return  a;
}



/**********************************************************************
  dcof_bwlp - calculates the d coefficients for a butterworth lowpass
  filter. The coefficients are returned as an array of doubles.

*/

template <class T>
valarray<T>
dcof_bwlp( int n, T fcf)
{
	int k;            // loop variables
	T theta;     // M_PI * fcf / 2.0
	T st;        // sine of theta
	T ct;        // cosine of theta
	T parg;      // pole angle
	T sparg;     // sine of the pole angle
	T cparg;     // cosine of the pole angle
	T a;         // workspace variable
	valarray<T>
		rcof (2*n), // binomial coefficients
		dcof;	    // dk coefficients

	theta = M_PI * fcf;
	st = sin(theta);
	ct = cos(theta);

	for ( k = 0; k < n; ++k ) {
		parg = M_PI * (T)(2*k+1)/(T)(2*n);
		sparg = sin(parg);
		cparg = cos(parg);
		a = 1.0 + st*sparg;
		rcof[2*k] = -ct/a;
		rcof[2*k+1] = -st*cparg/a;
	}

	dcof = binomial_mult( n, rcof);

	dcof[1] = dcof[0];
	dcof[0] = 1.0;
	for ( k = 3; k <= n; ++k )
		dcof[k] = dcof[2*k-2];

	return dcof;
}


/**********************************************************************
  dcof_bwhp - calculates the d coefficients for a butterworth highpass
  filter. The coefficients are returned as an array of doubles.

*/
template <class T>
inline valarray<T>
dcof_bwhp( int n, T fcf)
{
	return dcof_bwlp( n, fcf);
}


/**********************************************************************
  ccof_bwlp - calculates the c coefficients for a butterworth lowpass
  filter. The coefficients are returned as an array of integers.

*/

inline
valarray<int>
ccof_bwlp( unsigned n)
{
	int m, i;

	valarray<int> ccof (n+1);

	ccof[0] = 1;
	ccof[1] = n;
	m = n/2;
	for ( i = 2; i <= m; ++i ) {
		ccof[  i] = (n-i+1)*ccof[i-1]/i;
		ccof[n-i] = ccof[i];
	}
	ccof[n-1] = n;
	ccof[n] = 1;

	return ccof;
}


/**********************************************************************
  ccof_bwhp - calculates the c coefficients for a butterworth highpass
  filter. The coefficients are returned as an array of integers.

*/

inline
valarray<int>
ccof_bwhp( unsigned n)
{
	valarray<int> ccof = ccof_bwlp( n);
	for ( unsigned i = 0; i <= n; ++i )
		if ( i % 2 )
			ccof[i] = -ccof[i];

	return ccof;
}



/**********************************************************************
  sf_bwlp - calculates the scaling factor for a butterworth lowpass filter.
  The scaling factor is what the c coefficients must be multiplied by so
  that the filter response has a maximum value of 1.

*/

template <class T>
T
sf_bwlp( int n, T fcf)
{
	int m, k;    // loop variables
	T omega;     // M_PI * fcf
	T fomega;    // function of omega
	T parg0;     // zeroth pole angle
	T sf;        // scaling factor

	omega = M_PI * fcf;
	fomega = sin(omega);
	parg0 = M_PI / (T)(2*n);

	m = n / 2;
	sf = 1.0;
	for( k = 0; k < n/2; ++k )
		sf *= 1.0 + fomega * sin((T)(2*k+1)*parg0);

	fomega = sin(omega / 2.0);

	if( n % 2 )
		sf *= fomega + cos(omega / 2.0);
	sf = pow( fomega, n) / sf;

	return sf;
}




/**********************************************************************
  sf_bwhp - calculates the scaling factor for a butterworth highpass filter.
  The scaling factor is what the c coefficients must be multiplied by so
  that the filter response has a maximum value of 1.

*/

template <class T>
T
sf_bwhp( int n, T fcf)
{
	int m, k;    // loop variables
	T omega;     // M_PI * fcf
	T fomega;    // function of omega
	T parg0;     // zeroth pole angle
	T sf;        // scaling factor

	omega = M_PI * fcf;
	fomega = sin(omega);
	parg0 = M_PI / (T)(2*n);

	m = n / 2;
	sf = 1.0;
	for( k = 0; k < n/2; ++k )
		sf *= 1.0 + fomega * sin((T)(2*k+1)*parg0);

	fomega = cos(omega / 2.0);

	if( n % 2 )
		sf *= fomega + cos(omega / 2.0);
	sf = pow( fomega, n) / sf;

	return sf;
}



// --------------- end exstrom.com functions






template <class T>
valarray<T>
low_pass( const valarray<T>& in,
	  size_t samplerate,
	  size_t order, float cutoff, bool scale)
{
	T	fcf = 2. * cutoff/samplerate;
	valarray<T>
		dcof = dcof_bwlp( order, fcf);		/* the d coefficients */
	valarray<int>
		ccof_ = ccof_bwlp( order);		/* the c coefficients */
	valarray<T>
		ccof (ccof_.size());
	if ( scale )
		for ( size_t i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i] * sf_bwlp( order, fcf);
	else
		for ( size_t i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i];

	size_t	i, j,
		nc = order+1,
		nd = order+1,
		in_size = in.size(),
		out_size = in_size + nc;
	valarray<T> out (out_size);

	for( i = 0; i < out_size; ++i ) {
		T s1 = 0., s2 = 0.;
		for( j = (i < nd ? 0 : i - nd + 1); j < i; ++j )
			s1 += dcof[i-j] * out[j];

		for( j = (i < nc ? 0 : i - nc + 1); j <= (i < in_size ? i : in_size - 1); ++j )
			s2 += ccof[i-j] * in[j];

		out[i] = s2 - s1;
	}

	return out;
}


template <class T>
valarray<T>
high_pass( const valarray<T>& in,
	   size_t samplerate,
	   size_t order, float cutoff, bool scale)
{
	T	fcf = 2. * cutoff/samplerate;
	valarray<T>
		dcof = dcof_bwhp( order, fcf);		/* the d coefficients */
	valarray<int>
		ccof_ = ccof_bwhp( order);		/* the c coefficients */
	valarray<T>
		ccof (ccof_.size());
	if ( scale )
		for ( size_t i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i] * sf_bwhp( order, fcf);
	else
		for ( size_t i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i];

	size_t	i, j,
		nc = order+1,
		nd = order+1,
		in_size = in.size(),
		out_size = in_size + nc;
	valarray<T> out (out_size);

	for( i = 0; i < out_size; ++i ) {
		T s1 = 0., s2 = 0.;
		for( j = (i < nd ? 0 : i - nd + 1); j < i; ++j )
			s1 += dcof[i-j] * out[j];

		for( j = (i < nc ? 0 : i - nc + 1); j <= (i < in_size ? i : in_size - 1); ++j )
			s2 += ccof[i-j] * in[j];

		out[i] = s2 - s1;
	}

	return out;
}



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
signal_envelope( const valarray<T>& in,
		 size_t dh,  // tightness
		 size_t samplerate,
		 double dt,
		 valarray<T>& env_l,  // return interpolated
		 valarray<T>& env_u,
		 // optionally also return vector of points
		 vector<size_t> *envv_lp = NULL,
		 vector<size_t> *envv_up = NULL)
{
	assert (dh > 0);
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
signal_dzcdf( const valarray<T>& in,
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

      // calculate the bloody zdf
	float	window = .5;
	float	t = 0., tdiff;
	size_t	I = 0, J;
	for ( i = 0; i < out_size; ++i ) {
		xi[i] = i * (dt * samplerate);
//		printf( "%6zu<: ", i);
		for ( J = I; J > 0; --J ) {
			tdiff = zerocrossings[J] - t;
			if ( tdiff >  window/2. ) {
//				printf("-");
				continue;
			}
			if ( tdiff < -window/2. )
				break;
			y[ xi[i] ] += exp( -(tdiff*tdiff)/(sigma * sigma));
		}
//		printf("  >");
		for ( J = I+1; J < zerocrossings.size(); ++J ) {
			tdiff = zerocrossings[J] - t;
			if ( tdiff < -window/2. ) {
//				printf("+");
				continue;
			}
			if ( tdiff >  window/2. )
				break;
			y[ xi[i] ] += exp( -(tdiff*tdiff)/(sigma * sigma));
//			printf(":");
		}
//		printf("J = %zu\n", J);
//		printf("t = %g [%zu] %g  J = %zu\n", t, xi[i], y[xi[i]], J);
		t += dt;
		I = J;
	}
	return interpolate( xi, samplerate, y, 1./samplerate);
}







template <class T>
class CSignalPattern {
    private:
	CSignalPattern();

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
		dzcdf;

	float	a, b, c;

	size_t size_with_context() const
		{
			return course.size();
		}
	size_t size_essential() const
		{
			return size_with_context() - context_before - context_after;
		}

	CSignalPattern( const valarray<T>& pattern,
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
			course = low_pass( pattern, samplerate,
					   bwf_order, bwf_cutoff, bwf_scale);

			valarray<T> env_u, env_l;
			signal_envelope( pattern, env_tightness, samplerate,
					 1./samplerate,
					 env_l, env_u);
			breadth.resize( env_u.size());
			breadth = env_u - env_l;

			dzcdf = signal_dzcdf( pattern, samplerate,
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
CSignalPattern<T>::find( const valarray<T>&     fcourse,
			 const valarray<T>&     fbreadth,
			 const valarray<float>& fdzcdf,
			 ssize_t start,
			 int inc)
{
	if ( inc == 0 || inc > (ssize_t)fcourse.size() ) {
		fprintf( stderr, "CSignalPattern::find(): bad search increment: %d\n", inc);
		return (size_t)-1;
	}

	T	diff_course,
		diff_breadth,
		diff_dzcdf;

//	printf( "course.size = %zu, fcourse.size = %zu, start = %zu\n",
//		course.size(), fcourse.size(), start);
	ssize_t	iz = (inc > 0) ? fcourse.size() - size_with_context() : 0;
	size_t	essential_part = size_essential();
	for ( ssize_t i = start; (inc > 0) ? i < iz : i > iz; i += inc ) {
		diff_course = diff_breadth = diff_dzcdf = 0.;
		for ( size_t j = 0; j < essential_part; ++j ) {
			diff_course  += fabs( course [context_before + j] - fcourse [i+j]);
			diff_breadth += fabs( breadth[context_before + j] - fbreadth[i+j]);
			diff_dzcdf   += fabs( dzcdf  [context_before + j] - fdzcdf  [i+j]);
		}

		diff_course  /= essential_part;
		diff_breadth /= essential_part;
		diff_dzcdf   /= essential_part;

//		printf( "at %zu diff_course = %g,\tdiff_breadth = %g\t diff_dzcdf = %g\n", i, diff_course, diff_breadth, diff_dzcdf);
		if ( diff_course < a && diff_breadth < b && diff_dzcdf < c ) {
			match_a = diff_course, match_b = diff_breadth, match_c = diff_dzcdf;
			return i;
		}
	}

	return (size_t)-1;
}


template <class T>
size_t
CSignalPattern<T>::find( const valarray<T>& signal,
			 ssize_t start,
			 int inc)
{
      // low-pass signal being searched, too
	valarray<float> fcourse =
		low_pass( signal, samplerate,
			  bwf_order, bwf_cutoff, bwf_scale);

      // prepare for comparison by other criteria:
	// signal envelope and breadth
	valarray<T> env_u, env_l;
	signal_envelope( signal, env_tightness, samplerate,
			 1./samplerate, env_u, env_l);
	valarray<T> fbreadth (env_u.size());
	fbreadth = env_u - env_l;

	// dzcdf
	valarray<float> fdzcdf =
		signal_dzcdf( signal, samplerate,
			      dzcdf_step, dzcdf_sigma, dzcdf_smooth);

	return find( course, breadth, dzcdf,
		     start, inc);
}


}  // namespace NSignal

#endif

// eof
