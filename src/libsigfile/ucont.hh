// ;-*-C++-*-
/*
 *       File name:  libsigfile/ucont.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  CBinnedMicroConty ("EEG microcontinuity")
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_UCONT_H
#define _SIGFILE_UCONT_H

#include "../libsigproc/ext-filters.hh"
#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigfile {



struct SMicroContyParamSet {

	size_t	pagesize;

	TFloat	duefilter_minus_3db_frequency;

	int	ss_su_min,
		ss_su_max;
	size_t	piB_correlation_function_buffer_size;

	SMicroContyParamSet& operator=( const SMicroContyParamSet& rv) = default;
	bool operator==( const SMicroContyParamSet& rv) const
		{
			return pagesize == rv.pagesize &&
				duefilter_minus_3db_frequency == rv.duefilter_minus_3db_frequency &&
				ss_su_min == rv.ss_su_min &&
				ss_su_max == rv.ss_su_max &&
				piB_correlation_function_buffer_size == rv.piB_correlation_function_buffer_size;
		}
	bool validate()
		{
			return true;
		}

	SMicroContyParamSet( const SMicroContyParamSet& rv) = default;
	SMicroContyParamSet() = default;
};



class CBinnedMicroConty
  : public CPageMetrics_base, SMicroContyParamSet {

	CBinnedMicroConty() = delete;

    protected:
	CBinnedMicroConty( const CSource& F, int sig_no,
			   const SMicroContyParamSet &params)
	      : CPageMetrics_base (F, sig_no, params.pagesize, 1),
		SMicroContyParamSet (params),
		due_filter (params.duefilter_minus_3db_frequency,
			    samplerate()),
		se_filter (samplerate())
		{}

    public:
	int compute( const SMicroContyParamSet& req_params,
		     bool force = false);
	int compute( bool force = false)
		{
			return compute( *this, force);
		}

	string fname_base() const;

    private:
	sigproc::CFilterDUE
		due_filter;
	sigproc::CFilterSE
		se_filter;

	struct SMCJump {
		bool	processed;
		size_t	samples;
		TFloat	size;
	};
	SMCJump	LastMCJump;
	int	MinSamplesBetweenJumps,
		MaxSamplesHalfJump,
		MCEventDurationSamples,
		MCEventThreshold;
	TFloat	MCjumpThreshold,
		PiBExpInt;
	short	PiBLogConv;
	TFloat	SUsmooth,
		SSsmooth;
};


} // namespace sigfile


#endif // _SIGFILE_UCONT_H

// eof
