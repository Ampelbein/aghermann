// ;-*-C++-*-
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

using namespace std;

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
	aghui::__ss__ = g_string_new( "");

      // tell me what they are
	aghui::__visual = gdk_visual_get_system();

      // load glade
	aghui::__builder = gtk_builder_new();
	if ( !gtk_builder_add_from_file( aghui::__builder, PACKAGE_DATADIR "/" PACKAGE "/" AGH_UI_FILE, NULL) ) {
		pop_ok_message( NULL, "Failed to load UI description file.");
		return -1;
	}

	gtk_builder_connect_signals( aghui::__builder, NULL);

	return 0;
}



// EOF
