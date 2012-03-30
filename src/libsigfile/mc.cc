// ;-*-C++-*-
/*
 *       File name:  libsigfile/ucont.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  CBinnedMC ("EEG microcontinuity")
 *
 *         License:  GPL
 */

//#include <gsl/gsl_math.h>
#include <cassert>
#include <functional>

#include "../common/misc.hh"
#include "mc.hh"
#include "source.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


string
sigfile::CBinnedMC::
fname_base() const
{
	DEF_UNIQUE_CHARP (_);
	assert (asprintf( &_,
			  "%s-%s-%zu-%g-%c%c-%zu",
			  _using_F.filename(), _using_F.channel_by_id(_using_sig_no),
			  _using_F.pagesize(), 42.,
			  'a'+(char)0,
			  'a'+(char)0,
			  _signature) > 1);
	string ret {_};
	return ret;
}




inline int
heaviside( TFloat x)
{
	return x > 0.;
}






void
sigfile::SMCParamSet::
check() const throw (invalid_argument)
{
        if ( xpi_bplus < 1 || xpi_bminus > -1 || xpi_bzero < 1 || art_max_secs < 1 )
		throw invalid_argument ("Artifact thresholds or -spread incorrect");
        /*
        if (appConf.MCEventDuration < 1)
            appError.Add(string.Format("MCEventSamples = {0} but should be >= 1", appConf.MCEventDuration), NeuroLoopGainController.defaultErrorMessageId);
        */
        //TODO: Review the following modifications in order to allow higher output sampling rate
        if ( mc_event_duration < 1 )
		throw invalid_argument ("mc_event_duration must be >= 1");
        // if (PiBExpInt < AppConf.LogFloatY0 * 10)
        //   AppError.Add(string.Format("piB = {0} is smaller than LogFloat_Y0*10", PiBExpInt), NeuroLoopGainController.DefaultErrorMessageId);
        if ( mc_gain < 1.0 )
		throw invalid_argument ("mc_gain must be >= 1.0");
        if ( smooth_rate <= 0.0 || smooth_rate >= 1.0 )
		throw invalid_argument ("smooth_rate must be > 0.0 and < 1.0");
}




int
sigfile::CBinnedMC::
compute( const SMCParamSet& req_params,
	 bool force)
{
	hash_t req_signature = _using_F.artifacts( _using_sig_no).dirty_signature();
	if ( have_data() && (*this) == req_params
	     && _signature == req_signature )
		return 0;

	//cout << "Performing SU and SS reduction...\n";
	// DoSSSUReduction();
	do_sssu_reduction();

	//cout << "Computing PiB value...\n";
	do_detect_pib();

	//cout << "Detecting artifacts...";
	// DoComputeArtifactTraces();
	do_compute_artifact_traces();

	//cout << "Smoothing SU and SS...";
	do_smooth_sssu();
	
	//pbf.Message = "Detecting events...";
	// DoDetectMCEvents();
	
	//pbf.Message = "Re-smoothing signals and detecting jumps...";
	// Re-smooth SS and SU rejecting MC events
	// DoResmoothSSSU();
	
	//pbf.Message = "Computing final gains...";
	// DoComputeMC();
	

	return 0;
}


void
sigfile::CBinnedMC::
do_sssu_reduction()
{
	size_t	total_samples = pages() * CPageMetrics_base::pagesize() * samplerate();
	valarray<TFloat>
		due_filtered (total_samples),  // massive swap in 3..2..1..
		se_filtered  (total_samples);
	{
		auto signal = _using_F.get_signal_filtered(_using_sig_no);
		due_filter.apply( signal, due_filtered, 0, total_samples, 0);
		se_filter.apply( signal,  se_filtered, 0, total_samples, 0);
	}

	size_t	integrate_samples = SMCParamSet::pagesize * samplerate();
	for ( size_t p = 0; p < pages(); ++p ) {
		auto range = slice (p * integrate_samples, 1, integrate_samples);
		su[p] =
			(valarray<TFloat> {due_filtered[range]} * valarray<TFloat> {se_filtered[range]})
			.sum()
			/ SMCParamSet::pagesize;
		ss[p] =
			pow(valarray<TFloat> {se_filtered[range]}, (TFloat)2.)
			.sum() / samplerate()
			/ SMCParamSet::pagesize;
	}
}




