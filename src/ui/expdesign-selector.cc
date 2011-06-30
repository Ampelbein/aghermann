// ;-*-C++-*- *  Time-stamp: "2011-06-30 16:05:13 hmmr"
/*
 *       File name:  ui/expdesign-selector.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-15
 *
 *         Purpose:  experiment design selector
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "ui.hh"
#include "expdesign.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;

namespace aghui {



namespace expdselect {

inline namespace {

	string	wMainWindow_title;

	GtkListStore
		*mExpDesignList;

} // inline namespace


string	hist_filename;

void
read_histfile()
{
	using boost::property_tree::ptree;
	ptree pt;

	GtkTreeIter iter;
	try {
		read_xml( hist_filename, pt);
		settings::LastExpdesignDir = pt.get<string>( "Sessions.Last");
		string	list = pt.get<string>( "Sessions.List");

		settings::LastExpdesignDirNo = -1;

		guint i = 0;
		char *entry = strtok( &list[0], ";");
		gtk_list_store_clear( mExpDesignList);
		while ( entry && strlen( entry) ) {
			gtk_list_store_append( mExpDesignList, &iter);
			gtk_list_store_set( mExpDesignList, &iter,
					    0, entry,
					    -1);
			if ( settings::LastExpdesignDir == entry )
				settings::LastExpdesignDirNo = i;
			++i;
			entry = strtok( NULL, ";");
		}

	} catch (...) {
		gchar *cwd = g_get_current_dir();
		settings::LastExpdesignDir = g_build_filename( G_DIR_SEPARATOR_S,
							       cwd, "NewExperiment", NULL);
		g_free( cwd);

		settings::LastExpdesignDirNo = 0;

		gtk_list_store_clear( mExpDesignList);
		gtk_list_store_append( mExpDesignList, &iter);
		gtk_list_store_set( mExpDesignList, &iter,
				    0, settings::LastExpdesignDir.c_str(),
				    -1);
	}

	wMainWindow_title = string ("Aghermann: ") + settings::LastExpdesignDir;
	gtk_window_set_title( wMainWindow, wMainWindow_title.c_str());
}





void
write_histfile()
{
	GtkTreeIter iter;
	bool some_items_left =
		gtk_tree_model_get_iter_first( (GtkTreeModel*)mExpDesignList, &iter);
	if ( !some_items_left )
		settings::LastExpdesignDirNo = -1;

	char	*entry;
	string	agg;
	while ( some_items_left ) {
		gtk_tree_model_get( (GtkTreeModel*)mExpDesignList, &iter,  // at least one entry exists,
				    0, &entry,                             // added in read_histfile()
				    -1);
		agg += (string(entry) + ";");
		g_free( entry);
		some_items_left = gtk_tree_model_iter_next( (GtkTreeModel*)mExpDesignList, &iter);
	}

	using boost::property_tree::ptree;
	ptree pt;
	pt.put( "Sessions.List", agg);
	pt.put( "Sessions.Last", settings::LastExpdesignDir);

	gchar *dirname = g_path_get_dirname( hist_filename.c_str());
	g_mkdir_with_parents( dirname, 0755);
	g_free( dirname);

	write_xml( hist_filename, pt);
}



extern "C" void
wExpDesignChooser_show_cb( GtkWidget*, gpointer);

int
construct_once()
{
	mExpDesignList =
		gtk_list_store_new( 1, G_TYPE_STRING);

	if ( !(AGH_GBGETOBJ (GtkDialog, 	wExpDesignChooser)) ||
	     !(AGH_GBGETOBJ (GtkTreeView,	tvExpDesignList)) ||
	     !(AGH_GBGETOBJ (GtkButton,		bExpDesignSelect)) )
		return -1;

	g_signal_connect( wExpDesignChooser, "show", G_CALLBACK (wExpDesignChooser_show_cb), NULL);

	gtk_tree_view_set_model( tvExpDesignList,
				 (GtkTreeModel*)mExpDesignList);

	g_object_set( G_OBJECT (tvExpDesignList),
		      "headers-visible", FALSE,
		      NULL);

	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_text_new();
	g_object_set( (GObject*)renderer, "editable", FALSE, NULL);
	g_object_set_data( (GObject*)renderer, "column", GINT_TO_POINTER (0));
	gtk_tree_view_insert_column_with_attributes( tvExpDesignList,
						     -1, "ExpDesign", renderer,
						     "text", 0,
						     NULL);

	hist_filename = string (g_get_home_dir()) + "/.config/aghermann/sessions";

	return 0;
}

} // namespace expdselect


using namespace aghui;
using namespace aghui::expdselect;

extern "C" {

	void
	wExpDesignChooser_show_cb( GtkWidget *wid, gpointer u)
	{
		gtk_widget_set_sensitive( (GtkWidget*)bExpDesignSelect, FALSE);
	}

	void
	wExpDesignChooser_hide_cb( GtkWidget *wid, gpointer u)
	{
		gtk_widget_show( (GtkWidget*)wMainWindow);
	}

	void
	tvExpDesignList_cursor_changed_cb( GtkTreeView *tree_view, gpointer u)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection( tvExpDesignList);
		gtk_widget_set_sensitive( (GtkWidget*)bExpDesignSelect,
					  gtk_tree_selection_count_selected_rows( selection) > 0);
	}




	void
	bExpDesignSelect_clicked_cb( GtkButton *button, gpointer userdata)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection( tvExpDesignList);
		GtkTreeModel *model;
		GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
		GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
		g_list_free( paths);

		GtkTreeIter iter;
		gtk_tree_model_get_iter( model, &iter, path);
		gchar *entry;
		gtk_tree_model_get( model, &iter, 0, &entry, -1);
		settings::LastExpdesignDir = entry;
		g_free( entry);
		gint selected = gtk_tree_path_get_indices( path)[0];
		gtk_tree_path_free( path);

		gtk_widget_hide( (GtkWidget*)wExpDesignChooser);
		depopulate( true);

		settings::LastExpdesignDirNo = selected;

		delete AghCC;

		AghCC = new agh::CExpDesign( settings::LastExpdesignDir.c_str(), NULL /* progress_indicator */);
		populate( true);

		wMainWindow_title = string ("Aghermann: ") + settings::LastExpdesignDir;
		gtk_window_set_title( wMainWindow, wMainWindow_title.c_str());
	}





	void
	bExpDesignQuit_clicked_cb( GtkButton *button, gpointer userdata)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW (tvExpDesignList));
		GtkTreeModel *model;
		GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
		if ( paths ) {
			GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
			g_list_free( paths);

			settings::LastExpdesignDirNo = gtk_tree_path_get_indices( path)[0];

			gtk_tree_path_free( path);
		}

		gtk_main_quit();
	}



	void
	bExpDesignCreateNew_clicked_cb( GtkButton *button, gpointer userdata)
	{
		GtkWidget *dir_chooser = gtk_file_chooser_dialog_new( "Locate New Experiment Directory",
								      NULL,
								      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
								      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
								      NULL);
		if ( gtk_dialog_run( (GtkDialog*)dir_chooser) == GTK_RESPONSE_ACCEPT ) {
			gchar *new_dir = gtk_file_chooser_get_filename( (GtkFileChooser*)dir_chooser);

			GtkTreeIter iter, iter_cur;
			GtkTreePath *path;
			gtk_tree_view_get_cursor( tvExpDesignList, &path, NULL);
			if ( path ) {
				gtk_tree_model_get_iter( (GtkTreeModel*)mExpDesignList, &iter_cur, path);
				gtk_list_store_insert_after( mExpDesignList, &iter, &iter_cur);
			} else
				gtk_list_store_append( mExpDesignList, &iter);

			gtk_list_store_set( mExpDesignList, &iter,
					    0, new_dir,
					    -1);
			g_free( new_dir);

			if ( path )
				gtk_tree_path_next( path);
			else
				path = gtk_tree_model_get_path( (GtkTreeModel*)mExpDesignList, &iter);
			gtk_tree_view_set_cursor( tvExpDesignList,
						  path, NULL, TRUE);

			gtk_tree_path_free( path);

			gtk_widget_set_sensitive( (GtkWidget*)bExpDesignSelect, TRUE);
		}

		gtk_widget_destroy( dir_chooser);
	}


	void
	bExpDesignRemove_clicked_cb( GtkButton *button, gpointer userdata)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection( tvExpDesignList);
		GtkTreeModel *model;
		GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
		if ( !paths )
			return;
		GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
		g_list_free( paths);

		GtkTreeIter iter;
		gtk_tree_model_get_iter( model, &iter, path);
		gtk_list_store_remove( mExpDesignList, &iter);

		gtk_tree_path_free( path);

		gtk_widget_set_sensitive( (GtkWidget*)bExpDesignSelect, FALSE);
		gtk_widget_show( (GtkWidget*)wExpDesignChooser);
	}

}

} // namespace aghui

// eof
