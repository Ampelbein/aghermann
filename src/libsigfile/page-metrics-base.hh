// ;-*-C++-*-
/*
 *       File name:  libsigfile/page-metrics-base.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  Base class for various per-page EEG metrics (PSD, uCont)
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_PAGE_METRICS_BASE_H
#define _SIGFILE_PAGE_METRICS_BASE_H

#include <stdexcept>
#include <list>
#include <array>
#include <numeric>
#include <valarray>

#include "../common/misc.hh"
#include "../common/alg.hh"
#include "forward-decls.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigfile {


enum TMetricType { Psd, Mc };

inline const char*
__attribute__ ((pure))
metric_method( TMetricType t)
{
	switch ( t ) {
	case Psd:
		return "PSD";
	case Mc:
		return "Microcontinuity";
	default:
		return "(unknown metric)";
	}
}



// We better keep the internal storage as valarray<double> regardless
// of what TFloat today is, because the computed data are written/read
// to files (else, we'd need to mark files as holding double data, not float).
class CPageMetrics_base {

	CPageMetrics_base() = delete;
	void operator=( const CPageMetrics_base&) = delete;

    public:
	virtual const char* method() const = 0;

	bool have_data() const
		{
			return _status & TFlags::computed;
		}

	size_t pagesize() const
		{
			return _pagesize;
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
	list<agh::SSpan<size_t>> artifacts_in_samples() const;
	list<agh::SSpan<float>> artifacts_in_seconds() const;

	virtual int export_tsv( const string& fname) const;

	const CSource& source() const
		{
			return _using_F;
		}
	int sig_no() const
		{
			return _using_sig_no;
		}
      // filenames
	virtual string fname_base() const = 0;

    protected:
	enum TFlags : int { computed = 1 };
	int	_status;

	valarray<double>  // arrays in a given bin extracted by slices
		_data;    // it is always double because it is saved/loaded in this form
	size_t	_bins,
		_pagesize;

	CPageMetrics_base( const CSource& F, int sig_no,
			   size_t pagesize, size_t bins);

	size_t  // hash
		_signature;

	const CSource& _using_F;
	int _using_sig_no;

	int _mirror_enable( const char*);
	int _mirror_back( const char*);
};



template <>
inline valarray<double>
CPageMetrics_base::course() const
{
	return _data;
}


template <>
inline valarray<float>
CPageMetrics_base::course() const
{
	valarray<float> coursef (_data.size());
	for ( size_t i = 0; i < _data.size(); ++i )
		coursef[i] = _data[i];
	return coursef;
}


template <>
inline valarray<double>
CPageMetrics_base::course( size_t m) const
{
	return _data[ slice(m, pages(), _bins) ];
}


template <>
inline valarray<float>
CPageMetrics_base::course( size_t m) const
{
	valarray<double> course = _data[ slice(m, pages(), _bins) ];
	valarray<float> coursef (0., course.size());
	for ( size_t i = 0; i < course.size(); ++i )
		coursef[i] = (float)course[i];
	return coursef;
}



template <>
inline valarray<double>
CPageMetrics_base::spectrum( size_t p) const
{
	if ( unlikely (p >= pages()) )
		throw out_of_range("CPageMetrics_base::power_spectrum(): page out of range");
	return _data[ slice(p * _bins, _bins, 1) ];
}

template <>
inline valarray<float>
CPageMetrics_base::spectrum( size_t p) const
{
	valarray<double> dps = spectrum<double>(p);
	valarray<float> ps (dps.size());
	for ( size_t i = 0; i < ps.size(); ++i )
		ps[i] = dps[i];
	return ps;
}


} // namespace sigfile


#endif // _SIGFILE_PAGE_METRICS_BASE_H

// eof