void
sigfile::CBinnedMC::
do_detect_pib()
{
	valarray<TFloat>
		sssu (ss_su_max - ss_su_min + 1),
		sssu_smoothed (sssu.size());
	for ( size_t p = 0; p < pages(); ++p ) {
		int j = round( log( ss[p] - su[p]));
		if ( j != 0 && ss_su_min <= j && j < ss_su_max )
			++sssu[j - ss_su_min];
	}

	// if ( show_pib_histogram ) {
	{
		valarray<TFloat>
			sssu_match    (sssu.size()),
			sssu_template (pib_correlation_function_buffer_size);
		/*
		 * 2*SS_SUsmoother applied to the logarithmic converted values in the histogram
		 * corresponds to the original values of piBPeakWidth
		 */
		// Apply the smoothing (mean filter) to the histogram
		int sssu_smoother_width = log(1.0 + pib_peak_width) / _a / 2;
		for ( int k = sssu_smoother_width; k < (int)sssu.size() - sssu_smoother_width; ++k ) {
			for ( int j = k - sssu_smoother_width; j <= k + sssu_smoother_width; ++j )
				sssu_smoothed[k] += sssu[j];
			sssu_smoothed[k] = sssu_smoothed[k] / (2 * sssu_smoother_width + 1);
		}

		// todo: Marco: "gewoon" maximum zoeken, geen moeilijke functies.
		// geweldig, епта!

		// Construct the template to detect desired piB value peak in the smoothed histogram
		for ( size_t k = 1; k < sssu_template.size() - 1; ++k ) {
			auto	x = ((TFloat)k / (sssu_template.size() - 1)) * 3 * M_PI - 2 * M_PI;
			sssu_template[k] =
				2.0 / 3 * -0.5894 * (heaviside(x + 2 * M_PI) - heaviside(x + (M_PI / 2)))
				+ (heaviside(x + (M_PI / 2)) - heaviside(x)) * (sin(2 * x) / (2 * x))
				+ (heaviside(x) - heaviside(x - M_PI / 2)) * (sin(2 * x) / (2 * x));
		}

		// Calculate and substract template mean value and calculate resulting template's peak index
		sssu_template -= sssu_template.sum() / sssu_template.size();

		TFloat	template_peak_value = 0.;
		size_t	template_peak_idx = 0;
		for ( size_t k = 0; k < sssu_template.size(); ++k )
			if ( sssu_template[k] > template_peak_value ) {
				template_peak_idx = k;
				template_peak_value = sssu_template[k];
			}

		// Calculate correlation coefficients by shifting the template over the histogram
		for ( size_t k = 0; k < (sssu_smoothed.size() - sssu_template.size()); ++k ) {
			TFloat	v = 0.;
			for ( size_t j = 0; j < sssu_template.size(); ++j )
				v += sssu_template[j] * sssu_smoothed[k + j];
			sssu_match[k + template_peak_idx] = v;
		}

		TFloat	peak = 0.;
		size_t	peak_at = 0;
		// We'll take Pi*B as the x-value for the maximum correlation point
		for ( size_t k = 0; k < sssu_match.size(); ++k )
			if ( sssu_match[k] > peak ) {
				peak = sssu_match[k];
				peak_at = k;
			}
		pib = EXP( peak_at + ss_su_min);

		// // Show histogram
		// HistogramInfo histogramInfo = new HistogramInfo
		// 	{
		// 		PiBvalueLog = log_pib,
		// 		PiBvaluePhysi = PiBxx,
		// 		SmoothRate = smooth_rate,
		// 		SmoothTime = SMCParamSet::pagesize,
		// 	};
	}

}

void
sigfile::CBinnedMC::
do_compute_artifact_traces()
{
	mc_smooth( TSmoothOptions::GetArtifactsResetAll);
}


void
sigfile::CBinnedMC::
do_smooth_sssu()
{
	mc_smooth( TSmoothOptions::Smooth);
}




