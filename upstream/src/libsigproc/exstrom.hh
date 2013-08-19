/*
 *       File name:  sigproc/exstrom.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-12-26
 *
 *         Purpose:  various signal processing functions
 *                   borrowed from exstrom.com
 *
 *         License:  GPL
 */

// The following exogenous functions are those found in liir.c from exstrom.com.
// Code was C++ified by me.
// Original blurb from liir.c goes:
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

// In this file, I preserved individual comments by exstrom.com
// developers in their original form.

#ifndef AGH_LIBSIGPROC_EXSTROM_H_
#define AGH_LIBSIGPROC_EXSTROM_H_

#include <valarray>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace exstrom {

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

template <typename T>
valarray<T>
binomial_mult( unsigned n, const valarray<T>& p)
{
	valarray<T> a (2*n);

	unsigned i, j;
	for ( i = 0; i < n; ++i ) {
		for ( j = i; j > 0; --j ) {
			a[2*j  ] += p[2*i] * a[2*(j-1)  ] - p[2*i+1] * a[2*(j-1)+1];
			a[2*j+1] += p[2*i] * a[2*(j-1)+1] + p[2*i+1] * a[2*(j-1)  ];
		}
		a[0] += p[2*i  ];
		a[1] += p[2*i+1];
	}
	return  a;
}



/**********************************************************************
  trinomial_mult - multiplies a series of trinomials together and returns
  the coefficients of the resulting polynomial.

  The multiplication has the following form:

  (x^2 + b[0]x + c[0])*(x^2 + b[1]x + c[1])*...*(x^2 + b[n-1]x + c[n-1])

  The b[i] and c[i] coefficients are assumed to be complex and are passed
  to the function as a pointers to arrays of doubles of length 2n. The real
  part of the coefficients are stored in the even numbered elements of the
  array and the imaginary parts are stored in the odd numbered elements.

  The resulting polynomial has the following form:

  x^2n + a[0]*x^2n-1 + a[1]*x^2n-2 + ... +a[2n-2]*x + a[2n-1]

  The a[i] coefficients can in general be complex but should in most cases
  turn out to be real. The a[i] coefficients are returned by the function as
  a pointer to an array of doubles of length 4n. The real and imaginary
  parts are stored, respectively, in the even and odd elements of the array.
  Storage for the array is allocated by the function and should be freed by
  the calling program when no longer needed.

  Function arguments:

  n  -  The number of trinomials to multiply
  b  -  Pointer to an array of doubles of length 2n.
  c  -  Pointer to an array of doubles of length 2n.
*/

template <typename T>
valarray<T>
trinomial_mult( unsigned n, const valarray<T>& b, const valarray<T>& c)
{
	valarray<T> a (4*n);

	a[2] = c[0];
	a[3] = c[1];
	a[0] = b[0];
	a[1] = b[1];

	unsigned i, j;

	for( i = 1; i < n; ++i ) {
		a[2*(2*i+1)]   += c[2*i]*a[2*(2*i-1)]   - c[2*i+1]*a[2*(2*i-1)+1];
		a[2*(2*i+1)+1] += c[2*i]*a[2*(2*i-1)+1] + c[2*i+1]*a[2*(2*i-1)  ];

		for ( j = 2*i; j > 1; --j ) {
			a[2*j]   += b[2*i] * a[2*(j-1)]   - b[2*i+1] * a[2*(j-1)+1] +
				c[2*i] * a[2*(j-2)]   - c[2*i+1] * a[2*(j-2)+1];
			a[2*j+1] += b[2*i] * a[2*(j-1)+1] + b[2*i+1] * a[2*(j-1)] +
				c[2*i] * a[2*(j-2)+1] + c[2*i+1] * a[2*(j-2)  ];
		}

		a[2] += b[2*i] * a[0] - b[2*i+1] * a[1] + c[2*i  ];
		a[3] += b[2*i] * a[1] + b[2*i+1] * a[0] + c[2*i+1];
		a[0] += b[2*i  ];
		a[1] += b[2*i+1];
	}

	return a;
}

/**********************************************************************
  dcof_bwlp - calculates the d coefficients for a butterworth lowpass
  filter. The coefficients are returned as an array of doubles.

*/

