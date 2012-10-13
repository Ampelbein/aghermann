// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-filter_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-25
 *
 *         Purpose:  scoring facility Filters dialog callbacks
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "scoring-facility.hh"
#include "scoring-facility_cb.hh"

using namespace std;
using namespace aghui;


extern "C" {

void
iSFPageFilter_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD =  SF.filters_dialog;
	auto& H  = *SF.using_channel;
	gtk_spin_button_set_value( FD._p.eFilterLowPassCutoff,
				   SF.using_channel->filters.low_pass_cutoff);
	gtk_spin_button_set_value( FD._p.eFilterLowPassOrder,
				   SF.using_channel->filters.low_pass_order);
	gtk_spin_button_set_value( FD._p.eFilterHighPassCutoff,
				   SF.using_channel->filters.high_pass_cutoff);
	gtk_spin_button_set_value( FD._p.eFilterHighPassOrder,
				   SF.using_channel->filters.high_pass_order);
	gtk_combo_box_set_active( FD._p.eFilterNotchFilter,
				  (int)SF.using_channel->filters.notch_filter);

	snprintf_buf( "<big>Filters for channel <b>%s</b></big>", SF.using_channel->name);
	gtk_label_set_markup( FD._p.lFilterCaption,
			      __buf__);

	if ( gtk_dialog_run( FD._p.wFilters) == GTK_RESPONSE_OK ) {
		H.filters.high_pass_cutoff
			= roundf( gtk_spin_button_get_value( FD._p.eFilterHighPassCutoff)*10) / 10;
		H.filters.low_pass_cutoff
			= roundf( gtk_spin_button_get_value( FD._p.eFilterLowPassCutoff)*10) / 10;
		H.filters.high_pass_order
			= roundf( gtk_spin_button_get_value( FD._p.eFilterHighPassOrder)*10) / 10;
		H.filters.low_pass_order
			= roundf( gtk_spin_button_get_value( FD._p.eFilterLowPassOrder)*10) / 10;
		H.filters.notch_filter =
			(sigfile::SFilterPack::TNotchFilter)gtk_combo_box_get_active( FD._p.eFilterNotchFilter);

		SF.using_channel->get_signal_filtered();

		if ( H.type == sigfile::SChannel::TType::eeg ) {
			H.get_psd_course( true); // force redo fft due to it not keeping track of filters yet
			H.get_psd_in_bands( false);
			H.get_spectrum( SF.cur_page());
			H.get_mc_course( true);
		}
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);

		if ( strcmp( SF.using_channel->name, SF._p.AghH()) == 0 )
			SF.redraw_ssubject_timeline();
	}
}



void
eFilterHighPassCutoff_value_changed_cb( GtkSpinButton *spinbutton,
					gpointer       userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.filters_dialog;
	double other_freq = gtk_spin_button_get_value( FD._p.eFilterLowPassCutoff);
	gtk_widget_set_sensitive( (GtkWidget*)FD._p.bFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) < other_freq);
}

void
eFilterLowPassCutoff_value_changed_cb( GtkSpinButton *spinbutton,
				       gpointer       userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.filters_dialog;
	gdouble other_freq = gtk_spin_button_get_value( FD._p.eFilterHighPassCutoff);
	gtk_widget_set_sensitive( (GtkWidget*)FD._p.bFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) > other_freq);
}


} // extern "C"


// eof
