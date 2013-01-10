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
	double	freq_from,
		freq_inc;
	size_t	n_bins;

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
				smooth_side == rv.smooth_side &&
				freq_from == rv.freq_from &&
				freq_inc == rv.freq_inc &&
				n_bins == rv.n_bins;
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
			freq_from = rv.freq_from;
			freq_inc = rv.freq_inc;
			n_bins = rv.n_bins;
		}

	void check() const; // throws
	void reset();

	size_t
	compute_n_bins( size_t) const // to match psd::SPPack::compute_n_bins
		{
			return n_bins;
		}
};




class CProfile
  : public metrics::CProfile {

    public:
	CProfile (const sigfile::CSource&, int sig_no,
		  const SPPack&);

	SPPack Pp;

	const char* metric_name() const
		{
			return metrics::name( TType::mc);
		}

	valarray<TFloat> course( double binf) const
		{
			size_t	bin = agh::alg::value_within(
				(int)((binf - Pp.freq_from) / Pp.freq_inc),
				0, (int)bins()-1);
			return metrics::CProfile::course(bin);
		}

	int go_compute();
	string mirror_fname() const;

	string fname_base() const;
	int export_tsv( const string& fname) const;
	int export_tsv( size_t bin,
			const string& fname) const;

	// to enable use as mapped type
	CProfile (const CProfile& rv)
	      : metrics::CProfile (rv)
		{}
};



// mc.ii
// computation stages

template <typename T>
pair<valarray<T>, valarray<T>>
do_sssu_reduction( const valarray<T>&,
		   size_t, double, double, double,
		   double, double, double);

extern const size_t sssu_hist_size;

extern template
pair<valarray<TFloat>, valarray<TFloat>>
do_sssu_reduction( const valarray<TFloat>&,
		   size_t, double, double, double,
		   double, double, double);

template <typename T>
pair<valarray<T>, valarray<T>>
do_sssu_reduction( const valarray<T>& S,
		   size_t samplerate, double scope,
		   double mc_gain, double iir_backpolate,
		   double f0, double fc,
		   double bandwidth)
{
	sigproc::CFilterDUE<T>
		due_filter (samplerate, sigproc::TFilterDirection::forward,
			    mc_gain, iir_backpolate,
			    fc);
	sigproc::CFilterSE<T>
		se_filter (samplerate, sigproc::TFilterDirection::forward,
			   mc_gain, iir_backpolate,
			   f0, fc,
			   bandwidth);

	size_t	integrate_samples = scope * samplerate,
		lpages = S.size() / integrate_samples;
	valarray<T>
		due_filtered = due_filter.apply( S, false),
		se_filtered  =  se_filter.apply( S, false);

	valarray<T>
		ss (lpages),
		su (lpages);
	for ( size_t p = 0; p < lpages; ++p ) {
		auto range = slice (p * integrate_samples, integrate_samples, 1);
		su[p] =
			(valarray<T> {due_filtered[range]} * valarray<T> {se_filtered[range]})
			.sum() / integrate_samples;
		ss[p] =
			pow(valarray<T> {se_filtered[range]}, (T)2.)
			.sum() / samplerate / integrate_samples;
	}

	return {su, ss};
}



} // namespace mc
} // namespace metrics

#endif // _METRICS_MC_H

// eof