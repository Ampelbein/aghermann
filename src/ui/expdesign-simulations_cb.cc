// ;-*-C++-*- *  Time-stamp: "2011-07-03 14:14:34 hmmr"
/*
 *       File name:  ui/expdesign-simulations_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-03
 *
 *         Purpose:  SExpDesignUI simulation tab callbacks
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "ui.hh"
#include "expdesign.hh"
#include "modelrun-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

extern "C" {

	void
	iBatchRunAllChannels_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
	{
		settings::SimRunbatchIncludeAllChannels = gtk_check_menu_item_get_active( item);
	}

	void
	iBatchRunAllSessions_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
	{
		settings::SimRunbatchIncludeAllSessions = gtk_check_menu_item_get_active( item);
	}

	void
	iBatchRunIterateRanges_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
	{
		settings::SimRunbatchIterateRanges = gtk_check_menu_item_get_active( item);
	}


// void iBatchRunRedoSkip_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
// {  if ( gtk_check_menu_item_get_active( item) )  agh_sim_runbatch_redo_option = AGH_BATCHRUN_REDOSKIP;  }
//
// void iBatchRunRedoIfFailed_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
// {  if ( gtk_check_menu_item_get_active( item) )  agh_sim_runbatch_redo_option = AGH_BATCHRUN_REDOFAILED;  }
//
// void iBatchRunRedoIfSucceeded_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
// {  if ( gtk_check_menu_item_get_active( item) )  agh_sim_runbatch_redo_option = AGH_BATCHRUN_REDOSUCCEEDED;  }
//
// void iBatchRunRedoAlways_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
// {  if ( gtk_check_menu_item_get_active( item) )  agh_sim_runbatch_redo_option = AGH_BATCHRUN_REDOALWAYS;  }






	void
	bSimulationsRun_clicked_cb()
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection( tvSimulations);
		GtkTreeModel *model;
		GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
		if ( !paths )
			return;
		GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);

		if ( gtk_tree_path_get_depth( path) > 3 ) {
			CSimulation *modref;
			GtkTreeIter iter;
			gtk_tree_model_get_iter( (GtkTreeModel*)mSimulations, &iter, path);
			gtk_tree_model_get( (GtkTreeModel*)mSimulations, &iter,
					    AGH_TV_SIMULATIONS_MODREF_COL, &modref,
					    -1);
			if ( modref )
				new mf::SModelrunFacility( *modref);
		}

		gtk_tree_path_free( path);
		g_list_free( paths);
	}





	void
	bSimulationsSummary_clicked_cb()
	{
		auto f_chooser =
			(GtkDialog*)gtk_file_chooser_dialog_new( "Export Simulation Details",
								 NULL,
								 GTK_FILE_CHOOSER_ACTION_SAVE,
								 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
								 NULL);
		if ( gtk_dialog_run( f_chooser) == GTK_RESPONSE_ACCEPT ) {
			g_string_assign( __ss__, gtk_file_chooser_get_filename( (GtkFileChooser*)f_chooser));
			if ( !g_str_has_suffix( __ss__->str, ".tsv") && !g_str_has_suffix( __ss__->str, ".TSV") )
				g_string_append_printf( __ss__, ".tsv");
			modelrun_tsv_export_all( __ss__->str);
		}
		gtk_widget_destroy( (GtkWidget*)f_chooser);
	}





//static gboolean __interrupt_batch;

// void
// bBatchRunStop_activate_cb()
// {
// 	__interrupt_batch = TRUE;
// }


// void
// iBatchRun_activate_cb()
// {
// 	__interrupt_batch = FALSE;

// 	set_cursor_busy( TRUE, wMainWindow);
// 	simview::populate();

// 	set_cursor_busy( FALSE, wMainWindow);
// }

}

// eof

