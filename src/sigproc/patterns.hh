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

#include <tuple>
#include <vector>

#include <gsl/gsl_math.h>

#include "sigproc.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace pattern {

template <typename T>
class CMatch
  : public tuple<T, T, T, T> {
    public:
	CMatch ()
	      : tuple<T, T, T, T> (NAN, NAN, NAN, NAN)
		{}

	bool good_enough( const CMatch<T>& rv) const
		{
			return get<0>(*this) < get<0>(rv) &&
			       get<1>(*this) < get<1>(rv) &&
			       get<2>(*this) < get<2>(rv) &&
			       get<3>(*this) < get<3>(rv);
		}

};

template <typename T>
struct SPatternPPack {
	double	env_scope;
	double	bwf_ffrom,
		bwf_fupto;
	int	bwf_order;
	double 	dzcdf_step,
		dzcdf_sigma;
	int	dzcdf_smooth;
	bool operator==( const SPatternPPack<T>& rv) const // cannot be defaulted!
		{
			return	env_scope == rv.env_scope &&
				bwf_ffrom == rv.bwf_ffrom &&
				bwf_fupto == rv.bwf_fupto &&
				bwf_order == rv.bwf_order &&
				dzcdf_step == rv.dzcdf_step &&
				dzcdf_sigma == rv.dzcdf_sigma &&
				dzcdf_smooth == rv.dzcdf_smooth;
		}
}; // keep fields in order, or edit ctor by initializer_list



template <typename T>
class CPattern
  : public SPatternPPack<T> {
	DELETE_DEFAULT_METHODS (CPattern);

    public:
      // the complete pattern signature is made of:
      // (a) signal breadth at given tightness;
      // (b) its course;
      // (c) target frequency (band-passed);
      // (d) instantaneous frequency at fine intervals;

	CPattern (const sigproc::SSignalRef<T>& thing,
		  size_t ctx_before_, size_t ctx_after_,
		  const SPatternPPack<T>& Pp_)
	      : SPatternPPack<T> (Pp_),
		penv (thing),
		ptarget_freq (thing),
		pdzcdf (thing),
		samplerate (thing.samplerate),
		ctx_before (ctx_before_), ctx_after (ctx_after_)
		{
			if ( ctx_before + ctx_after >= thing.signal.size() )
				throw invalid_argument ("pattern size too small");
		}

	int do_search( const sigproc::SSignalRef<T>& field,
		       size_t inc);
	int do_search( const valarray<T>& field,
		       size_t inc);
	int do_search( const valarray<T>& env_u,  // broken-down field
		       const valarray<T>& env_l,
		       const valarray<T>& target_freq,
		       const valarray<T>& dzcdf,
		       size_t inc);

	vector<CMatch<T>>
		diff;

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
	sigproc::SCachedEnvelope<T>
		penv;
	sigproc::SCachedBandPassCourse<T>
		ptarget_freq;
	sigproc::SCachedDzcdf<T>
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
