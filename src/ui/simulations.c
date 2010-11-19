// ;-*-C-*- *  Time-stamp: "2010-11-18 01:30:18 hmmr"
/*
 *       File name:  ui/simulations.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  simulation results view
 *
 *         License:  GPL
 */


#include <glade/glade.h>
#include "../core/iface.h"
#include "misc.h"
#include "ui.h"


GtkWidget
	*tvSimulations,
	*eSimulationsSession,
	*eSimulationsChannel;






void eSimulationsSession_changed_cb();
void eSimulationsChannel_changed_cb();


static const gchar* const __agh_tunable_column_names[] = {
	"S\342\202\200", "S\342\210\236",
	"fc(REM)",	 "fc(Wake)",
	"t\342\206\227", "t\342\206\230",
	"rate const.",	 "rise rate",
	"gain const.",
	"gc2", "gc3", "gc4",
};



gint
agh_ui_construct_Simulations( GladeXML *xml)
{
	GtkCellRenderer *renderer;

     // ------------- tvSimulations
	if ( !(tvSimulations = glade_xml_get_widget( xml, "tvSimulations")) )
		return -1;

	gtk_tree_view_set_model( GTK_TREE_VIEW (tvSimulations),
				 GTK_TREE_MODEL (agh_mSimulations));

	g_object_set( G_OBJECT (tvSimulations),
		      "expander-column", 0,
		      "enable-tree-lines", FALSE,
		      "headers-clickable", FALSE,
		      NULL);
	g_signal_connect( tvSimulations, "map", G_CALLBACK (gtk_tree_view_expand_all), NULL);

	renderer = gtk_cell_renderer_text_new();
	g_object_set( G_OBJECT (renderer),
		      "editable", FALSE,
		      NULL);
	gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW (tvSimulations),
						     -1, "Subject", renderer,
						     "text", 0,
						     NULL);
	guint t;
	for ( t = 0; t < AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL; ++t ) {
		renderer = gtk_cell_renderer_text_new();
		g_object_set( G_OBJECT (renderer),
			      "editable", FALSE,
			      "xalign", 1.,
			      NULL);
		g_object_set_data( G_OBJECT (renderer), "column", GINT_TO_POINTER (t+1));
		GtkTreeViewColumn *col =
			gtk_tree_view_column_new_with_attributes( __agh_tunable_column_names[t],
								  renderer,
								  "text", t+1,
								  "visible", AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL,
								  NULL);
		gtk_tree_view_column_set_alignment( col, .9);
//		gtk_tree_view_column_set_expand( col, TRUE);
		gtk_tree_view_append_column( GTK_TREE_VIEW (tvSimulations), col);
	}


     // ------------- eSimulations{Session,Channel}
	if ( !(eSimulationsSession = glade_xml_get_widget( xml, "eSimulationsSession")) ||
	     !(eSimulationsChannel = glade_xml_get_widget( xml, "eSimulationsChannel")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eSimulationsSession),
				 GTK_TREE_MODEL (agh_mSessions));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eSimulationsSession), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eSimulationsSession), renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( GTK_COMBO_BOX (eSimulationsChannel),
				 GTK_TREE_MODEL (agh_mEEGChannels));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eSimulationsChannel), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eSimulationsChannel), renderer,
					"text", 0,
					NULL);
//      // ---------- iBatch*
//	if ( !(iBatchRunAllChannels	= glade_xml_get_widget( xml, "iBatchRunAllChannels")) ||
//	     !(iBatchRunAllSessions	= glade_xml_get_widget( xml, "iBatchRunAllSessions")) ||
////	     !(iBatchRunIterateRanges	= glade_xml_get_widget( xml, "iBatchRunIterateRanges")) ||
//	     !(wBatchRunProgress	= glade_xml_get_widget( xml, "wBatchRunProgress")) ||
//	     !(lBatchRunStatus		= glade_xml_get_widget( xml, "lBatchRunStatus")) ||
//	     !(rBatchRunPercentComplete	= glade_xml_get_widget( xml, "rBatchRunPercentComplete")) ||
//	     !(bBatchRunStop		= glade_xml_get_widget( xml, "bBatchRunStop")) )
//		return -1;

	return 0;
}






void
eSimulationsSession_changed_cb()
{
	AghDi = gtk_combo_box_get_active( GTK_COMBO_BOX (eSimulationsSession));

	gtk_combo_box_set_active( GTK_COMBO_BOX (eMsmtSession), AghDi);

	agh_populate_mSimulations();
}

void
eSimulationsChannel_changed_cb()
{
	AghTi = gtk_combo_box_get_active( GTK_COMBO_BOX (eSimulationsChannel));

	gtk_combo_box_set_active( GTK_COMBO_BOX (eMsmtChannel), AghTi);

	agh_populate_mSimulations();
}




















