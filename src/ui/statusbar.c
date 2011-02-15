// ;-*-C-*- *  Time-stamp: "2011-02-14 01:25:24 hmmr"
/*
 *       File name:  ui/statusbar.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  main menu statusbar widgets
 *
 *         License:  GPL
 */


#include <glade/glade.h>
#include "../libagh/iface.h"
#include "misc.h"
#include "ui.h"


GtkListStore	*agh_mExpDesignList;

gchar	*AghLastExpdesignDir;
gint	 AghLastExpdesignDirNo;
static GString	*agh_expdesign_hist_filename;

gchar *agh_wMainWindow_title;

GtkWidget
	*wScanLog,
	*lScanLog,
	*sbMainStatusBar;


static GtkWidget
	*tREADME,
	*wExpDesignChooser,
	*tvExpDesignList,
	*bExpDesignSelect;

guint agh_sb_context_id_General;


static void
__desensitize_select_button()
{
	gtk_widget_set_sensitive( bExpDesignSelect, FALSE);
}

// -------------------------------

gint
agh_ui_construct_StatusBar( GladeXML *xml)
{
	if ( !(sbMainStatusBar		= glade_xml_get_widget( xml, "sbMainStatusBar")) ||
	     !(tREADME			= glade_xml_get_widget( xml, "tREADME")) ||
	     !(wExpDesignChooser	= glade_xml_get_widget( xml, "wExpDesignChooser")) ||
	     !(tvExpDesignList		= glade_xml_get_widget( xml, "tvExpDesignList")) ||
	     !(bExpDesignSelect		= glade_xml_get_widget( xml, "bExpDesignSelect")) ||
	     !(wScanLog			= glade_xml_get_widget( xml, "wScanLog")) ||
	     !(lScanLog			= glade_xml_get_widget( xml, "lScanLog")) )
		return -1;

	agh_sb_context_id_General = gtk_statusbar_get_context_id( GTK_STATUSBAR (sbMainStatusBar), "General context");

	g_signal_connect( wExpDesignChooser, "show", G_CALLBACK (__desensitize_select_button), NULL);

	gtk_tree_view_set_model( GTK_TREE_VIEW (tvExpDesignList),
				 GTK_TREE_MODEL (agh_mExpDesignList));

	g_object_set( G_OBJECT (tvExpDesignList),
		      "headers-visible", FALSE,
		      NULL);

	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_text_new();
	g_object_set( G_OBJECT (renderer), "editable", FALSE, NULL);
	g_object_set_data( G_OBJECT (renderer), "column", GINT_TO_POINTER (0));
	gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW (tvExpDesignList),
						     -1, "ExpDesign", renderer,
						     "text", 0,
						     NULL);

	char *contents;
	snprintf_buf( "%s/README", PACKAGE_DATADIR);
	GFile *file = g_file_new_for_path( __buf__);
	g_file_load_contents( file, NULL,
			      &contents, NULL,
			      NULL, NULL);
	gtk_text_buffer_set_text( gtk_text_view_get_buffer( GTK_TEXT_VIEW (tREADME)),
				  contents, -1);
	g_object_unref( file);

	return 0;
}







