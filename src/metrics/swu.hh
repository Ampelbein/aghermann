// ;-*-C++-*-
/*
 *       File name:  metrics/swu.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-11-10
 *
 *         Purpose:  CBinnedSWU and related stuff
 *
 *         License:  GPL
 */

#ifndef _METRICS_SWU_H
#define _METRICS_SWU_H

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
  : public metrics:: SPPack {

	double	min_upswing_duration;
	size_t
	compute_n_bins( size_t _samplerate) const
		{
			return 1;
		}

	void check() const;  // throws if not ok
	void reset();
};



class CProfile
  : public metrics::CProfile {

    public:
	CProfile (const sigfile::CSource&, int sig_no,
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


#endif // _METRICS_SWU_H

// eof
