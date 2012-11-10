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

#include "sigproc/sigproc.hh"
#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {
namespace psd {


struct SPPack {
	SPPack (const SPPack& rv) = default;
	SPPack ()
		{
			reset();
		}

	size_t
	compute_n_bins( size_t samplerate) const
		{
			return (samplerate * pagesize + 1) / 2 / samplerate / binsize;
		}

	SPPack& operator=( const SPPack& rv) = default;
	bool operator==( const SPPack& rv) const
		{
			return	pagesize == rv.pagesize &&
				welch_window_type == rv.welch_window_type &&
				binsize == rv.binsize;
		}
	void check() const;  // throws if not ok
	void reset();

	size_t	pagesize;
	sigproc::TWinType
		welch_window_type;
	double	binsize;
};




// this is an odd bit never used in libagh
enum TBand : unsigned short {
	delta,
	theta,
	alpha,
	beta,
	gamma,
	_total,
};





class CProfile
  : public CProfile_base,
    public SPPack {

    protected:
	CProfile (const sigfile::CSource& F, int sig_no,
		  const SPPack &fft_params);

    public:
	const char* method() const
		{
			return metric_method( TType::psd);
		}

	// in a frequency range
	template <class T>
	valarray<T> course( float from, float upto) const
		{
			valarray<T> acc (0., pages());
			size_t	bin_a = min( (size_t)(from / binsize), _bins),
				bin_z = min( (size_t)(upto / binsize), _bins);
			for ( size_t b = bin_a; b < bin_z; ++b )
				acc += CProfile_base::course<T>(b);
			return acc;
		}

      // obtain
	int compute( const SPPack& req_params,
		     bool force = false);
	void compute( bool force = false)
	// possibly reuse that already obtained unless factors affecting signal or fft are different
		{
			compute( *this, force);
		}

	string fname_base() const;

	int export_tsv( const string& fname) const;
	int export_tsv( float from, float upto,
			const string& fname) const;
};

} // namespace psd
} // namespace metrics


#endif // _METRICS_PSD_H

// eof
