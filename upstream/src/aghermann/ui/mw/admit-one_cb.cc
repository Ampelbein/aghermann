/*
 *       File name:  aghermann/ui/mw/admit-one_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-08-11
 *
 *         Purpose:  SExpDesignUI edf import via dnd
 *
 *         License:  GPL
 */


#include "mw.hh"

using namespace std;
using namespace agh::ui;

extern "C" {

gboolean
check_gtk_entry_nonempty_cb(
	GtkEditable*,
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
cMeasurements_drag_data_received_cb(
	const GtkWidget        *widget,
	GdkDragContext         *context,
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
cMeasurements_drag_drop_cb(
	GtkWidget*,
	GdkDragContext*,
	gint x,
	gint y,
	guint time,
	gpointer userdata)
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
// c-basic-offset: 8
// End:

