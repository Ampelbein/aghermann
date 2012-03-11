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

#include <cassert>
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
	if (
		//cout << "Performing SU and SS reduction...\n";
		DoSSSUReduction() ||
		//cout << "Computing PiB value...\n";
		DoDetectPiB() ||
		//pbf.Message = "Detecting artifacts...";
		DoComputeArtifactTraces() ||
		//pbf.Message = "Smoothing SU and SS...";
		DoSmoothSSSU() ||
		//pbf.Message = "Detecting events...";
		DoDetectMCEvents() ||
		//pbf.Message = "Re-smoothing signals and detecting jumps...";
		// Re-smooth SS and SU rejecting MC events
		DoResmoothSSSU() ||
		//pbf.Message = "Computing final gains...";
		DoComputeMC() ) {
		return -1;
	}
	return 0;
}

int
sigfile::CBinnedMicroConty::DoSSSUReduction()
{
	// Do
	size_t	total_samples = pages() * CPageMetrics_base::pagesize() * samplerate();

	size_t	process_samples =
		(iir_undersampler == 1)
		?  total_samples
		: (total_samples / iir_undersampler) + 1;
	valarray<TFloat>
		IIR_Input (process_samples),
		IIR_OutDU (process_samples),
		IIR_OutSE (process_samples);

	int outRecSample = 0;
	int underSample = 1; // underSample: 1..IIR_UnderSamler
	TFloat SSactualSample = 0;
	TFloat SUactualSample = 0;
	TFloat integratedTime = 0;
	int integrateCount = 0;
	bool oneOutputSampleObtained = false;

	// Processing input block by block
	int totalRecords = InputEDFFile.FileInfo.NrDataRecords;
	int currentOutputRecord = 0;
	if (CopyInputSignal)
	{
		OutputEDFFile.ReadDataBlock(currentOutputRecord);
	}
	for (int k = 0; k < totalRecords; k++)
	{
		// Reading input block to the buffer
		InputEDFFile.ReadDataBlock(k);

		IIR_Input.Fill(0);
		int IIR_LastSample = 0;

		for (int k1 = 0; k1 < inputSamples; k1++)
		{
			if (underSample == AppConf.IIRUnderSampler)
			{
				IIR_Input[IIR_LastSample] =
					InputEDFFile.SignalInfo[InputSignalSelected].DigiToPhysi(InputEDFFile.DataBuffer[InputBufferOffsets[InputSignalSelected] + k1]);
				underSample = 1;
				IIR_LastSample++;
			}
			else
				underSample++;
		}
		IIR_LastSample--;

		DUEfilter.FilterSamples(IIR_Input, IIR_OutDU, 0, IIR_LastSample, 0);
		SEfilter.FilterSamples(IIR_Input, IIR_OutSE, 0, IIR_LastSample, 0);

		// SUSSsmoothingTime secs integration into EDF output file
		for (int k1 = 0; k1 <= IIR_LastSample; k1++)
		{
			SUactualSample += IIR_OutDU[k1] * IIR_OutSE[k1];
			SSactualSample += Math.Pow(IIR_OutSE[k1], 2) * iir_delta;
			integratedTime += iir_delta;
			integrateCount++;

			// todo: Marco: Check what is happening here ===>
                     
			if (integratedTime >= AppConf.SmoothTime)
			{
				TFloat normFactor = integrateCount * iir_delta;
				SUactualSample = SUactualSample / normFactor;
				SSactualSample = SSactualSample / normFactor;
				integrateCount = 0;
				integratedTime -= AppConf.SmoothTime;
				oneOutputSampleObtained = true;
			}

			// todo: Marco: until here.... <===

			if (oneOutputSampleObtained)
			{
				/*
				  LogFloat is used because:
				  IIR_Gain_du and IIR_Gain_s and INT_Gain (the derived
				  INT_Gain_SU and INT_Gain_SS) are optimized for the years 1995-1997
				  slow-wave and analysis of sigma 12-bit ADC signals (from
				  EEG devices like tape recorders within integer range (OutReal) account.
				  This range was therefore (+/-) 1 .. 32767. 16-bit signals (EDF) and other model
				  and sampling frequencies can give up to 16 * 16 * 100 times greater powers. 
				  8-bit signals (eg BrainSpy) and other model and sampling frequencies can give
				  16 * 16 * 100 times smaller powers. So the range is OutReal, in general,
				  about 1E-4 til 10000*1E+5. This range can be projected in EDF integers with an accuracy of 0.1%
				  using a log-conversion (see Kemp et al, Journal of Sleep Research 1998 and
				  also www.medfac.leidenuniv.nl/neurology/KNF/kemp/edffloat.htm). This has the advantage that
				  analysis results can be viewed with a simple EDF viewer
				*/
				OutputEDFFile.DataBuffer[OutputBufferOffsets[1] + outRecSample] = MathEx.LogFloat(SUactualSample, AppConf.LogFloatY0, AppConf.LogFloatA);
				OutputEDFFile.DataBuffer[OutputBufferOffsets[2] + outRecSample] = MathEx.LogFloat(SSactualSample, AppConf.LogFloatY0, AppConf.LogFloatA);
				outRecSample++;
				if (outRecSample == MCsignalsBlockSamples)
				{
					OutputEDFFile.WriteDataBlock(currentOutputRecord);
					outRecSample = 0;
					currentOutputRecord++;
					if ((currentOutputRecord < OutputEDFFile.FileInfo.NrDataRecords) && AppConf.CopyInputSignal)
					{
						OutputEDFFile.ReadDataBlock(currentOutputRecord);
					}
				}
				SUactualSample = 0;
				SSactualSample = 0;
				oneOutputSampleObtained = false;
			}
		} // SUSSsmoothingTime secs integration

	}// processing input block by block

	// Complete and store last output block with zeros
	if (outRecSample != 0)
	{
		for (int k = outRecSample; k < MCsignalsBlockSamples; k++)
		{
			OutputEDFFile.DataBuffer[OutputBufferOffsets[1] + k] = 0;
			OutputEDFFile.DataBuffer[OutputBufferOffsets[2] + k] = 0;
		}
		OutputEDFFile.WriteDataBlock(currentOutputRecord);
	}

	OutputEDFFile.CommitChanges();

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
