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
using namespace aghui;


int
aghui::SExpDesignUI::
dnd_maybe_admit_one( const char* fname)
{
	using namespace sigfile;
	CTypedSource *Fp = nullptr;

	string info;
	try {
		Fp = new CTypedSource (fname, ED->fft_params.pagesize);
		switch ( Fp->type() ) {
		case CTypedSource::TType::edf:
		{
			CEDFFile& F = *static_cast<CEDFFile*> (&(*Fp)());
			if ( F.subtype() == CEDFFile::TSubtype::edfplus_d ) {
				pop_ok_message(
					wMainWindow, "EDF+D Unsupported", "The file <b>%s</b> is in EDF+D format, which is not supported yet",
					fname);
				return 0;
			}
			if ( F.status() & CEDFFile::TStatus::inoperable ) {
				pop_ok_message(
					wMainWindow, "Bad EDF file", "The file <b>%s</b> cannot be processed due to these issues:\n\n%s",
					fname, F.explain_status().c_str());
				return 0;
			}

			info = (*Fp)().details( 0|sigfile::CEDFFile::with_channels);
			gtk_label_set_markup(
				lEdfImportCaption,
				(snprintf_buf( "File: <i>%s</i>", fname),
				 __buf__));
			gtk_label_set_markup(
				lEdfImportSubject,
				(snprintf_buf( "<b>%s</b> (%s)", F.subject().id.c_str(), F.subject().name.c_str()),
				 __buf__));
		}
		break;
		default:
			pop_ok_message( wMainWindow, "Unsupported format", "The file <b>%s</b> is in unrecognised format. Sorry.", fname);
			return 0;
		}

	} catch ( exception& ex) {
		pop_ok_message( wMainWindow, "Corrupted EDF file", "File <b>%s</b> doesn't appear to have a valid header:\n\n%s",
				fname, (*Fp)().explain_status().c_str());
		return 0;
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
			ED->group_of( (*Fp)().subject().id.c_str()));
		gtk_widget_set_sensitive( (GtkWidget*)eEdfImportGroup, FALSE);
	} catch (invalid_argument ex) {
		for ( auto &i : AghGG ) {
			gtk_list_store_append( m_groups, &iter);
			gtk_list_store_set( m_groups, &iter, 0, i.c_str(), -1);
		}
		gtk_combo_box_set_model( eEdfImportGroup,
					 (GtkTreeModel*)m_groups);
		gtk_combo_box_set_entry_text_column( eEdfImportGroup, 0);
		// gtk_entry_set_text(
		// 	(GtkEntry*)gtk_bin_get_child( (GtkBin*)eEdfImportGroup),
		// 	"");
		gtk_widget_set_sensitive( (GtkWidget*)eEdfImportGroup, TRUE);
	}

	for ( auto &i : AghEE ) {
		gtk_list_store_append( m_episodes, &iter);
		gtk_list_store_set( m_episodes, &iter, 0, i.c_str(), -1);
	}
	gtk_combo_box_set_model( eEdfImportEpisode,
				 (GtkTreeModel*)m_episodes);
	gtk_combo_box_set_entry_text_column( eEdfImportEpisode, 0);

	for ( auto &i : AghDD ) {
		gtk_list_store_append( m_sessions, &iter);
		gtk_list_store_set( m_sessions, &iter, 0, i.c_str(), -1);
	}
	gtk_combo_box_set_model( eEdfImportSession,
				 (GtkTreeModel*)m_sessions);
	gtk_combo_box_set_entry_text_column( eEdfImportSession, 0);

      // guess episode from fname
	char *fname2 = g_strdup( fname), *episode = strrchr( fname2, '/')+1;
	if ( g_str_has_suffix( episode, ".edf") || g_str_has_suffix( episode, ".EDF") )
		*strrchr( episode, '.') = '\0';
	gtk_entry_set_text( (GtkEntry*)gtk_bin_get_child( (GtkBin*)eEdfImportEpisode),
			    episode);

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
		char *dest_path, *dest, *cmd;
		dest_path = g_strdup_printf( "%s/%s/%s/%s",
					     ED->session_dir().c_str(),
					     selected_group,
					     (*Fp)().subject().id.c_str(),
					     selected_session);
		dest = g_strdup_printf( "%s/%s.edf",
					dest_path,
					selected_episode);
		if ( gtk_toggle_button_get_active( (GtkToggleButton*)bEdfImportAttachCopy) )
			cmd = g_strdup_printf( "mkdir -p '%s' && cp -n '%s' '%s'", dest_path, fname, dest);
		else if ( gtk_toggle_button_get_active( (GtkToggleButton*)bEdfImportAttachMove) )
			cmd = g_strdup_printf( "mkdir -p '%s' && mv -n '%s' '%s'", dest_path, fname, dest);
		else
			cmd = g_strdup_printf( "mkdir -p '%s' && ln -s '%s' '%s'", dest_path, fname, dest);
		char* cmde = g_markup_escape_text( cmd, -1);

		int cmd_exit = system( cmd);
		if ( cmd_exit )
			pop_ok_message( wMainWindow,
					"Failed to create recording path in experiment tree",
					"Command\n <span font=\"monospace\">%s</span>\nexited with code %d", cmde, cmd_exit);

		g_free( cmd);
		g_free( cmde);
		g_free( dest);
		g_free( dest_path);
	}
	    break;
	case GTK_RESPONSE_CANCEL: // Drop
		break;
	}

      // finalise
	g_free( fname2);

	g_object_unref( m_groups);
	g_object_unref( m_sessions);
	g_object_unref( m_episodes);

	return 0;
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
// End:

