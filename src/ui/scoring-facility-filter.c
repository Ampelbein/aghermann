// ;-*-C-*- *  Time-stamp: "2011-03-15 00:25:43 hmmr"
/*
 *       File name:  ui/scoring-facility-filter.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-30
 *
 *         Purpose:  scoring facility Butterworth filter dialog
 *
 *         License:  GPL
 */




#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include <glade/glade.h>
#include <cairo-svg.h>

#include "../libexstrom/iface.h"
#include "misc.h"
#include "ui.h"
#include "settings.h"
#include "scoring-facility.h"

#if HAVE_CONFIG_H
#  include <config.h>
#endif


GtkWidget
	*wFilter;

static GtkWidget
	*lFilterCaption,
	*eFilterLowPassCutoff,
	*eFilterHighPassCutoff,
	*eFilterLowPassOrder,
	*eFilterHighPassOrder,
	*bFilterOK;



gint
agh_ui_construct_ScoringFacility_Filter( GladeXML *xml)
{
      // ------- wFilter
	if ( !(wFilter			= glade_xml_get_widget( xml, "wFilter")) ||
	     !(lFilterCaption		= glade_xml_get_widget( xml, "lFilterCaption")) ||
	     !(eFilterLowPassCutoff	= glade_xml_get_widget( xml, "eFilterLowPassCutoff")) ||
	     !(eFilterHighPassCutoff	= glade_xml_get_widget( xml, "eFilterHighPassCutoff")) ||
	     !(eFilterLowPassOrder	= glade_xml_get_widget( xml, "eFilterLowPassOrder")) ||
	     !(eFilterHighPassOrder	= glade_xml_get_widget( xml, "eFilterHighPassOrder")) ||
	     !(bFilterOK		= glade_xml_get_widget( xml, "bFilterOK")) )
		return -1;

	return 0;
}


void
iSFFilter_activate_cb()
{
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eFilterLowPassCutoff),
				   __clicked_channel->low_pass_cutoff);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eFilterLowPassOrder),
				   __clicked_channel->low_pass_order);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eFilterHighPassCutoff),
				   __clicked_channel->high_pass_cutoff);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eFilterHighPassOrder),
				   __clicked_channel->high_pass_order);

	snprintf_buf( "<big>Filters for channel <b>%s</b></big>", __clicked_channel->name);
	gtk_label_set_markup( GTK_LABEL (lFilterCaption),
			      __buf__);

	if ( gtk_dialog_run( GTK_DIALOG (wFilter)) == GTK_RESPONSE_OK ) {
		agh_edf_set_lowpass_cutoff(
			__source_ref, __clicked_channel->name,
			__clicked_channel->low_pass_cutoff
			= roundf( gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFilterLowPassCutoff))*10) / 10);
		agh_edf_set_lowpass_order(
			__source_ref, __clicked_channel->name,
			__clicked_channel->low_pass_order
			= roundf( gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFilterLowPassOrder))*10) / 10);
		agh_edf_set_highpass_cutoff(
			__source_ref, __clicked_channel->name,
			__clicked_channel->high_pass_cutoff
			= roundf( gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFilterHighPassCutoff))*10) / 10);
		agh_edf_set_highpass_order(
			__source_ref, __clicked_channel->name,
			__clicked_channel->high_pass_order
			= roundf( gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFilterHighPassOrder))*10) / 10);

		agh_msmt_get_signal_filtered_as_float( __clicked_channel->rec_ref,
						       &__clicked_channel->signal_filtered, NULL, NULL);

		gtk_widget_queue_draw( __clicked_channel->da_page);
		gtk_widget_queue_draw( __clicked_channel->da_power);
		gtk_widget_queue_draw( __clicked_channel->da_spectrum);
	}
}




void
eFilterHighPassCutoff_value_changed_cb( GtkSpinButton *spinbutton,
					gpointer       user_data)
{
	gdouble other_freq = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFilterLowPassCutoff));
	gtk_widget_set_sensitive( bFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) < other_freq);
}

void
eFilterLowPassCutoff_value_changed_cb( GtkSpinButton *spinbutton,
				       gpointer       user_data)
{
	gdouble other_freq = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFilterHighPassCutoff));
	gtk_widget_set_sensitive( bFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) > other_freq);
}