void iBatchRunAllChannels_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
{  AghSimRunbatchIncludeAllChannels = gtk_check_menu_item_get_active( item);  }

void iBatchRunAllSessions_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
{  AghSimRunbatchIncludeAllSessions = gtk_check_menu_item_get_active( item);  }

void iBatchRunIterateRanges_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
{  AghSimRunbatchIterateRanges = gtk_check_menu_item_get_active( item);  }


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
bSimulationRun_clicked_cb()
{
	static GString
		*title = NULL;
	static gchar
		*j_name = NULL;
	if ( !title )
		title = g_string_sized_new(120);

	GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW (tvSimulations));
	GtkTreeModel *model;
	GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
	if ( !paths )
		return;
	GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);

	if ( gtk_tree_path_get_depth( path) > 1 ) {
		g_free(j_name);
		GtkTreeIter iter;
		gtk_tree_model_get_iter( GTK_TREE_MODEL (agh_mSimulations), &iter, path);
		gtk_tree_model_get( GTK_TREE_MODEL (agh_mSimulations), &iter,
				    0, &j_name,
				    -1);
		AghJ = agh_subject_find_by_name( j_name, NULL);

		if ( agh_prepare_modelrun_facility() ) {
			gtk_widget_show_all( wModelRun);
			g_string_printf( title, "Simulation: %s %s", j_name, AghH);
			gtk_window_set_title( GTK_WINDOW (wModelRun),
					      title->str);
		}
	}

	gtk_tree_path_free( path);
	g_list_free( paths);
}





void
bSimulationExport_clicked_cb()
{
	GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Export Simulation Details",
							    NULL,
							    GTK_FILE_CHOOSER_ACTION_SAVE,
							    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							    NULL);
	if ( gtk_dialog_run( GTK_DIALOG (f_chooser)) == GTK_RESPONSE_ACCEPT ) {
		GString *fname = g_string_new( gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (f_chooser)));
		if ( !g_str_has_suffix( fname->str, ".tsv") && !g_str_has_suffix( fname->str, ".TSV") )
			g_string_append_printf( fname, ".tsv");
		agh_modelrun_tsv_export_all( fname->str);
	}
	gtk_widget_destroy( f_chooser);
}






static gboolean __interrupt_batch;

void
bBatchRunStop_activate_cb()
{
//	gtk_widget_set_sensitive( bBatchRunStop, FALSE);
	__interrupt_batch = TRUE;
}




void
iBatchRun_activate_cb()
{
	__interrupt_batch = FALSE;

	set_cursor_busy( TRUE, wMainWindow);

	TModelRef
		Ri;
	guint	d, h,
		cur_run = 0,
		total_runs = agh_subject_get_n_of()
				* (AghSimRunbatchIncludeAllSessions ? AghDs : 1)
				* (AghSimRunbatchIncludeAllChannels ? AghTs : 1);
	gchar	label_text[60];


	const char *g_name = agh_group_find_first();
	while ( g_name ) {
		const struct SSubject *_j = agh_subject_find_first_in_group(g_name, NULL);

		for ( d = 0; d < AghDs; ++d ) {
			if ( !AghSimRunbatchIncludeAllSessions && d != AghDi ) {
				cur_run += agh_subject_get_n_of_in_group(g_name) * AghTs;
				continue;
			}

			for ( h = 0; h < AghTs; ++h, ++cur_run ) {
				if ( !AghSimRunbatchIncludeAllChannels && h != AghTi )
					continue;

				snprintf( label_text, 59, "<big>%s (%s) <b>%s %s</b></big>",
					  _j->name, g_name, AghDD[d], AghTT[h]);
//				gtk_label_set_markup( GTK_LABEL (lBatchRunStatus), label_text);
//				gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR (rBatchRunPercentComplete),
//							       (double)(cur_run) / total_runs);

				while ( gtk_events_pending() )
					gtk_main_iteration();

				if ( __interrupt_batch )
					goto out;

				if ( (Ri = agh_modelrun_find_by_jdhq( _j->name, AghDD[d], AghTT[h],
								      AghSimOperatingRangeFrom, AghSimOperatingRangeUpto))
				     == NULL )
					if ( agh_modelrun_setup( _j->name, AghDD[d], AghTT[h],
								 AghSimOperatingRangeFrom, AghSimOperatingRangeUpto,
								 &Ri) == -1 )
						continue;

				//agh_sim_modelrun_get_snapshot( R, -1, NULL, NULL, &t_set, NULL);

				agh_modelrun_run(Ri);
			}
			_j = agh_subject_find_next_in_group(NULL);
		}
		g_name = agh_group_find_next();
	}
out:
	agh_populate_mSimulations();

	set_cursor_busy( FALSE, wMainWindow);
}



// EOF
