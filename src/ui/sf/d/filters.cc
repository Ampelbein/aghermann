/*
 *       File name:  ui/sf/d/filters.cc
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


aghui::SScoringFacility::SFiltersDialog&
aghui::SScoringFacility::
filters_d()
{
	if ( not _filters_d )
		_filters_d = new SFiltersDialog(*this);
	return *_filters_d;
}

aghui::SScoringFacility::SFiltersDialog::
SFiltersDialog (SScoringFacility& p_)
      : _p (parent)
{
	W_V.reg( eSFFilterLowPassCutoff,  P.low_pass_cutoff);
	W_V.reg( eSFFilterLowPassOrder,   P.low_pass_order);
	W_V.reg( eSFFilterHighPassCutoff, P.high_pass_cutoff);
	W_V.reg( eSFFilterHighPassOrder,  P.high_pass_order);
	W_V.reg( eSFFilterNotchFilter,    (int*)&P.notch_filter);
}

aghui::SScoringFacility::SFiltersDialogWidgets::
~SScoringFacilityWidgets ()
{
	// destroy toplevels
	gtk_widget_destroy( (GtkWidget*)wSFFilters);
	g_object_unref( (GObject*)builder);
}

// Local Variables:
// indent-tabs-mode: 8
// End:
