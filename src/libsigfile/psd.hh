// ;-*-C++-*-
/*
 *       File name:  libsigfile/psd.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2010-04-28
 *
 *         Purpose:  CBinnedPower and related stuff
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_PSD_H
#define _SIGFILE_PSD_H

#include <cstring>
#include <string>
#include <stdexcept>
#include <list>
#include <array>
#include <numeric>
#include <valarray>

#include "../misc.hh"
#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigfile {



struct SFFTParamSet {

	typedef unsigned short TWinType_underlying_type;
	enum class TWinType : TWinType_underlying_type {
		bartlett,
		blackman,
		blackman_harris,
		hamming,
		hanning,
		parzen,
		square,
		welch,
		_total
	};

	static const array<const char*, 8> welch_window_type_names;
	static const char* welch_window_type_name( TWinType i)
		{
			return welch_window_type_names[(int)i];
		}

	size_t	pagesize;
	TWinType
		welch_window_type;
	double	//freq_trunc,  // unused yet
		binsize;

	size_t
	compute_n_bins( size_t samplerate) const
		{
			return (samplerate * pagesize + 1) / 2 / samplerate / binsize;
		}

	SFFTParamSet& operator=( const SFFTParamSet& rv)
		{
			pagesize = rv.pagesize;
			welch_window_type = rv.welch_window_type;
			//freq_trunc = rv.freq_trunc;
			binsize = rv.binsize;
			return *this;
			// don't touch samplerate
		}
	bool operator==( const SFFTParamSet& rv) const
		{
			return	pagesize == rv.pagesize &&
				//freq_trunc == rv.freq_trunc &&
				binsize == rv.binsize &&
				welch_window_type == rv.welch_window_type;
		}
	bool validate();

	SFFTParamSet( const SFFTParamSet& rv) = default;
	SFFTParamSet() = default;
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





class CBinnedPower
  : public CPageMetrics_base, public SFFTParamSet {

	CBinnedPower() = delete;

    protected:
	CBinnedPower( const CSource& F, int sig_no,
		      const SFFTParamSet &fft_params);

    public:
	// in a frequency range
	template <class T>
	valarray<T> course( float from, float upto) const
		{
			valarray<T> acc (0., pages());
			size_t	bin_a = min( (size_t)(from / binsize), _bins),
				bin_z = min( (size_t)(upto / binsize), _bins);
			for ( size_t b = bin_a; b < bin_z; ++b )
				acc += CPageMetrics_base::course<T>(b);
			return acc;
		}

      // obtain
	int compute( const SFFTParamSet& req_params,
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

} // namespace sigfile


#endif // _SIGFILE_PSD_H

// eof
