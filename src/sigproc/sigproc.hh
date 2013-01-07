// ;-*-C++-*-
/*
 *       File name:  sigproc/sigproc.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-26
 *
 *         Purpose:  various standalone signal processing functions, class CPattern
 *
 *         License:  GPL
 */

#ifndef _SIGPROC_SIGPROC_H
#define _SIGPROC_SIGPROC_H

#include <cmath>
#include <vector>
#include <valarray>
#include <stdexcept>

#include <gsl/gsl_math.h>
#include <samplerate.h>

#include "common/lang.hh"
#include "exstrom.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigproc {

template <typename T>
void
smooth( valarray<T>&, size_t side);

template <typename T>
void
normalize( valarray<T>&);

template <typename T>
valarray<T>
derivative( const valarray<T>&);


valarray<float>
resample_f( const valarray<float>&,
	    size_t, size_t, size_t, int);

inline valarray<float>
resample( const valarray<float>& signal,
	  size_t start, size_t end,
	  size_t to_size,
	  int alg = SRC_SINC_FASTEST)
{
	return resample_f( signal, start, end, to_size, alg);
}

valarray<double>
resample( const valarray<double>& signal,
	  size_t start, size_t end,
	  size_t to_size,
	  int alg);




valarray<double>
interpolate_d( const vector<size_t>&,
	       unsigned, const valarray<double>&, float);

valarray<float>
interpolate( const vector<size_t>& xi,
	     unsigned samplerate,
	     const valarray<float>& y,
	     float dx);

inline valarray<double>
interpolate( const vector<size_t>& xi,
	     size_t samplerate,
	     const valarray<double>& y,
	     float dx)
{
	return interpolate_d( xi, samplerate, y, dx);
}



template <typename T>
size_t
envelope( const valarray<T>& in,
	  size_t dh,  // tightness
	  size_t samplerate,
	  float dt,
	  valarray<T>* env_lp = nullptr,  // return interpolated
	  valarray<T>* env_up = nullptr,  // return vector of points
	  vector<size_t> *mini_p = nullptr,
	  vector<size_t> *maxi_p = nullptr);





template <typename T>
int
sign( const T& v)
{
	return v >= 0. ? 1 : -1;
}



template <typename T>
valarray<T>
dzcdf( const valarray<T>& in,
       size_t samplerate,
       float dt,
       float sigma,
       size_t smooth);


struct SPatternParamPack {
	int	bwf_order;
	double	bwf_cutoff;
	double 	dzcdf_step,
		dzcdf_sigma;
	int	dzcdf_smooth,
		env_tightness;
	bool operator==( const SPatternParamPack& rv) const // cannot be defaulted!
		{
			return	bwf_order == rv.bwf_order &&
				bwf_cutoff == rv.bwf_cutoff &&
				dzcdf_step == rv.dzcdf_step &&
				dzcdf_sigma == rv.dzcdf_sigma &&
				dzcdf_smooth == rv.dzcdf_smooth &&
				env_tightness == rv.env_tightness;
		}
}; // keep fields in order, or edit ctor by initializer_list



template <typename T>
class CPattern {
	CPattern () = delete;

    public:
      // the complete pattern signature is made of:
      // (a) course of the mean (low-freq component);
      // (b) instantaneous frequency at fine intervals;
      // (c) signal breadth at given tightness.

      // data for individual constituents of the pattern:
	SPatternParamPack
		params;

	float	a, b, c; // strictness

	// resulting
	float	match_a,
		match_b,
		match_c;

	CPattern (const valarray<T>& pattern,
		  size_t _context_before, size_t _context_after,
		  size_t _samplerate,
		  const SPatternParamPack&,
		  float _a, float _b, float _c);

	size_t size_with_context() const
		{
			return course.size();
		}
	size_t size_essential() const
		{
			return size_with_context() - context_before - context_after;
		}

	size_t find( const valarray<T>& course,
		     const valarray<T>& breadth,
		     const valarray<T>& dzcdf,
		     ssize_t start,
		     int inc);
	size_t find( const valarray<T>& signal,
		     ssize_t start,
		     int inc);

    private:
	valarray<T>
		course,
		breadth,
		dzcd;
	size_t	samplerate,
		context_before,
		context_after;
};



template <typename T>
double
sig_diff( const valarray<T>& a, const valarray<T>& b, int d);




template <typename T>
double
phase_diff( const valarray<T>& sig1,
	    const valarray<T>& sig2,
	    size_t samplerate,
	    size_t sa, size_t sz,
	    float fa, float fz,
	    unsigned order,
	    size_t scope);

#include "sigproc.ii"

} // namespace sigproc


#endif

// eof
