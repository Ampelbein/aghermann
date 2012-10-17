// ;-*-C++-*-
/*
 *       File name:  ui/ed-simulations_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-03
 *
 *         Purpose:  SExpDesignUI simulation tab callbacks
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "ed.hh"
#include "mf.hh"

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
	agh::ach::CModelRun *modref;
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

	gtk_entry_set_text( ED.eBatchSetupSubjects, agh::str::join( ED.ED->enumerate_subjects(), "; ").c_str());
	gtk_entry_set_text( ED.eBatchSetupSessions, agh::str::join( ED.ED->enumerate_sessions(), "; ").c_str());
	gtk_entry_set_text( ED.eBatchSetupChannels, agh::str::join( ED.ED->enumerate_eeg_channels(), "; ").c_str());

      // prevent inapplicable inputs when type == mc
	if ( ED.display_profile_type == sigfile::TMetricType::Mc ) {
		gtk_spin_button_set_value( ED.eBatchSetupRangeWidth, ED.ED->mc_params.bandwidth);
		gtk_spin_button_set_value( ED.eBatchSetupRangeInc, ED.ED->mc_params.bandwidth);
		gtk_widget_set_sensitive( (GtkWidget*)ED.eBatchSetupRangeWidth, FALSE);
		gtk_widget_set_sensitive( (GtkWidget*)ED.eBatchSetupRangeInc, FALSE);
	} else {
		auto bw = ED.operating_range_upto - ED.operating_range_from;
		gtk_spin_button_set_value( ED.eBatchSetupRangeWidth, bw);
		gtk_spin_button_set_value( ED.eBatchSetupRangeInc, bw);
		gtk_widget_set_sensitive( (GtkWidget*)ED.eBatchSetupRangeWidth, TRUE);
		gtk_widget_set_sensitive( (GtkWidget*)ED.eBatchSetupRangeInc, TRUE);
	}

	if ( gtk_dialog_run( ED.wBatchSetup) == GTK_RESPONSE_OK ) {
		ED.ED->remove_untried_modruns();
		ED.populate_2();

		list<string>
			use_subjects = agh::str::tokens( gtk_entry_get_text( ED.eBatchSetupSubjects), ";"),
			use_sessions = agh::str::tokens( gtk_entry_get_text( ED.eBatchSetupSessions), ";"),
			use_channels = agh::str::tokens( gtk_entry_get_text( ED.eBatchSetupChannels), ";");
		float	freq_from  = gtk_spin_button_get_value( ED.eBatchSetupRangeFrom),
			freq_width = gtk_spin_button_get_value( ED.eBatchSetupRangeWidth),
			freq_inc   = gtk_spin_button_get_value( ED.eBatchSetupRangeInc);
		size_t	freq_steps = gtk_spin_button_get_value( ED.eBatchSetupRangeSteps);

		aghui::SBusyBlock bb (ED.wMainWindow);
		for ( auto& J : use_subjects )
			for ( auto& D : use_sessions )
				for ( auto& H : use_channels ) {
					float	range_from = freq_from,
						range_upto = freq_from + freq_width;
					for ( size_t step = 0; step < freq_steps;
					      ++step, range_from += freq_width, range_upto += freq_width ) {
						agh::ach::CModelRun *sim;
						int retval =
							ED.ED->setup_modrun( J.c_str(), D.c_str(), H.c_str(),
									     ED.display_profile_type,
									     range_from, range_upto,
									     sim);
					}
				}
		using namespace agh;
		CExpDesign::TModelRunOpFun F =
			[]( ach::CModelRun& R)
			{
				R.watch_simplex_move( nullptr);
			};
		CExpDesign::TModelRunReportFun report =
			[&ED]( const CJGroup&,
			       const CSubject& J,
			       const string& D,
			       const sigfile::TMetricType& T,
			       const string& H,
			       const pair<float,float>& Q,
			       const ach::CModelRun&,
			       size_t i, size_t n)
			{
				snprintf_buf( "(%zu of %zu) Running simulation in channel %s (%g-%g Hz) for %s (session %s) ...",
					      i, n, H.c_str(), Q.first, Q.second,
					      J.name(), D.c_str());
				ED.buf_on_main_status_bar();
				gtk_flush();
			};
		CExpDesign::TModelRunFilterFun filter =
			[]( ach::CModelRun&) -> bool
			{
				return true;
			};
		ED.ED->for_all_modruns( F, report, filter);

		ED.populate_2();

		snprintf_buf( "Done");
		ED.buf_on_main_status_bar();
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

