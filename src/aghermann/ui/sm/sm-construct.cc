/*
 *       File name:  aghermann/ui/sm/sm-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-10
 *
 *         Purpose:  session manager (widgets)
 *
 *         License:  GPL
 */


#include "aghermann/ui/ui.hh"
#include "aghermann/ui/misc.hh"

#include "sm.hh"
#include "sm_cb.hh"

using namespace std;


int
agh::ui::SSessionChooser::
construct_widgets()
{
      // load glade
	GtkBuilder *builder = gtk_builder_new();
	if ( !gtk_builder_add_from_resource(
		     builder,
		     "/org/gtk/aghermann/sm.glade",
		     NULL) ) {
		return -1;
	}

	gtk_builder_connect_signals( builder, NULL);

	GtkCellRenderer *renderer;

	mSessionChooserList =
		gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	if ( !(AGH_GBGETOBJ (GtkDialog, 	wSessionChooser)) ||
	     !(AGH_GBGETOBJ (GtkTreeView,	tvSessionChooserList)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserOpen)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserClose)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserCreateNew)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserRemove)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bSessionChooserQuit)) ||
	     !(AGH_GBGETOBJ (GtkStatusbar,	sbSessionChooserStatusBar)) )
		return -1;

	G_CONNECT_1 (wSessionChooser, show);
	G_CONNECT_1 (wSessionChooser, destroy);

	G_CONNECT_2 (tvSessionChooserList, row, activated);
	g_signal_connect( gtk_tree_view_get_selection( tvSessionChooserList), "changed",
			  (GCallback)tvSessionChooserList_changed_cb,
			  this);
	G_CONNECT_1 (bSessionChooserOpen, clicked);
	G_CONNECT_1 (bSessionChooserClose, clicked);
	G_CONNECT_1 (bSessionChooserCreateNew, clicked);
	G_CONNECT_1 (bSessionChooserRemove, clicked);
	G_CONNECT_1 (bSessionChooserQuit, clicked);

	gtk_tree_view_set_model( tvSessionChooserList,
				 (GtkTreeModel*)mSessionChooserList);

	g_object_set( (GObject*)tvSessionChooserList,
		      "headers-visible", FALSE,
		      NULL);

	gtk_tree_view_set_model( tvSessionChooserList, (GtkTreeModel*)mSessionChooserList);
	int c = 0;
	for ( auto& C : {"Last visited", "Recordings", "Directory"} ) {
		renderer = gtk_cell_renderer_text_new();
		g_object_set( (GObject*)renderer,
			      "editable", FALSE,
			      "xalign", (c != 2) ? 1. : 0.,
			      NULL);
		g_object_set_data( (GObject*)renderer, "column", GINT_TO_POINTER (c));
		gtk_tree_view_insert_column_with_attributes( tvSessionChooserList,
							     -1, C, renderer,
							     "text", c,
							     NULL);
		++c;
	}

	gtk_tree_view_set_headers_visible( tvSessionChooserList, TRUE);

	sbChooserContextIdGeneral = gtk_statusbar_get_context_id( sbSessionChooserStatusBar, "General context");

	g_object_unref( (GObject*)builder);

	return 0;
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
