// ;-*-C++-*- *  Time-stamp: "2011-04-28 02:30:42 hmmr"
/*
 *       File name:  ui/scoring-facility-da_hypnogram.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-27
 *
 *         Purpose:  scoring facility (hypnogram)
 *
 *         License:  GPL
 */




#include <cairo/cairo-svg.h>

#include "misc.hh"
#include "ui.hh"
#include "settings.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {
namespace sf {


inline namespace {

}




} // namespace sf



// callbacks



extern "C" {



// -------------------- Hypnogram

gboolean
daScoringFacHypnogram_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer unused)
{
	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	cairo_t *cr = gdk_cairo_create( wid->window);

      // bg
	cairo_set_source_rgb( cr,
			      (double)__bg1__[cHYPNOGRAM].red/65536,
			      (double)__bg1__[cHYPNOGRAM].green/65536,
			      (double)__bg1__[cHYPNOGRAM].blue/65536);
	cairo_rectangle( cr, 0., 0., wd, ht);
	cairo_fill( cr);
	cairo_stroke( cr);

	cairo_set_source_rgba( cr,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].red/65536,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].green/65536,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].blue/65536,
			       .5);
	cairo_set_line_width( cr, .4);
	guint i;
	for ( i = 1; i < 8; ++i ) {
		cairo_move_to( cr, 0,   __score_hypn_depth[i]);
		cairo_line_to( cr, wd,  __score_hypn_depth[i]);
	}
	cairo_stroke( cr);

      // scores
	cairo_set_source_rgba( cr,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].red/65536,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].green/65536,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].blue/65536,
			       1.);
	cairo_set_line_width( cr, 3.);
	// these lines can be discontinuous
	for ( i = 0; i < __total_pages; ++i ) {
		gchar c;
		if ( (c = __hypnogram[i]) != AghScoreCodes[AGH_SCORE_NONE] ) {
			gint y = __score_hypn_depth[ SCOREID(c) ];
			cairo_move_to( cr, lroundf( (float) i   /__total_pages * wd), y);
			cairo_line_to( cr, lroundf( (float)(i+1)/__total_pages * wd), y);
		}
	}
	cairo_stroke( cr);

	cairo_set_source_rgba( cr,
			       (double)__bg1__[cCURSOR].red/65536,
			       (double)__bg1__[cCURSOR].green/65536,
			       (double)__bg1__[cCURSOR].blue/65536,
			       .7);
	cairo_rectangle( cr,
			 (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
			 ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht-1);
	cairo_fill( cr);

	cairo_stroke( cr);
	cairo_destroy( cr);

	return TRUE;
}




gboolean
daScoringFacHypnogram_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer unused)
{
	gint wd;
	gdk_drawable_get_size( wid->window, &wd, NULL);

	switch ( event->button ) {
	case 1:
		__cur_page = (event->x / wd) * __total_pages;
		__cur_page_app = P2AP (__cur_page);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	    break;
	case 3:
		gtk_menu_popup( GTK_MENU (mSFScore),
				NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}
	return TRUE;
}






// -- Score

void
iSFScoreAssist_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
{
	if ( agh_episode_assisted_score_by_jde( __our_j->name, __our_d, __our_e) == 0 ) {
		__have_unsaved_scores = TRUE;
		free( __hypnogram);
		agh_edf_get_scores( __source_ref, &__hypnogram, NULL);

		__repaint_score_stats();
		REDRAW_ALL;
	}
}



void
iSFScoreImport_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Import Scores",
							    NULL,
							    GTK_FILE_CHOOSER_ACTION_OPEN,
							    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							    NULL);
	if ( gtk_dialog_run( GTK_DIALOG (f_chooser)) == GTK_RESPONSE_ACCEPT ) {
		gchar *fname = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (f_chooser));
		agh_edf_import_scores_custom( __source_ref,
					      fname,
					      AghExtScoreCodes);
	}
	gtk_widget_destroy( f_chooser);
	free( __hypnogram);
	agh_edf_get_scores( __source_ref,
			    &__hypnogram, NULL);
	REDRAW_ALL;
	__repaint_score_stats();
}

void
iSFScoreExport_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
{
	agh_edf_put_scores( __source_ref,
			    __hypnogram);
	GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Export Scores",
							    NULL,
							    GTK_FILE_CHOOSER_ACTION_SAVE,
							    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							    NULL);
	if ( gtk_dialog_run( GTK_DIALOG (f_chooser)) == GTK_RESPONSE_ACCEPT ) {
		gchar *fname = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (f_chooser));
		agh_edf_export_scores( __source_ref, fname);
	}
	gtk_widget_destroy( f_chooser);
}



void
iSFScoreRevert_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
{
	free( __hypnogram);
	agh_edf_get_scores( __source_ref, &__hypnogram, NULL);

	REDRAW_ALL;
}

void
iSFScoreClear_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
{
	memset( __hypnogram, (int)AghScoreCodes[AGH_SCORE_NONE], __total_pages);

	REDRAW_ALL;

	snprintf_buf( "<b>%3.1f</b> %% scored", __percent_scored());
	gtk_label_set_markup( lScoringFacPercentScored, __buf__);
}






} // extern "C"

} // namespace aghui


// eof