void
agh_histfile_read()
{
	if ( !agh_expdesign_hist_filename ) {
		agh_expdesign_hist_filename = g_string_sized_new( 80);
		g_string_printf( agh_expdesign_hist_filename,
				 "%s/.config/aghermann/sessions", g_get_home_dir());
	}

	if ( AghLastExpdesignDir )
		g_free( AghLastExpdesignDir);

	GKeyFile *kf = g_key_file_new();
	GError *kf_error = NULL;

	gint last = -1;
	gchar **list = NULL;
	g_key_file_load_from_file( kf, agh_expdesign_hist_filename->str, G_KEY_FILE_KEEP_COMMENTS, &kf_error);
	if ( !kf_error )
		last = g_key_file_get_integer( kf, "Sessions", "Last", &kf_error);
	if ( !kf_error )
		list = g_key_file_get_string_list( kf, "Sessions", "List", NULL, &kf_error);

	GtkTreeIter iter;
	if ( last != -1 && list[last] ) {
		AghLastExpdesignDir = g_strdup( list[last]);
		AghLastExpdesignDirNo = last;

		guint i = 0;
		gchar *entry;
		if ( list ) {
			gtk_list_store_clear( agh_mExpDesignList);
			while ( (entry = list[i++]) ) {
				gtk_list_store_append( agh_mExpDesignList, &iter);
				gtk_list_store_set( agh_mExpDesignList, &iter,
						    0, entry,
						    -1);
			}
		}
	} else {
		gchar *cwd = g_get_current_dir();
		AghLastExpdesignDir = g_build_filename( G_DIR_SEPARATOR_S,
							   cwd, "NewExperiment", NULL);
		g_free( cwd);

		AghLastExpdesignDirNo = 0;

		gtk_list_store_clear( agh_mExpDesignList);
		gtk_list_store_append( agh_mExpDesignList, &iter);
		gtk_list_store_set( agh_mExpDesignList, &iter,
				    0, AghLastExpdesignDir,
				    -1);

		g_error_free( kf_error);
	}

	agh_wMainWindow_title = g_strdup_printf( "Aghermann: %s", AghLastExpdesignDir);
	gtk_window_set_title( GTK_WINDOW (wMainWindow), agh_wMainWindow_title);

	if ( list )  g_strfreev( list);
	g_key_file_free( kf);
}





void
agh_histfile_write()
{
	GtkTreeIter iter;
	gboolean some_items_left =
		gtk_tree_model_get_iter_first( GTK_TREE_MODEL (agh_mExpDesignList), &iter);
	if ( !some_items_left )
		AghLastExpdesignDirNo = -1;

	gchar	 *entry;
	gchar **list;
	GString *agg = g_string_sized_new( 400);
	gsize n = 0;
	GKeyFile *kf = g_key_file_new();
	//GError *kf_error = NULL;

	while ( some_items_left ) {
		gtk_tree_model_get( GTK_TREE_MODEL (agh_mExpDesignList), &iter,  // at least one entry exists,
				    0, &entry,                                   // added by prec func
				    -1);
		g_string_append_printf( agg, "%s;", entry);
		g_free( entry);
		n++;
		some_items_left = gtk_tree_model_iter_next( GTK_TREE_MODEL (agh_mExpDesignList), &iter);
	}
	list = g_strsplit( agg->str, ";", -1);

	g_key_file_set_string_list( kf, "Sessions", "List", (const char* const*)list, n);
	g_key_file_set_integer( kf, "Sessions", "Last", AghLastExpdesignDirNo);

	g_string_free( agg, TRUE);
	if ( list )  g_strfreev( list);

	gchar *dirname = g_path_get_dirname( agh_expdesign_hist_filename->str);
	g_mkdir_with_parents( dirname, 0755);
	g_free( dirname);

	gchar *towrite = g_key_file_to_data( kf, NULL, NULL);
	g_file_set_contents( agh_expdesign_hist_filename->str, towrite, -1, NULL);
	g_free( towrite);

	g_key_file_free( kf);
}









void
tREADME_map_cb()
{
}




void
tvExpDesignList_cursor_changed_cb()
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW (tvExpDesignList));
	gtk_widget_set_sensitive( bExpDesignSelect, gtk_tree_selection_count_selected_rows( selection) > 0);
}




void
bExpDesignSelect_clicked_cb()
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW (tvExpDesignList));
	GtkTreeModel *model;
	GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
	GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
	g_list_free( paths);

	GtkTreeIter iter;
	gtk_tree_model_get_iter( model, &iter, path);
	gtk_tree_model_get( model, &iter, 0, &AghLastExpdesignDir, -1);
	gint selected = gtk_tree_path_get_indices( path)[0];
	gtk_tree_path_free( path);

	gtk_widget_hide( wExpDesignChooser);
	agh_ui_depopulate();
	gtk_widget_show( wMainWindow);

	AghLastExpdesignDirNo = selected;

	agh_expdesign_shutdown();

	agh_expdesign_init( AghLastExpdesignDir, NULL /* progress_indicator */);
	agh_ui_populate();

	if ( agh_wMainWindow_title )
		g_free( agh_wMainWindow_title);
	agh_wMainWindow_title = g_strdup_printf( "Aghermann: %s", AghLastExpdesignDir);
	gtk_window_set_title( GTK_WINDOW (wMainWindow), agh_wMainWindow_title);

}





