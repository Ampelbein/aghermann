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

#include "../sigproc/ext-filters.hh"
#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigfile {



struct SMCParamSet {
	double	scope,
		f0fc,//f0, // = 1.,
		//fc, // = 1.8;
		bandwidth, // = 1.5;
		iir_backpolate,			// = 0.5;	// 0.0 < Backpolate < 1.0 on s: standard 0.5
		mc_gain;			// = 10.0;	// Gain (DigiRange/PhysiRange) of MicroContinuity
	size_t	smooth_side;

	SMCParamSet& operator=( const SMCParamSet& rv) = default;
	bool operator==( const SMCParamSet& rv) const
		{
			return	scope == rv.scope &&
				iir_backpolate == rv.iir_backpolate &&
				mc_gain == rv.mc_gain &&
				f0fc == rv.f0fc &&
				bandwidth == rv.bandwidth &&
				smooth_side == rv.smooth_side;
		}
	void check( size_t pagesize) const; // throws
	void reset();

	size_t
	compute_n_bins( size_t) const // to match SFFTParamSet::compute_n_bins
		{
			return 5;
		}
	static constexpr double freq_from = .5;

	SMCParamSet( const SMCParamSet& rv) = default;
	SMCParamSet()
		{
			reset();
		}
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
	const char* method() const
		{
			return metric_method( TMetricType::Mc);
		}

	int
	compute( const SMCParamSet& req_params,
		 bool force = false);
	int
	compute( bool force = false)
		{
			return compute( *this, force);
		}

	string fname_base() const;
	int export_tsv( const string& fname) const;
	int export_tsv( size_t bin,
			const string& fname) const;

      // computation stages
	typedef pair<valarray<TFloat>, valarray<TFloat>> TSSSU;

	static TSSSU
	do_sssu_reduction( const valarray<TFloat>& signal,
			   size_t samplerate, double scope,
			   double mc_gain, double iir_backpolate,
			   double f0, double fc,
			   double bandwidth);

	static const size_t sssu_hist_size = 100;
};




} // namespace sigfile


#endif // _SIGFILE_MC_H

// eof
