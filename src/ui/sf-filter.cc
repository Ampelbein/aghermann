// ;-*-C++-*-
/*
 *       File name:  ui/sf-filter.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-30
 *
 *         Purpose:  scoring facility Butterworth filter dialog
 *
 *         License:  GPL
 */


#include "sf.hh"

using namespace std;




aghui::SScoringFacility::SFiltersDialog::
~SFiltersDialog()
{
	gtk_widget_destroy( (GtkWidget*)_p.wFilters);
	// done in a swoop in ~SF
}


// eof