void
sigfile::CBinnedMC::
mc_smooth_update_artifacts( bool smooth_reset, size_t p)
{
	if ( smooth_reset )
		art_hf = art_lf = art_zero = 0;

	TFloat art_factor = value_within((ss[p] - su[p] - pib) / pib, -1000., 1000.); // Avoid overflow of art_HF

	if ( art_factor >= xpi_bplus ) // XpiBPlus >= 1
		// todo: Bob controleren art_HF, art_LF en art_Zero: eerst afronden daarna *SmoothTime ?
		art_hf += round( art_factor / xpi_bplus) * SMCParamSet::pagesize;
	else
		art_hf -= SMCParamSet::pagesize;

	ensure_within( art_hf, (TFloat)0., art_max_secs);

	if ( art_factor <= xpi_bminus)
		art_lf += round( art_factor / xpi_bminus) * SMCParamSet::pagesize;
	else
		art_lf -= SMCParamSet::pagesize;
	ensure_within( art_lf, 0., art_max_secs);

	if ( ss[p] <= pib / xpi_bzero )
		art_zero += round( (pib / xpi_bzero) - ss[p]) * SMCParamSet::pagesize;
	else
		art_zero -= SMCParamSet::pagesize;

	ensure_within( art_zero, 0., min( (size_t)1, SMCParamSet::pagesize));
}

void
sigfile::CBinnedMC::
mc_smooth_reset_all( size_t p)
{
	su_plus[p] = su_minus[p] = ss_plus[p] = ss_minus[p] = ssp[p] = ss0[p] = 0.;

	hf_art[p] = round( art_hf / art_phys_dim_res);
	lf_art[p] = round( art_lf / art_phys_dim_res);
	missing_signal[p] = round( art_zero / art_phys_dim_res);

	mc[p] = mc_jump[p] = mc_event[p] = 0;
}

void
sigfile::CBinnedMC::
mc_smooth_reset_all()
{
	su_plus = su_minus = ss_plus = ss_minus = ssp = ss0 = 0.;

	hf_art = round( art_hf / art_phys_dim_res);
	lf_art = round( art_lf / art_phys_dim_res);
	missing_signal = round( art_zero / art_phys_dim_res);

	mc = mc_jump = mc_event = 0;
}






void
sigfile::CBinnedMC::
mc_smooth( TSmoothOptions option)
{
	smp.mc_event_duration_samples = mc_event_duration * samplerate();
	smp.min_samples_between_jumps = (size_t)round(1. / (smooth_rate * SMCParamSet::pagesize)) + 1;
	smp.mc_jump_threshold         = (mc_jump_find / SMCParamSet::pagesize) * 100 * mc_gain;
	smp.mc_event_threshold        = round((mc_event_reject / SMCParamSet::pagesize) * smooth_rate * 100 * mc_gain);

	_suForw.resize( smp.mc_event_duration_samples + 1);
	_suBack.resize( smp.mc_event_duration_samples + 1);
	_ssForw.resize( smp.mc_event_duration_samples + 1, pib);
	_ssBack.resize( smp.mc_event_duration_samples + 1, pib);

	size_t p;
      // traverse forward
	bool	smooth_reset = false;
	switch (option) {
	case GetArtifactsResetAll:
		for ( p = 0; p < pages(); ++p )
			// UpdateArtifacts uses SS,SU,SmoothReset,XpiBArt,ArtSpreadSamples
			mc_smooth_update_artifacts( smooth_reset, p);
		mc_smooth_reset_all();
	    break;
	case DetectEventsResetJumps:
		for ( p = 0; p < pages(); ++p )
			mc_smooth_detect_events_reset_jumps( p, TDirection::Forward);
	    break;
	case Smooth:
		for ( p = 0; p < pages(); smooth_reset = false, ++p )
			mc_smooth_forward( p, smooth_reset, false);
	    break;
	case SmoothResetAtJumps:
		for ( p = 0; p < pages(); smooth_reset = false, ++p )
			mc_smooth_forward( p, smooth_reset, true);
		break;
	}

      // now go backward
	smooth_reset = true;
	_suForw = _suBack = 0.;
	_ssForw = _ssBack = pib;

	switch (option) {
	case GetArtifactsResetAll:
		for ( p = pages()-1; p > 0; --p ) {
			// UpdateArtifacts uses SS,SU,SmoothReset,XpiBArt,ArtSpreadSamples
			mc_smooth_update_artifacts( smooth_reset, p);
			hf_art[p] += art_hf;
			lf_art[p] += art_lf;
			missing_signal[p] += art_zero;
		}
	    break;
	case DetectEventsResetJumps:
		for ( p = pages()-1; p > 0; --p )
			mc_smooth_detect_events_reset_jumps( p, TDirection::Back);
	    break;
	case Smooth:
		for ( p = pages()-1; p > 0; smooth_reset = false, --p )
			mc_smooth_backward( p, smooth_reset, false);
	    break;
	case SmoothResetAtJumps:
		for ( p = pages()-1; p > 0; smooth_reset = false, --p )
			mc_smooth_backward( p, smooth_reset, true);
	    break;
	}
}






