/*
 *       File name:  aghermann/ui/mw/admit-one.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-18
 *
 *         Purpose:  SExpDesignUI edf import via dnd
 *
 *         License:  GPL
 */


#include "libsigfile/edf.hh"
#include "aghermann/ui/misc.hh"
#include "mw.hh"

using namespace std;
using namespace agh::ui;


int
agh::ui::SExpDesignUI::
dnd_maybe_admit_one( const char* fname)
{
	try {
		string info;
		sigfile::CTypedSource F_ (fname, ED->fft_params.pagesize);
		switch ( F_.type() ) {
		case sigfile::CTypedSource::TType::edf:
		{
			sigfile::CEDFFile& F = F_.obj<sigfile::CEDFFile>();
			if ( F.subtype() == sigfile::CEDFFile::TSubtype::edfplus_d ) {
				pop_ok_message(
					wMainWindow,
					"EDF+D is unsupported",
					"The file <b>%s</b> is in EDF+D format, which is not supported yet",
					fname);
				return 0;
			}
			if ( F.status() & sigfile::CEDFFile::TStatus::inoperable ) {
				pop_ok_message(
					wMainWindow,
					"Bad EDF file",
					"The file <b>%s</b> cannot be processed due to these issues:\n\n%s",
					fname, F.explain_status().c_str());
				return 0;
			}
		}
		break;

		case sigfile::CTypedSource::TType::ascii:
		break;

		default:
			pop_ok_message(
				wMainWindow,
				"Unsupported format",
				"The file <b>%s</b> is in unrecognised format. Sorry.", fname);
			return 0;
		}

		auto& F = F_();

		info = F.details( 0|sigfile::CSource::TDetails::with_channels);

		{
			char* mike;
			mike = g_markup_escape_text(
				agh::str::homedir2tilda( F.filename()).c_str(),
				-1);
			gtk_label_set_markup(
				lEdfImportCaption,
				snprintf_buf( "File: <i>%s</i>", mike));
			free( (void*)mike);

			mike = g_markup_escape_text(
				F.subject().name.empty() ? "<no name>" : F.subject().name.c_str(),
				-1);
			gtk_label_set_markup(
				lEdfImportSubject,
				snprintf_buf(
					"<b>%s</b> (%s)",
					F.subject().id.c_str(),
					mike));
			free( (void*)mike);
		}

		gtk_text_buffer_set_text( tEDFFileDetailsReport, info.c_str(), -1);

		GtkTreeIter iter;
		// populate and attach models
		GtkListStore
			*m_groups = gtk_list_store_new( 1, G_TYPE_STRING),
			*m_episodes = gtk_list_store_new( 1, G_TYPE_STRING),
			*m_sessions = gtk_list_store_new( 1, G_TYPE_STRING);
		// when adding a source for an already existing subject, disallow group selection
		try {
			gtk_entry_set_text(
				eEdfImportGroupEntry,
				ED->group_of( F.subject().id.c_str()));
			gtk_widget_set_sensitive( (GtkWidget*)eEdfImportGroup, FALSE);
		} catch (invalid_argument ex) {
			for ( auto &i : AghGG ) {
				gtk_list_store_append( m_groups, &iter);
				gtk_list_store_set( m_groups, &iter, 0, i.c_str(), -1);
			}
			gtk_combo_box_set_model(
				eEdfImportGroup,
				(GtkTreeModel*)m_groups);
			gtk_combo_box_set_entry_text_column( eEdfImportGroup, 0);
			// gtk_entry_set_text(
			// 	(GtkEntry*)gtk_bin_get_child( (GtkBin*)eEdfImportGroup),
			// 	"");
			gtk_widget_set_sensitive( (GtkWidget*)eEdfImportGroup, TRUE);
		}

		// enumerate known sessions and episodes
		// suggest those from the file proper
		for ( auto &i : AghEE ) {
			gtk_list_store_append( m_episodes, &iter);
			gtk_list_store_set( m_episodes, &iter, 0, i.c_str(), -1);
		}
		gtk_combo_box_set_model(
			eEdfImportEpisode,
			(GtkTreeModel*)m_episodes);
		gtk_combo_box_set_entry_text_column(
			eEdfImportEpisode, 0);
		gtk_entry_set_text(
			(GtkEntry*)gtk_bin_get_child( (GtkBin*)eEdfImportEpisode),
			F.episode());

		for ( auto &i : AghDD ) {
			gtk_list_store_append( m_sessions, &iter);
			gtk_list_store_set( m_sessions, &iter, 0, i.c_str(), -1);
		}
		gtk_combo_box_set_model(
			eEdfImportSession,
			(GtkTreeModel*)m_sessions);
		gtk_combo_box_set_entry_text_column(
			eEdfImportSession, 0);
		gtk_entry_set_text(
			(GtkEntry*)gtk_bin_get_child( (GtkBin*)eEdfImportSession),
			F.session());

		// display
		g_signal_emit_by_name( eEdfImportGroupEntry, "changed");

		gint response = gtk_dialog_run( (GtkDialog*)wEdfImport);
		const gchar
			*selected_group   = gtk_entry_get_text( eEdfImportGroupEntry),
			*selected_session = gtk_entry_get_text( eEdfImportSessionEntry),
			*selected_episode = gtk_entry_get_text( eEdfImportEpisodeEntry);
		switch ( response ) {
		case GTK_RESPONSE_OK: // Admit
		{
			string dest_path, dest, cmd;
			using agh::str::sasprintf;
			dest_path = sasprintf(
				"%s/%s/%s/%s",
				ED->session_dir().c_str(), selected_group,
				F.subject().id.c_str(), selected_session);
			dest = sasprintf(
				"%s/%s.edf",
				dest_path.c_str(), selected_episode);
			if ( gtk_toggle_button_get_active( (GtkToggleButton*)bEdfImportAttachCopy) )
				cmd = sasprintf( "mkdir -p '%s' && cp -n '%s' '%s'", dest_path.c_str(), fname, dest.c_str());
			else if ( gtk_toggle_button_get_active( (GtkToggleButton*)bEdfImportAttachMove) )
				cmd = sasprintf( "mkdir -p '%s' && mv -n '%s' '%s'", dest_path.c_str(), fname, dest.c_str());
			else
				cmd = sasprintf( "mkdir -p '%s' && ln -s '%s' '%s'", dest_path.c_str(), fname, dest.c_str());
			char* cmde = g_markup_escape_text( cmd.c_str(), -1);

			int cmd_exit = system( cmd.c_str());
			if ( cmd_exit )
				pop_ok_message(
					wMainWindow,
					"Failed to create recording path in experiment tree",
					"Command\n <span font=\"monospace\">%s</span>\nexited with code %d",
					cmde, cmd_exit);
			g_free( cmde);
		}
		break;
		case GTK_RESPONSE_CANCEL: // Drop
			break;
		}

		g_object_unref( m_groups);
		g_object_unref( m_sessions);
		g_object_unref( m_episodes);

		return 0;

	} catch ( exception& ex) {
		pop_ok_message(
			wMainWindow,
			"Corrupted source file", "File <b>%s</b> could not be processed.",
			fname);
		return 0;
	}
}



