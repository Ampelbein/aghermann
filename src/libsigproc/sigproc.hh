/*
 *       File name:  libsigproc/sigproc.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-26
 *
 *         Purpose:  various standalone signal processing functions
 *
 *         License:  GPL
 */

#ifndef AGH_LIBSIGPROC_SIGPROC_H_
#define AGH_LIBSIGPROC_SIGPROC_H_

#include <cmath>
#include <vector>
#include <valarray>
#include <stdexcept>

#include <gsl/gsl_math.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include <samplerate.h>

#include "common/lang.hh"
#include "common/alg.hh"
#include "exstrom.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigproc {

// simple functions operating irrespective of samplerate

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






template <typename T, class Container>
valarray<T>
interpolate( const vector<unsigned long>& xi,
	     unsigned samplerate, const Container& y, double dt);




// signal with samplerate

template <typename T>
struct SSignalRef {
	const valarray<T>&
		signal;
	size_t	samplerate;
};





template <typename T>
size_t
envelope( const SSignalRef<T>& in,
	  double dh,  // tightness
	  double dt,
	  valarray<T>* env_lp = nullptr,  // return interpolated
	  valarray<T>* env_up = nullptr,  // return vector of points
	  vector<size_t> *mini_p = nullptr,
	  vector<size_t> *maxi_p = nullptr);

template <typename T>
valarray<T>
raw_signal_profile( const SSignalRef<T>& sigref,
		    double env_dh, double env_dt)
{
	valarray<T>
		ret,
		env_u, env_l;
	envelope( sigref,
		  env_dh, env_dt,
		  &env_l, &env_u);
	ret.resize( env_l.size());
	ret = env_u - env_l;
	return ret;
}


template <typename T>
valarray<T>
dzcdf( const SSignalRef<T>& in,
       double dt,
       double sigma,
       size_t smooth);




// cached signal property providers

template <typename T>
struct SCachedEnvelope
  : public SSignalRef<T> {
	SCachedEnvelope (const SSignalRef<T>& signal_)
	      : SSignalRef<T> (signal_)
		{}
	SCachedEnvelope (const SCachedEnvelope&) = delete;

	const pair<valarray<T>&, valarray<T>&>
	operator()( double scope_)
		{
			if ( lower.size() == 0 ||
			     scope != scope_ ) {
				envelope( (SSignalRef<T>)*this,
					  scope = scope_,
					  1./SSignalRef<T>::samplerate,
					  &lower,
					  &upper); // don't need anchor points, nor their count
				mid.resize(lower.size());
				mid = (upper + lower)/2;
			}
			return {lower, upper};
		}
	void drop()
		{
			upper.resize(0);
			lower.resize(0);
		}

	T breadth( double scope_, size_t i)
		{
			operator()( scope_);
			return upper[i] - lower[i];
		}
	valarray<T> breadth( double scope_)
		{
			operator()( scope_);
			return upper - lower;
		}

	T centre( double scope_, size_t i)
		{
			operator()( scope_);
			return mid[i];
		}
	valarray<T> centre( double scope_)
		{
			operator()( scope_);
			return mid;
		}

    private:
	double	scope;
	valarray<T>
		upper,
		mid,
		lower;
};

template <typename T>
struct SCachedDzcdf
  : public SSignalRef<T> {
	SCachedDzcdf (const SSignalRef<T>& signal_)
	      : SSignalRef<T> (signal_)
		{}
	SCachedDzcdf (const SCachedDzcdf&) = delete;
	// other ctors deleted implicitly due to a member of reference type

	const valarray<T>&
	operator()( double step_, double sigma_, unsigned smooth_)
		{
			if ( data.size() == 0 ||
			     step != step_ || sigma != sigma_ || smooth != smooth_ )
				data = dzcdf<T>(
					(SSignalRef<T>)*this,
					step = step_, sigma = sigma_, smooth = smooth_);
			return data;
		}
	void drop()
		{
			data.resize(0);
		}
    private:
	double	step,
		sigma;
	unsigned
		smooth;
	valarray<T>
		data;
};

template <typename T>
struct SCachedLowPassCourse
  : public SSignalRef<T> {
	SCachedLowPassCourse (const SSignalRef<T>& signal_)
	      : SSignalRef<T> (signal_)
		{}
	SCachedLowPassCourse (const SCachedLowPassCourse&) = delete;

	const valarray<T>&
	operator()( double fcutoff_, unsigned order_)
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
	double	fcutoff;
	unsigned
		order;
	valarray<TFloat>
		data;
};

template <typename T>
struct SCachedBandPassCourse
  : public SSignalRef<T> {
	SCachedBandPassCourse (const SSignalRef<T>& signal_)
	      : SSignalRef<T> (signal_)
		{}
	SCachedBandPassCourse (const SCachedBandPassCourse&) = delete;

	const valarray<T>&
	operator()( double ffrom_, double fupto_, unsigned order_)
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
	double	ffrom, fupto;
	unsigned
		order;
	valarray<TFloat>
		data;
};






template <typename T>
double
sig_diff( const valarray<T>& a, const valarray<T>& b, int d);


template <typename T>
double
phase_diff( const SSignalRef<T>& sig1,
	    const SSignalRef<T>& sig2,
	    size_t sa, size_t sz,
	    double fa, double fz,
	    unsigned order,
	    size_t scope);






#include "sigproc.ii"

} // namespace sigproc


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
