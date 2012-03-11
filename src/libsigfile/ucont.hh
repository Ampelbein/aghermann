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

	int	iir_undersampler;
	TFloat	minus_3db_frequency;

	SMicroContyParamSet& operator=( const SMicroContyParamSet& rv)
		{
			return *this;
			// don't touch samplerate
		}
	bool operator==( const SMicroContyParamSet& rv) const
		{
			return false;
		}
	bool validate();

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
		due_filter (params.minus_3db_frequency,
			    samplerate() / params.iir_undersampler),
		se_filter (samplerate() / params.iir_undersampler)
		{}

    public:
      // obtain
	int compute( const SMicroContyParamSet& req_params,
		     bool force = false);
	// possibly reuse that already obtained unless factors affecting signal or fft are different
	void compute( bool force = false)
		{
			compute( *this, force);
		}

	string fname_base() const;

    private:
	sigproc::CFilterDUE
		due_filter;
	sigproc::CFilterSE
		se_filter;
	valarray<TFloat>
		_suForw, _suBack,
		_ssForw, _ssBack;
	struct SMCJump {
		bool	processed;
		int	no_sample;
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

	int DoSSSUReduction();
	int DoDetectPiB();
	int DoComputeArtifactTraces();
	int DoSmoothSSSU();
	int DoDetectMCEvents();
	int DoResmoothSSSU();
	int DoComputeMC();
};


} // namespace sigfile


#endif // _SIGFILE_UCONT_H

// eof
