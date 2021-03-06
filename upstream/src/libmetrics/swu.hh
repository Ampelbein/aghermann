/*
 *       File name:  libmetrics/swu.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-11-10
 *
 *         Purpose:  CBinnedSWU and related stuff
 *
 *         License:  GPL
 */

#ifndef AGH_LIBMETRICS_SWU_H_
#define AGH_LIBMETRICS_SWU_H_

#include <string>
#include <list>
#include <valarray>

#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {
namespace swu {


struct SPPack
  : public metrics::SPPack {

	double	min_upswing_duration;

	SPPack ()
		{
			reset();
		}
	size_t
	compute_n_bins( size_t _samplerate) const
		{
			return 1;
		}

	void check() const  // throws if not ok
		{
			metrics::SPPack::check();
		}

	void reset()
		{
			metrics::SPPack::reset();
			min_upswing_duration = .3;
		}
};



class CProfile
  : public metrics::CProfile {

    public:
	CProfile (const sigfile::CTypedSource&, int sig_no,
		  const SPPack&);

	SPPack Pp;

	const char* metric_name() const
		{
			return metrics::name( TType::swu);
		}

	valarray<TFloat> course( double) const
		{
			size_t	bin = 0; // (size_t)(binf - freq_from / Pp.freq_inc);
			return metrics::CProfile::course(bin);
		}

	int go_compute();

	string fname_base() const;
	string mirror_fname() const;

	int export_tsv( const string&) const;
	int export_tsv( float, float,
			const string&) const;

	// to enable use as mapped type
	CProfile (const CProfile& rv)
	      : metrics::CProfile (rv)
		{}
};

} // namespace swu
} // namespace metrics


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
