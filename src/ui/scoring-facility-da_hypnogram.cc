// ;-*-C++-*- *  Time-stamp: "2011-06-07 19:33:54 hmmr"
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
	unsigned short __score_hypn_depth[8] = {
		0, 20, 23, 30, 33, 5, 10, 1
	};
}




} // namespace sf



// callbacks


using namespace aghui;
using namespace aghui::sf;

extern "C" {



// -------------------- Hypnogram

	gboolean
	daScoringFacHypnogram_configure_event_cb( GtkWidget *wid, GdkEventConfigure *event, gpointer userdata)
	{
		FAFA;
		if ( event->type == GDK_CONFIGURE ) {
			auto& SF = *(SScoringFacility*)userdata;
			SF.daScoringFacHypnogram_ht = event->height;
			SF.daScoringFacHypnogram_wd = event->width;
		}
		return FALSE;

	}


	gboolean
	daScoringFacHypnogram_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
	{
		FAFA;
		auto& SF = *(SScoringFacility*)userdata;

//		cairo_t *cr = gdk_cairo_create( gtk_widget_get_window(wid));

		// bg
		CwB[TColour::hypnogram].set_source_rgb( cr);
		cairo_rectangle( cr, 0., 0., SF.daScoringFacHypnogram_wd, SF.daScoringFacHypnogram_ht);
		cairo_fill( cr);
		cairo_stroke( cr);

		CwB[TColour::hypnogram_scoreline].set_source_rgba( cr, .5);
		cairo_set_line_width( cr, .4);
		for ( size_t i = 1; i < (size_t)TScore::_total; ++i ) {
			cairo_move_to( cr, 0,   __score_hypn_depth[i]);
			cairo_line_to( cr, SF.daScoringFacHypnogram_wd,  __score_hypn_depth[i]);
		}
		cairo_stroke( cr);

		// scores
		CwB[TColour::hypnogram_scoreline].set_source_rgba( cr, 1.);
		cairo_set_line_width( cr, 3.);
		// these lines can be discontinuous
		for ( size_t i = 0; i < SF.total_pages(); ++i ) {
			char c;
			if ( (c = SF.hypnogram[i]) != agh::SPage::score_code( TScore::none) ) {
				int y = __score_hypn_depth[ (size_t)agh::SPage::char2score(c) ];
				cairo_move_to( cr, lroundf( (float) i   /SF.total_pages() * SF.daScoringFacHypnogram_wd), y);
				cairo_line_to( cr, lroundf( (float)(i+1)/SF.total_pages() * SF.daScoringFacHypnogram_wd), y);
			}
		}
		cairo_stroke( cr);

		CwB[TColour::cursor].set_source_rgba( cr, .7);
		cairo_rectangle( cr,
				 (float) SF.cur_vpage() / SF.total_vpages() * SF.daScoringFacHypnogram_wd,  0,
				 ceil( 1. / SF.total_vpages() * SF.daScoringFacHypnogram_wd), SF.daScoringFacHypnogram_ht-1);
		cairo_fill( cr);

		cairo_stroke( cr);
//		cairo_destroy( cr);

		return TRUE;
	}




	gboolean
	daScoringFacHypnogram_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;

		switch ( event->button ) {
		case 1:
			gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
						   (event->x / SF.daScoringFacHypnogram_wd) * SF.total_vpages()+1);
		    break;
		case 3:
			gtk_menu_popup( SF.mSFScore,
					NULL, NULL, NULL, NULL, 3, event->time);
		    break;
		}
		return TRUE;
	}






// -- Score

	void
	iSFScoreAssist_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;

		if ( SF.sepisode().assisted_score() == 0 ) {
			SF.get_hypnogram();
			SF.calculate_scored_percent();
			SF.repaint_score_stats();
			SF.queue_redraw_all();
		}
	}



	void
	iSFScoreImport_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;

		GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Import Scores",
								    NULL,
								    GTK_FILE_CHOOSER_ACTION_OPEN,
								    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
								    NULL);
		if ( gtk_dialog_run( (GtkDialog*)f_chooser) == GTK_RESPONSE_ACCEPT ) {
			gchar *fname = gtk_file_chooser_get_filename( (GtkFileChooser*)f_chooser);
			for_each( SF.sepisode().sources.begin(), SF.sepisode().sources.end(),
				  [&] ( agh::CEDFFile& F)
				  {
					  F.load_canonical( fname, settings::ExtScoreCodes);
				  });
			SF.get_hypnogram();
			SF.calculate_scored_percent();
			SF.repaint_score_stats();
			SF.queue_redraw_all();
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
				     agh::SPage::score_code( agh::TScore::none));
		SF.put_hypnogram();  // side-effect being, implicit flash of SScoringFacility::sepisode.sources
		SF.calculate_scored_percent();
		SF.repaint_score_stats();
		SF.queue_redraw_all();
//		snprintf_buf( "<b>%3.1f</b> %% scored", scored_percent());
//		gtk_label_set_markup( lScoringFacPercentScored, __buf__);
	}






} // extern "C"

} // namespace aghui


// eof

