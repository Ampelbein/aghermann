/*
 *       File name:  libmetrics/page-metrics-base.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  Base class for various per-page EEG metrics (PSD, uCont)
 *
 *         License:  GPL
 */

#ifndef AGH_LIBMETRICS_PAGE_METRICS_BASE_H_
#define AGH_LIBMETRICS_PAGE_METRICS_BASE_H_

#include <list>
#include <valarray>

#include "common/lang.hh"
#include "common/alg.hh"
#include "libsigfile/source.hh"
#include "forward-decls.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {

enum class TType { raw, psd, mc, swu };

inline const char*
__attribute__ ((pure))
name( TType t)
{
	switch ( t ) {
	case TType::psd:
		return "PSD";
	case TType::mc:
		return "Microcontinuity";
	case TType::swu:
		return "SW Upswing";
	default:
		return "(unknown metric)";
	}
}



struct SPPack {
	double	pagesize,
		step;

	SPPack ()
		{
			reset();
		}

	virtual bool same_as( const SPPack& rv) const
		{
			return pagesize == rv.pagesize && step == rv.step;
		}
	virtual void make_same( const SPPack& rv)
		{
			pagesize = rv.pagesize;
			step = rv.step;
		}

	void check() const; // throws
	void reset();
};



// We better keep the internal storage as valarray<double> regardless
// of what TFloat today is, because the computed data are written/read
// to files (else, we'd need to mark files as holding double data, not float).
class CProfile {

    protected:
	CProfile (const sigfile::CTypedSource&, int sig_no,
		  double pagesize, double step, size_t bins);
	CProfile (const CProfile&) = default;
    public:
	SPPack	Pp;

	virtual const char* metric_name() const = 0;

	const sigfile::CSource& source() const
		{
			return _using_F();
		}
	int sig_no() const
		{
			return _using_sig_no;
		}

	bool have_data() const
		{
			return _status & TFlags::computed;
		}

	size_t bins() const
		{
			return _bins;
		}

	size_t steps() const; // overlapping pages
	size_t samplerate() const;

      // accessors
	TFloat&
	nmth_bin( size_t p, size_t b)
		{
			// if ( unlikely (b >= n_bins()) )
			// 	throw out_of_range("CPageMetrics_base::nmth_bin(): bin out of range");
			// if ( unlikely (p >= n_pages()) )
			// 	throw out_of_range("CPageMetrics_base::nmth_bin(): page out of range");
			return _data[p * _bins + b];
		}
	const TFloat&
	nmth_bin( size_t p, size_t b) const
		{
			return _data[p * _bins + b];
		}

      // power course
	// full (note the returned array size is length * n_bins)
	valarray<TFloat> course() const
		{
			return _data;
		}

	// in a bin
	valarray<TFloat> course( size_t m) const
		{
			return _data[ slice(m, steps(), _bins) ];
		}

	valarray<TFloat> spectrum( size_t p) const
		{
			return _data[ slice(p * _bins, _bins, 1) ];
		}


    public:
      // artifacts
	list<agh::alg::SSpan<size_t>> artifacts_in_samples() const;
	list<agh::alg::SSpan<double>> artifacts_in_seconds() const;

	virtual int export_tsv( const string& fname) const;

	int compute( const SPPack&);
	int compute()
		{
			return compute( Pp);
		}
	bool need_compute( const SPPack&);  // not const because it does mirror_back
	bool need_compute()
		{
			return need_compute( Pp);
		}


    protected:
	virtual int go_compute() = 0;
	virtual string fname_base() const = 0;
	virtual string mirror_fname() const = 0;

	enum TFlags : int {
		computed = (1<<0),
		computable = (1<<1)
	};
	int	_status;

	valarray<TFloat>  // arrays in a given bin extracted by slices
		_data;    // it is always double because it is saved/loaded in this form
	size_t	_bins;

	hash_t	_signature_when_mirrored;

	const sigfile::CTypedSource& _using_F;
	int _using_sig_no;

	int mirror_enable( const string&);
	int mirror_back( const string&);
};



} // namespace metrics

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
