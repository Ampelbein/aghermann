// ;-*-C++-*-
/*
 *       File name:  ui/session-chooser_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-09
 *
 *         Purpose:  SSession::SSessionChooser callbacks
 *
 *         License:  GPL
 */


#include "../common/string.hh"
#include "ui.hh"
#include "session-chooser.hh"
#include "session-chooser_cb.hh"

using namespace std;
using namespace aghui;


extern "C" {

void
wSessionChooser_show_cb( GtkWidget *wid, gpointer userdata)
{
	auto& SC = *(SSessionChooser*)userdata;
	SC.last_dir_no = -1;

	SC.conditionally_enable_buttons();
}

void
wSessionChooser_destroy_cb( GtkWidget *wid, gpointer userdata)
{
	auto& SC = *(SSessionChooser*)userdata;

	SC.write_sessionrc();

	gtk_main_quit();
}

void
tvSessionChooserList_changed_cb( GtkTreeSelection *selection, gpointer userdata)
{
	auto& SC = *(SSessionChooser*)userdata;
	SC.conditionally_enable_buttons();
}

void
tvSessionChooserList_row_activated_cb( GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer userdata)
{
	bSessionChooserOpen_clicked_cb( nullptr, userdata);
}

void
bSessionChooserOpen_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& SC = *(SSessionChooser*)userdata;

	aghui::SBusyBlock bb (SC.wSessionChooser);

	int ret = SC.open_selected_session();

	if ( ret )
		gtk_widget_show( (GtkWidget*)SC.wSessionChooser);
	else
		gtk_widget_hide( (GtkWidget*)SC.wSessionChooser);
}



void
bSessionChooserClose_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& SC = *(SSessionChooser*)userdata;

	SC.close_current_session();

	gtk_widget_show_all( (GtkWidget*)SC.wSessionChooser);
}

void
bSessionChooserQuit_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SC = *(SSessionChooser*)userdata;

	// GtkTreeSelection *selection = gtk_tree_view_get_selection( (GtkTreeView*)SC.tvSessionChooserList);
	// GtkTreeModel *model;
	// GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
	// if ( paths ) {
	// 	GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
	// 	g_list_free( paths);

	// 	SC.last_dir_no = gtk_tree_path_get_indices( path)[0];

	// 	// gtk_tree_path_free( path); // leak it
	// }

	// user is quitting from the selector, not directly from an expdesign, so

	SC.write_sessionrc();

	gtk_main_quit();
}



void
bSessionChooserCreateNew_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SC = *(SSessionChooser*)userdata;

	GtkWidget *dir_chooser = gtk_file_chooser_dialog_new( "Locate New Experiment Directory",
							      NULL,
							      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
							      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							      NULL);
	if ( gtk_dialog_run( (GtkDialog*)dir_chooser) == GTK_RESPONSE_ACCEPT ) {
		const char *new_dir = gtk_file_chooser_get_filename( (GtkFileChooser*)dir_chooser);
		string new_dir_ {new_dir};
		g_free( (void*)new_dir);
		new_dir = agh::str::homedir2tilda(new_dir_).c_str();

		GtkTreeIter iter, iter_cur;
		GtkTreePath *path;
		gtk_tree_view_get_cursor( SC.tvSessionChooserList, &path, NULL);
		if ( path ) {
			gtk_tree_model_get_iter( (GtkTreeModel*)SC.mSessionChooserList, &iter_cur, path);
			gtk_list_store_insert_after( SC.mSessionChooserList, &iter, &iter_cur);
		} else
			gtk_list_store_append( SC.mSessionChooserList, &iter);

		gtk_list_store_set( SC.mSessionChooserList, &iter,
				    2, new_dir,
				    -1);

		if ( path )
			gtk_tree_path_next( path);
		else
			path = gtk_tree_model_get_path( (GtkTreeModel*)SC.mSessionChooserList, &iter);
		gtk_tree_view_set_cursor( SC.tvSessionChooserList,
					  path, NULL, TRUE);

		SC._sync_model_to_list();

		gtk_tree_path_free( path);

		SC.conditionally_enable_buttons();
	}

	gtk_widget_destroy( dir_chooser);
}


void
bSessionChooserRemove_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SC = *(SSessionChooser*)userdata;

	GtkTreeSelection *selection = gtk_tree_view_get_selection( SC.tvSessionChooserList);
	GtkTreeModel *model;
	GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
	if ( !paths )
		return;
	GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
	g_list_free( paths);

	GtkTreeIter iter;
	char *entry;
	gtk_tree_model_get_iter( model, &iter, path);
	gtk_tree_model_get( model, &iter,
			    0, &entry,
			    -1);
	// SC.sessions.remove_if( [&entry]( aghui::SSession& S) { return S == entry; });
	gtk_list_store_remove( SC.mSessionChooserList, &iter);
	SC._sync_model_to_list();

	gtk_tree_path_free( path);

	SC.last_dir_no = -1;

	SC.conditionally_enable_buttons();
	gtk_widget_show( (GtkWidget*)SC.wSessionChooser);
}

}

// eof
