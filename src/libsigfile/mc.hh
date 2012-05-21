// ;-*-C++-*-
/*
 *       File name:  libsigfile/mc.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  CBinnedMC ("EEG microcontinuity")
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

      // orignial ones
	// filters
	// double	duefilter_minus_3db_frequency; // = fc

	// 'App' settings
	double	iir_backpolate;			// = 0.5;	// 0.0 < Backpolate < 1.0 on s: standard 0.5
	double	f0, // = 1.,
		fc, // = 1.8;
		bandwidth, // = 1.5;
		mc_gain;

	SMCParamSet& operator=( const SMCParamSet& rv) = default;
	bool operator==( const SMCParamSet& rv) const
		{
			return	//duefilter_minus_3db_frequency == rv.duefilter_minus_3db_frequency &&
				iir_backpolate == rv.iir_backpolate &&
				f0 == rv.f0 &&
				fc == rv.fc &&
				bandwidth == rv.bandwidth &&
				mc_gain == rv.mc_gain;
				// safety_factor == rv.safety_factor;
		}
	void check( size_t pagesize) const; // throws
	void reset( size_t pagesize);

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
	string fname_base() const;

	int compute( const SMCParamSet& req_params,
		     bool force = false);
	int compute( bool force = false)
		{
			return compute( *this, force);
		}

      // essential computed variables
	// TFloat	pib;

	valarray<TFloat>
		ss,
		su;
    private:
      // computation stages
	void do_sssu_reduction();

      // odd variables we hold and carry between stages
	sigproc::CFilterDUE
		due_filter;
	sigproc::CFilterSE
		se_filter;
};


} // namespace sigfile


#endif // _SIGFILE_MC_H

// eof
