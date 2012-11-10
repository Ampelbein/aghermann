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
				binsize == rv.binsize;
		}
	void check() const;  // throws if not ok
	void reset();

	size_t	pagesize;
	double	binsize;
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
			return metric_method( TType::swu);
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
		{
			compute( *this, force);
		}

	string fname_base() const;

	int export_tsv( const string& fname) const;
	int export_tsv( float from, float upto,
			const string& fname) const;
};

} // namespace swu
} // namespace metrics


#endif // _METRICS_SWU_H

// eof