void
sigfile::CBinnedMC::
mc_smooth_backward( size_t p, bool& smooth_reset, bool reset_at_jumps)
{
	size_t	n;
	if ( reset_at_jumps ) {
		if ( smooth_reset )
			lmj = { true, p, smp.mc_jump_threshold };
		if ( abs(mc_jump[p]) >= abs(lmj.size) )
			lmj = { false, p, (TFloat)mc_jump[p] };
		if ( !lmj.processed ) {
			size_t m = lmj.at;
			if ( m - p >= smp.min_samples_between_jumps ||
			     mc_jump[p] / lmj.size < 0 ) {  // sorry, this cries foul to me

				// Jump complete: initialize its processing
				// Get 'past' (at n) reset values from smoother
				n = max( 0ul, m - smp.max_samples_half_jump());
				// Reset backward smoother to 'past' forward-smoothed state
				// TODO: Check if next line is still valid for the case of MCEventDuration > 1
				su_smooth = _suBack[0] = su_plus[n]; // SU+
				ss_smooth = _ssBack[0] = ss_plus[n]; // SS+

				// Go to jump (at m) but preserve 1 sample of the jump
				n = max(p, m - 1); // smoother will start at ifileSampleNr = LastJump - 1
				lmj = { true, p, smp.mc_jump_threshold };
			}
		}
	}
	n = min( p - n, smp.max_samples_half_jump()); // Reset jump-sample counter

	for ( ; n >= p; --n ) {
		bool artifact =
			(hf_art[n] > 0 ||
			 lf_art[n] > 0 ||
			 missing_signal[n] > 0 ||
			 abs(mc_event[n]) > smp.mc_event_threshold);
		if ( reset_at_jumps && n > 0 ) {
			artifact = true;
			--n;
		}
		// SU and SS back-smoothed into SU- and SS-
		mc_smooth_suss( n, artifact, smooth_reset); // sets su_smooth, ss_smooth
		su_minus[n] = su_smooth;
		ss_minus[n] = ss_smooth;
		_suBack[1] = _suBack[0];
		_ssBack[1] = _ssBack[0];
		smooth_reset = false;
		//FSUback[0] = MathEx.ExpInteger(outBuffer[outputBufferOffsets[4] + idataBlockSample], appConf.LogFloatY0, appConf.LogFloatA);
		//FSSback[0] = MathEx.ExpInteger(outBuffer[outputBufferOffsets[6] + idataBlockSample], appConf.LogFloatY0, appConf.LogFloatA);
		_suBack[0] = su_smooth;
		_ssBack[0] = ss_smooth;
		_suForw[0] = su_plus[n];
		_ssForw[0] = ss_plus[n];

		// Combine for -and backwards to symmetric SSP, SS0, MC and MCjump
		ssp[n] = _ssForw[0] + _ssBack[1];
		mc[n] = (ssp[n] <= 0.) ? 0. : (_suBack[1] + _suForw[0]) / ssp[n];
		ssp[n] /= 2;
		ss0[n] = ssp[n] * (1. - mc[n]);
		mc[n] *= mc_gain * 100;
	}
}



