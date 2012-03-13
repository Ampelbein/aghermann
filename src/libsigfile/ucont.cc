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
	{
		size_t	elements = ss_su_max - ss_su_min + 1;
		valarray<short>
			sssu (elements);
		valarray<TFloat>
			sssu_smoothed (elements),
			sssu_match    (elements),
			sssu_template (piB_correlation_function_buffer_size);
		TFloat	value,
			x;

		int dataBlockSamples = OutputEDFFile.SignalInfo[1].NrSamples;

		for ( size_t p = 0; p < pages(); ++p ) {

			// Do not add zeros from unrecorded end of file
			short j = ss_buffer[p] - su_buffer[p];
			//Watch it! SS_SUmin and SS_SUmax now refer to log-transformed values
			if (Range.InRange(j, AppConf.SS_SUmin, AppConf.SS_SUmax) && (sssu[j - AppConf.SS_SUmin] < short.MaxValue) && (j != 0))
				sssu[j - AppConf.SS_SUmin]++;
			}
		}
	}
	//pbf.Message = "Detecting artifacts...";
	// DoComputeArtifactTraces();
	
	//pbf.Message = "Smoothing SU and SS...";
	// DoSmoothSSSU();
	
	//pbf.Message = "Detecting events...";
	// DoDetectMCEvents();
	
	//pbf.Message = "Re-smoothing signals and detecting jumps...";
	// Re-smooth SS and SU rejecting MC events
	// DoResmoothSSSU();
	
	//pbf.Message = "Computing final gains...";
	// DoComputeMC();
	

	return 0;
}


int
sigfile::CBinnedMicroConty::DoDetectPiB()
{
	return 0;
}

int
sigfile::CBinnedMicroConty::DoComputeArtifactTraces()
{
	return 0;
}

int
sigfile::CBinnedMicroConty::DoSmoothSSSU()
{
	return 0;
}

int
sigfile::CBinnedMicroConty::DoDetectMCEvents()
{
	return 0;
}

int
sigfile::CBinnedMicroConty::DoResmoothSSSU()
{
	return 0;
}

int
sigfile::CBinnedMicroConty::DoComputeMC()
{
	return 0;
}



// eof
