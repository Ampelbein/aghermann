// ;-*-C++-*- *  Time-stamp: "2010-12-29 03:16:22 hmmr"
/*
 *       File name:  signal.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-12-26
 *
 *         Purpose:  various standalone signal processing functions
 *
 *         License:  GPL
 */

#include "signal.hh"


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
  ccof_bwlp - calculates the c coefficients for a butterworth lowpass
  filter. The coefficients are returned as an array of integers.

*/

valarray<int>
ccof_bwlp( int n)
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
  sf_bwlp - calculates the scaling factor for a butterworth lowpass filter.
  The scaling factor is what the c coefficients must be multiplied by so
  that the filter response has a maximum value of 1.

*/

template <class T>
T
sf_bwlp( int n, T fcf)
{
	int m, k;         // loop variables
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

	if( n % 2 ) sf *= fomega + cos(omega / 2.0);
	sf = pow( fomega, n ) / sf;

	return sf;
}

// --------------- end exstrom.com functions



int
low_pass( const valarray<float>& signal,
	  size_t samplerate,
	  size_t order, float cutoff, bool scale,
	  valarray<float>& out)
{
	float	fcf = 2 * cutoff/samplerate;
	valarray<float>
		dcof = dcof_bwlp( order, fcf);		/* the d coefficients */
	valarray<int>
		ccof = ccof_bwlp( order);		/* the c coefficients */
	if ( scale )
		ccof *= sf_bwlp( order, fcf);

	size_t	signal_size = signal.size(),
		out_size = order+1 + signal_size;
	out.resize( out_size);
	out = 0.;

	size_t i, j;
	for( i = 0; i < out_size; ++i ) {
		float s1 = 0., s2 = 0.;
		for( j = (i < order ? 0 : i - order + 1); j < i; ++j )
			s1 += dcof[i-j] * out[j];

		for( j = (i < order ? 0 : i - order + 1); j <= (i < signal_size ? i : signal_size - 1); ++j )
			s2 += ccof[i-j] * signal[j];

		out[i] = s2 - s1;
	}

	return 0;
}










size_t
signal_envelope( const valarray<float>& filtered,
		 vector<size_t>& env_l,
		 vector<size_t>& env_u,
		 size_t over)
{
	size_t	i, j, dh = (over-1)/2+1,
		n_samples = filtered.size();

	env_l.resize( 0);
	env_u.resize( 0);

	for ( i = dh; i < n_samples-dh; ++i ) {
		for ( j = 1; j <= dh; ++j )
			if ( filtered[i-j] <= filtered[i] )  // [i] is not a local min
				goto inner_continue;
		for ( j = 1; j <= dh; ++j )
			if ( filtered[i+j] <= filtered[i] )  // [i] is not
				goto inner_continue;
		env_l.push_back( i);
		continue;
	inner_continue:
		for ( j = 1; j <= dh; ++j )
			if ( filtered[i-j] >= filtered[i] )  // [i] is not a local max
				goto outer_continue;
		for ( j = 1; j <= dh; ++j )
			if ( filtered[i+j] >= filtered[i] )  // [i] is not
				goto outer_continue;
		env_u.push_back( i);
	outer_continue:
		;
	}

	return env_u.size();
}


size_t
signal_breadth( const valarray<float>& signal,
		const vector<size_t>& env_u,
		const vector<size_t>& env_l,
		valarray<float>& sig_breadth)
{
	size_t	ia = max( *env_u. begin(), *env_l. begin()),
		iz = min( *env_u.rbegin(), *env_l.rbegin());
	sig_breadth.resize( signal.size());

	auto Iu = env_u.begin(), Il = env_l.begin();
	for ( size_t i = ia; i < iz; ++i ) {
		float	frac_u = (float)(i - *Iu)/(*next(Iu) - *Iu),
			frac_l = (float)(i - *Il)/(*next(Il) - *Il);
		float	dyu = signal[*next(Iu)] - signal[*Iu],
			dyl = signal[*next(Il)] - signal[*Il];
		sig_breadth[i] =
			 (signal[*Iu] + frac_u * dyu) +
			-(signal[*Il] + frac_l * dyl);
		if ( i == *Iu )
			++Iu;
		if ( i == *Il )
			++Il;
	}

	return iz - ia;
}






size_t
find_pattern( const CSignalPattern<float>& pattern,
	      valarray<float>& signal,
	      size_t start,
	      float tolerance)
{
      // low-pass signal being searched, too
	valarray<float> course;
	low_pass( signal, pattern.samplerate,
		  pattern.bwf_order, pattern.bwf_cutoff, pattern.bwf_scale,
		  course);

      // prepare for comparison by other criteria:
	// signal breadth
	vector<size_t> env_u, env_l;
	signal_envelope( signal, env_u, env_l, pattern.env_tightness);

	// instantaneous rate
	;

	size_t diff;
	size_t iz = signal.size() - pattern.size();
	for ( size_t i = 0; i < iz; ++i ) {
		diff = 0;
		for ( size_t j = 0; j < pattern.size(); ++j )
			diff += fabs( pattern.course[j] - course[i+j]);
		float likeness = (float)diff / pattern.size();
	}

	return 0;
}

// eof