void
bExpDesignQuit_clicked_cb()
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW (tvExpDesignList));
	GtkTreeModel *model;
	GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
	if ( paths ) {
		GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
		g_list_free( paths);

		AghLastExpdesignDirNo = gtk_tree_path_get_indices( path)[0];

		gtk_tree_path_free( path);
	}

	gtk_main_quit();
}



void
bExpDesignCreateNew_clicked_cb()
{
	GtkWidget *dir_chooser = gtk_file_chooser_dialog_new( "Locate New Experiment Directory",
							      NULL,
							      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
							      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							      NULL);
	if ( gtk_dialog_run( GTK_DIALOG (dir_chooser)) == GTK_RESPONSE_ACCEPT ) {
		gchar *new_dir = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (dir_chooser));

		GtkTreeIter iter, iter_cur;
		GtkTreePath *path;
		gtk_tree_view_get_cursor( GTK_TREE_VIEW (tvExpDesignList), &path, NULL);
		if ( path ) {
			gtk_tree_model_get_iter( GTK_TREE_MODEL (agh_mExpDesignList), &iter_cur, path);
			gtk_list_store_insert_after( agh_mExpDesignList, &iter, &iter_cur);
		} else
			gtk_list_store_append( agh_mExpDesignList, &iter);

		gtk_list_store_set( agh_mExpDesignList, &iter,
				    0, new_dir,
				    -1);
		g_free( new_dir);

		if ( path )
			gtk_tree_path_next( path);
		else
			path = gtk_tree_model_get_path( GTK_TREE_MODEL (agh_mExpDesignList), &iter);
		gtk_tree_view_set_cursor( GTK_TREE_VIEW (tvExpDesignList),
					  path, NULL, TRUE);

		gtk_tree_path_free( path);

		gtk_widget_set_sensitive( bExpDesignSelect, TRUE);
	}

	gtk_widget_destroy( dir_chooser);
}


void
bExpDesignRemove_clicked_cb()
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW (tvExpDesignList));
	GtkTreeModel *model;
	GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
	if ( !paths )
		return;
	GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
	g_list_free( paths);

	GtkTreeIter iter;
	gtk_tree_model_get_iter( model, &iter, path);
	gtk_list_store_remove( agh_mExpDesignList, &iter);

	gtk_tree_path_free( path);

	gtk_widget_set_sensitive( bExpDesignSelect, FALSE);
}










void
bExpChange_clicked_cb()
{
	gtk_widget_hide( wMainWindow);
	if ( gtk_widget_get_visible( wScoringFacility) )
		gtk_widget_hide( wScoringFacility);
	if ( gtk_widget_get_visible( wPattern) )
		gtk_widget_hide( wPattern);
	gtk_widget_show( wExpDesignChooser);
}


void
progress_indicator( const char* current, size_t n, size_t i)
{
	snprintf_buf( "(%zu of %zu) %s", i, n, current);
	gtk_statusbar_pop( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General);
	gtk_statusbar_push( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General,
			    __buf__);
	while ( gtk_events_pending() )
		gtk_main_iteration();
}

void
do_rescan_tree()
{
	set_cursor_busy( TRUE, wMainWindow);
	gtk_widget_set_sensitive( wMainWindow, FALSE);
	while ( gtk_events_pending() )
		gtk_main_iteration();
	agh_ui_depopulate();
	agh_expdesign_scan_tree( progress_indicator);
	agh_ui_populate();

	set_cursor_busy( FALSE, wMainWindow);
	gtk_widget_set_sensitive( wMainWindow, TRUE);
	gtk_statusbar_push( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General,
			    "Scanning complete");
}


void
bScanTree_clicked_cb()
{
	do_rescan_tree();
}






// EOF
