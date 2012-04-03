// ;-*-C++-*-
/*
 *       File name:  libsigfile/ucont.hh
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

	size_t	pagesize;

      // orignial ones
	// filters
	// double	duefilter_minus_3db_frequency; // = fc

	// 'App' settings
	int	xpi_bplus,			// = 9;		// >0. HF artifact if [SS-SDU-piB]/piB >= XpiBplus : st. 9
		xpi_bminus,			// = -9;	// <0. LF artifact if [SS-SDU-piB]/piB <= XpiBminus: st.-9
		xpi_bzero;			// = 10;	// >0. No signal if piB/[SS-SDU] >= XpiBzero : st.10
	double	iir_backpolate;			// = 0.5;	// 0.0 < Backpolate < 1.0 on s: standard 0.5
	int	ss_su_min,			// -2000	// min value for hystogram's x-axis
		ss_su_max;			// 30000	// max value for hystogram's x-axis
	//int	piBSecsSure;			// = 1200;	// Surely artifact-free seconds in InFile: st.1200
	double	pib_peak_width;			// = 0.2;	// Peak width as a fraction (0..1) of piB: st.0.2

	double	mc_gain,			// = 10.0;	// Gain (DigiRange/PhysiRange) of MicroContinuity
		art_max_secs;			// = 7;		// Maximum 'spread' (in s) of an artifact: st.7

	size_t	mc_event_duration;		// = 1;		// 0..MCEventMaxDur: expected duration MC-event: st.1
	double	mc_event_reject,		// = 2.0;	// >0.0. Reject if Event>MCEvRej*SmRate*100%: st.2.0
		mc_jump_find;			// = 0.5;	// Reset smoother at jumps > MCjumpFind*100%: st.0.5

	static const size_t
		pib_correlation_function_buffer_size = 6000;

	double	f0, // = 1.,
		fc, // = 1.8;
		band_width; // = 1.5;
	//int IIRUnderSampler = 0;
	double	//suss_smoothing_time, // = 1;
		smooth_rate; // = 1./60
		//safety_factor; // = 3;

	SMCParamSet& operator=( const SMCParamSet& rv) = default;
	bool operator==( const SMCParamSet& rv) const
		{
			return	pagesize == rv.pagesize &&
				//duefilter_minus_3db_frequency == rv.duefilter_minus_3db_frequency &&
				xpi_bplus == rv.xpi_bplus &&
				xpi_bminus == rv.xpi_bminus &&
				xpi_bzero == rv.xpi_bzero &&
				iir_backpolate == rv.iir_backpolate &&
				ss_su_min == rv.ss_su_min &&
				ss_su_max == rv.ss_su_max &&
				pib_peak_width == rv.pib_peak_width &&
				mc_gain == rv.mc_gain &&
				art_max_secs == rv.art_max_secs &&
				mc_event_duration == rv.mc_event_duration &&
				mc_event_reject == rv.mc_event_reject &&
				mc_jump_find == rv.mc_jump_find &&
				pib_correlation_function_buffer_size == rv.pib_correlation_function_buffer_size &&
				f0 == rv.f0 &&
				fc == rv.fc &&
				band_width == rv.band_width &&
				//suss_smoothing_time == rv.suss_smoothing_time &&
				smooth_rate == rv.smooth_rate;
				// safety_factor == rv.safety_factor;
		}
	void check() const;

	SMCParamSet( const SMCParamSet& rv) = default;
	SMCParamSet() = default;

	static constexpr TFloat
		_a = 0.001,
		_y0 = 0.0001;
	static TFloat EXP( TFloat v)
		{
			// kill this with fire
			if ( v > 0. )
				return _y0 * exp(_a * v);
			else if ( v < 0. )
				return -_y0 * exp(-_a * v);
			else
				return 0.;

		}
	static int LOG( TFloat v)
		{
			return round( (v > _y0)
				? log(v) - log(_y0)
				: -log(-v) + log(_y0)) / _a;
		}
};



class CBinnedMC
  : public CPageMetrics_base, SMCParamSet {

	CBinnedMC() = delete;

    protected:
	CBinnedMC( const CSource& F, int sig_no,
		   const SMCParamSet &params)
	      : CPageMetrics_base (F, sig_no, params.pagesize, 1),
		SMCParamSet (params),
		ss (pages()),
		su (pages()),
		su_plus  (pages()),
		su_minus (pages()),
		ss_plus  (pages()),
		ss_minus (pages()),
		ssp (pages()),
		ss0 (pages()),
		hf_art (pages()),
		lf_art (pages()),
		missing_signal (pages()),
		mc (pages()),
		mc_jump (pages()),
		mc_event (pages()),
		due_filter (params.fc, samplerate()),
		se_filter (samplerate())
		{
			SMCParamSet::check(); // throw if not ok
		}

    public:
	string fname_base() const;

	int compute( const SMCParamSet& req_params,
		     bool force = false);
	int compute( bool force = false)
		{
			return compute( *this, force);
		}

      // essential computed variables
	TFloat	pib;
	// TFloat pib() const
	// 	{
	// 		return EXP(log_pib);
	// 	}

	valarray<TFloat>
		ss,		su,
		su_plus,	su_minus,
		ss_plus,	ss_minus,
		ssp,
		ss0;
	valarray<int>
		hf_art,
		lf_art,
		missing_signal,
		mc,
		mc_jump,
		mc_event;
    private:
      // internal types
	struct SMCJump {
		bool	processed;
		size_t	at,
			size;
	};
	struct SSmoothParams {
		size_t	mc_event_duration_samples,
			min_samples_between_jumps;
		size_t max_samples_half_jump() const
			{
				return min_samples_between_jumps / 20 + 1;
			}
		size_t	mc_jump_threshold,
			mc_event_threshold;
	};
	enum TSmoothOptions {
		GetArtifactsResetAll,
		DetectEventsResetJumps,
		Smooth,
		SmoothResetAtJumps
	};
	enum TDirection {
		Forward, Back
	};

      // computation stages
	void do_sssu_reduction();
	void do_detect_pib();
	void do_compute_artifact_traces();
	void do_smooth_sssu();

      // odd variables we hold and carry between stages
	sigproc::CFilterDUE
		due_filter;
	sigproc::CFilterSE
		se_filter;

	valarray<TFloat>
		_suForw, _suBack,
		_ssForw, _ssBack;
	TFloat	art_lf,
		art_hf,
		art_zero,
		su_smooth,
		ss_smooth;

	SMCJump	lmj;
	SSmoothParams
		smp;

      // helpers
	void mc_smooth_reset_all( size_t);
	void mc_smooth_reset_all();
	void mc_smooth_update_artifacts( bool, size_t);
	void mc_smooth_detect_events_reset_jumps( size_t at, TDirection);
	void mc_smooth( TSmoothOptions);
	void mc_smooth_forward( size_t, bool&, bool);
	void mc_smooth_backward( size_t, bool&, bool);
	void mc_smooth_suss( size_t,  // sets su_smooth, ss_smooth
			     bool artifact, bool smoother_reset);
      // various const
	static constexpr TFloat art_phys_dim_res = 1.;
};


} // namespace sigfile


#endif // _SIGFILE_UCONT_H

// eof
