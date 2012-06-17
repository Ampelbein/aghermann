// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-hypnogram_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-15
 *
 *         Purpose:  scoring facility (hypnogram) (callbacks)
 *
 *         License:  GPL
 */




#include <fstream>

#include "misc.hh"
#include "ui.hh"
#include "scoring-facility.hh"

using namespace std;

using namespace aghui;

extern "C" {



gboolean
daSFHypnogram_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.draw_hypnogram( cr);

	return TRUE;
}




gboolean
daSFHypnogram_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	switch ( event->button ) {
	case 1:
		SF.set_cur_vpage( (event->x / SF.da_wd) * SF.total_vpages());
		SF.hypnogram_button_down = true;
	    break;
	case 2:
		SF.alt_hypnogram = !SF.alt_hypnogram;
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
	    break;
	case 3:
		gtk_menu_popup( SF.mSFScore,
				NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}
	return TRUE;
}

gboolean
daSFHypnogram_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	switch ( event->button ) {
	case 1:
		SF.hypnogram_button_down = false;
	    break;
	}
	return TRUE;
}



gboolean
daSFHypnogram_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.hypnogram_button_down ) {
		SF.set_cur_vpage( (event->x / SF.da_wd) * SF.total_vpages());
		gdk_event_request_motions( event);
	}
	return TRUE;
}


void
iSFScoreAssist_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	if ( SF.sepisode().assisted_score() == 0 ) {
		SF.get_hypnogram();
		SF.calculate_scored_percent();
		//SF.repaint_score_stats();
		SF.queue_redraw_all();
	}
}



void
iSFScoreImport_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	GtkWidget *f_chooser =
		gtk_file_chooser_dialog_new(
			"Import Scores",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	if ( gtk_dialog_run( (GtkDialog*)f_chooser) == GTK_RESPONSE_ACCEPT ) {
		gchar *fname = gtk_file_chooser_get_filename( (GtkFileChooser*)f_chooser);
		// count lines first
		ifstream f (fname);
		string t;
		size_t c = 0;
		while ( not getline(f, t).eof() )
			++c;
		size_t our_pages = SF.sepisode().sources.front().pages();
		if ( c != our_pages && // allow for last page scored but discarded in CHypnogram as incomplete
		     c != our_pages+1 )
			pop_ok_message(
				SF.wScoringFacility,
				"Page count in current hypnogram (%zu,"
				" even allowing for one incomplete extra) is not equal"
				" to the number of lines in <i>%s</i> (%zu).\n\n"
				"Please trim the file contents and try again.",
				fname, c, our_pages);
		else {
			for ( auto &F : SF.sepisode().sources )
				F.load_canonical( fname, SF._p.ext_score_codes);
			SF.get_hypnogram();
			SF.calculate_scored_percent();
			SF.queue_redraw_all();
		}
	}
	gtk_widget_destroy( f_chooser);
}

void
iSFScoreExport_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Export Scores",
							    NULL,
							    GTK_FILE_CHOOSER_ACTION_SAVE,
							    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							    NULL);
	if ( gtk_dialog_run( (GtkDialog*)f_chooser) == GTK_RESPONSE_ACCEPT ) {
		gchar *fname = gtk_file_chooser_get_filename( (GtkFileChooser*)f_chooser);
		SF.put_hypnogram();  // side-effect being, implicit flash of SScoringFacility::sepisode.sources
		SF.sepisode().sources.front().save_canonical( fname);
	}
	gtk_widget_destroy( f_chooser);
}



void
iSFScoreClear_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.hypnogram.assign( SF.hypnogram.size(),
			     sigfile::SPage::score_code( sigfile::SPage::TScore::none));
	SF.put_hypnogram();  // side-effect being, implicit flash of SScoringFacility::sepisode.sources
	SF.calculate_scored_percent();
	SF.queue_redraw_all();
}

} // extern "C"


// eof
