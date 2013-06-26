/*
 *       File name:  aghermann/ui/sf/d/filters_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-25
 *
 *         Purpose:  scoring facility Filters dialog callbacks
 *
 *         License:  GPL
 */


#include "aghermann/ui/misc.hh"

#include "filters.hh"

using namespace std;
using namespace aghui;


extern "C" {

void
eSFFilterHighPassCutoff_value_changed_cb(
	GtkSpinButton *spinbutton,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SFiltersDialog*)userdata;
	double other_freq = gtk_spin_button_get_value( FD.eSFFilterLowPassCutoff);
	gtk_widget_set_sensitive( (GtkWidget*)FD.bSFFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) < other_freq);
}

void
eSFFilterLowPassCutoff_value_changed_cb(
	GtkSpinButton *spinbutton,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SFiltersDialog*)userdata;
	gdouble other_freq = gtk_spin_button_get_value( FD.eSFFilterHighPassCutoff);
	gtk_widget_set_sensitive( (GtkWidget*)FD.bSFFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) > other_freq);
}


} // extern "C"


// Local Variables:
// indent-tabs-mode: 8
// tab-width: 8
// End:
