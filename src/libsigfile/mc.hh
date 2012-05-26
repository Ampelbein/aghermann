// ;-*-C++-*-
/*
 *       File name:  libsigfile/mc.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  CBinnedMC ("EEG microcontinuity")
 *                   (lite)
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_MC_H
#define _SIGFILE_MC_H

#include "../libsigproc/ext-filters.hh"
#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigfile {



struct SMCParamSet {
	double	iir_backpolate,	// = 0.5;	// 0.0 < Backpolate < 1.0 on s: standard 0.5
		f0fc, // = 1.8 - 1.;
		bandwidth, // = 1.5;
		mc_gain;

	SMCParamSet& operator=( const SMCParamSet& rv) = default;
	bool operator==( const SMCParamSet& rv) const
		{
			return	iir_backpolate == rv.iir_backpolate &&
				f0fc == rv.f0fc &&
				bandwidth == rv.bandwidth &&
				mc_gain == rv.mc_gain;
		}
	void check( size_t pagesize) const; // throws
	void reset( size_t pagesize);

	size_t
	compute_n_bins( size_t samplerate) const // to match SFFTParamSet::compute_n_bins
		{
			return 10;
		}
	static constexpr double freq_from = .5;

	SMCParamSet( const SMCParamSet& rv) = default;
	SMCParamSet() = default;
};



class CBinnedMC
  : public CPageMetrics_base,
    public SMCParamSet {

	CBinnedMC() = delete;
	void operator=( const CBinnedMC&) = delete;

    protected:
	CBinnedMC( const CSource& F, int sig_no,
		   const SMCParamSet &params,
		   size_t pagesize);

    public:
	int compute( const SMCParamSet& req_params,
		     bool force = false);
	int compute( bool force = false)
		{
			return compute( *this, force);
		}

	string fname_base() const;
	int export_tsv( const string& fname) const;
	int export_tsv( size_t bin,
			const string& fname) const;
    private:
	valarray<TFloat>
		ss,
		su;
      // computation stages
	void do_sssu_reduction( size_t bin);
};


} // namespace sigfile


#endif // _SIGFILE_MC_H

// eof
