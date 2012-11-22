// ;-*-C++-*-
/*
 *       File name:  metrics/mc.hh
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

#ifndef _METRICS_MC_H
#define _METRICS_MC_H

#include "sigproc/ext-filters.hh"
#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {
namespace mc {

struct SPPack
  : public metrics::SPPack {
	double	scope,
		f0fc,//f0, // = 1.,
		//fc, // = 1.8;
		bandwidth, // = 1.5;
		iir_backpolate,			// = 0.5;	// 0.0 < Backpolate < 1.0 on s: standard 0.5
		mc_gain;			// = 10.0;	// Gain (DigiRange/PhysiRange) of MicroContinuity
	size_t	smooth_side;
	static constexpr double freq_from = .5;

	SPPack (const SPPack&) = default;
	SPPack ()
		{
			reset();
		}

	bool same_as( const SPPack& rv) const
		{
			return	metrics::SPPack::same_as(rv) &&
				scope == rv.scope &&
				iir_backpolate == rv.iir_backpolate &&
				mc_gain == rv.mc_gain &&
				f0fc == rv.f0fc &&
				bandwidth == rv.bandwidth &&
				smooth_side == rv.smooth_side;
		}
	void make_same( const SPPack& rv)
		{
			metrics::SPPack::make_same(rv);
			scope = rv.scope;
			iir_backpolate = rv.iir_backpolate;
			mc_gain = rv.mc_gain;
			f0fc = rv.f0fc;
			bandwidth = rv.bandwidth;
			smooth_side = rv.smooth_side;
		}

	void check() const; // throws
	void reset();

	size_t
	compute_n_bins( size_t) const // to match psd::SPPack::compute_n_bins
		{
			return 5;
		}
};




class CProfile
  : public metrics::CProfile {

    public:
	CProfile (const sigfile::CSource&, int sig_no,
		  const SPPack&);

	SPPack Pp;

	const char* method() const
		{
			return metric_method( TType::mc);
		}

	int go_compute();
	string mirror_fname() const;

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

	// to enable use as mapped type
	CProfile (const CProfile& rv)
	      : metrics::CProfile (rv)
		{}
};


} // namespace mc
} // namespace metrics

#endif // _METRICS_MC_H

// eof
