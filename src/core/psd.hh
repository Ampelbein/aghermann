// ;-*-C++-*- *  Time-stamp: "2010-10-02 13:55:24 hmmr"

/*
 * Author: Andrei Zavada (johnhommer@gmail.com)
 *
 * License: GPL
 *
 * Initial version: 2010-04-28
 */

#ifndef _AGH_PSD_H
#define _AGH_PSD_H

#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <numeric>
#include <valarray>

#include "common.h"

using namespace std;


class CMeasurement;





struct SFFTParamSet {

	size_t	page_size;
	float	bin_size;
	TFFTWinType
		welch_window_type,
		af_dampen_window_type;
	size_t	smoothover;

	void assign( const SFFTParamSet& rv)
		{
			page_size = rv.page_size;
			bin_size = rv.bin_size;
			welch_window_type = rv.welch_window_type;
			af_dampen_window_type = rv.af_dampen_window_type;
			smoothover = rv.smoothover;
		}
	bool is_equal( const SFFTParamSet& rv) const
		{
			return	page_size == rv.page_size &&
				bin_size == rv.bin_size &&
				welch_window_type == rv.welch_window_type &&
				af_dampen_window_type == rv.af_dampen_window_type &&
				smoothover == rv.smoothover;
		}
    protected:
	size_t	samplerate;  // always recomputed from the edf source

      // changing any of these fields in CExpDesign will invalidate
      // all previously obtained measurements; hence we need to keep
      // them all with ourself to detect such a change
};




#define AGH_BP_NEWARTIFACTS	(1 << 1)

class CEDFFile;
class CSimulation;

class CBinnedPower
  : protected SFFTParamSet {

	CBinnedPower();

    protected:
	int	_status;

	valarray<double>  // arrays in a given bin extracted by slices
		_data;
	string	_artifacts;

	CBinnedPower( const SFFTParamSet &fft_params)
	      : SFFTParamSet (fft_params),
		_status (0),
		_using_F (NULL),
		_using_sig_no (-1)
		{}

    public:
	bool has_power() const
		{
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
			return page_size / bin_size;
		}
	size_t n_pages() const
		{
			return _data.size() / n_bins();
		}

      // resize
	void resize( size_t _n_pages)
		{
			_artifacts.resize( _n_pages * page_size, ' ');
			_data.resize( _n_pages * n_bins());
		}

      // accessors
	double &nmth_bin( size_t p, size_t b)
		{
			return _data[p * n_bins() + b];
		}
	const double &nmth_bin( size_t p, size_t b) const
		{
			return _data[p * n_bins() + b];
		}

	valarray<double> power_spectrum( size_t p) const
		{
			return _data[ slice(p * page_size, n_bins(), 1) ];
		}
	valarray<float> power_spectrumf( size_t p) const
		{
			valarray<double> dps = power_spectrum(p);
			valarray<float> ps (n_bins());
			for ( size_t i = 0; i < n_bins(); ++i )
				ps[i] = dps[i];
			return ps;
		}

      // power course
	// full (note the returned array size is length * n_bins)
	valarray<double> power_course() const
		{
			return _data;
		}
	valarray<float> power_coursef() const
		{
			valarray<float> coursef (_data.size());
			for ( size_t i = 0; i < _data.size(); ++i )
				coursef[i] = _data[i];
			return coursef;
		}

	// in a bin
	valarray<double> power_course( size_t m) const
		{
			return _data[ slice(m, n_pages(), n_bins()) ];
		}
	valarray<float> power_coursef( size_t m) const
		{
			valarray<double> course = _data[ slice(m, n_pages(), n_bins()) ];
			valarray<float> coursef (course.size());
			for ( size_t i = 0; i < course.size(); ++i )
				coursef[i] = course[i];
			return coursef;
		}

	// in a range
	valarray<double> power_course( float from, float upto) const
		{
			valarray<double> acc (n_pages());
			size_t bin_a = from/bin_size, bin_z = upto/bin_size;
			if ( bin_z > n_bins() )
				bin_z = n_bins()-1;
			for ( auto b = bin_a; b <= bin_z; ++b )
				acc += power_course(b);
			return acc;
		}
	valarray<float> power_coursef( float from, float upto) const
		{
			valarray<float> acc (n_pages());
			size_t bin_a = from/bin_size, bin_z = upto/bin_size;
			if ( bin_z > n_bins() )
				bin_z = n_bins()-1;
			for ( auto b = bin_a; b <= bin_z; ++b )
				acc += power_coursef(b);
			return acc;
		}

      // artifacts
	const char *artifacts() const
		{
			return _artifacts.c_str();
		}
	void import_artifacts( const char *track)
		{
//			_artifacts.assign( _artifacts.size(), ' ') . assign( track, _artifacts.size());
			if ( _using_F )
				obtain_power( *_using_F, _using_sig_no,
					      *this, track);
		}
	char &nth_second( size_t i)
		{
			return _artifacts[i];
		}

      // obtain, export power
	int obtain_power( const CEDFFile&, int h,
			  const SFFTParamSet& req_params,
			  const char* req_artifacts);

	int export_tsv( const char *fname) const;
	int export_tsv( float from, float upto,
			const char *fname) const;

	const CEDFFile& source() const
		{
			return *_using_F;
		}
	int sig_no() const
		{
			return _using_sig_no;
		}

      // filenames
	string fname_base();

    private:
	const CEDFFile *_using_F;
	int _using_sig_no;

	int _mirror_enable( const char*);
	int _mirror_back( const char*);
};






#endif

// EOF
