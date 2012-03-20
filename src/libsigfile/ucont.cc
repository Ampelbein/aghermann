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
sigfile::CBinnedMicroConty::fname_base() const
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





int
sigfile::CBinnedMicroConty::compute( const SMicroContyParamSet& req_params,
				     bool force)
{
	hash_t req_signature = _using_F.artifacts( _using_sig_no).dirty_signature();
	if ( have_data() && (*this) == req_params
	     && _signature == req_signature )
		return 0;

	size_t	total_samples = pages() * CPageMetrics_base::pagesize() * samplerate();

	//cout << "Performing SU and SS reduction...\n";
	// DoSSSUReduction();
	valarray<TFloat>
		su_buffer (pages()),
		ss_buffer (pages());

	{
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
	valarray<int>
		sssu (ss_su_max - ss_su_min + 1);
	{
		valarray<TFloat>
			sssu_smoothed (ss_su_max - ss_su_min + 1),
			sssu_match    (ss_su_max - ss_su_min + 1),
			sssu_template (pib_correlation_function_buffer_size);

		for ( size_t p = 0; p < pages(); ++p ) {
			int j = round( log( ss_buffer[p] - su_buffer[p]));
			if ( j != 0 && ss_su_min <= j && j < ss_su_max )
				++sssu[j - ss_su_min];
		}

		
		/*
		 * 2*SS_SUsmoother applied to the logarithmic converted values in the histogram 
		 * corresponds to the original values of piBPeakWidth
		 */
		int sssuSmootherWidth = (int)Range.EnsureRange(Math.Truncate(Math.Log(1.0 + AppConf.piBPeakWidth) / AppConf.LogFloatA / 2), -int.MaxValue, int.MaxValue);

		// Apply the smoothing (mean filter) to the histogram
		for (int k = sssuSmootherWidth; k <= AppConf.SS_SUmax - AppConf.SS_SUmin - sssuSmootherWidth; k++)
		{
			for (int k1 = k - sssuSmootherWidth; k1 <= k + sssuSmootherWidth; k1++)
			{
				sssuSmoothed[k] += sssu[k1];
			}
			sssuSmoothed[k] = sssuSmoothed[k] / (2 * sssuSmootherWidth + 1);
		}

		// todo: Marco: "gewoon" maximum zoeken, geen moeilijke functies.

		// Construct the template to detect desired piB value peak in the smoothed histogram
		for (int k = 1; k < sssuTemplate.Length - 1; k++)
		{
			x = ((double)k / (sssuTemplate.Length - 1)) * 3 * Math.PI - 2 * Math.PI;
			value = 2.0 / 3 * (-0.5894) * (MathEx.Heav(x + 2 * Math.PI) - MathEx.Heav(x + (Math.PI / 2))) +
				(MathEx.Heav(x + (Math.PI / 2)) - MathEx.Heav(x)) * (Math.Sin(2 * x) / (2 * x)) +
				(MathEx.Heav(x) - MathEx.Heav(x - Math.PI / 2)) * (Math.Sin(2 * x) / (2 * x));
			sssuTemplate[k] = value;
		}

		// Calculate and substract template mean value and calculate resulting template's peak index
		x = MathEx.Average(sssuTemplate);
		value = 0;
		int templatePeakIdx = 0;
		for (int k = 0; k < sssuTemplate.Length - 1; k++)
		{
			sssuTemplate[k] -= x;
			if (sssuTemplate[k] > value)
			{
				templatePeakIdx = k;
				value = sssuTemplate[k];
			}
		}

		// Calculate correlation coefficients by shifting the template over the histogram
		for (int k = 0; k < (sssuSmoothed.Length - sssuTemplate.Length); k++)
		{
			value = 0;
			for (int k1 = 0; k1 < sssuTemplate.Length; k1++)
				value += sssuTemplate[k1] * sssuSmoothed[k + k1];
			sssuMatch[k + templatePeakIdx] = value;
		}

		LogPiBxx = 0;
		double peak = 0;
		// We'll take Pi*B as the x-value for the maximum correlation point
		for (int k = 0; k < sssuMatch.Length; k++)
		{
			if (sssuMatch[k] > peak)
			{
				peak = sssuMatch[k];
				//piB = (short)Range.EnsureRange(k + appConf.SS_SUmin, -short.MaxValue, short.MaxValue);
				LogPiBxx = (short)(k + AppConf.SS_SUmin);
			}
		}


		// Show histogram
		if (AppConf.ShowPiBHistogram)
		{
			HistogramInfo histogramInfo = new HistogramInfo
				{
					FileName = InputEDFFileName,
					SignalLabel = InputEDFFile.SignalInfo[InputSignalSelected].SignalLabel,
					SS_SUmax = AppConf.SS_SUmax,
					SS_SUmin = AppConf.SS_SUmin,
					SU_SS = sssu,
					SU_SSsmoothed = sssuSmoothed,
					UnderSampling = AppConf.IIRUnderSampler,
					SU_SSmatch = sssuMatch,
					F0 = AppConf.F0,
					FC = AppConf.FC,
					B = AppConf.BandWidth,
					PiBvalueLog = LogPiBxx,
					PiBvaluePhysi = PiBxx,
					SmoothRate = AppConf.SmoothRate,
					SmoothTime = AppConf.SmoothTime
				};

			FormPiBHistogram formHistogram = new FormPiBHistogram();
			formHistogram.SetHistogramInfo(histogramInfo);
			formHistogram.Show();
		}

	}
	//pbf.Message = "Detecting artifacts...";
	// DoComputeArtifactTraces();
	
	//pbf.Message = "Smoothing SU and SS...";
	// DoSmoothSSSU();
        {
		//TODO: Review the following modifications in order to allow higher output sampling rate
		size_t	mc_event_duration_samples = mc_event_duration * samplerate(),
			min_samples_between_jumps = round(1. / (smooth_rate * SMicroContyParamSet::pagesize)) + 1,
			max_samples_half_jump	  = (min_samples_between_jumps / 20) + 1; // Bob's 'nose'
		TFloat	mc_jump_threshold  = (mc_jump_find / SMicroContyParamSet::pagesize) * 100 * mic_gain;
		size_t	mc_event_threshold = round((mc_event_reject / SMicroContyParamSet::pagesize) * smooth_rate * 100 * mic_gain);

		valarray<TFloat>
			_suForw (mc_event_duration_samples + 1),
			_suBack (mc_event_duration_samples + 1),
			_ssForw (mc_event_duration_samples + 1, PiBxx),
			_ssBack (mc_event_duration_samples + 1, PiBExpInt);

		MCsignalsFileSamples = MCsignalsBlockSamples * OutputEDFFile.FileInfo.NrDataRecords;

		// Forward direction through EDF file
		double su;
		double ss;
		for (int k = 0; k < OutputEDFFile.FileInfo.NrDataRecords; k++)
		{
			OutputEDFFile.ReadDataBlock(k);

			for (int k1 = 0; k1 < MCsignalsBlockSamples; k1++)
			{
				su = MathEx.ExpInteger(OutputEDFFile.DataBuffer[OutputBufferOffsets[1] + k1], AppConf.LogFloatY0, AppConf.LogFloatA);
				ss = MathEx.ExpInteger(OutputEDFFile.DataBuffer[OutputBufferOffsets[2] + k1], AppConf.LogFloatY0, AppConf.LogFloatA);
				switch (option)
				{
				case SmoothOption.GetArtifactsResetAll:
					// UpdateArtifacts uses SS,SU,SmoothReset,XpiBArt,ArtSpreadSamples
					MCSmooth_UpdateArtifacts(smoothReset, ss, su);
					MCSmooth_ResetAll(OutputEDFFile.DataBuffer, k1);
					break;
				case SmoothOption.DetectEventsResetJumps:
					MCSmooth_DetectEvents_ResetJumps(OutputEDFFile.DataBuffer, k1, true);
					break;
				case SmoothOption.Smooth:
					MCSmooth_DoSmooth_Forward(OutputEDFFile.DataBuffer, k, k1, fileSampleNr, ref smoothReset, false);
					break;
				case SmoothOption.SmoothResetAtJumps:
					MCSmooth_DoSmooth_Forward(OutputEDFFile.DataBuffer, k, k1, fileSampleNr, ref smoothReset, true);
					break;
				}
				smoothReset = false;
				fileSampleNr++;
			}

			OutputEDFFile.WriteDataBlock(k);
		}
		// Backward direction through EDF file
		smoothReset = true;
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
					MCSmooth_UpdateArtifacts(smoothReset, ss, su);
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
					MCSmooth_DoSmooth_Backward(OutputEDFFile.DataBuffer, k, k1, fileSampleNr, ref smoothReset, false);
					break;
				case SmoothOption.SmoothResetAtJumps:
					MCSmooth_DoSmooth_Backward(OutputEDFFile.DataBuffer, k, k1, fileSampleNr, ref smoothReset, true);
					break;
				}
				smoothReset = false;
			}

			OutputEDFFile.WriteDataBlock(k);
		}

        }
	
	//pbf.Message = "Detecting events...";
	// DoDetectMCEvents();
	
	//pbf.Message = "Re-smoothing signals and detecting jumps...";
	// Re-smooth SS and SU rejecting MC events
	// DoResmoothSSSU();
	
	//pbf.Message = "Computing final gains...";
	// DoComputeMC();
	

	return 0;
}


// eof
