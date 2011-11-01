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


#include <sys/stat.h>

#include "misc.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif


using namespace std;
using namespace aghui;

aghui::SScoringFacility::SFiltersDialog::SFiltersDialog( aghui::SScoringFacility& parent)
      : _p (parent)
{}



int
aghui::SScoringFacility::SFiltersDialog::construct_widgets()
{
      // ------- wFilter
	if ( !(AGH_GBGETOBJ3 (_p.builder, GtkDialog, wFilters)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkLabel, lFilterCaption)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton, eFilterLowPassCutoff)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton, eFilterHighPassCutoff)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton, eFilterLowPassOrder)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton, eFilterHighPassOrder)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkButton, bFilterOK)) )
		return -1;

	g_signal_connect_after( (GObject*)eFilterHighPassCutoff, "value-changed",
				(GCallback)eFilterHighPassCutoff_value_changed_cb,
				this);
	g_signal_connect_after( (GObject*)eFilterLowPassCutoff, "value-changed",
				(GCallback)eFilterLowPassCutoff_value_changed_cb,
				this);
	return 0;
}




extern "C" {
	void
	iSFPageFilter_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		auto& FD = SF.filters_dialog;
		gtk_spin_button_set_value( FD.eFilterLowPassCutoff,
					   SF.using_channel->low_pass.cutoff);
		gtk_spin_button_set_value( FD.eFilterLowPassOrder,
					   SF.using_channel->low_pass.order);
		gtk_spin_button_set_value( FD.eFilterHighPassCutoff,
					   SF.using_channel->high_pass.cutoff);
		gtk_spin_button_set_value( FD.eFilterHighPassOrder,
					   SF.using_channel->high_pass.order);

		snprintf_buf( "<big>Filters for channel <b>%s</b></big>", SF.using_channel->name);
		gtk_label_set_markup( FD.lFilterCaption,
				      __buf__);

		if ( gtk_dialog_run( FD.wFilters) == GTK_RESPONSE_OK ) {
			SF.using_channel->low_pass.cutoff
				= roundf( gtk_spin_button_get_value( FD.eFilterLowPassCutoff)*10) / 10;
			SF.using_channel->low_pass.order
				= roundf( gtk_spin_button_get_value( FD.eFilterLowPassOrder)*10) / 10;
			SF.using_channel->high_pass.cutoff
				= roundf( gtk_spin_button_get_value( FD.eFilterHighPassCutoff)*10) / 10;
			SF.using_channel->high_pass.order
				= roundf( gtk_spin_button_get_value( FD.eFilterHighPassOrder)*10) / 10;

			SF.using_channel->get_signal_filtered();

			gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
		}
	}



	void
	eFilterHighPassCutoff_value_changed_cb( GtkSpinButton *spinbutton,
						gpointer       userdata)
	{
		auto& FD = *(SScoringFacility::SFiltersDialog*)userdata;
		double other_freq = gtk_spin_button_get_value( FD.eFilterLowPassCutoff);
		gtk_widget_set_sensitive( (GtkWidget*)FD.bFilterOK,
					  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) < other_freq);
	}

	void
	eFilterLowPassCutoff_value_changed_cb( GtkSpinButton *spinbutton,
					       gpointer       userdata)
	{
		auto& FD = *(SScoringFacility::SFiltersDialog*)userdata;
		gdouble other_freq = gtk_spin_button_get_value( FD.eFilterHighPassCutoff);
		gtk_widget_set_sensitive( (GtkWidget*)FD.bFilterOK,
					  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) > other_freq);
	}

} // extern "C"


// eof
