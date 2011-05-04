// ;-*-C++-*- *  Time-stamp: "2011-05-01 14:27:53 hmmr"

/*
 *       File name:  libagh/psd.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2010-04-28
 *
 *         Purpose:  CBinnedPower and related stuff
 *
 *         License:  GPL
 */

#ifndef _AGH_PSD_H
#define _AGH_PSD_H

#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <numeric>
#include <valarray>

#include "enums.hh"
#include "edf.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif

using namespace std;

namespace agh {

class CMeasurement;



struct SFFTParamSet {

	static const array<const char*, 8> welch_window_type_names;
	static const char* welch_window_type_name( TFFTWinType i)
		{
			return welch_window_type_names[(int)i];
		}

	size_t	page_size;
	float	bin_size;
	TFFTWinType
		welch_window_type;

	void assign( const SFFTParamSet& rv)
		{
			page_size = rv.page_size;
			bin_size = rv.bin_size;
			welch_window_type = rv.welch_window_type;
		}
	bool operator==( const SFFTParamSet& rv) const
		{
			return	page_size == rv.page_size &&
				bin_size == rv.bin_size &&
				welch_window_type == rv.welch_window_type;
		}

	SFFTParamSet()
	      : page_size (30),
		bin_size (.5),
		welch_window_type (TFFTWinType::welch)
		{}
    protected:
	size_t	samplerate;  // always recomputed from the edf source

      // changing any of these fields in CExpDesign will invalidate
      // all previously obtained measurements; hence we need to keep
      // them all with ourself to detect such a change
};




//#define AGH_BP_NEWARTIFACTS	(1 << 1)

class CSimulation;

class CBinnedPower
  : protected SFFTParamSet {

	CBinnedPower() = delete;

    protected:
	int	_status;

	valarray<double>  // arrays in a given bin extracted by slices
		_data;

	CBinnedPower( const SFFTParamSet &fft_params)
	      : SFFTParamSet (fft_params),
		_status (0),
		_using_F (NULL),
		_using_sig_no (-1)
		{}

    public:
	bool have_power() const
		{
			// the CRecording::CRecording() wouldn't have called obtain_power at construct
			return _data.size() > 0;
		}

	size_t pagesize() const
		{
			return SFFTParamSet::page_size;
		}
	float binsize() const
		{
			return SFFTParamSet::bin_size;
		}

	size_t length_in_seconds() const
		{
			return n_pages() * page_size;
		}
	size_t n_bins() const
		{
			return page_size / bin_size / 2;
		}
	size_t n_pages() const
		{
			return _data.size() / n_bins();
		}

      // resize
	void resize( size_t _n_pages)
		{
			_data.resize( _n_pages * n_bins());
		}

      // accessors
	double &nmth_bin( size_t p, size_t b)
		{
			if ( b >= n_bins() )
				throw out_of_range("CBinnedPower::nmth_bin(): bin out of range");
			if ( p >= n_pages() )
				throw out_of_range("CBinnedPower::nmth_bin(): page out of range");
			return _data[p * n_bins() + b];
		}
	const double &nmth_bin( size_t p, size_t b) const
		{
			if ( b >= n_bins() )
				throw out_of_range("CBinnedPower::nmth_bin(): bin out of range");
			if ( p >= n_pages() )
				throw out_of_range("CBinnedPower::nmth_bin(): page out of range");
			return _data[p * n_bins() + b];
		}

	template <class T>
	valarray<T> power_spectrum( size_t p) const;

      // power course
	// full (note the returned array size is length * n_bins)
	template <class T>
	valarray<T> power_course() const;

	// in a bin
	template <class T>
	valarray<T> power_course( size_t m) const;

	// in a range
	template <class T>
	valarray<T> power_course( float from, float upto) const
		{
			valarray<T> acc (0., n_pages());
			size_t	bin_a = min( (size_t)(from/bin_size), n_bins()),
				bin_z = min( (size_t)(upto/bin_size), n_bins());
			if ( bin_a < bin_z )
				for ( size_t b = bin_a; b < bin_z; ++b )
					acc += power_course<T>(b);
			return acc;
		}

      // artifacts
	list<pair<float,float>> artifacts()
		{
			auto &src = _using_F->signals[_using_sig_no].artifacts;
			list<pair<float,float> > ret (src.size());
			auto A = src.begin();
			auto B = ret.begin();
			while ( A != src.end() ) {
				(B++)->first  = (A++)->first  / (float)samplerate;
				(B++)->second = (A++)->second / (float)samplerate;
			}
			return ret;
		}
    protected:
	size_t  // hash
		_signature;

    public:
      // obtain, export power
	int obtain_power( const CEDFFile&, int h,
			  const SFFTParamSet& req_params);
	// possibly reuse that already obtained unless factors affecting signal or fft are different
	void obtain_power()
		{
			if ( _using_F )
				obtain_power( *_using_F, _using_sig_no,
					      *this);
		}

	int export_tsv( const string& fname);
	int export_tsv( float from, float upto,
			const string& fname);

	const CEDFFile& source() const
		{
			return *_using_F;
		}
	int sig_no() const
		{
			return _using_sig_no;
		}

      // filenames
	string fname_base() const;

    private:
	const CEDFFile *_using_F;
	int _using_sig_no;

	int _mirror_enable( const char*);
	int _mirror_back( const char*);
};



template <>
inline valarray<double>
CBinnedPower::power_course() const
{
	return _data;
}


template <>
inline valarray<float>
CBinnedPower::power_course() const
{
	valarray<float> coursef (_data.size());
	for ( size_t i = 0; i < _data.size(); ++i )
		coursef[i] = _data[i];
	return coursef;
}


template <>
inline valarray<double>
CBinnedPower::power_course( size_t m) const
{
	return _data[ slice(m, n_pages(), n_bins()) ];
}


template <>
inline valarray<float>
CBinnedPower::power_course( size_t m) const
{
	valarray<double> course = _data[ slice(m, n_pages(), n_bins()) ];
	valarray<float> coursef (0., course.size());
	for ( size_t i = 0; i < course.size(); ++i )
		coursef[i] = (float)course[i];
	return coursef;
}



template <>
inline valarray<double>
CBinnedPower::power_spectrum( size_t p) const
{
	if ( p >= n_pages() )
		throw out_of_range("CBinnedPower::power_spectrum(): page out of range");
	return _data[ slice(p * n_bins(), n_bins(), 1) ];
}

template <>
inline valarray<float>
CBinnedPower::power_spectrum( size_t p) const
{
	valarray<double> dps = power_spectrum<double>(p);
	valarray<float> ps (dps.size());
	for ( size_t i = 0; i < ps.size(); ++i )
		ps[i] = dps[i];
	return ps;
}


} // namespace agh


#endif

// EOF
