// ;-*-C++-*-
/*
 *       File name:  libexstrom/sigproc.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-26
 *
 *         Purpose:  various standalone signal processing functions, class CPattern
 *
 *         License:  GPL
 */

#ifndef _SIGPROC_HH
#define _SIGPROC_HH

#include <cmath>
#include <vector>
#include <valarray>
#include <stdexcept>

#include "exstrom.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigproc {


template <typename T>
valarray<T>&
smooth( valarray<T>&, size_t side);


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
	     double dx);

template <>
inline valarray<double>
interpolate( const vector<size_t>& xi,
	     size_t samplerate,
	     const valarray<double>& y,
	     double dt)
{
	return interpolate_d( xi, samplerate, y, dt);
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
	  vector<size_t> *envv_lp = nullptr,
	  vector<size_t> *envv_up = nullptr);



template <class T>
inline int
sign( const T& v)
{
	return v >= 0. ? 1 : -1;
}



template <class T>
valarray<T>
dzcdf( const valarray<T>& in,
       size_t samplerate,
       float dt,
       float sigma,
       size_t smooth);


template <class T>
class CPattern {

	CPattern() = delete;

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
	valarray<T>
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
		  float _a, float _b, float _c);

	size_t find( const valarray<T>& course,
		     const valarray<T>& breadth,
		     const valarray<T>& dzcdf,
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
inline double
sig_diff( const valarray<T>& a, const valarray<T>& b, int d);




template <class T>
double
phase_diff( const valarray<T>& sig1,
	    const valarray<T>& sig2,
	    size_t samplerate,
	    size_t sa, size_t sz,
	    float fa, float fz,
	    unsigned order,
	    size_t scope);

} // namespace sigproc


#include "sigproc.ii"

#endif

// eof
