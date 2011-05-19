// ;-*-C++-*- *  Time-stamp: "2011-05-19 02:29:53 hmmr"
/*
 *       File name:  ui/statusbar.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  main menu statusbar widgets
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "ui.hh"

using namespace std;


namespace aghui {

GtkStatusbar
	*sbMainStatusBar;
GtkButton
	*bExpChange;

GtkDialog
	*wScanLog;
GtkTextView
	*lScanLog,
	*tREADME;

namespace sb {


guint	sbContextIdGeneral;


inline namespace {

}


void
progress_indicator( const char* current, size_t n, size_t i)
{
	snprintf_buf( "(%zu of %zu) %s", i, n, current);
	buf_on_status_bar();
}





int
construct_once()
{
	if ( !(AGH_GBGETOBJ (GtkStatusbar,	sbMainStatusBar)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bExpChange)) )
		return -1;

	sbContextIdGeneral = gtk_statusbar_get_context_id( sbMainStatusBar, "General context");

	if ( !(AGH_GBGETOBJ (GtkTextView,	tREADME)) )
		return -1;

	if ( !(AGH_GBGETOBJ (GtkDialog,		wScanLog)) ||
	     !(AGH_GBGETOBJ (GtkTextView,	lScanLog)) )
		return -1;

	char *contents;
	snprintf_buf( "%s/doc/%s/README", PACKAGE_DATADIR, PACKAGE);
	GFile *file = g_file_new_for_path( __buf__);
	gtk_text_buffer_set_text(
		gtk_text_view_get_buffer( tREADME),
		g_file_load_contents( file, NULL, &contents, NULL, NULL, NULL)
		? contents
		: "(The contents of " PACKAGE_DATADIR "/README was supposed to be here;\n"
		  "this file was not found in that location, too bad.)", -1);
	g_object_unref( file);

	return 0;
}

} // namespce sb









extern "C" {

	void
	bExpChange_clicked_cb( GtkButton *button, gpointer userdata)
	{
		gtk_window_get_position( wMainWindow, &GeometryMain.x, &GeometryMain.y);
		gtk_window_get_size( wMainWindow, &GeometryMain.w, &GeometryMain.h);

		gtk_widget_hide( (GtkWidget*)wMainWindow);
		// if ( gtk_widget_get_visible( (GtkWidget*)wScoringFacility) )
		// 	gtk_widget_hide( (GtkWidget*)wScoringFacility);
		// better make sure bExpChange is greyed out on opening any child windows
		gtk_widget_show( (GtkWidget*)wExpDesignChooser);
	}



	void
	bScanTree_clicked_cb( GtkButton *button, gpointer userdata)
	{
		do_rescan_tree();
	}





	gboolean
	wMainWindow_destroy_event_cb( GtkWidget *wid, gpointer userdata)
	{
		gtk_window_get_position( wMainWindow, &GeometryMain.x, &GeometryMain.y);
		gtk_window_get_size( wMainWindow, &GeometryMain.w, &GeometryMain.h);

		gtk_main_quit();

		return FALSE; // whatever
	}

	gboolean
	wMainWindow_delete_event_cb( GtkWidget *wid, gpointer userdata)
	{
		return wMainWindow_destroy_event_cb( wid, NULL);
	}

} // extern "C"


} // namespace aghui

// eof
