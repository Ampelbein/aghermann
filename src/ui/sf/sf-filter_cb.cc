// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-filter_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-25
 *
 *         Purpose:  scoring facility Filters dialog callbacks
 *
 *         License:  GPL
 */


#include "ui/misc.hh"
#include "sf.hh"
#include "sf_cb.hh"

using namespace std;
using namespace aghui;


extern "C" {

void
iSFPageFilter_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD =  SF.filters_dialog;
	auto& H  = *SF.using_channel;
	aghui::SUIVarCollection WV;
	WV.reg( FD._p.eSFFilterLowPassCutoff,  &H.filters.low_pass_cutoff);
	WV.reg( FD._p.eSFFilterLowPassOrder,  (int*)&H.filters.low_pass_order);
	WV.reg( FD._p.eSFFilterHighPassCutoff, &H.filters.high_pass_cutoff);
	WV.reg( FD._p.eSFFilterHighPassOrder, (int*)&H.filters.high_pass_order);
	WV.reg( FD._p.eSFFilterNotchFilter,   (int*)&H.filters.notch_filter);
	WV.up();

	snprintf_buf( "<big>Filters for channel <b>%s</b></big>", SF.using_channel->name);
	gtk_label_set_markup( FD._p.lSFFilterCaption,
			      __buf__);

	if ( gtk_dialog_run( FD._p.wSFFilters) == GTK_RESPONSE_OK ) {
		WV.down();
		H.get_signal_filtered();

		if ( H.type == sigfile::SChannel::TType::eeg ) {
			H.get_psd_course();
			H.get_psd_in_bands();
			H.get_spectrum( SF.cur_page());
			H.get_mc_course();
		}
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);

		if ( strcmp( SF.using_channel->name, SF._p.AghH()) == 0 )
			SF.redraw_ssubject_timeline();
	}
}



void
eSFFilterHighPassCutoff_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	double other_freq = gtk_spin_button_get_value( SF.eSFFilterLowPassCutoff);
	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) < other_freq);
}

void
eSFFilterLowPassCutoff_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	gdouble other_freq = gtk_spin_button_get_value( SF.eSFFilterHighPassCutoff);
	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) > other_freq);
}


} // extern "C"


// eof
