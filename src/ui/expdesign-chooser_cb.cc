// ;-*-C++-*- *  Time-stamp: "2011-07-07 01:35:04 hmmr"
/*
 *       File name:  ui/expdesign-chooser_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-15
 *
 *         Purpose:  SExpDesignUI::SExpDesignChooser callbacks
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "ui.hh"
#include "expdesign.hh"

using namespace std;
using namespace aghui;

#if HAVE_CONFIG_H
#  include "config.h"
#endif


extern "C" {

	void
	wExpDesignChooser_show_cb( GtkWidget *wid, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		gtk_widget_set_sensitive( (GtkWidget*)ED.bExpDesignChooserSelect, FALSE);
	}

	void
	wExpDesignChooser_hide_cb( GtkWidget *wid, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		gtk_widget_show( (GtkWidget*)ED.wMainWindow);
	}

	void
	tvExpDesignChooserList_cursor_changed_cb( GtkTreeView *tree_view, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		auto selection = gtk_tree_view_get_selection( ED.tvExpDesignChooserList);
		gtk_widget_set_sensitive( (GtkWidget*)ED.bExpDesignChooserSelect,
					  gtk_tree_selection_count_selected_rows( selection) > 0);
	}




	void
	bExpDesignChooserSelect_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;

		gtk_widget_hide( (GtkWidget*)ED.wExpDesignChooser);

		ED.depopulate( true);

		delete ED.ED;

		string	new_ed_dir = ED.chooser_get_selected_dir(),
			shorter = new_ed_dir;
		homedir2tilda( shorter); // preserve original
		gtk_window_set_title( ED.wMainWindow,
				      (string ("Aghermann: ") + shorter).c_str());

		ED.ED = new agh::CExpDesign( new_ed_dir,
					     {bind( &SExpDesignUI::sb_progress_indicator, &ED, _1, _2, _3)});
		ED.populate( true);
		ED.chooser_write_histfile();
	}





	void
	bExpDesignChooserQuit_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;

		GtkTreeSelection *selection = gtk_tree_view_get_selection( (GtkTreeView*)ED.tvExpDesignChooserList);
		GtkTreeModel *model;
		GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
		if ( paths ) {
			GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
			g_list_free( paths);

			ED.chooser.last_dir_no = gtk_tree_path_get_indices( path)[0];

			gtk_tree_path_free( path);
		}

		gtk_main_quit();
	}



	void
	bExpDesignChooserCreateNew_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;

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
			gtk_tree_view_get_cursor( ED.tvExpDesignChooserList, &path, NULL);
			if ( path ) {
				gtk_tree_model_get_iter( (GtkTreeModel*)ED.mExpDesignChooserList, &iter_cur, path);
				gtk_list_store_insert_after( ED.mExpDesignChooserList, &iter, &iter_cur);
			} else
				gtk_list_store_append( ED.mExpDesignChooserList, &iter);

			gtk_list_store_set( ED.mExpDesignChooserList, &iter,
					    0, new_dir,
					    -1);
			g_free( new_dir);

			if ( path )
				gtk_tree_path_next( path);
			else
				path = gtk_tree_model_get_path( (GtkTreeModel*)ED.mExpDesignChooserList, &iter);
			gtk_tree_view_set_cursor( ED.tvExpDesignChooserList,
						  path, NULL, TRUE);

			gtk_tree_path_free( path);

			gtk_widget_set_sensitive( (GtkWidget*)ED.bExpDesignChooserSelect, TRUE);
		}

		gtk_widget_destroy( dir_chooser);
	}


	void
	bExpDesignChooserRemove_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;

		GtkTreeSelection *selection = gtk_tree_view_get_selection( ED.tvExpDesignChooserList);
		GtkTreeModel *model;
		GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
		if ( !paths )
			return;
		GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
		g_list_free( paths);

		GtkTreeIter iter;
		gtk_tree_model_get_iter( model, &iter, path);
		gtk_list_store_remove( ED.mExpDesignChooserList, &iter);

		gtk_tree_path_free( path);

		gtk_widget_set_sensitive( (GtkWidget*)ED.bExpDesignChooserSelect, FALSE);
		gtk_widget_show( (GtkWidget*)ED.wExpDesignChooser);
	}

}

// eof
