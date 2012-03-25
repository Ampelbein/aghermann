// ;-*-C++-*-
/*
 *       File name:  libsigfile/ucont.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  CBinnedMicroConty ("EEG microcontinuity")
 *
 *         License:  GPL
 */

//#include <gsl/gsl_math.h>
#include <cassert>
#include <functional>

#include "../misc.hh"
#include "ucont.hh"
#include "source.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


string
sigfile::CBinnedMicroConty::
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
sigfile::CBinnedMicroConty::
compute( const SMicroContyParamSet& req_params,
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

		size_t	integrate_samples = SMicroContyParamSet::pagesize * samplerate();
		for ( size_t p = 0; p < pages(); ++p ) {
			auto range = slice (p * integrate_samples, 1, integrate_samples);
			su_buffer[p] =
				(valarray<TFloat> {due_filtered[range]} * valarray<TFloat> {se_filtered[range]})
				.sum()
				/ SMicroContyParamSet::pagesize;
			ss_buffer[p] =
				pow(valarray<TFloat> {se_filtered[range]}, (TFloat)2.)
				.sum() / samplerate()
				/ SMicroContyParamSet::pagesize;
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
			int sssu_smoother_width = log(1.0 + pib_peak_width) / logfloat_a / 2;
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
			// 		SmoothTime = SMicroContyParamSet::pagesize,
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
sigfile::CBinnedMicroConty::
do_smooth_sssu( valarray<TFloat>& ss_buffer, valarray<TFloat>& su_buffer, TSmoothOptions option)
{
	size_t	min_samples_between_jumps,
		max_samples_half_jump,
		mc_event_duration_samples;
	TFloat	mc_event_threshold,
		mc_jump_threshold;

	//TODO: Review the following modifications in order to allow higher output sampling rate
	mc_event_duration_samples = mc_event_duration * samplerate();
	min_samples_between_jumps = round(1. / (smooth_rate * SMicroContyParamSet::pagesize)) + 1;
	max_samples_half_jump	  = (min_samples_between_jumps / 20) + 1; // Bob's 'nose'
	mc_jump_threshold	  = (mc_jump_find / SMicroContyParamSet::pagesize) * 100 * mic_gain;
	mc_event_threshold	  = round((mc_event_reject / SMicroContyParamSet::pagesize) * smooth_rate * 100 * mic_gain);

	valarray<TFloat>
		_suForw (mc_event_duration_samples + 1),
		_suBack (mc_event_duration_samples + 1),
		_ssForw (mc_event_duration_samples + 1, pib()),
		_ssBack (mc_event_duration_samples + 1, pib());

	bool	smooth_reset = false;
	switch (option) {
	case GetArtifactsResetAll:
		for ( size_t p = 0; p < pages(); smooth_reset = false, ++p ) {
			// UpdateArtifacts uses SS,SU,SmoothReset,XpiBArt,ArtSpreadSamples
			mc_smooth_update_artifacts( smooth_reset, ss_buffer[p], su_buffer[p]);
			mc_smooth_reset_all(p);
		}
		break;
	case DetectEventsResetJumps:
		for ( size_t p = 0; p < pages(); smooth_reset = false, ++p )
			MCSmooth_DetectEvents_ResetJumps(OutputEDFFile.DataBuffer, k1, true);
		break;
	case Smooth:
		for ( size_t p = 0; p < pages(); smooth_reset = false, ++p )
			MCSmooth_DoSmooth_Forward(OutputEDFFile.DataBuffer, k, k1, fileSampleNr, ref smooth_reset, false);
		break;
	case SmoothResetAtJumps:
		for ( size_t p = 0; p < pages(); smooth_reset = false, ++p )
			MCSmooth_DoSmooth_Forward(OutputEDFFile.DataBuffer, k, k1, fileSampleNr, ref smooth_reset, true);
		break;
	}

	// Backward direction through EDF file
	smooth_reset = true;
	_suForw.Fill(0);
	_suBack.Fill(0);
	_ssForw.Fill(PiBExpInt);
	_ssBack.Fill(PiBExpInt);
	for (int k = OutputEDFFile.FileInfo.NrDataRecords - 1; k >= 0; k--)
	{
		OutputEDFFile.ReadDataBlock(k);

		for (int k1 = MCsignalsBlockSamples - 1; k1 >= 0; k1--)
		{
			fileSampleNr--;
			su = MathEx.ExpInteger(OutputEDFFile.DataBuffer[OutputBufferOffsets[1] + k1], AppConf.LogFloatY0, AppConf.LogFloatA);
			ss = MathEx.ExpInteger(OutputEDFFile.DataBuffer[OutputBufferOffsets[2] + k1], AppConf.LogFloatY0, AppConf.LogFloatA);
			switch (option)
			{
			case SmoothOption.GetArtifactsResetAll:
				// UpdateArtifacts uses SS,SU,SmoothReset,XpiBArt,ArtSpreadSamples
				MCSmooth_UpdateArtifacts(smooth_reset, ss, su);
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

		OutputEDFFile.WriteDataBlock(k);
	}

}



void
sigfile::CBinnedMicroConty::
mc_smooth_update_artifacts( bool smooth_reset, TFloat ss, TFloat su)
{
	if ( smooth_reset )
		art_hf = art_lf = art_zero = 0;

	TFloat art_factor = clamp((ss - su - pib()) / pib(), -1000., 1000.); // Avoid overflow of art_HF

	if ( art_factor >= xpi_bplus ) // XpiBPlus >= 1
		// todo: Bob controleren art_HF, art_LF en art_Zero: eerst afronden daarna *SmoothTime ?
		art_hf += round( art_factor / xpi_bplus) * SMicroContyParamSet::pagesize;
	else
		art_hf -= SMicroContyParamSet::pagesize;

	clamp( art_hf, 0., art_max_secs);

	if ( art_factor <= xpi_bminus)
		art_lf += round( art_factor / xpi_bminus) * SMicroContyParamSet::pagesize;
	else
		art_lf -= SMicroContyParamSet::pagesize;
	clamp( art_lf, 0., art_max_secs);

	if ( ss <= pib() / xpi_bzero )
		art_zero += round( (pib() / xpi_bzero) - ss) * SMicroContyParamSet::pagesize;
	else
		art_zero -= SMicroContyParamSet::pagesize;

	clamp( art_zero, 0., min( 1, SMicroContyParamSet::pagesize));
}

void
sigfile::CBinnedMicroConty::
mc_smooth_reset_all( size_t p)
{
	su_plus[p] = su_minus[p] = ss_plus[p] = ss_minus[p] = ssp[p] = ss0[p] = 0.;

	hf_art[p] = round( art_hf / art_phys_dim_res);
	lf_art[p] = round( art_lf / art_phys_dim_res);
	missing_signal[p] = round( art_zero / art_phys_dim_res);

	mc[p] = mc_jump[p] = mc_event[p] = 0;
}


// eof
