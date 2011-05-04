// ;-*-C++-*- *  Time-stamp: "2011-05-01 00:15:50 hmmr"
/*
 *       File name:  ui/measurements-admit-one.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-18
 *
 *         Purpose:  edf import via dnd
 *
 *         License:  GPL
 */


#include <cairo.h>
#include <cairo-svg.h>

#include "misc.hh"
#include "ui.hh"
#include "settings.hh"
#include "measurements.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;
//using namespace agh;


namespace aghui {

      // widgets
	GtkDialog
		*wEdfImport;
	GtkComboBox
		*eEdfImportGroup,
		*eEdfImportSession,
		*eEdfImportEpisode;
	GtkLabel
		*lEdfImportSubject,
		*lEdfImportCaption,
		*lEdfImportFileInfo;
	GtkButton
		*bEdfImportAdmit,
		*bEdfImportScoreSeparately,
		*bEdfImportAttachCopy,
		*bEdfImportAttachMove;

namespace msmtview {
namespace dnd {

inline namespace {

      // supporting gtk stuff
	GtkTextBuffer
		*textbuf2;
}

int
construct( GtkBuilder *builder)
{
      // ------- wEdfImport
	if ( !AGH_GBGETOBJ (GtkDialog,		wEdfImport) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eEdfImportGroup) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eEdfImportSession) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eEdfImportEpisode) ||
	     !AGH_GBGETOBJ (GtkLabel,		lEdfImportSubject) ||
	     !AGH_GBGETOBJ (GtkLabel,		lEdfImportCaption) ||
	     !AGH_GBGETOBJ (GtkLabel,		lEdfImportFileInfo) ||
	     !AGH_GBGETOBJ (GtkButton,		bEdfImportAttachCopy) ||
	     !AGH_GBGETOBJ (GtkButton,		bEdfImportAttachMove) ||
	     !AGH_GBGETOBJ (GtkButton,		bEdfImportAdmit) ||
	     !AGH_GBGETOBJ (GtkButton,		bEdfImportScoreSeparately) )
		return -1;

	g_object_set( lEdfImportFileInfo,
		      "tabs", pango_tab_array_new_with_positions( 2, TRUE,
								  PANGO_TAB_LEFT, 130,
								  PANGO_TAB_LEFT, 190),
		      NULL);
	textbuf2 = gtk_text_view_get_buffer( (GtkTextView*)lEdfImportFileInfo);

	g_signal_connect_after( gtk_bin_get_child( (GtkBin*)eEdfImportGroup),
			  "key-release-event", G_CALLBACK (check_gtk_entry_nonempty),
			  NULL);
	g_signal_connect_after( gtk_bin_get_child( (GtkBin*)eEdfImportSession),
			  "key-release-event", G_CALLBACK (check_gtk_entry_nonempty),
			  NULL);
	g_signal_connect_after( gtk_bin_get_child( (GtkBin*)eEdfImportEpisode),
			  "key-release-event", G_CALLBACK (check_gtk_entry_nonempty),
			  NULL);
	return 0;
}



