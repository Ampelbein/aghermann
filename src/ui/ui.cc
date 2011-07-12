// ;-*-C++-*- *  Time-stamp: "2011-07-11 23:27:37 hmmr"
/*
 *       File name:  ui/ui.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  common ui variables, GTK+ widgets, and models
 *
 *         License:  GPL
 */



#include "misc.hh"
#include "ui.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"
#include "modelrun-facility.hh"


#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;
using namespace aghui;

GString	*aghui::__ss__;

GtkBuilder
	*aghui::__builder;

GdkVisual
	*aghui::__visual;


using namespace aghui;

inline namespace {
}

int
aghui::prepare_for_expdesign()
{
	__ss__ = g_string_new( "");

      // tell me what they are
	__visual = gdk_visual_get_system();

      // load glade
	__builder = gtk_builder_new();
	if ( !gtk_builder_add_from_file( __builder, PACKAGE_DATADIR "/" PACKAGE "/" AGH_UI_FILE, NULL) ) {
		pop_ok_message( NULL, "Failed to load UI description file.");
		return -1;
	}

	gtk_builder_connect_signals( __builder, NULL);

	// populate_static_models();
	// -- is in glade, where it belongs

	return 0;
}







void
aghui::pop_ok_message( GtkWindow *parent, const char *str, ...)
{
	va_list ap;
	va_start (ap, str);

	static GString *buf = NULL;
	if ( buf == NULL )
		buf = g_string_new("");

	g_string_vprintf( buf, str, ap);
	GtkWidget *msg = gtk_message_dialog_new( parent,
						 (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
						 GTK_MESSAGE_INFO,
						 GTK_BUTTONS_OK,
						 buf->str, NULL);
	va_end (ap);

	gtk_dialog_run( (GtkDialog*)msg);
	gtk_widget_destroy( msg);
}


int
aghui::pop_question( GtkWindow* parent, const gchar *str)
{
	GtkWidget *msg = gtk_message_dialog_new( parent,
						 (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
						 GTK_MESSAGE_INFO,
						 GTK_BUTTONS_YES_NO,
						 str, NULL);
	gint response = gtk_dialog_run( (GtkDialog*)msg);
	gtk_widget_destroy( msg);
	return response;
}




void
aghui::set_cursor_busy( bool busy, GtkWidget *wid)
{
	static GdkCursor *cursor_busy   = NULL,
			 *cursor_normal = NULL;
	if ( cursor_normal == NULL) {
		cursor_busy   = gdk_cursor_new_from_name( gdk_display_get_default(), "watch");
		cursor_normal = gdk_cursor_new_from_name( gdk_display_get_default(), "left_ptr");
	}

	gdk_window_set_cursor( gtk_widget_get_window( wid), busy ? cursor_busy : cursor_normal);

	while ( gtk_events_pending () )
		gtk_main_iteration();
}


// EOF
