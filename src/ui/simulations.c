// ;-*-C-*- *  Time-stamp: "2011-03-09 02:35:45 hmmr"
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


#include <string.h>
#include <glade/glade.h>
#include "../libagh/iface.h"
#include "../common.h"
#include "misc.h"
#include "ui.h"
#include "settings.h"


GtkWidget
	*tvSimulations;

GtkWidget
	*eSimulationsChannel,
	*eSimulationsSession;


/*
typedef struct {
	struct SSubject
		*subject;
	struct SConsumerTunableSet
		t_set;
} SSubjectPresentation;

typedef struct {
	struct SGroup
		*group;
	GArray	*subjects;
	gboolean
		visible;
} SGroupPresentation;

static void
free_group_presentation( SGroupPresentation* g)
{
	g_array_free( g->subjects, TRUE);
}

static GArray	*GG;

*/




void eSimulationsSession_changed_cb(void);
void eSimulationsChannel_changed_cb(void);
gulong	eSimulationsSession_changed_cb_handler_id,
	eSimulationsChannel_changed_cb_handler_id;


static const gchar* const __agh_simulations_column_names[] = {
	"Id", "Status",
	"rise rate",	 "rate const.",
	"fc(REM)",	 "fc(Wake)",
	"S\342\202\200", "S\342\210\236",
	"t\342\206\227", "t\342\206\230",
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
	guint t;
	GtkTreeViewColumn *col;
	for ( t = 0; t < AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL; ++t ) {
		renderer = gtk_cell_renderer_text_new();
		g_object_set( G_OBJECT (renderer), "editable", FALSE, NULL);
		g_object_set_data( G_OBJECT (renderer), "column", GINT_TO_POINTER (t));
		col = gtk_tree_view_column_new_with_attributes( __agh_simulations_column_names[t],
								renderer,
								"text", t,
								NULL);
		if ( t > 2 )
			gtk_tree_view_column_set_alignment( col, .9);
		gtk_tree_view_column_set_expand( col, TRUE);
		gtk_tree_view_append_column( GTK_TREE_VIEW (tvSimulations), col);
	}
	gtk_tree_view_append_column( GTK_TREE_VIEW (tvSimulations),
				     col = gtk_tree_view_column_new());
	gtk_tree_view_column_set_visible( col, FALSE);


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
//	eSimulationsSession_changed_cb_handler_id =
//		g_signal_connect( eSimulationsSession, "changed", eSimulationsSession_changed_cb, NULL);

	gtk_combo_box_set_model( GTK_COMBO_BOX (eSimulationsChannel),
				 GTK_TREE_MODEL (agh_mEEGChannels));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eSimulationsChannel), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eSimulationsChannel), renderer,
					"text", 0,
					NULL);