void
sigfile::CBinnedMC::
mc_smooth_forward( size_t p, bool& smooth_reset, bool reset_at_jumps)
{
	size_t	n;

	if ( reset_at_jumps ) {
		if ( smooth_reset )
			lmj = { true, p, smp.mc_jump_threshold };
		if (abs(mc_jump[p]) >= abs(lmj.size))
			lmj = { false, p, (TFloat)mc_jump[p] };
		if ( !lmj.processed ) {
			size_t	m = lmj.at;
			if ( p - m >= smp.min_samples_between_jumps ||
			     mc_jump[p] / lmj.size < 0 ) {
				// Jump complete: initialize its processing
				// Get 'future' (at n) resetvalues from smoother
				n = min( m + smp.max_samples_half_jump(), pages() - 1);
				// Reset forward smoother to 'future' back-smoothed state
				su_smooth = su_minus[n];
				ss_smooth = ss_minus[n];

				// Go to jump (at m) but preserve 1 sample of the jump
				n = min( m, p); // smoother will start at ifileSampleNr = LastJump + 1
				lmj = {true, p, smp.mc_jump_threshold };
			}
		}
	}
	n = min( p - n, smp.max_samples_half_jump()); // Reset jump-sample counter

	for ( ; n <= p; ++n ) {
		bool artifact;
		if ( reset_at_jumps && n > 0 ) {
			artifact = true;
			n--;
		} else
			artifact = (hf_art[n] > 0 ||
				    lf_art[n] > 0 ||
				    missing_signal[n] > 0 ||
				    abs(mc_event[n]) > mc_event_threshold);
        // SU and SS forward-smoothed into SU+ and SS+
        double s;
        double r;
        MCSmooth_SmoothSUSS(MathEx.ExpInteger(outBuffer[OutputBufferOffsets[1] + idataBlockSample], AppConf.LogFloatY0, AppConf.LogFloatA),
            MathEx.ExpInteger(outBuffer[OutputBufferOffsets[2] + idataBlockSample], AppConf.LogFloatY0, AppConf.LogFloatA),
            out r, out s, artifact, smooth_reset);
        outBuffer[OutputBufferOffsets[3] + idataBlockSample] = MathEx.LogFloat(r, AppConf.LogFloatY0, AppConf.LogFloatA);
        outBuffer[OutputBufferOffsets[5] + idataBlockSample] = MathEx.LogFloat(s, AppConf.LogFloatY0, AppConf.LogFloatA);
        smooth_reset = false;
        _suForw[1] = _suForw[0];
        _ssForw[1] = _ssForw[0];
        _suBack[0] = MathEx.ExpInteger(outBuffer[OutputBufferOffsets[4] + idataBlockSample], AppConf.LogFloatY0, AppConf.LogFloatA);
        _ssBack[0] = MathEx.ExpInteger(outBuffer[OutputBufferOffsets[6] + idataBlockSample], AppConf.LogFloatY0, AppConf.LogFloatA);
        //FSUforw[0] = MathEx.ExpInteger(outBuffer[outputBufferOffsets[3] + idataBlockSample], appConf.LogFloatY0, appConf.LogFloatA); //TODO: substitute by r (Log conversion always implies a loss)
        //FSSforw[0] = MathEx.ExpInteger(outBuffer[outputBufferOffsets[5] + idataBlockSample], appConf.LogFloatY0, appConf.LogFloatA); //TODO: substitute by s (Log conversion always implies a loss)
        _suForw[0] = r;
        _ssForw[0] = s;
        // This MCjump is biased but OK for detection of jumps
        double ssp = _ssForw[1] + _ssBack[0];
        if (ssp <= 0)
          mcJump = 0;
        else
          mcJump = (_suBack[0] - _suForw[1]) / ssp;
        /*
          The below computation of MCjump is more correct but the
          variance of the result strongly depends on MC. Therefore
          the threshold should adapt to this variance: a bridge too far
          r6:=1.0*SSforw[1]*SSback[0];
          if(r6 < 1.0) then
            MCjump:=0.0
          else
            MCjump:=(1.0*SUback[0]*SSforw[1]-SUforw[1]*SSback[0])/r6;
         */

        /*MCjump = MicGain * 100 * MCjump;
        if (abs(MCjump) > short.MaxValue)
            MCjump = Math.Sign(MCjump) * short.MaxValue;*/
        mcJump = Range.EnsureRange(AppConf.MicGain * 100 * mcJump, -short.MaxValue, short.MaxValue);
        outBuffer[OutputBufferOffsets[13] + idataBlockSample] = (short)MathEx.RoundNearest(mcJump);
        ifileSampleNr++;
        idataBlockSample++;
      }
    }





