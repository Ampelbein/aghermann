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

      // orignial ones
	// filters
	TFloat	duefilter_minus_3db_frequency;

	// 'App' settings
	int	xpi_bplus,			// = 9;		// >0. HF artifact if [SS-SDU-piB]/piB >= XpiBplus : st. 9
		xpi_bminus,			// = -9;	// <0. LF artifact if [SS-SDU-piB]/piB <= XpiBminus: st.-9
		xpi_bzero;			// = 10;	// >0. No signal if piB/[SS-SDU] >= XpiBzero : st.10
	TFloat	iir_backpolate;			// = 0.5;	// 0.0 < Backpolate < 1.0 on s: standard 0.5
	int	ss_su_min,			// -2000	// min value for hystogram's x-axis
		ss_su_max;			// 30000	// max value for hystogram's x-axis
	//int	piBSecsSure;			// = 1200;	// Surely artifact-free seconds in InFile: st.1200
	TFloat	pib_peakwidth;			// = 0.2;	// Peak width as a fraction (0..1) of piB: st.0.2

	TFloat	mic_gain;			// = 10.0;	// Gain (DigiRange/PhysiRange) of MicroContinuity
	int	art_max_secs;			// = 7;		// Maximum 'spread' (in s) of an artifact: st.7

	int	mc_event_duration;		// = 1;		// 0..MCEventMaxDur: expected duration MC-event: st.1
	TFloat	mc_event_reject,		// = 2.0;	// >0.0. Reject if Event>MCEvRej*SmRate*100%: st.2.0
		mc_jump_find;			// = 0.5;	// Reset smoother at jumps > MCjumpFind*100%: st.0.5

	size_t	pib_correlation_function_buffer_size; // = 6000;

	TFloat	f0, // = 1.,
		fc, // = 1.8;
		band_width; // = 1.5;
	//int IIRUnderSampler = 0;
	TFloat	suss_smoothing_time, // = 1;
		smooth_rate, // = 1./60
		safety_factor; // = 3;

	SMicroContyParamSet& operator=( const SMicroContyParamSet& rv) = default;
	bool operator==( const SMicroContyParamSet& rv) const
		{
			return	pagesize == rv.pagesize &&
				duefilter_minus_3db_frequency == rv.duefilter_minus_3db_frequency &&
				xpi_bplus == rv.xpi_bplus &&
				xpi_bminus == rv.xpi_bminus &&
				xpi_bzero == rv.xpi_bzero &&
				iir_backpolate == rv.iir_backpolate &&
				ss_su_min == rv.ss_su_min &&
				ss_su_max == rv.ss_su_max &&
				pib_peakwidth == rv.pib_peakwidth &&
				mic_gain == rv.mic_gain &&
				art_max_secs == rv.art_max_secs &&
				mc_event_duration == rv.mc_event_duration &&
				mc_event_reject == rv.mc_event_reject &&
				mc_jump_find == rv.mc_jump_find &&
				pib_correlation_function_buffer_size == rv.pib_correlation_function_buffer_size &&
				f0 == rv.f0 &&
				fc == rv.fc &&
				band_width == rv.band_width &&
				suss_smoothing_time == rv.suss_smoothing_time &&
				smooth_rate == rv.smooth_rate &&
				safety_factor == rv.safety_factor;
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
