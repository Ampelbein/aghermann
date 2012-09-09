// ;-*-C++-*-
/*
 *       File name:  ui/session-chooser-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-10
 *
 *         Purpose:  experiment design selector (construct widgets)
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "ui.hh"
#include "session-chooser.hh"
#include "session-chooser_cb.hh"

using namespace std;



int
aghui::SSessionChooser::
construct_widgets()
{
      // load glade
	GtkBuilder *builder = gtk_builder_new();
	if ( !gtk_builder_add_from_file( builder, PACKAGE_DATADIR "/" PACKAGE "/" AGH_UI_SESSION_CHOOSER_GLADE, NULL) ) {
		pop_ok_message( NULL, "Failed to load " PACKAGE_DATADIR "/" PACKAGE "/" AGH_UI_SESSION_CHOOSER_GLADE);
		return -1;
	}

	gtk_builder_connect_signals( builder, NULL);

	GtkCellRenderer *renderer;

	mSessionChooserList =
		gtk_list_store_new( 1, G_TYPE_STRING);

	if ( !(AGH_GBGETOBJ (GtkDialog, 	wSessionChooser)) ||
	     !(AGH_GBGETOBJ (GtkTreeView,	tvSessionChooserList)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserOpen)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserClose)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserCreateNew)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserRemove)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserQuit)) ||
	     !(AGH_GBGETOBJ (GtkStatusbar,	sbSessionChooserStatusBar)) )
		return -1;

	g_signal_connect( wSessionChooser, "show",
			  (GCallback)wSessionChooser_show_cb,
			  this);
	g_signal_connect( wSessionChooser, "hide",
			  (GCallback)wSessionChooser_hide_cb,
			  this);

	g_signal_connect( tvSessionChooserList, "row-activated",
			  (GCallback)tvSessionChooserList_row_activated_cb,
			  this);
	g_signal_connect( gtk_tree_view_get_selection( tvSessionChooserList), "changed",
			  (GCallback)tvSessionChooserList_changed_cb,
			  this);
	g_signal_connect( bSessionChooserOpen, "clicked",
			  (GCallback)bSessionChooserOpen_clicked_cb,
			  this);
	g_signal_connect( bSessionChooserClose, "clicked",
			  (GCallback)bSessionChooserClose_clicked_cb,
			  this);
	g_signal_connect( bSessionChooserCreateNew, "clicked",
			  (GCallback)bSessionChooserCreateNew_clicked_cb,
			  this);
	g_signal_connect( bSessionChooserRemove, "clicked",
			  (GCallback)bSessionChooserRemove_clicked_cb,
			  this);
	g_signal_connect( bSessionChooserQuit, "clicked",
			  (GCallback)bSessionChooserQuit_clicked_cb,
			  this);

	gtk_tree_view_set_model( tvSessionChooserList,
				 (GtkTreeModel*)mSessionChooserList);

	g_object_set( (GObject*)tvSessionChooserList,
		      "headers-visible", FALSE,
		      NULL);

	renderer = gtk_cell_renderer_text_new();
	g_object_set( (GObject*)renderer, "editable", FALSE, NULL);
	g_object_set_data( (GObject*)renderer, "column", GINT_TO_POINTER (0));
	gtk_tree_view_insert_column_with_attributes( tvSessionChooserList,
						     -1, "Session", renderer,
						     "text", 0,
						     NULL);

	sbChooserContextIdGeneral = gtk_statusbar_get_context_id( sbSessionChooserStatusBar, "General context");

	g_object_unref( (GObject*)builder);

	return 0;
}

// eof
