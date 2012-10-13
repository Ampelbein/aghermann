// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-artifacts.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-05
 *
 *         Purpose:  scoring facility: artifact detection dialog
 *
 *         License:  GPL
 */


#include "scoring-facility.hh"
#include "scoring-facility-widgets.hh"

using namespace std;


aghui::SScoringFacility::SArtifactDetectionDialog::
SArtifactDetectionDialog (aghui::SScoringFacility& p_)
      : P (),
	_p (p_)
{
	W_V.reg( _p.eSFADScope,		&P.scope);
	W_V.reg( _p.eSFADUpperThr,	&P.upper_thr);
	W_V.reg( _p.eSFADLowerThr,	&P.lower_thr);
	W_V.reg( _p.eSFADF0,		&P.f0);
	W_V.reg( _p.eSFADFc,		&P.fc);
	W_V.reg( _p.eSFADBandwidth,	&P.bandwidth);
	W_V.reg( _p.eSFADMCGain,	&P.mc_gain);
	W_V.reg( _p.eSFADBackpolate,	&P.iir_backpolate);
	W_V.reg( _p.eSFADEstimateE,	&P.estimate_e);
	W_V.reg( _p.eSFADEValue,	&P.E);
	W_V.reg( _p.eSFADHistRangeMin,	&P.dmin);
	W_V.reg( _p.eSFADHistRangeMax,	&P.dmax);
	W_V.reg( _p.eSFADHistBins,	(int*)&P.sssu_hist_size);
	W_V.reg( _p.eSFADSmoothSide,	(int*)&P.smooth_side);
	W_V.reg( _p.eSFADUseThisRange,	&P.use_range);

	W_V.up();
}

// eof
