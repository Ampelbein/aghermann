/*
 *       File name:  aghermann/ui/sf/d/filters.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-30
 *
 *         Purpose:  scoring facility Butterworth filter dialog
 *
 *         License:  GPL
 */


#include "filters.hh"

using namespace std;
using namespace agh::ui;

SScoringFacility::SFiltersDialog&
SScoringFacility::
filters_d()
{
	if ( not _filters_d )
		_filters_d = new SFiltersDialog(*this);
	return *_filters_d;
}

SScoringFacility::SFiltersDialog::
SFiltersDialog (SScoringFacility& p_)
      : _p (p_)
{
	W_V.reg( eSFFilterLowPassCutoff,  &P.low_pass_cutoff);
	W_V.reg( eSFFilterLowPassOrder,   (int*)&P.low_pass_order);
	W_V.reg( eSFFilterHighPassCutoff, &P.high_pass_cutoff);
	W_V.reg( eSFFilterHighPassOrder,  (int*)&P.high_pass_order);
	W_V.reg( eSFFilterNotchFilter,    (int*)&P.notch_filter);
}


// Local Variables:
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
