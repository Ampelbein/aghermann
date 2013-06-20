/*
 *       File name:  libmetrics/psd.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2010-04-28
 *
 *         Purpose:  CProfile and related stuff
 *
 *         License:  GPL
 */

#ifndef AGH_LIBMETRICS_PSD_H_
#define AGH_LIBMETRICS_PSD_H_

#include <string>
#include <list>
#include <valarray>

#include <fftw3.h>

#include "libsigproc/winfun.hh"
#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {
namespace psd {


enum TFFTWPlanType {
	estimate,
	measure,
	TFFTWPlanType_total
};

inline int
plan_flags( TFFTWPlanType t)
{
	switch ( t ) {
	case TFFTWPlanType::estimate:
		return 0|FFTW_ESTIMATE;
	case TFFTWPlanType::measure:
		return 0|FFTW_MEASURE;
	default:
		return 0;
	}
}

inline TFFTWPlanType
plan_type( int f)
{
	// this is oversimplified
	if ( f & FFTW_MEASURE )
		return TFFTWPlanType::measure;
	if ( f & FFTW_ESTIMATE )
		return TFFTWPlanType::estimate;
	return (TFFTWPlanType)0;
}

struct SPPack
  : public metrics::SPPack {
	double	binsize;
	static sigproc::TWinType
		welch_window_type;
	static TFFTWPlanType
		plan_type;

	SPPack ()
		{
			reset();
		}

	bool same_as( const SPPack& rv) const
		{
			return metrics::SPPack::same_as(rv);
		}
	void make_same( const SPPack& rv)
		{
			metrics::SPPack::make_same(rv);
		}

	size_t
	compute_n_bins( size_t samplerate) const
		{
			return (samplerate * pagesize + 1) / 2 / samplerate / binsize;
		}

	void check() const
		{
#ifdef _OPENMP
#pragma omp single
#endif
			{
				metrics::SPPack::check();

				if ( welch_window_type > sigproc::TWinType::TWinType_total )
					throw invalid_argument ("Invalid window type");

				if ( plan_type != metrics::psd::TFFTWPlanType::estimate &&
				     plan_type != metrics::psd::TFFTWPlanType::measure )
					throw invalid_argument ("Invalid FFTW plan type");

				for ( auto c : {.1, .25, .5} )
					if ( binsize == c )
						return;
				throw invalid_argument ("Invalid binsize");
			}
		}

	void reset()
		{
			metrics::SPPack::reset();
			binsize = .25;
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


#endif // AGH_LIBMETRICS_PSD_H_

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
