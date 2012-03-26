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

#include "../misc.hh"
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


template <typename T>
T
clamp( T& v, T l, T h)
{
	if ( v < l )
		v = l;
	else if ( v > h )
		v = h;
	return v;
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
	valarray<TFloat>
		su_buffer (pages()),
		ss_buffer (pages());
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
			su_buffer[p] =
				(valarray<TFloat> {due_filtered[range]} * valarray<TFloat> {se_filtered[range]})
				.sum()
				/ SMCParamSet::pagesize;
			ss_buffer[p] =
				pow(valarray<TFloat> {se_filtered[range]}, (TFloat)2.)
				.sum() / samplerate()
				/ SMCParamSet::pagesize;
		}
	}

	//cout << "Computing PiB value...\n";
	// DoDetectPiB();
	{
		valarray<TFloat>
			sssu (ss_su_max - ss_su_min + 1),
			sssu_smoothed (sssu.size());
		for ( size_t p = 0; p < pages(); ++p ) {
			int j = round( log( ss_buffer[p] - su_buffer[p]));
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

			log_pib = 0;
			TFloat	peak = 0.;
			// We'll take Pi*B as the x-value for the maximum correlation point
			for ( size_t k = 0; k < sssu_match.size(); ++k )
				if ( sssu_match[k] > peak ) {
					peak = sssu_match[k];
					//piB = (short)Range.EnsureRange(k + appConf.ss_su_min, -short.MaxValue, short.MaxValue);
					log_pib = k + ss_su_min;
				}

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
	//pbf.Message = "Detecting artifacts...";
	// DoComputeArtifactTraces();
	
	//pbf.Message = "Smoothing SU and SS...";
	do_smooth_sssu( ss_buffer, su_buffer, TSmoothOptions::Smooth);
	
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
do_smooth_sssu( valarray<TFloat>& ss_buffer, valarray<TFloat>& su_buffer, TSmoothOptions option)
{
	size_t	min_samples_between_jumps,
		max_samples_half_jump,
		mc_event_duration_samples;
	TFloat	mc_event_threshold,
		mc_jump_threshold;

	//TODO: Review the following modifications in order to allow higher output sampling rate
	mc_event_duration_samples = mc_event_duration * samplerate();
	min_samples_between_jumps = round(1. / (smooth_rate * SMCParamSet::pagesize)) + 1;
	max_samples_half_jump	  = (min_samples_between_jumps / 20) + 1; // Bob's 'nose'
	mc_jump_threshold	  = (mc_jump_find / SMCParamSet::pagesize) * 100 * mic_gain;
	mc_event_threshold	  = round((mc_event_reject / SMCParamSet::pagesize) * smooth_rate * 100 * mic_gain);

	valarray<TFloat>
		_suForw (mc_event_duration_samples + 1),
		_suBack (mc_event_duration_samples + 1),
		_ssForw (mc_event_duration_samples + 1, pib()),
		_ssBack (mc_event_duration_samples + 1, pib());

	bool	smooth_reset = false;
	switch (option) {
	case GetArtifactsResetAll:
		for ( size_t p = 0; p < pages(); ++p ) {
			// UpdateArtifacts uses SS,SU,SmoothReset,XpiBArt,ArtSpreadSamples
			mc_smooth_update_artifacts( smooth_reset, ss_buffer[p], su_buffer[p]);
			mc_smooth_reset_all(p);
		}
	    break;
	case DetectEventsResetJumps:
		for ( size_t p = 0; p < pages(); ++p )
			mc_smooth_detect_events_reset_jumps( p, true,
							     _suForw, _suBack,
							     _ssForw, _ssBack);
	    break;
	case Smooth:
		for ( size_t p = 0; p < pages(); smooth_reset = false, ++p )
			mc_smooth_forward( p, smooth_reset, false);
	    break;
	case SmoothResetAtJumps:
		for ( size_t p = 0; p < pages(); smooth_reset = false, ++p )
			mc_smooth_forward( p, smooth_reset, true);
	    break;
	}

	// Backward direction through EDF file
	smooth_reset = true;
	_suForw = 0.;
	_suBack = 0.;
	_ssForw = pib();
	_ssBack = pib();

	switch (option) {
	case GetArtifactsResetAll:
		for ( size_t p = pages()-1; p > 0; --p ) {
			case SmoothOption.GetArtifactsResetAll:
				// UpdateArtifacts uses SS,SU,SmoothReset,XpiBArt,ArtSpreadSamples
				mc_smooth_update_artifacts( smooth_reset, ss_buffer[p], su_buffer[p]);
				OutputEDFFile.DataBuffer[OutputBufferOffsets[9] + k1] += ArtHF;
				OutputEDFFile.DataBuffer[OutputBufferOffsets[10] + k1] += ArtLF;
				OutputEDFFile.DataBuffer[OutputBufferOffsets[11] + k1] += ArtZero;
				break;
			case SmoothOption.DetectEventsResetJumps:
				//TODO: temp for debugging
				/*if ((k == 0) && (k1 == 0))
				  {
				  int llegamos = 1;
				  }*/
				MCSmooth_DetectEvents_ResetJumps(OutputEDFFile.DataBuffer, k1, false);
				break;
			case SmoothOption.Smooth:
				MCSmooth_DoSmooth_Backward(OutputEDFFile.DataBuffer, k, k1, fileSampleNr, ref smooth_reset, false);
				break;
			case SmoothOption.SmoothResetAtJumps:
				MCSmooth_DoSmooth_Backward(OutputEDFFile.DataBuffer, k, k1, fileSampleNr, ref smooth_reset, true);
				break;
			}
			smooth_reset = false;
		}
	}

}



void
sigfile::CBinnedMC::
mc_smooth_update_artifacts( bool smooth_reset, TFloat ss, TFloat su)
{
	if ( smooth_reset )
		art_hf = art_lf = art_zero = 0;

	TFloat art_factor = clamp((ss - su - pib()) / pib(), -1000., 1000.); // Avoid overflow of art_HF

	if ( art_factor >= xpi_bplus ) // XpiBPlus >= 1
		// todo: Bob controleren art_HF, art_LF en art_Zero: eerst afronden daarna *SmoothTime ?
		art_hf += round( art_factor / xpi_bplus) * SMCParamSet::pagesize;
	else
		art_hf -= SMCParamSet::pagesize;

	clamp( art_hf, 0., art_max_secs);

	if ( art_factor <= xpi_bminus)
		art_lf += round( art_factor / xpi_bminus) * SMCParamSet::pagesize;
	else
		art_lf -= SMCParamSet::pagesize;
	clamp( art_lf, 0., art_max_secs);

	if ( ss <= pib() / xpi_bzero )
		art_zero += round( (pib() / xpi_bzero) - ss) * SMCParamSet::pagesize;
	else
		art_zero -= SMCParamSet::pagesize;

	clamp( art_zero, 0., min( 1, SMCParamSet::pagesize));
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
mc_smooth_detect_events_reset_jumps( size_t at, bool forward_processing,
				     valarray<TFloat>& _suForw, valarray<TFloat>& _suBack,
				     valarray<TFloat>& _ssForw, valarray<TFloat>& _ssBack)
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

	if ( forward_processing ) {
		//TODO: Review the following modifications in order to allow higher output sampling rate
		mc_jump [at] = (_suBack[kz] - _suForw[kz]) / 2;
		mc_event[at] = (_ssBack[kz] + _ssForw[kz]) / 2;
	} else {
		TFloat	r = EXP(mc_jump [at]) + (_suForw[kz] - _suBack[kz]) / 2,
			s = EXP(mc_event[at]) + (_ssForw[kz] + _ssBack[kz]) / 2;
		mc_event[at] = round( clamp(100 * mic_gain * r / s, (TFloat)-INT_MAX, (TFloat)INT_MAX));
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

void
sigfile::CBinnedMC::
mc_smooth_forward( size_t, size_t, size_t, bool&, bool)
 int dataBlock, int dataBlockSample, int fileSampleNr, ref bool smoothReset, bool resetAtJumps)
    {
      double mcJump;
      int n;

      int idataBlock = dataBlock;
      int idataBlockSample = dataBlockSample;
      int ifileSampleNr = fileSampleNr;

      if (resetAtJumps)
      {
        if (smoothReset)
        {
          LastMCJump.Processed = true;
          LastMCJump.SampleNr = fileSampleNr;
          LastMCJump.Size = MCjumpThreshold;
        }
        mcJump = outBuffer[OutputBufferOffsets[13] + dataBlockSample];
        if (Math.Abs(mcJump) >= Math.Abs(LastMCJump.Size))
        {
          LastMCJump.Processed = false;
          LastMCJump.SampleNr = fileSampleNr;
          LastMCJump.Size = mcJump;
        }
        if (!LastMCJump.Processed)
        {
          int m = LastMCJump.SampleNr;
          if (((fileSampleNr - m) >= MinSamplesBetweenJumps) || ((mcJump / LastMCJump.Size) < 0))
          {
            // Jump complete: initialize its processing
            // Save current samplerec with any earlier processed jumps
            OutputEDFFile.WriteDataBlock(dataBlock);
            // Get 'future' (at n) resetvalues from smoother
            n = Math.Min(m + MaxSamplesHalfJump, MCsignalsFileSamples - 1);
            idataBlock = Math.DivRem(n, MCsignalsBlockSamples, out idataBlockSample);
            OutputEDFFile.ReadDataBlock(idataBlock);
            // Reset forward smoother to 'future' back-smoothed state
            SUsmooth = MathEx.ExpInteger(outBuffer[OutputBufferOffsets[4] + idataBlockSample], AppConf.LogFloatY0, AppConf.LogFloatA);
            SSsmooth = MathEx.ExpInteger(outBuffer[OutputBufferOffsets[6] + idataBlockSample], AppConf.LogFloatY0, AppConf.LogFloatA);
            // Go to jump (at m) but preserve 1 sample of the jump
            ifileSampleNr = Math.Min(m, fileSampleNr); // smoother will start at ifileSampleNr = LastJump + 1
            idataBlock = Math.DivRem(ifileSampleNr, MCsignalsBlockSamples, out idataBlockSample);
            OutputEDFFile.ReadDataBlock(idataBlock);
            LastMCJump.Processed = true;
            LastMCJump.SampleNr = fileSampleNr;
            LastMCJump.Size = MCjumpThreshold;
          }
        }
      }
      n = Math.Min(fileSampleNr - ifileSampleNr, MaxSamplesHalfJump); // Reset jump-sample counter
      while (ifileSampleNr <= fileSampleNr)
      {
        if (idataBlockSample == MCsignalsBlockSamples)
        {
          OutputEDFFile.WriteDataBlock(idataBlock);
          idataBlock++;
          OutputEDFFile.ReadDataBlock(idataBlock);
          idataBlockSample = 0;
        }
        bool artifact = ((outBuffer[OutputBufferOffsets[9] + idataBlockSample] > 0) || (outBuffer[OutputBufferOffsets[10] + idataBlockSample] > 0) || (outBuffer[OutputBufferOffsets[11] + idataBlockSample] > 0) || (Math.Abs(outBuffer[OutputBufferOffsets[14] + idataBlockSample]) > MCEventThreshold));
        if ((resetAtJumps) && (n > 0))
        {
          artifact = true;
          n--;
        }
        // SU and SS forward-smoothed into SU+ and SS+
        double s;
        double r;
        MCSmooth_SmoothSUSS(MathEx.ExpInteger(outBuffer[OutputBufferOffsets[1] + idataBlockSample], AppConf.LogFloatY0, AppConf.LogFloatA),
            MathEx.ExpInteger(outBuffer[OutputBufferOffsets[2] + idataBlockSample], AppConf.LogFloatY0, AppConf.LogFloatA),
            out r, out s, artifact, smoothReset);
        outBuffer[OutputBufferOffsets[3] + idataBlockSample] = MathEx.LogFloat(r, AppConf.LogFloatY0, AppConf.LogFloatA);
        outBuffer[OutputBufferOffsets[5] + idataBlockSample] = MathEx.LogFloat(s, AppConf.LogFloatY0, AppConf.LogFloatA);
        smoothReset = false;
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
        if (Math.Abs(MCjump) > short.MaxValue)
            MCjump = Math.Sign(MCjump) * short.MaxValue;*/
        mcJump = Range.EnsureRange(AppConf.MicGain * 100 * mcJump, -short.MaxValue, short.MaxValue);
        outBuffer[OutputBufferOffsets[13] + idataBlockSample] = (short)MathEx.RoundNearest(mcJump);
        ifileSampleNr++;
        idataBlockSample++;
      }
    }



// eof
