// ;-*-C++-*-
/*
 *       File name:  metrics/psd.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2010-04-28
 *
 *         Purpose:  CProfile and related stuff
 *
 *         License:  GPL
 */

#ifndef _METRICS_PSD_H
#define _METRICS_PSD_H

#include <string>
#include <list>
#include <valarray>

#include "sigproc/winfun.hh"
#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {
namespace psd {


// this is an odd bit never used in libagh
enum TBand {
	delta,
	theta,
	alpha,
	beta,
	gamma,
	_total,
};



struct SPPack
  : public metrics::SPPack {
	double	binsize;
	sigproc::TWinType
		welch_window_type;

	SPPack ()
		{
			reset();
		}

	bool same_as( const SPPack& rv) const
		{
			return	metrics::SPPack::same_as(rv) &&
				welch_window_type == rv.welch_window_type;
		}
	void make_same( const SPPack& rv)
		{
			metrics::SPPack::make_same(rv);
			welch_window_type = rv.welch_window_type;
		}

	size_t
	compute_n_bins( size_t samplerate) const
		{
			return (samplerate * pagesize + 1) / 2 / samplerate / binsize;
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
			return metrics::name( TType::psd);
		}

	// in a frequency range
	valarray<TFloat> course( double from, double upto) const
		{
			valarray<TFloat> acc (0., pages());
			size_t	bin_a = min( (size_t)(from / Pp.binsize), _bins),
				bin_z = min( (size_t)(upto / Pp.binsize), _bins);
			for ( size_t b = bin_a; b < bin_z; ++b )
				acc += metrics::CProfile::course(b);
			return acc;
		}

	int go_compute();
	string mirror_fname() const;

	string fname_base() const;

	int export_tsv( const string&) const;
	int export_tsv( float, float,
			const string&) const;

	// to enable use as mapped type
	CProfile (const CProfile& rv)
	      : metrics::CProfile (rv)
		{}
};

} // namespace psd
} // namespace metrics


#endif // _METRICS_PSD_H

// eof