extern "C" {

gboolean
check_gtk_entry_nonempty_cb( GtkEditable*,
			     // GdkEventKey *event,
			     const gpointer  userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	gtk_widget_set_sensitive( (GtkWidget*)ED.bEdfImportAdmit, TRUE);

	const gchar *e;
	gchar *ee;

	ee = NULL;
	e = gtk_entry_get_text( ED.eEdfImportGroupEntry);
	if ( !e || !*g_strchug( g_strchomp( ee = g_strdup( e))) ) {
		gtk_widget_set_sensitive( (GtkWidget*)ED.bEdfImportAdmit, FALSE);
	}
	g_free( ee);

	ee = NULL;
	e = gtk_entry_get_text( ED.eEdfImportSessionEntry);
	if ( !e || !*g_strchug( g_strchomp( ee = g_strdup( e))) ) {
		gtk_widget_set_sensitive( (GtkWidget*)ED.bEdfImportAdmit, FALSE);
	}
	g_free( ee);

	ee = NULL;
	e = gtk_entry_get_text( ED.eEdfImportEpisodeEntry);
	if ( !e || !*g_strchug( g_strchomp( ee = g_strdup( e))) ) {
		gtk_widget_set_sensitive( (GtkWidget*)ED.bEdfImportAdmit, FALSE);
	}
	g_free( ee);

	gtk_widget_queue_draw( (GtkWidget*)ED.bEdfImportAdmit);

	return false;
}




void
cMeasurements_drag_data_received_cb( const GtkWidget        *widget,
				     GdkDragContext   *context,
				     const gint              x,
				     const gint              y,
				     const GtkSelectionData *selection_data,
				     const guint             info,
				     const guint             time,
				     const gpointer          userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	gchar **uris = gtk_selection_data_get_uris( selection_data);
	if ( uris != NULL ) {

		guint i = 0;
		while ( uris[i] ) {
			if ( strncmp( uris[i], "file://", 7) == 0 ) {
				char *fname = g_filename_from_uri( uris[i], NULL, NULL);
				int retval = ED.dnd_maybe_admit_one( fname);
				g_free( fname);
				if ( retval )
					break;
			}
			++i;
		}

		// fear no shortcuts
		ED.do_rescan_tree( true);

		g_strfreev( uris);
	}

	gtk_drag_finish (context, TRUE, FALSE, time);
}


gboolean
__attribute__ ((const))
cMeasurements_drag_drop_cb( GtkWidget      *widget,
			    GdkDragContext *context,
			    gint            x,
			    gint            y,
			    guint           time,
			    gpointer        userdata)
{
		//auto& ED = *(SExpDesignUI*)userdata;
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




} // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:

