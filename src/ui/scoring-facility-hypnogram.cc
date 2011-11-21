// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-hypnogram.cc
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
#include "expdesign.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

using namespace aghui;


inline namespace {
	unsigned short __score_hypn_depth[8] = {
		0, 20, 23, 30, 33, 5, 10, 1
	};
}


void
aghui::SScoringFacility::draw_hypnogram( cairo_t *cr)
{
      // bg
	_p.CwB[SExpDesignUI::TColour::hypnogram].set_source_rgb( cr);
	cairo_rectangle( cr, 0., 0., da_wd, HypnogramHeight);
	cairo_fill( cr);
	cairo_stroke( cr);

	_p.CwB[SExpDesignUI::TColour::hypnogram_scoreline].set_source_rgba( cr, .5);
	cairo_set_line_width( cr, .4);
	for ( size_t i = 1; i < (size_t)sigfile::SPage::TScore::_total; ++i ) {
		cairo_move_to( cr, 0,     __score_hypn_depth[i]);
		cairo_line_to( cr, da_wd, __score_hypn_depth[i]);
	}
	cairo_stroke( cr);

      // scores
	_p.CwB[SExpDesignUI::TColour::hypnogram_scoreline].set_source_rgba( cr, 1.);
	cairo_set_line_width( cr, 3.);
	// these lines can be discontinuous
	for ( size_t i = 0; i < total_pages(); ++i ) {
		char c;
		if ( (c = hypnogram[i]) != sigfile::SPage::score_code( sigfile::SPage::TScore::none) ) {
			int y = __score_hypn_depth[ (size_t)sigfile::SPage::char2score(c) ];
			cairo_move_to( cr, (float)i/total_pages() * da_wd, y);
			cairo_rel_line_to( cr, 1./total_pages() * da_wd, 0);
		}
	}
	cairo_stroke( cr);

      // extra: annotations
	{
		_p.CwB[SExpDesignUI::TColour::annotations].set_source_rgba( cr, .6);
		cairo_set_line_width( cr, 18.);

	      // // get aggregate annotations from all channels
	      // 	list<agh::CEDFFile::SSignal::SAnnotation*>
	      // 		agg;
		auto total_seconds = total_pages() * pagesize();
		for ( auto &H : channels ) {
			size_t this_sr = H.samplerate();
			for ( auto &A : H.annotations ) {
				// agg.push_back( &A);
				cairo_move_to( cr, (double)A.span.first / this_sr / total_seconds * da_wd, 2);
				cairo_line_to( cr, (double)A.span.second / this_sr / total_seconds * da_wd, 2);
			}
		}
		cairo_stroke( cr);
	}

      // cursor
	_p.CwB[SExpDesignUI::TColour::cursor].set_source_rgba( cr, .7);
	cairo_rectangle( cr,
			 (float) cur_vpage() / total_vpages() * da_wd,  0,
			 ceil( 1. / total_vpages() * da_wd), da_ht-1);
	cairo_fill( cr);

	cairo_stroke( cr);
}


extern "C" {

// -------------------- Hypnogram


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
		gtk_spin_button_set_value( SF.eSFCurrentPage,
					   (event->x / SF.da_wd) * SF.total_vpages()+1);
	    break;
	case 3:
		gtk_menu_popup( SF.mSFScore,
				NULL, NULL, NULL, NULL, 3, event->time);
	    break;
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

	GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Import Scores",
							    NULL,
							    GTK_FILE_CHOOSER_ACTION_OPEN,
							    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							    NULL);
	if ( gtk_dialog_run( (GtkDialog*)f_chooser) == GTK_RESPONSE_ACCEPT ) {
		gchar *fname = gtk_file_chooser_get_filename( (GtkFileChooser*)f_chooser);
		for ( auto &F : SF.sepisode().sources )
			F.load_canonical( fname, SF._p.ext_score_codes);
		SF.get_hypnogram();
		SF.calculate_scored_percent();
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
			     sigfile::SPage::score_code( sigfile::SPage::TScore::none));
	SF.put_hypnogram();  // side-effect being, implicit flash of SScoringFacility::sepisode.sources
	SF.calculate_scored_percent();
	SF.queue_redraw_all();
}






} // extern "C"


// eof