inline namespace {
	int
	maybe_admit_one( const char* fname)
	{
		agh::CEDFFile *F;
		string info;
		try {
			F = new agh::CEDFFile (fname, AghCC->fft_params.page_size);
			if ( F->status() & TEdfStatus::inoperable ) {
				pop_ok_message( GTK_WINDOW (wMainWindow), "The header seems to be corrupted in \"%s\"", fname);
				return 0;
			}
			info = F->details();

			snprintf_buf( "File: <i>%s</i>", fname);
			gtk_label_set_markup( GTK_LABEL (lEdfImportCaption), __buf__);
			snprintf_buf( "<b>%s</b>", F->patient.c_str());
			gtk_label_set_markup( GTK_LABEL (lEdfImportSubject), __buf__);

		} catch ( invalid_argument ex) {
			pop_ok_message( GTK_WINDOW (wMainWindow), "Could not read edf header in \"%s\"", fname);
			return 0;
		}
		gtk_text_buffer_set_text( textbuf2, info.c_str(), -1);

	      // populate and attach models
		GtkListStore
			*m_groups = gtk_list_store_new( 1, G_TYPE_STRING),
			*m_episodes = gtk_list_store_new( 1, G_TYPE_STRING),
			*m_sessions = gtk_list_store_new( 1, G_TYPE_STRING);
		GtkTreeIter iter;
		for ( auto i = AghGG.begin(); i != AghGG.end(); ++i ) {
			gtk_list_store_append( m_groups, &iter);
			gtk_list_store_set( m_groups, &iter, 0, i->c_str(), -1);
		}
		gtk_combo_box_set_model( GTK_COMBO_BOX (eEdfImportGroup),
					 GTK_TREE_MODEL (m_groups));
		gtk_combo_box_set_entry_text_column( GTK_COMBO_BOX (eEdfImportGroup), 0);

		for ( auto i = AghEE.begin(); i != AghEE.end(); ++i ) {
			gtk_list_store_append( m_episodes, &iter);
			gtk_list_store_set( m_episodes, &iter, 0, i->c_str(), -1);
		}
		gtk_combo_box_set_model( GTK_COMBO_BOX (eEdfImportEpisode),
					 GTK_TREE_MODEL (m_episodes));
		gtk_combo_box_set_entry_text_column( GTK_COMBO_BOX (eEdfImportEpisode), 0);

		for ( auto i = AghDD.begin(); i != AghDD.end(); ++i ) {
			gtk_list_store_append( m_sessions, &iter);
			gtk_list_store_set( m_sessions, &iter, 0, i->c_str(), -1);
		}
		gtk_combo_box_set_model( GTK_COMBO_BOX (eEdfImportSession),
					 GTK_TREE_MODEL (m_sessions));
		gtk_combo_box_set_entry_text_column( GTK_COMBO_BOX (eEdfImportSession), 0);

	      // guess episode from fname
		char *fname2 = g_strdup( fname), *episode = strrchr( fname2, '/')+1;
		if ( g_str_has_suffix( episode, ".edf") || g_str_has_suffix( episode, ".EDF") )
			*strrchr( episode, '.') = '\0';
		gtk_entry_set_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportEpisode))),
				    episode);

	      // display
		gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportAdmit), FALSE);
		gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportScoreSeparately), FALSE);
		gint response = gtk_dialog_run( GTK_DIALOG (wEdfImport));
		const gchar
			*selected_group   = gtk_entry_get_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportGroup)))),
			*selected_session = gtk_entry_get_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportSession)))),
			*selected_episode = gtk_entry_get_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportEpisode))));
		switch ( response ) {
		case GTK_RESPONSE_OK: // Admit
		{	char *dest_path, *dest, *cmd;
			dest_path = g_strdup_printf( "%s/%s/%s/%s",
						     AghCC->session_dir(),
						     selected_group,
						     F->patient.c_str(),
						     selected_session);
			dest = g_strdup_printf( "%s/%s.edf",
						dest_path,
						selected_episode);
			if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (bEdfImportAttachCopy)) )
				cmd = g_strdup_printf( "mkdir -p '%s' && cp -n '%s' '%s'", dest_path, fname, dest);
			else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (bEdfImportAttachMove)) )
				cmd = g_strdup_printf( "mkdir -p '%s' && mv -n '%s' '%s'", dest_path, fname, dest);
			else
				cmd = g_strdup_printf( "mkdir -p '%s' && ln -s '%s' '%s'", dest_path, fname, dest);

			int cmd_exit = system( cmd);
			if ( cmd_exit )
				pop_ok_message( GTK_WINDOW (wMainWindow), "Command\n %s\nexited with code %d", cmd_exit);

			g_free( cmd);
			g_free( dest);
			g_free( dest_path);

			aghui::depopulate( 0);
			aghui::populate( 0);
		}
		    break;
		case GTK_RESPONSE_CANCEL: // Drop
			break;
		case -7: // GTK_RESPONSE_CLOSE:  View separately
			break;
		}

	      // finalise
		g_free( fname2);

		g_object_unref( m_groups);
		g_object_unref( m_sessions);
		g_object_unref( m_episodes);

		return 0;
	}
}

}
}


