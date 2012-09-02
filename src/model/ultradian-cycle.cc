// ;-*-C++-*-
/*
 *       File name:  model/ultradian-cycle.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-20
 *
 *         Purpose:  Detect NREM-REM cycle
 *
 *         License:  GPL
 */


#include "../libsigfile/psd.hh"
#include "../libsigfile/mc.hh"
#include "../expdesign/recording.hh"
#include "beersma.hh"

using namespace std;



list<agh::beersma::SUltradianCycle>
agh::beersma::
ultradian_cycles( const agh::CRecording& M,
		  const agh::beersma::SUltradianCycleCtl& P)
{
      // set up
	size_t	pp;
	size_t	pagesize;

	valarray<double> course;
	switch ( P.metric ) {
	case sigfile::TMetricType::Psd:
		pp = M.CBinnedPower::pages();
		pagesize = ((sigfile::CBinnedPower)M).CPageMetrics_base::pagesize();
		course = M.CBinnedPower::course<double>( P.freq_from, P.freq_upto);
	    break;
	case sigfile::TMetricType::Mc:
		pp = M.CBinnedMC::pages();
		pagesize = ((sigfile::CBinnedMC)M).CPageMetrics_base::pagesize();
		course = M.CBinnedMC::course<double>(
			min( (size_t)((P.freq_from - M.freq_from) / M.bandwidth),
			     M.CBinnedMC::bins()-1));
	    break;
	default:
		throw runtime_error ("ultradian_cycles(): Invalid profile type");
	}

      // determine A (SWA_0) and b (SWA_L)
	double	SWA_0, SWA_L;
	{
		// this one doesn't throw
		agh::CSCourse tmp (M, {
				P.metric, P.freq_from, P.freq_upto,
					0., // _req_percent_scored;
					5, // _swa_laden_pages_before_SWA_0;
					true }); // _ScoreUnscoredAsWake:1;
		SWA_0 = tmp.SWA_0();
		SWA_L = tmp.SWA_L();
	}

	list<agh::beersma::SUltradianCycle> ret;

	return ret;
}


// eof
