/*
 *       File name:  aghermann/ui/sf/hypnogram_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-15
 *
 *         Purpose:  scoring facility (hypnogram) (callbacks)
 *
 *         License:  GPL
 */

#include "aghermann/rk1968/rk1968.hh"
#include "aghermann/ui/globals.hh"
#include "sf.hh"

using namespace std;
using namespace agh::ui;

extern "C" {

gboolean
daSFHypnogram_draw_cb(
	GtkWidget*,
	cairo_t *cr,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.draw_hypnogram( cr);

	return TRUE;
}




gboolean
daSFHypnogram_button_press_event_cb(
	GtkWidget *wid,
	GdkEventButton *event,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	switch ( event->button ) {
	case 1:
		SF.set_cur_vpage( (event->x / SF.da_wd) * SF.total_vpages());
		SF.hypnogram_button_down = true;
		SF.queue_redraw_all();
	    break;
	case 2:
		SF.alt_hypnogram = !SF.alt_hypnogram;
		gtk_widget_queue_draw( wid);
	    break;
	case 3:
		gtk_menu_popup(
			SF.iiSFScore,
			NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}
	return TRUE;
}

gboolean
daSFHypnogram_button_release_event_cb(
	GtkWidget*,
	GdkEventButton *event,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	switch ( event->button ) {
	case 1:
		SF.set_cur_vpage(
			agh::alg::value_within( event->x / SF.da_wd, 0., 1.) * SF.total_vpages());
		SF.hypnogram_button_down = false;
		SF.queue_redraw_all();
	    break;
	}
	return TRUE;
}



gboolean
daSFHypnogram_motion_notify_event_cb(
	GtkWidget*,
	GdkEventMotion *event,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.hypnogram_button_down ) {
		SF.set_cur_vpage( (event->x / SF.da_wd) * SF.total_vpages());
		gdk_event_request_motions( event);
		SF.queue_redraw_all();
	}
	return TRUE;
}


void
iSFScoreAssist_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	if ( agh::rk1968::score( SF.sepisode()) == 0 ) {
		SF.get_hypnogram();
		SF.calculate_scored_percent();
		//SF.repaint_score_stats();
		SF.queue_redraw_all();
	}
}



void
iSFScoreImport_activate_cb(
	GtkMenuItem*,
	gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.do_dialog_import_hypnogram();
}

void
iSFScoreExport_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.do_dialog_export_hypnogram();
}



void
iSFScoreClear_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.do_clear_hypnogram();
}

} // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
