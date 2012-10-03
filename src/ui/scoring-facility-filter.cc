// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-filter.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-30
 *
 *         Purpose:  scoring facility Butterworth filter dialog
 *
 *         License:  GPL
 */


#include "scoring-facility.hh"

using namespace std;




aghui::SScoringFacility::SFiltersDialog::
~SFiltersDialog()
{
	// gtk_widget_destroy( (GtkWidget*)wFilters);
	// done in a swoop in ~SF
}
// bool
// aghui::SScoringFacility::SChannel::validate_filters()
// {
// 	if ( low_pass.cutoff >= 0. && low_pass.order < 6 &&
// 	     high_pass.cutoff >= 0. && high_pass.order < 6
// 	     && ((low_pass.cutoff > 0. && high_pass.cutoff > 0. && high_pass.cutoff < low_pass.cutoff)
// 		 || high_pass.cutoff == 0. || low_pass.cutoff == 0.) )
// 		return true;
// 	low_pass.cutoff = high_pass.cutoff = 0;
// 	low_pass.order = high_pass.order = 1;
// 	return false;
// }


// eof
