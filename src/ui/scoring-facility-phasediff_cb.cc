// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-phasediff_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-18
 *
 *         Purpose:  scoring facility phase diff dialog (callbacks)
 *
 *         License:  GPL
 */




#include "../common/misc.hh"
#include "scoring-facility.hh"


using namespace std;
using namespace aghui;

extern "C" {

gboolean
daPhaseDiff_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;

	if ( PD.course.size() == 0 ) {
		cairo_show_text( cr, "(uninitialized)");
		cairo_stroke( cr);
		return TRUE;
	}
	if ( PD.channel1->samplerate() != PD.channel2->samplerate() ) {
		cairo_show_text( cr, "incompatible channels (different samplerate)");
		cairo_stroke( cr);
		return TRUE;
	}

	PD.draw( cr, gtk_widget_get_allocated_width( wid), gtk_widget_get_allocated_height( wid));

	return TRUE;
}


gboolean
daPhaseDiff_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;
	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		PD.display_scale *= 1.1;
		break;
	case GDK_SCROLL_DOWN:
		PD.display_scale /= 1.1;
	default:
		break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}




void
ePhaseDiffChannelA_changed_cb( GtkComboBox *cbox, gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;
	PD.channel1 = PD.channel_from_cbox( cbox);
	gtk_widget_set_sensitive( (GtkWidget*)PD.bPhaseDiffApply,
				  PD.channel1 != PD.channel2);
}

void
ePhaseDiffChannelB_changed_cb( GtkComboBox *cbox, gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;
	PD.channel2 = PD.channel_from_cbox( cbox);
	gtk_widget_set_sensitive( (GtkWidget*)PD.bPhaseDiffApply,
				  PD.channel1 != PD.channel2);
}




void
ePhaseDiffFreqFrom_value_changed_cb( GtkSpinButton *spinbutton,
				     gpointer       userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;
	PD.from = fabs( gtk_spin_button_get_value( spinbutton) * 10) / 10;
	gtk_widget_set_sensitive( (GtkWidget*)PD.bPhaseDiffApply,
				  PD.from < PD.upto);
}

void
ePhaseDiffFreqUpto_value_changed_cb( GtkSpinButton *spinbutton,
				     gpointer       userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;
	PD.upto = fabs( gtk_spin_button_get_value( spinbutton) * 10) / 10;
	gtk_widget_set_sensitive( (GtkWidget*)PD.bPhaseDiffApply,
				  PD.from < PD.upto);
}



void
bPhaseDiffApply_clicked_cb( GtkButton *button,
			    gpointer   userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;
	PD.update_course();
	gtk_widget_queue_draw( (GtkWidget*)PD.daPhaseDiff);
}



void
wPhaseDiff_show_cb( GtkWidget *wid, gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;
	if ( gtk_combo_box_get_active( PD.ePhaseDiffChannelA) == -1 ||
	     gtk_combo_box_get_active( PD.ePhaseDiffChannelB) == -1 ) {
		PD.channel1 = &*PD._p.channels.begin();
		PD.channel2 = &*next(PD._p.channels.begin());
		PD.preselect_channel( PD.ePhaseDiffChannelA, PD.channel1->name);
		PD.preselect_channel( PD.ePhaseDiffChannelB, PD.channel2->name);
	} else {
		// they have been nicely set, havent't they
		// PD.channel1 = PD.channel_from_cbox( ePhaseDiffChannelA);
		// PD.channel2 = PD.channel_from_cbox( ePhaseDiffChannelB);
	}

	gtk_spin_button_set_value( PD.ePhaseDiffFreqFrom, PD.from);
	gtk_spin_button_set_value( PD.ePhaseDiffFreqUpto, PD.upto);

	gtk_widget_set_sensitive( (GtkWidget*)PD.bPhaseDiffApply,
				  PD.channel1 != PD.channel2 &&
				  PD.from < PD.upto);
}

void
wPhaseDiff_hide_cb( GtkWidget *wid, gpointer userdata)
{
	auto& PD = *(SScoringFacility::SPhasediffDialog*)userdata;
	gtk_toggle_button_set_active( PD._p.bSFShowPhaseDiffDialog, FALSE);
}

} // extern "C"


// eof
