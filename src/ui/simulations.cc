// ;-*-C++-*- *  Time-stamp: "2011-05-17 01:36:52 hmmr"
/*
 *       File name:  ui/simulations.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  simulation results view
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "ui.hh"
#include "settings.hh"
#include "modelrun-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {

GtkTreeView
	*tvSimulations;

GtkLabel
	*lSimulationsChannel,
	*lSimulationsSession;

namespace simview {

// like to move it move it
bool	RunbatchIncludeAllChannels,
	RunbatchIncludeAllSessions,
	RunbatchIterateRanges;

GtkTreeStore
	*mSimulations;

inline namespace {

	const char* __simulations_column_names[] = {
		       "Id", "Status",
		       "rise rate",	 "rate const.",
		       "fc(REM)",	 "fc(Wake)",
		       "S\342\202\200", "S\342\210\236",
		       "t\342\206\227", "t\342\206\230",
		       "gain const.",
		       "gc2", "gc3", "gc4",
	};

	int
	modelrun_tsv_export_all( const char* fname)
	{
		using namespace agh;

		FILE *f = fopen( fname, "w");
		if ( !f )
			return -1;

		auto t = TTunable::rs;
		fprintf( f, "#");
		for ( ; t < TTunable::_all_tunables; ++t )
			fprintf( f, "\t%s", STunableSet::tunable_name(t).c_str());
		fprintf( f, "\n");

		for ( auto Gi = AghCC->groups_begin(); Gi != AghCC->groups_end(); ++Gi )
			for ( auto Ji = Gi->second.begin(); Ji != Gi->second.end(); ++Ji )
				for ( auto Di = Ji->measurements.begin(); Di != Ji->measurements.end(); ++Di )
					for ( auto RSi = Di->second.modrun_sets.begin(); RSi != Di->second.modrun_sets.end(); ++RSi )
						for ( auto Ri = RSi->second.begin(); Ri != RSi->second.end(); ++Ri )
							if ( Ri->second.status & AGH_MODRUN_TRIED ) {
								fprintf( f, "# ----- Subject: %s;  Session: %s;  Channel: %s;  Range: %g-%g Hz\n",
									 Ri->second.subject(), Ri->second.session(), Ri->second.channel(),
									 Ri->second.freq_from(), Ri->second.freq_upto());
								t = TTunable::rs;
								do {
									fprintf( f, "%g%s", Ri->second.cur_tset[t] * STunableSet::stock[(TTunable_underlying_type)t].display_scale_factor,
										 (t == Ri->second.cur_tset.last()) ? "\n" : "\t");
								} while ( ++t != (TTunable)Ri->second.cur_tset.size() );
							}

		fclose( f);

		return 0;
	}

}

int
construct_once()
{
	mSimulations =
		gtk_tree_store_new( 16,
				    G_TYPE_STRING,	// group, subject, channel, from-upto
				    G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,	// tunables
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_BOOLEAN,
				    G_TYPE_POINTER);

	GtkCellRenderer *renderer;

     // ------------- tvSimulations
	if ( !(AGH_GBGETOBJ (GtkTreeView, tvSimulations)) )
		return -1;

	gtk_tree_view_set_model( tvSimulations,
				 (GtkTreeModel*)mSimulations);

	g_object_set( (GObject*)tvSimulations,
		      "expander-column", 0,
		      "enable-tree-lines", FALSE,
		      "headers-clickable", FALSE,
		      NULL);
	g_signal_connect( tvSimulations, "map", G_CALLBACK (gtk_tree_view_expand_all), NULL);

	renderer = gtk_cell_renderer_text_new();
	g_object_set( (GObject*)renderer,
		      "editable", FALSE,
		      NULL);
	guint t;
	GtkTreeViewColumn *col;
	for ( t = 0; t < AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL; ++t ) {
		renderer = gtk_cell_renderer_text_new();
		g_object_set( (GObject*)renderer, "editable", FALSE, NULL);
		g_object_set_data( (GObject*)renderer, "column", GINT_TO_POINTER (t));
		col = gtk_tree_view_column_new_with_attributes( __simulations_column_names[t],
								renderer,
								"text", t,
								NULL);
		if ( t > 2 )
			gtk_tree_view_column_set_alignment( col, .9);
		gtk_tree_view_column_set_expand( col, TRUE);
		gtk_tree_view_append_column( tvSimulations, col);
	}
	gtk_tree_view_append_column( tvSimulations,
				     col = gtk_tree_view_column_new());
	gtk_tree_view_column_set_visible( col, FALSE);


     // ------------- eSimulations{Session,Channel}
	if ( !(AGH_GBGETOBJ (GtkLabel, lSimulationsSession)) ||
	     !(AGH_GBGETOBJ (GtkLabel, lSimulationsChannel)) )
		return -1;

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
populate()
{
	gtk_tree_store_clear( mSimulations);

      // clean up
	AghCC->remove_untried_modruns();

	GtkTreeIter iter_g, iter_j, iter_h, iter_q;

	for ( auto G = AghCC->groups_begin(); G != AghCC->groups_end(); ++G ) {

		gtk_tree_store_append( mSimulations, &iter_g, NULL);
		gtk_tree_store_set( mSimulations, &iter_g,
				    0, G->first.c_str(),
				    AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL, TRUE,
				    -1);

		for ( auto J = G->second.begin(); J != G->second.end(); ++J ) {
			if ( not J->have_session(*_AghDi) ) // subject lacking one
				continue;

			gtk_tree_store_append( mSimulations, &iter_j, &iter_g);
			gtk_tree_store_set( mSimulations, &iter_j,
					    0, J->name(),
					    AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL, TRUE,
					    -1);

		      // collect previously obtained modruns
			for ( auto RS = J->measurements[*_AghDi].modrun_sets.begin(); RS != J->measurements[*_AghDi].modrun_sets.end(); ++RS ) {
				const string& H = RS->first;

				gtk_tree_store_append( mSimulations, &iter_h, &iter_j);
				gtk_tree_store_set( mSimulations, &iter_h,
						    0, H.c_str(),
						    AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL, TRUE,
						    -1);

				for ( auto R = RS->second.begin(); R != RS->second.end(); ++R ) {
					float	from = R->first.first,
						upto = R->first.second;
					CSimulation&
						M = R->second;

					snprintf_buf( "%g\342\200\223%g", from, upto);
					gtk_tree_store_append( mSimulations, &iter_q, &iter_h);
					gtk_tree_store_set( mSimulations, &iter_q,
							    0, __buf__,
							    AGH_TV_SIMULATIONS_MODREF_COL, (gpointer)&M,
							    AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL, TRUE,
							    -1);
					// status (put CF here)
					snprintf_buf( "CF = %g", M.snapshot());
					gtk_tree_store_set( mSimulations, &iter_q,
							    1, __buf__,
							    -1);

					// tunable columns
					for ( TTunable_underlying_type t = 0; t < M.cur_tset.size(); ++t ) {
						const auto& td = agh::STunableSet::stock[t];
						snprintf_buf( td.fmt,
							      M.cur_tset[t] * td.display_scale_factor);
						gtk_tree_store_set( mSimulations, &iter_q,
								    2+t, __buf__,
								    -1);
					}

				}
			}
		      // and a virgin offering
			gtk_tree_store_append( mSimulations, &iter_h, &iter_j);
			gtk_tree_store_set( mSimulations, &iter_h,
					    0, AghT(),
					    -1);
			snprintf_buf( "%g\342\200\223%g *", settings::OperatingRangeFrom, settings::OperatingRangeUpto);
			gtk_tree_store_append( mSimulations, &iter_q, &iter_h);
			gtk_tree_store_set( mSimulations, &iter_q,
					    0, __buf__,
					    -1);

			agh::CSimulation *virgin;
			TSimPrepError retval = AghCC->setup_modrun( J->name(), AghD(), AghT(),
								    settings::OperatingRangeFrom, settings::OperatingRangeUpto,
								    virgin);
			if ( retval != TSimPrepError::ok ) {
				gtk_tree_store_set( mSimulations, &iter_q,
						    1, simprep_perror(retval),
						    AGH_TV_SIMULATIONS_MODREF_COL, NULL,
						    -1);
			} else {
				gtk_tree_store_set( mSimulations, &iter_q,
						    1, "untried",
						    AGH_TV_SIMULATIONS_MODREF_COL, (gpointer)virgin,
						    -1);
			}
		}
	}
	gtk_tree_view_expand_all( tvSimulations);
}


void
cleanup()
{
	AghCC->remove_untried_modruns();
	msmtview::populate();
}


} // namespace simview



// callbacks

using namespace simview;

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

} // namespace aghui

// EOF
