/*
 *       File name:  aghermann/ui/sf/hypnogram.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-27
 *
 *         Purpose:  scoring facility (hypnogram)
 *
 *         License:  GPL
 */


#include <fstream>

#include <cairo/cairo.h>

#include "sf.hh"

using namespace std;
using namespace agh::ui;


namespace {

unsigned short __score_hypn_depth[8] = {
	0, 20, 23, 30, 33, 5, 10, 1
};

}


void
SScoringFacility::
draw_hypnogram( cairo_t *cr)
{
      // bg
	_p.CwB[SExpDesignUI::TColour::sf_hypnogram].set_source_rgb( cr);
	cairo_rectangle( cr, 0., 0., da_wd, HypnogramHeight);
	cairo_fill( cr);
	cairo_stroke( cr);

	if ( alt_hypnogram ) {
		for ( size_t i = 0; i < total_pages(); ++i ) {
			auto s = sigfile::SPage::char2score( hypnogram[i]);
			if ( s != sigfile::SPage::TScore::none ) {
				_p.CwB[SExpDesignUI::score2colour(s)].set_source_rgba( cr, .4);
				cairo_rectangle( cr,
						 (float)i/total_pages() * da_wd, 0,
						 1./total_pages() * da_wd, da_ht);
				cairo_fill( cr);
				cairo_stroke( cr);
			}
		}
	} else {
		_p.CwB[SExpDesignUI::TColour::sf_hypnogram].set_source_rgba_contrasting( cr, .4);
		cairo_set_line_width( cr, .4);
		for ( size_t i = 1; i < (size_t)sigfile::SPage::TScore::TScore_total; ++i ) {
			cairo_move_to( cr, 0,     __score_hypn_depth[i]);
			cairo_line_to( cr, da_wd, __score_hypn_depth[i]);
		}
		cairo_stroke( cr);

	      // scores
		_p.CwB[SExpDesignUI::TColour::sf_hypnogram].set_source_rgba_contrasting( cr, 1.);
		cairo_set_line_width( cr, 3.);
		// these lines can be discontinuous
		for ( size_t i = 0; i < total_pages(); ++i ) {
			char c = hypnogram[i];
			if ( c != sigfile::SPage::score_code( sigfile::SPage::TScore::none) ) {
				int y = __score_hypn_depth[ (size_t)sigfile::SPage::char2score(c) ];
				cairo_move_to( cr, (float)i/total_pages() * da_wd, y);
				cairo_rel_line_to( cr, 1./total_pages() * da_wd, 0);
			}
		}
		cairo_stroke( cr);
	}

      // extra: annotations
	{
		_p.CwB[SExpDesignUI::TColour::sf_annotations].set_source_rgba( cr, .9);
		cairo_set_line_width( cr, 18.);

		auto total_seconds = total_pages() * pagesize();
		for ( auto &H : channels ) {
			for ( auto &A : H.annotations ) {
				cairo_move_to( cr, A.span.a / total_seconds * da_wd, 4);
				cairo_line_to( cr, A.span.z / total_seconds * da_wd, 4);
			}
		}
		cairo_stroke( cr);

		_p.CwB[SExpDesignUI::TColour::sf_embedded_annotations].set_source_rgba( cr, .9);
		for ( auto &SA : common_annotations ) {
			auto& A = *SA.second;
			cairo_move_to( cr, A.span.a / total_seconds * da_wd - 1, 4); // extend by one pixel to prevent
			cairo_line_to( cr, A.span.z / total_seconds * da_wd + 1, 4); // zero-length annotations to vanish
		}

		cairo_stroke( cr);
	}

      // extra: artifacts
	{
		_p.CwB[SExpDesignUI::TColour::sf_artifact].set_source_rgba( cr);
		cairo_set_line_width( cr, 12.);

		auto total_seconds = total_pages() * pagesize();
		for ( auto &H : channels ) {
			for ( auto &A : H.artifacts() ) {
				cairo_move_to( cr, A.a / total_seconds * da_wd - 1, 12);
				cairo_line_to( cr, A.z / total_seconds * da_wd + 1, 12);
			}
		}
		cairo_stroke( cr);
	}

      // hour ticks
	_draw_hour_ticks( cr, 0, HypnogramHeight);
}







void
SScoringFacility::
do_dialog_import_hypnogram()
{
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
		size_t our_pages = sepisode().sources.front().pages();
		if ( c != our_pages && // allow for last page scored but discarded in CHypnogram as incomplete
		     c != our_pages+1 )
			pop_ok_message(
				wSF,
				"Page count in current hypnogram (%zu,"
				" even allowing for one incomplete extra) is not equal"
				" to the number of lines in <i>%s</i> (%zu).\n\n"
				"Please trim the file contents and try again.",
				fname, c, our_pages);
		else {
			for ( auto &F : sepisode().sources )
				F.load_canonical( fname, _p.ext_score_codes);
			get_hypnogram();
			calculate_scored_percent();
			queue_redraw_all();
		}
	}
	gtk_widget_destroy( f_chooser);
}


void
SScoringFacility::
do_dialog_export_hypnogram() const
{
	GtkWidget *f_chooser =
		gtk_file_chooser_dialog_new(
			"Export Scores",
			NULL,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	if ( gtk_dialog_run( (GtkDialog*)f_chooser) == GTK_RESPONSE_ACCEPT ) {
		gchar *fname = gtk_file_chooser_get_filename( (GtkFileChooser*)f_chooser);
		// put_hypnogram();  // side-effect being, implicit flash of SScoringFacility::sepisode.sources // do this elsewhere
		sepisode().sources.front().save_canonical( fname);
	}
	gtk_widget_destroy( f_chooser);
}



void
SScoringFacility::
do_clear_hypnogram()
{
	hypnogram.assign(
		hypnogram.size(),
		sigfile::SPage::score_code( sigfile::SPage::TScore::none));
	put_hypnogram();  // side-effect being, implicit flash of SScoringFacility::sepisode.sources
	calculate_scored_percent();
	queue_redraw_all();
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
