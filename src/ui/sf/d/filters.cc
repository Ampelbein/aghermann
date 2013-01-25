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

aghui::SScoringFacility::SFiltersDialog::
SFiltersDialog(SScoringFacility& p_)
      : _p (p_)
{
	auto& H = *_p.using_channel;
	W_V.reg( eSFFilterLowPassCutoff,  &H.filters.low_pass_cutoff);
	W_V.reg( eSFFilterLowPassOrder,  (int*)&H.filters.low_pass_order);
	W_V.reg( eSFFilterHighPassCutoff, &H.filters.high_pass_cutoff);
	W_V.reg( eSFFilterHighPassOrder, (int*)&H.filters.high_pass_order);
	W_V.reg( eSFFilterNotchFilter,   (int*)&H.filters.notch_filter);
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
