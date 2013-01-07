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





template <typename T>
struct SSignalRef {
	const valarray<T>&
		signal;
	unsigned
		samplerate;
};



// cached signal property providers

template <typename T>
struct SCachedDzcdf
  : public SSignalRef<T> {
	SCachedDzcdf (const valarray<T>& signal_, unsigned samplerate_)
	      : SSignalRef<T> {signal_, samplerate_}
		{}
	SCachedDzcdf (const SCachedDzcdf&) = delete;
	// other ctors deleted implicitly due to a member of reference type

	const valarray<T>&
	operator()( float step_, float sigma_, unsigned smooth_)
		{
			if ( data.size() == 0 ||
			     step != step_ || sigma != sigma_ || smooth != smooth_ )
				data = dzcdf<T>(
					SSignalRef<T>::signal, SSignalRef<T>::samplerate,
					step = step_, sigma = sigma_, smooth = smooth_);
			return data;
		}
	void drop()
		{
			data.resize(0);
		}
    private:
	float	step,
		sigma;
	unsigned
		smooth;
	valarray<T>
		data;
};

template <typename T>
struct SCachedEnvelope
  : public SSignalRef<T> {
	SCachedEnvelope (const valarray<T>& signal_, unsigned samplerate_)
	      : SSignalRef<T> {signal_, samplerate_}
		{}
	SCachedEnvelope (const SCachedEnvelope&) = delete;

	const pair<valarray<T>&, valarray<T>&>
	operator()( unsigned tightness_)
		{
			if ( lower.size() == 0 ||
			     tightness != tightness_ )
				envelope( SSignalRef<T>::signal,
					  tightness = tightness_, SSignalRef<T>::samplerate,
					  1./SSignalRef<T>::samplerate,
					  &lower,
					  &upper); // don't need anchor points, nor their count
			return {lower, upper};
		}
	void drop()
		{
			upper.resize(0);
			lower.resize(0);
		}

	float breadth( unsigned tightness_, size_t i)
		{
			(*this)( tightness_);
			return upper[i] - lower[i];
		}
	valarray<T> breadth( unsigned tightness_)
		{
			(*this)( tightness_);
			return upper - lower;
		}

    private:
	unsigned
		tightness;
	valarray<T>
		upper,
		lower;
};

template <typename T>
struct SCachedLowPassCourse
  : public SSignalRef<T> {
	SCachedLowPassCourse (const valarray<T>& signal_, unsigned samplerate_)
	      : SSignalRef<T> {signal_, samplerate_}
		{}
	SCachedLowPassCourse (const SCachedLowPassCourse&) = delete;

	const valarray<T>&
	operator()( float fcutoff_, unsigned order_)
		{
			if ( data.size() == 0 ||
			     fcutoff != fcutoff_ || order != order_ )
				data = exstrom::low_pass( SSignalRef<T>::signal, SSignalRef<T>::samplerate,
							  fcutoff = fcutoff_, order = order_,
							  true);
			return data;
		}
	void drop()
		{
			data.resize(0);
		}

    private:
	float	fcutoff;
	unsigned
		order;
	valarray<TFloat>
		data;
};

template <typename T>
struct SCachedBandPassCourse
  : public SSignalRef<T> {
	SCachedBandPassCourse (const valarray<T>& signal_, unsigned samplerate_)
	      : SSignalRef<T> {signal_, samplerate_}
		{}
	SCachedBandPassCourse (const SCachedBandPassCourse&) = delete;

	const valarray<T>&
	operator()( float ffrom_, float fupto_, unsigned order_)
		{
			if ( data.size() == 0 ||
			     ffrom != ffrom_ || fupto != fupto_ || order != order_ )
				data = exstrom::band_pass( SSignalRef<T>::signal, SSignalRef<T>::samplerate,
							   ffrom = ffrom_, fupto = fupto_, order = order_,
							   true);
			return data;
		}
	void drop()
		{
			data.resize(0);
		}

    private:
	float	ffrom, fupto;
	unsigned
		order;
	valarray<TFloat>
		data;
};



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