void
sigfile::CBinnedMC::
mc_smooth_suss( size_t n,
		bool artifact, bool smoother_reset)
{
      // Recursive smoother of SUin and SSin by updating internal variable by 'smoothrate'
      // Lowlimits for output SUout and SSout are 0 and piB respectively
      // SU and SS keep the internal state of smooth with double precision
	if ( smoother_reset ) {
		su_smooth = 0;
		ss_smooth = pib;
	}
	if ( artifact ) {
		su_smooth = (1 - smooth_rate) * su_smooth;
		ss_smooth = (1 - smooth_rate) * ss_smooth;
        /* Be carefull with changing the following lower limit on SS. These influences
         * the zero-signal detection by ArtZero and XpiBArtZero
         */
		if ( ss_smooth < pib )
			ss_smooth = pib;
	} else {
		auto	SU = su[n],
			SS = ss[n];
		TFloat	dSU = (SU > -pib)
				? SU - su_smooth
				: -pib - su_smooth, // Clip SUin at -piB: mitigate artifacts
			dSS = SS - ss_smooth;
		su_smooth = max((TFloat)0., su_smooth + smooth_rate * dSU);
		ss_smooth += smooth_rate * dSS;
		/* Be carefull with changing the following lower limit on SS. These influences
		 * the zero-signal detection d.m.v. ArtZero and XpiBArtZero
		 */
		if ( ss_smooth < pib )
			ss_smooth = pib;
	}
}


void
sigfile::CBinnedMC::
mc_smooth_detect_events_reset_jumps( size_t at, TDirection direction)
{
	//TODO: Review the following modifications in order to allow higher output sampling rate
	size_t kz = mc_event_duration * samplerate();
	for ( size_t k = kz; k > 0; --k ) {
		_suForw[k] = _suForw[k - 1];
		_suBack[k] = _suBack[k - 1];
		_ssForw[k] = _ssForw[k - 1];
		_ssBack[k] = _ssBack[k - 1];
	}
	_suForw[0] = su_plus [at];
	_suBack[0] = su_minus[at];
	_ssForw[0] = ss_plus [at];
	_ssBack[0] = ss_minus[at];

	if ( direction == TDirection::Forward ) {
		//TODO: Review the following modifications in order to allow higher output sampling rate
		mc_jump [at] = (_suBack[kz] - _suForw[kz]) / 2;
		mc_event[at] = (_ssBack[kz] + _ssForw[kz]) / 2;
	} else {
		TFloat	r = E??XP(mc_jump [at]) + (_suForw[kz] - _suBack[kz]) / 2,
			s = EXP(mc_event[at]) + (_ssForw[kz] + _ssBack[kz]) / 2;
		mc_event[at] = round( clamp(100 * mc_gain * r / s, (TFloat)-INT_MAX, (TFloat)INT_MAX));
		// Clean MCJump from this temporary storage necessary for MCevent
		mc_jump [at] = 0;
	}
	/* BK 22.3.96: event detection as below stronger discriminate
	   bigger from smaller K-complexes. However, the small ones are
	   still clear. So I decided for the above procedure.
	   MCjump:=0.0;
	   MCevent:=(1.0*SUmin [MCEventSamples]/SSmin [MCEventSamples]) - (1.0*SUplus[MCEventSamples]/SSplus[MCEventSamples]);
	   MCevent:=MCevent*100.0*MCgain;
	   end of BK 22.3.96
	*/
	// wow
}





// eof