template <typename T>
valarray<T>
dcof_bwlp( unsigned n, T fcf)
{
	T	theta,     // M_PI * fcf / 2.0
		st,        // sine of theta
		ct,        // cosine of theta
		parg,      // pole angle
		sparg,     // sine of the pole angle
		cparg,     // cosine of the pole angle
		a;         // workspace variable
	valarray<T>
		rcof (2*n), // binomial coefficients
		dcof;	    // dk coefficients

	theta = M_PI * fcf;
	st = sin(theta);
	ct = cos(theta);

	unsigned k;
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
template <typename T>
valarray<T>
dcof_bwhp( unsigned n, T fcf)
{
	return dcof_bwlp( n, fcf);
}


/**********************************************************************
  dcof_bwbp - calculates the d coefficients for a butterworth bandpass
  filter. The coefficients are returned as an array of doubles.

*/

template <typename T>
valarray<T>
dcof_bwbp( unsigned n, T f1f, T f2f)
{
	T	theta,     // M_PI * (f2f - f1f) / 2.0
		cp,        // cosine of phi
		st,        // sine of theta
		ct,        // cosine of theta
		s2t,       // sine of 2*theta
		c2t,       // cosine 0f 2*theta
		parg,      // pole angle
		sparg,     // sine of pole angle
		cparg,     // cosine of pole angle
		a;         // workspace variables

	cp = cos(M_PI * (f2f + f1f) / 2.0);
	theta = M_PI * (f2f - f1f) / 2.0;
	st = sin(theta);
	ct = cos(theta);
	s2t = 2.0*st*ct;        // sine of 2*theta
	c2t = 2.0*ct*ct - 1.0;  // cosine 0f 2*theta

	valarray<T>
		rcof (2*n), // z^-2 coefficients
		tcof (2*n); // z^-1 coefficients

	unsigned k;
	for ( k = 0; k < n; ++k ) {
		parg = M_PI * (T)(2*k+1)/(T)(2*n);
		sparg = sin(parg);
		cparg = cos(parg);
		a = 1.0 + s2t*sparg;
		rcof[2*k] = c2t/a;
		rcof[2*k+1] = -s2t*cparg/a;
		tcof[2*k] = -2.0*cp*(ct+st*sparg)/a;
		tcof[2*k+1] = 2.0*cp*st*cparg/a;
	}

	valarray<T> dcof // dk coefficients
		= trinomial_mult( n, tcof, rcof);

	dcof[1] = dcof[0];
	dcof[0] = 1.0;
	for ( k = 3; k <= 2*n; ++k )
		dcof[k] = dcof[2*k-2];
	return dcof;
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
  ccof_bwbp - calculates the c coefficients for a butterworth bandpass
  filter. The coefficients are returned as an array of integers.

*/

inline
valarray<int>
ccof_bwbp( unsigned n)
{
	valarray<int>
		ccof (2*n + 1);
	valarray<int>
		tcof = ccof_bwhp( n);

	for ( size_t i = 0; i < n; ++i ) {
		ccof[2*i] = tcof[i];
		ccof[2*i+1] = 0.0;
	}
	ccof[2*n] = tcof[n];
	return ccof;
}


/**********************************************************************
  ccof_bwbs - calculates the c coefficients for a butterworth bandstop 
  filter. The coefficients are returned as an array of integers.

*/

template <typename T>
valarray<T>
ccof_bwbs( unsigned n, T f1f, T f2f)
{
	T alpha = -2.0 * cos( M_PI * (f2f + f1f) / 2.0) / cos(M_PI * (f2f - f1f) / 2.0);
	valarray<T> ccof (2*n + 1);

	ccof[0] = 1.0;

	ccof[2] = 1.0;
	ccof[1] = alpha;

	for( size_t i = 1; i < n; ++i ) {
		ccof[2*i + 2] += ccof[2*i];
		for( size_t j = 2*i; j > 1; --j )
			ccof[j+1] += alpha * ccof[j] + ccof[j-1];

		ccof[2] += alpha * ccof[1] + 1.0;
		ccof[1] += alpha;
	}

	return ccof;
}




/**********************************************************************
  sf_bwlp - calculates the scaling factor for a butterworth lowpass filter.
  The scaling factor is what the c coefficients must be multiplied by so
  that the filter response has a maximum value of 1.

*/

template <typename T>
T
sf_bwlp( unsigned n, T fcf)
{
	T	omega = M_PI * fcf,      // M_PI * fcf
		fomega = sin(omega),     // function of omega
		parg0 = M_PI / (T)(2*n), // zeroth pole angle
		sf;                      // scaling factor

	unsigned k;
	//m = n / 2;
	sf = 1.0;
	for( k = 0; k < n/2; ++k )
		sf *= 1.0 + fomega * sin((T)(2*k+1)*parg0);

	fomega = sin(omega / 2.0);

	if ( n % 2 )
		sf *= fomega + cos(omega / 2.0);
	sf = pow( fomega, n) / sf;

	return sf;
}




/**********************************************************************
  sf_bwhp - calculates the scaling factor for a butterworth highpass filter.
  The scaling factor is what the c coefficients must be multiplied by so
  that the filter response has a maximum value of 1.

*/

template <typename T>
T
sf_bwhp( unsigned n, T fcf)
{
	T	omega = M_PI * fcf,      // M_PI * fcf
		fomega = sin(omega),     // function of omega
		parg0 = M_PI / (T)(2*n), // zeroth pole angle
		sf;                      // scaling factor

	unsigned k;
	//m = n / 2;
	sf = 1.0;
	for ( k = 0; k < n/2; ++k )
		sf *= 1.0 + fomega * sin((T)(2*k+1)*parg0);

	fomega = cos(omega / 2.0);

	if ( n % 2 )
		sf *= fomega + cos(omega / 2.0);
	sf = pow( fomega, n) / sf;

	return sf;
}



/**********************************************************************
  sf_bwbp - calculates the scaling factor for a butterworth bandpass filter.
  The scaling factor is what the c coefficients must be multiplied by so
  that the filter response has a maximum value of 1.

*/

template <typename T>
T
__attribute__ ((const))
sf_bwbp( unsigned n, T f1f, T f2f )
{
	T	ctt = 1.0 / tan(M_PI * (f2f - f1f) / 2.0),       // cotangent of theta
		sfr = 1., sfi = 0.,  // real and imaginary parts of the scaling factor
		parg,      // pole angle
		sparg,     // sine of pole angle
		cparg,     // cosine of pole angle
		a, b, c;   // workspace variables

	for ( unsigned k = 0; k < n; ++k ) {
		parg = M_PI * (T)(2*k+1)/(T)(2*n);
		sparg = ctt + sin(parg);
		cparg = cos(parg);
		a = (sfr + sfi)*(sparg - cparg);
		b = sfr * sparg;
		c = -sfi * cparg;
		sfr = b - c;
		sfi = a - b - c;
	}

	return 1.0 / sfr;
}


template <typename T>
T
__attribute__ ((const))
sf_bwbs( unsigned n, T f1f, T f2f )
{
	T	tt  = tan(M_PI * (f2f - f1f) / 2.0),       // tangent of theta
		sfr = 1., sfi = 0.,  // real and imaginary parts of the scaling factor
		parg,      // pole angle
		sparg,     // sine of pole angle
		cparg,     // cosine of pole angle
		a, b, c;   // workspace variables

	for ( unsigned k = 0; k < n; ++k ) {
		parg = M_PI * (double)(2*k+1)/(double)(2*n);
		sparg = tt + sin(parg);
		cparg = cos(parg);
		a = (sfr + sfi)*(sparg - cparg);
		b = sfr * sparg;
		c = -sfi * cparg;
		sfr = b - c;
		sfi = a - b - c;
	}

	return 1.0 / sfr;
}


// --------------- end exstrom.com functions






template <typename T>
valarray<T>
low_pass( const valarray<T>& in,
	  size_t samplerate,
	  float cutoff, unsigned order, bool scale)
{
//	printf( "low_pass( %zu, %g, %u, %d)\n", samplerate, cutoff, order, scale);
	size_t	i, j;

	T	fcf = 2. * cutoff/samplerate;
	valarray<T>
		dcof = dcof_bwlp( order, fcf);		/* the d coefficients */
	valarray<int>
		ccof_ = ccof_bwlp( order);		/* the c coefficients */
	valarray<T>
		ccof (ccof_.size());
	if ( scale )
		for ( i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i] * sf_bwlp( order, fcf);
	else
		for ( i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i];

	unsigned
		nc = ccof.size(),
		nd = dcof.size();
	size_t	in_size = in.size(),
		out_size = in_size + nc;
	valarray<T> out (out_size);

	for ( i = 0; i < out_size; ++i ) {
		T s1 = 0., s2 = 0.;
		for( j = (i < nd ? 0 : i - nd + 1); j < i; ++j )
			s1 += dcof[i-j] * out[j];

		for( j = (i < nc ? 0 : i - nc + 1); j <= (i < in_size ? i : in_size - 1); ++j )
			s2 += ccof[i-j] * in[j];

		out[i] = s2 - s1;
	}

	return out;
}


template <typename T>
valarray<T>
high_pass( const valarray<T>& in,
	   size_t samplerate,
	   float cutoff, unsigned order, bool scale)
{
	size_t	i, j;

	T	fcf = 2. * cutoff/samplerate;
	valarray<T>
		dcof = dcof_bwhp( order, fcf);		/* the d coefficients */
	// printf( "dcof: ");
	// for ( i = 0; i < dcof.size(); ++i )
	// 	printf( " %g", dcof[i]);
	// printf( "\n");
	valarray<int>
		ccof_ = ccof_bwhp( order);		/* the c coefficients */
	valarray<T>
		ccof (ccof_.size());
	if ( scale )
		for ( i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i] * sf_bwhp( order, fcf);
	else
		for ( i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i];
	// printf( "ccof: ");
	// for ( i = 0; i < ccof.size(); ++i )
	// 	printf( " %g", ccof[i]);
	// printf( "\n");

	unsigned
		nc = ccof.size(),
		nd = dcof.size();
	size_t	in_size = in.size(),
		out_size = in_size + nc;
	valarray<T> out (out_size);

	for ( i = 0; i < out_size; ++i ) {
		T s1 = 0., s2 = 0.;
		for( j = (i < nd ? 0 : i - nd + 1); j < i; ++j )
			s1 += dcof[i-j] * out[j];

		for( j = (i < nc ? 0 : i - nc + 1); j <= (i < in_size ? i : in_size - 1); ++j )
			s2 += ccof[i-j] * in[j];

		out[i] = s2 - s1;
	}

	return out;
}




template <typename T>
valarray<T>
band_pass( const valarray<T>& in,
	   size_t samplerate,
	   float lo_cutoff, float hi_cutoff,
	   unsigned order, bool scale)
{
	size_t	i, j;

	T	f1f = 2. * lo_cutoff/samplerate,
		f2f = 2. * hi_cutoff/samplerate;
	valarray<T>
		dcof = dcof_bwbp( order, f1f, f2f);		/* the d coefficients */
	// printf( "dcof: ");
	// for ( i = 0; i < dcof.size(); ++i )
	// 	printf( " %g", dcof[i]);
	// printf( "\n");
	valarray<int>
		ccof_ = ccof_bwbp( order);	/* the c coefficients */
	valarray<T>
		ccof (ccof_.size());
	if ( scale ) {
		T sf = sf_bwbp( order, f1f, f2f);
		for ( i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i] * sf;
	} else
		for ( i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i];
	// printf( "ccof: ");
	// for ( i = 0; i < ccof.size(); ++i )
	// 	printf( " %g", ccof[i]);
	// printf( "\n");

	unsigned
		nc = ccof.size(),
		nd = dcof.size();
	size_t	in_size = in.size(),
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


template <typename T>
valarray<T>
band_stop( const valarray<T>& in,
	   size_t samplerate,
	   float lo_cutoff, float hi_cutoff,
	   unsigned order, bool scale)
{
	size_t	i, j;

	T	f1f = 2. * lo_cutoff/samplerate,
		f2f = 2. * hi_cutoff/samplerate;
	valarray<T>
		dcof = dcof_bwbp( order, f1f, f2f);		/* the d coefficients */
	// printf( "dcof: ");
	// for ( i = 0; i < dcof.size(); ++i )
	// 	printf( " %g", dcof[i]);
	// printf( "\n");
	valarray<T>
		ccof_ = ccof_bwbs<T>( order, f1f, f2f);	/* the c coefficients */
	valarray<T>
		ccof (ccof_.size());
	if ( scale ) {
		T sf = sf_bwbs( order, f1f, f2f);
		for ( i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i] * sf;
	} else
		for ( i = 0; i < ccof_.size(); ++i )
			ccof[i] = ccof_[i];
	// printf( "ccof: ");
	// for ( i = 0; i < ccof.size(); ++i )
	// 	printf( " %g", ccof[i]);
	// printf( "\n");

	unsigned
		nc = ccof.size(),
		nd = dcof.size();
	size_t	in_size = in.size(),
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


extern template valarray<TFloat> binomial_mult( unsigned, const valarray<TFloat>&);
extern template valarray<TFloat> trinomial_mult( unsigned, const valarray<TFloat>&, const valarray<TFloat>&);
extern template valarray<TFloat> dcof_bwlp( unsigned, TFloat);
extern template valarray<TFloat> dcof_bwbp( unsigned, TFloat, TFloat);
extern template valarray<TFloat> ccof_bwbs( unsigned, TFloat, TFloat);
extern template TFloat sf_bwlp( unsigned, TFloat);
extern template TFloat sf_bwhp( unsigned, TFloat);
extern template TFloat sf_bwbp( unsigned, TFloat, TFloat);
extern template TFloat sf_bwbs( unsigned, TFloat, TFloat);
extern template valarray<TFloat> low_pass( const valarray<TFloat>&, size_t, float, unsigned, bool);
extern template valarray<TFloat> high_pass( const valarray<TFloat>&, size_t, float, unsigned, bool);
extern template valarray<TFloat> band_pass( const valarray<TFloat>&, size_t, float, float, unsigned, bool);
extern template valarray<TFloat> band_stop( const valarray<TFloat>&, size_t, float, float, unsigned, bool);



}  // namespace exstrom

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
