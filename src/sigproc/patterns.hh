// ;-*-C++-*-
/*
 *       File name:  sigproc/patterns.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-09
 *
 *         Purpose:  class CPattern
 *
 *         License:  GPL
 */

#ifndef _SIGPROC_PATTERNS_H
#define _SIGPROC_PATTERNS_H

#include <gsl/gsl_math.h>

#include "sigproc.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigproc {

template <typename T>
struct TMatch : public valarray<T> {
	TMatch (T _1, T _2, T _3, T _4)
	      : valarray<T> ({_1, _2, _3, _4})
		{}
	TMatch<T> ()
	      : valarray<T> (4)
		{}

	bool operator==( const TMatch<T>& rv) const
		{
			for ( size_t i = 0; i < 4; ++i )
				if ( (*this)[i] != rv[i] )
					return false;
			return true;
		}
	bool good_enough( const TMatch<T>& rv) const
		{
			for ( size_t i = 0; i < 4; ++i )
				if ( (*this)[i] > rv[i] )
					return false;
			return true;
		}
};

template <typename T>
struct SPatternPPack {
	int	env_tightness;
	double	bwf_ffrom,
		bwf_fupto;
	int	bwf_order;
	double 	dzcdf_step,
		dzcdf_sigma;
	int	dzcdf_smooth;
	bool operator==( const SPatternPPack<T>& rv) const // cannot be defaulted!
		{
			return	env_tightness == rv.env_tightness &&
				bwf_ffrom == rv.bwf_ffrom &&
				bwf_fupto == rv.bwf_fupto &&
				bwf_order == rv.bwf_order &&
				dzcdf_step == rv.dzcdf_step &&
				dzcdf_sigma == rv.dzcdf_sigma &&
				dzcdf_smooth == rv.dzcdf_smooth &&
				criteria == rv.criteria;
		}
	TMatch<T>
		criteria;
}; // keep fields in order, or edit ctor by initializer_list



template <typename T>
class CPattern
  : public SPatternPPack<T> {
	CPattern () = delete;

    public:
      // the complete pattern signature is made of:
      // (a) signal breadth at given tightness;
      // (b) its course;
      // (c) target frequency (band-passed);
      // (d) instantaneous frequency at fine intervals;

	TMatch<T>
		match; // resulting

	CPattern (const SSignalRef<T>& thing,
		  size_t ctx_before_, size_t ctx_after_,
		  const SPatternPPack<T>& Pp_)
	      : SPatternPPack<T> (Pp_),
		match (NAN, NAN, NAN, NAN),
		penv (thing),
		ptarget_freq (thing),
		pdzcdf (thing),
		samplerate (thing.samplerate),
		ctx_before (ctx_before_), ctx_after (ctx_after_)
		{
			if ( ctx_before + ctx_after >= thing.signal.size() )
				throw invalid_argument ("pattern.size too small");
		}

	size_t find( const SSignalRef<T>& field,
		     ssize_t start,
		     int inc);
	size_t find( const valarray<T>& field,
		     ssize_t start,
		     int inc);
	size_t find( const valarray<T>& env_u,  // broken-down field
		     const valarray<T>& env_l,
		     const valarray<T>& target_freq,
		     const valarray<T>& dzcdf,
		     ssize_t start,
		     int inc);

	size_t size_with_context() const
		{
			return ptarget_freq.signal.size();
		}
	size_t size_essential() const
		{
			return size_with_context()
				- ctx_before - ctx_after;
		}

    private:
	SCachedEnvelope<T>
		penv;
	SCachedBandPassCourse<T>
		ptarget_freq;
	SCachedDzcdf<T>
		pdzcdf;

	size_t	samplerate;
	size_t	ctx_before,
		ctx_after;

	T	crit_linear_unity;
	double	crit_dzcdf_unity;
};

#include "patterns.ii"

} // namespace sigproc


#endif

// eof
