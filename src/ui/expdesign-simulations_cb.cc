// ;-*-C++-*-
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

using namespace std;
using namespace aghui;

extern "C" {



void
tvSimulations_row_activated_cb( GtkTreeView* tree_view,
				GtkTreePath* path,
				GtkTreeViewColumn *column,
				gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	agh::CModelRun *modref;
	GtkTreeIter iter;
	gtk_tree_model_get_iter( (GtkTreeModel*)ED.mSimulations, &iter, path);
	gtk_tree_model_get( (GtkTreeModel*)ED.mSimulations, &iter,
			    ED.msimulations_modref_col, &modref,
			    -1);
	if ( modref )
		new SModelrunFacility( *modref, ED);
}



void
iSimulationsRunBatch_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	gtk_entry_set_text( ED.eBatchSetupSubjects, string_join( ED.ED->enumerate_subjects(), "; ").c_str());
	gtk_entry_set_text( ED.eBatchSetupSessions, string_join( ED.ED->enumerate_sessions(), "; ").c_str());
	gtk_entry_set_text( ED.eBatchSetupChannels, string_join( ED.ED->enumerate_eeg_channels(), "; ").c_str());
	if ( gtk_dialog_run( ED.wBatchSetup) == GTK_RESPONSE_OK ) {
		ED.ED->remove_all_modruns();
		ED.populate_2();

		list<string>
			use_subjects = string_tokens( gtk_entry_get_text( ED.eBatchSetupSubjects), ";"),
			use_sessions = string_tokens( gtk_entry_get_text( ED.eBatchSetupSessions), ";"),
			use_channels = string_tokens( gtk_entry_get_text( ED.eBatchSetupChannels), ";");
		float	freq_from  = gtk_spin_button_get_value( ED.eBatchSetupRangeFrom),
			freq_width = gtk_spin_button_get_value( ED.eBatchSetupRangeWidth),
			freq_inc   = gtk_spin_button_get_value( ED.eBatchSetupRangeInc);
		size_t	freq_steps = gtk_spin_button_get_value( ED.eBatchSetupRangeSteps);

		set_cursor_busy( true, (GtkWidget*)ED.wMainWindow);
		gtk_widget_set_sensitive( (GtkWidget*)ED.wMainWindow, FALSE);
		gtk_flush();
		for ( auto& J : use_subjects )
			for ( auto& D : use_sessions )
				for ( auto& H : use_channels ) {
					float	range_from = freq_from,
						range_upto = freq_from + freq_width;
					for ( size_t step = 0; step < freq_steps;
					      ++step, range_from += freq_width, range_upto += freq_width ) {
						agh::CModelRun *sim;
						int retval =
							ED.ED->setup_modrun( J.c_str(), D.c_str(), H.c_str(),
									     range_from, range_upto,
									     sim);
						if ( retval ) {
							; // didn't work
						} else {
							snprintf_buf( "Running simulation in channel %s (%g-%g Hz) for %s (session %s) ...",
								      H.c_str(), range_from, range_upto,
								      J.c_str(), D.c_str());
							ED.buf_on_status_bar();
							sim -> watch_simplex_move( nullptr);
						}
					}
					ED.populate_2();
					gtk_flush();
				}
		snprintf_buf( "Done");
		ED.buf_on_status_bar();
		set_cursor_busy( false, (GtkWidget*)ED.wMainWindow);
		gtk_widget_set_sensitive( (GtkWidget*)ED.wMainWindow, TRUE);
	}
}


void
iSimulationsRunClearAll_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.ED->remove_all_modruns();
	ED.populate_2();
}



void
iSimulationsReportGenerate_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

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
		ED.ED->export_all_modruns( {__ss__->str});
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

