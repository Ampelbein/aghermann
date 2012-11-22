// ;-*-C++-*-
/*
 *       File name:  metrics/page-metrics-base.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  Base class for various per-page EEG metrics (PSD, uCont)
 *
 *         License:  GPL
 */

#ifndef _METRICS_PAGE_METRICS_BASE_H
#define _METRICS_PAGE_METRICS_BASE_H

#include <list>
#include <valarray>

#include "common/lang.hh"
#include "common/alg.hh"
#include "libsigfile/forward-decls.hh"
#include "forward-decls.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {

enum class TType { invalid, psd, mc, swu };

inline const char*
__attribute__ ((pure))
metric_method( TType t)
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
	size_t	pagesize;

	SPPack ()
		{
			reset();
		}

	virtual bool same_as( const SPPack& rv) const
		{
			return pagesize == rv.pagesize;
		}
	virtual void make_same( const SPPack& rv)
		{
			pagesize = rv.pagesize;
		}

	void check() const; // throws
	void reset();
};



// We better keep the internal storage as valarray<double> regardless
// of what TFloat today is, because the computed data are written/read
// to files (else, we'd need to mark files as holding double data, not float).
class CProfile {

    protected:
	CProfile (const sigfile::CSource&, int sig_no,
		  size_t pagesize, size_t bins);
	CProfile (const CProfile&) = default;
    public:
	SPPack	Pp;

	virtual const char* method() const = 0;

	const sigfile::CSource& source() const
		{
			return _using_F;
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

	size_t pages() const;
	size_t samplerate() const;

      // accessors
	double&
	nmth_bin( size_t p, size_t b)
		{
			// if ( unlikely (b >= n_bins()) )
			// 	throw out_of_range("CPageMetrics_base::nmth_bin(): bin out of range");
			// if ( unlikely (p >= n_pages()) )
			// 	throw out_of_range("CPageMetrics_base::nmth_bin(): page out of range");
			return _data[p * _bins + b];
		}
	const double&
	nmth_bin( size_t p, size_t b) const
		{
			return _data[p * _bins + b];
		}

	template <class T>
	valarray<T> spectrum( size_t p) const;

      // power course
	// full (note the returned array size is length * n_bins)
	template <class T>
	valarray<T> course() const;

	// in a bin
	template <class T>
	valarray<T> course( size_t m) const;

    public:
      // artifacts
	list<agh::alg::SSpan<size_t>> artifacts_in_samples() const;
	list<agh::alg::SSpan<float>> artifacts_in_seconds() const;

	virtual int export_tsv( const string& fname) const;

	int compute( const SPPack&);
	int compute()
		{
			return compute( Pp);
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

	valarray<double>  // arrays in a given bin extracted by slices
		_data;    // it is always double because it is saved/loaded in this form
	size_t	_bins;

	hash_t	_signature_when_mirrored;

	const sigfile::CSource& _using_F;
	int _using_sig_no;

	int mirror_enable( const string&);
	int mirror_back( const string&);
};



template <>
inline valarray<double>
CProfile::course() const
{
	return _data;
}


template <>
inline valarray<float>
CProfile::course() const
{
	valarray<float> coursef (_data.size());
	for ( size_t i = 0; i < _data.size(); ++i )
		coursef[i] = _data[i];
	return coursef;
}


template <>
inline valarray<double>
CProfile::course( size_t m) const
{
	return _data[ slice(m, pages(), _bins) ];
}


template <>
inline valarray<float>
CProfile::course( size_t m) const
{
	valarray<double> course = _data[ slice(m, pages(), _bins) ];
	valarray<float> coursef (0., course.size());
	for ( size_t i = 0; i < course.size(); ++i )
		coursef[i] = (float)course[i];
	return coursef;
}


template <>
inline valarray<double>
CProfile::spectrum( size_t p) const
{
	return _data[ slice(p * _bins, _bins, 1) ];
}

template <>
inline valarray<float>
CProfile::spectrum( size_t p) const
{
	valarray<double> dps = spectrum<double>(p);
	valarray<float> ps (dps.size());
	for ( size_t i = 0; i < ps.size(); ++i )
		ps[i] = dps[i];
	return ps;
}

inline valarray<double>
to_vad( valarray<double>&& rv)
{
	return move(rv);
}

inline valarray<double>
to_vad( const valarray<float>& rv)
{
	valarray<double> ret;
	ret.resize( rv.size());
	for ( size_t i = 0; i < rv.size(); ++i )
		ret[i] = rv[i];
	return ret;
}


} // namespace metrics

#endif // _SIGFILE_PAGE_METRICS_BASE_H

// eof