//	eSimulationsChannel_changed_cb_handler_id =
//		g_signal_connect( eSimulationsChannel, "changed", eSimulationsChannel_changed_cb, NULL);


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
agh_populate_mSimulations( gboolean thorough)
{
	printf( "agh_populate_mSimulations(%d)\n", thorough);
	gtk_tree_store_clear( agh_mSimulations);

      // clean up
	agh_modelrun_remove_untried();
      // get current expdesign snapshot
	if ( thorough )
		agh_expdesign_snapshot( &agh_cc);

	GtkTreeIter iter_g, iter_j, iter_h, iter_q;

	for ( guint g = 0; g < agh_cc.n_groups; ++g ) {

		gtk_tree_store_append( agh_mSimulations, &iter_g, NULL);
		gtk_tree_store_set( agh_mSimulations, &iter_g,
				    0, agh_cc.groups[g].name,
				    AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL, TRUE,
				    -1);

		for ( guint j = 0; j < agh_cc.groups[g].n_subjects; ++j ) {
			struct SSubject *_j = &agh_cc.groups[g].subjects[j];
			guint d;
			for ( d = 0; d < _j->n_sessions; ++d )
				if ( strcmp( AghD, _j->sessions[d].name) == 0 )
					break;
			if ( d == _j->n_sessions ) // subject lacking one
				continue;

			gtk_tree_store_append( agh_mSimulations, &iter_j, &iter_g);
			gtk_tree_store_set( agh_mSimulations, &iter_j,
					    0, _j->name,
					    AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL, TRUE,
					    -1);

		      // collect previously obtained modruns
			struct SSession *_d = &_j->sessions[d];
			for ( guint rs = 0; rs < _d->n_modrun_sets; ++rs ) {
				const char *channel = _d->modrun_sets[rs].channel;

				gtk_tree_store_append( agh_mSimulations, &iter_h, &iter_j);
				gtk_tree_store_set( agh_mSimulations, &iter_h,
						    0, channel,
						    AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL, TRUE,
						    -1);

				for ( guint r = 0; r < _d->modrun_sets[rs].n_modruns; ++r ) {
					float	from = _d->modrun_sets[rs].modruns[r].from,
						upto = _d->modrun_sets[rs].modruns[r].upto;
					TModelRef
						modref = _d->modrun_sets[rs].modruns[r].modref;

					snprintf_buf( "%g\342\200\223%g", from, upto);
					gtk_tree_store_append( agh_mSimulations, &iter_q, &iter_h);
					gtk_tree_store_set( agh_mSimulations, &iter_q,
							    0, __buf__,
							    AGH_TV_SIMULATIONS_MODREF_COL, (gpointer)modref,
							    AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL, TRUE,
							    -1);
					// status (put CF here)
					snprintf_buf( "CF = %g", agh_modelrun_snapshot( modref));
					gtk_tree_store_set( agh_mSimulations, &iter_q,
							    1, __buf__,
							    -1);

					// tunable columns
					struct SConsumerTunableSet t_set;
					agh_modelrun_get_tunables( modref, &t_set);
					for ( gushort t = 0; t < t_set.n_tunables; ++t ) {
						const struct STunableDescription *t_desc;
						t_desc = agh_tunable_get_description( t);
						snprintf_buf( t_desc->fmt,
							      t_set.tunables[t] * t_desc->display_scale_factor);
						gtk_tree_store_set( agh_mSimulations, &iter_q,
								    2+t, __buf__,
								    -1);
					}

				}
			}
		      // and a virgin offering
			printf( "j->name = %s; D %s; T %s\n", _j->name, AghD, AghT);
			gtk_tree_store_append( agh_mSimulations, &iter_h, &iter_j);
			gtk_tree_store_set( agh_mSimulations, &iter_h,
					    0, AghT,
					    -1);
			snprintf_buf( "%g\342\200\223%g *", AghOperatingRangeFrom, AghOperatingRangeUpto);
			gtk_tree_store_append( agh_mSimulations, &iter_q, &iter_h);
			gtk_tree_store_set( agh_mSimulations, &iter_q,
					    0, __buf__,
					    -1);

			TModelRef virgin_modref;
			const char *status;
			int retval = agh_modelrun_setup( _j->name, AghD, AghT,
							 AghOperatingRangeFrom, AghOperatingRangeUpto,
							 &virgin_modref, &status);
			if ( retval ) {
				gtk_tree_store_set( agh_mSimulations, &iter_q,
						    1, simprep_perror(retval),
						    AGH_TV_SIMULATIONS_MODREF_COL, NULL,
						    -1);
			} else {
				gtk_tree_store_set( agh_mSimulations, &iter_q,
						    1, "untried",
						    AGH_TV_SIMULATIONS_MODREF_COL, (gpointer)virgin_modref,
						    -1);
			}
		}
	}
	gtk_tree_view_expand_all( GTK_TREE_VIEW (tvSimulations));
}


void
agh_cleanup_mSimulations()
{
	agh_modelrun_remove_untried();
	agh_populate_cMeasurements();
}







void
iBatchRunAllChannels_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
{
	AghSimRunbatchIncludeAllChannels = gtk_check_menu_item_get_active( item);
}

void
iBatchRunAllSessions_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
{
	AghSimRunbatchIncludeAllSessions = gtk_check_menu_item_get_active( item);
}

void
iBatchRunIterateRanges_toggled_cb( GtkCheckMenuItem *item, gpointer unused)
{
	AghSimRunbatchIterateRanges = gtk_check_menu_item_get_active( item);
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
	GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW (tvSimulations));
	GtkTreeModel *model;
	GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
	if ( !paths )
		return;
	GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);

	if ( gtk_tree_path_get_depth( path) > 3 ) {
		TModelRef modref;
		GtkTreeIter iter;
		gtk_tree_model_get_iter( GTK_TREE_MODEL (agh_mSimulations), &iter, path);
		gtk_tree_model_get( GTK_TREE_MODEL (agh_mSimulations), &iter,
				    AGH_TV_SIMULATIONS_MODREF_COL, &modref,
				    -1);
		if ( modref ) {
			if ( agh_prepare_modelrun_facility( modref) ) {
				float from, upto;
				agh_modelrun_get_freqrange( modref, &from, &upto);
				snprintf_buf( "Simulation: %s (%s) in %s, %g-%g Hz",
					      agh_modelrun_get_subject( modref),
					      AghD, AghH, from, upto);
				gtk_window_set_title( GTK_WINDOW (wModelRun),
						      __buf__);
				gtk_window_set_default_size( GTK_WINDOW (wModelRun),
							     gdk_screen_get_width( gdk_screen_get_default()) * .80,
							     gdk_screen_get_height( gdk_screen_get_default()) * .6);
		FAFA;
				gtk_widget_show_all( wModelRun);
			}
		}
	}

	gtk_tree_path_free( path);
	g_list_free( paths);
}





void
bSimulationsSummary_clicked_cb()
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
/*
	TModelRef
		Ri;
	guint	d, h,
		cur_run = 0,
		total_runs = agh_subject_get_n_of()
				* (AghSimRunbatchIncludeAllSessions ? AghDs : 1)
				* (AghSimRunbatchIncludeAllChannels ? AghTs : 1);

				*/
//out:
	agh_populate_mSimulations( TRUE);

	set_cursor_busy( FALSE, wMainWindow);
}





// EOF