extern "C" {

	gboolean
	check_gtk_entry_nonempty( GtkWidget *ignored,
				  GdkEventKey *event,
				  gpointer  user_data)
	{
		gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportAdmit), TRUE);
		gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportScoreSeparately), TRUE);

		const gchar *e;
		gchar *ee;

		ee = NULL;
		e = gtk_combo_box_get_active_id( GTK_COMBO_BOX (eEdfImportGroup));
		if ( !e || !*g_strchug( g_strchomp( ee = g_strdup( e))) ) {
			gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportAdmit), FALSE);
			gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportScoreSeparately), FALSE);
		}
		g_free( ee);

		ee = NULL;
		e = gtk_combo_box_get_active_id( GTK_COMBO_BOX (eEdfImportSession));
		if ( !e || !*g_strchug( g_strchomp( ee = g_strdup( e))) ) {
			gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportAdmit), FALSE);
			gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportScoreSeparately), FALSE);
		}
		g_free( ee);

		ee = NULL;
		e = gtk_combo_box_get_active_id( GTK_COMBO_BOX (eEdfImportEpisode));
		if ( !e || !*g_strchug( g_strchomp( ee = g_strdup( e))) ) {
			gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportAdmit), FALSE);
			gtk_widget_set_sensitive( GTK_WIDGET (bEdfImportScoreSeparately), FALSE);
		}
		g_free( ee);

		gtk_widget_queue_draw( GTK_WIDGET (bEdfImportAdmit));
		gtk_widget_queue_draw( GTK_WIDGET (bEdfImportScoreSeparately));

		return false;
	}




	gboolean
	cMeasurements_drag_data_received_cb( GtkWidget        *widget,
					     GdkDragContext   *context,
					     gint              x,
					     gint              y,
					     GtkSelectionData *selection_data,
					     guint             info,
					     guint             time,
					     gpointer          user_data)
	{
		gchar **uris = gtk_selection_data_get_uris( selection_data);
		if ( uris != NULL ) {

			guint i = 0;
			while ( uris[i] ) {
				if ( strncmp( uris[i], "file://", 7) == 0 ) {
					char *fname = g_filename_from_uri( uris[i], NULL, NULL);
					int retval = msmtview::dnd::maybe_admit_one( fname);
					g_free( fname);
					if ( retval )
						break;
				}
				++i;
			}

			// fear no shortcuts
			do_rescan_tree();

			g_strfreev( uris);
		}

		gtk_drag_finish (context, TRUE, FALSE, time);
		return TRUE;
	}


	gboolean
	cMeasurements_drag_drop_cb( GtkWidget      *widget,
				    GdkDragContext *context,
				    gint            x,
				    gint            y,
				    guint           time,
				    gpointer        user_data)
	{
//	GdkAtom         target_type;
//
//      if ( context->targets ) {
//              // Choose the best target type
//              target_type = GDK_POINTER_TO_ATOM
//                      (g_list_nth_data( context->targets, 0));
//		unsigned i = g_list_length(context->targets);
//		while ( i-- )
//			printf( "%zu: %s\n", i, gdk_atom_name( GDK_POINTER_TO_ATOM (g_list_nth_data( context->targets, i))));
//
//		//Request the data from the source.
//              gtk_drag_get_data(
//                      widget,         // will receive 'drag-data-received' signal
//                      context,        // represents the current state of the DnD
//                      target_type,    // the target type we want
//                      time);          // time stamp
//
//	} else { // No target offered by source => error
//              return FALSE;
//	}
//
		return  TRUE;
	}
}
} // namespace aghui

// eof

