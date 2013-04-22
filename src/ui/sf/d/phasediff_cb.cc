/*
 *       File name:  ui/sf/d/phasediff_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-18
 *
 *         Purpose:  scoring facility phase diff dialog (callbacks)
 *
 *         License:  GPL
 */

#include "common/lang.hh"
#include "ui/misc.hh"
#include "phasediff.hh"

using namespace std;
using namespace aghui;

extern "C" {

gboolean
daSFPD_draw_cb(
	GtkWidget *wid,
	cairo_t *cr,
	const gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;

	if ( PD.suspend_draw )
		return TRUE;

	PD.draw( cr,
		 gtk_widget_get_allocated_width( wid),
		 gtk_widget_get_allocated_height( wid));

	return TRUE;
}


gboolean
daSFPD_scroll_event_cb(
	GtkWidget *wid,
	GdkEventScroll *event,
	const gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;

	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		PD.display_scale *= 1.05;
		break;
	case GDK_SCROLL_DOWN:
		PD.display_scale /= 1.05;
	default:
		break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}




void
eSFPDChannelA_changed_cb(
	GtkComboBox *cbox,
	const gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;

	if ( PD.suspend_draw )
		return;

	PD.channel1 = PD.channel_from_cbox( cbox);

	PD.update_course();
	gtk_widget_queue_draw( (GtkWidget*)PD.daSFPD);
}

void
eSFPDChannelB_changed_cb(
	GtkComboBox *cbox,
	const gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;

	if ( PD.suspend_draw )
		return;

	PD.channel2 = PD.channel_from_cbox( cbox);

	PD.update_course();
	gtk_widget_queue_draw( (GtkWidget*)PD.daSFPD);
}




void
eSFPDFreqFrom_value_changed_cb(
	GtkSpinButton *spinbutton,
	const gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;

	if ( PD.suspend_draw )
		return;

	PD.from = gtk_spin_button_get_value( spinbutton);
	PD.upto = PD.from + gtk_spin_button_get_value( PD.eSFPDBandwidth);

	PD.update_course();
	gtk_widget_queue_draw( (GtkWidget*)PD.daSFPD);
}

void
eSFPDBandwidth_value_changed_cb(
	GtkSpinButton *spinbutton,
	const gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;

	if ( PD.suspend_draw )
		return;

	PD.upto = PD.from + gtk_spin_button_get_value( spinbutton);

	PD.update_course();
	gtk_widget_queue_draw( (GtkWidget*)PD.daSFPD);
}



void
eSFPDSmooth_value_changed_cb(
	GtkScaleButton *b,
	const gdouble v,
	const gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;

	snprintf_buf( "Smooth: %zu",
		      PD.smooth_side = v);
	gtk_button_set_label( (GtkButton*)b, __buf__);
	if ( PD.suspend_draw )
		return;

	gtk_widget_queue_draw( (GtkWidget*)PD.daSFPD);
}



void
wSFPD_show_cb(
	GtkWidget*,
	const gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;
	auto& SF = PD._p;

	PD.suspend_draw = true;
	if ( gtk_combo_box_get_active( PD.eSFPDChannelA) == -1 ||
	     gtk_combo_box_get_active( PD.eSFPDChannelB) == -1 ) {
		PD.channel1 = &*SF.channels.begin();
		PD.channel2 = &*next(SF.channels.begin());
		PD.preselect_channel( PD.eSFPDChannelA, PD.channel1->name.c_str());
		PD.preselect_channel( PD.eSFPDChannelB, PD.channel2->name.c_str());
	} else {
		// they have been nicely set before, havent't they
		// PD.channel1 = PD.channel_from_cbox( eSFPDChannelA);
		// PD.channel2 = PD.channel_from_cbox( eSFPDChannelB);
	}

	gtk_spin_button_set_value( PD.eSFPDFreqFrom, PD.from);
	gtk_spin_button_set_value( PD.eSFPDBandwidth, PD.upto - PD.from);
	snprintf_buf( "Smooth: %zu", PD.smooth_side);
	gtk_button_set_label( (GtkButton*)PD.eSFPDSmooth, __buf__);

	PD.update_course();
	PD.suspend_draw = false;
	gtk_widget_queue_draw( (GtkWidget*)PD.daSFPD);
}

void
wSFPD_hide_cb(
	GtkWidget *wid,
	const gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;

	gtk_toggle_button_set_active( PD._p.bSFShowPhaseDiffDialog, FALSE);
}

} // extern "C"


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

