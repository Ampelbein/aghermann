// ;-*-C++-*- *  Time-stamp: "2011-07-25 11:07:30 hmmr"
/*
 *       File name:  tools/edfed-gtk.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-20
 *
 *         Purpose:  EDF header editor utility (using gtk)
 *
 *         License:  GPL
 */


#include <gtk/gtk.h>
#include "libagh/edf.hh"

#include "edfhed.hh"


namespace aghui {
	void pop_ok_message( GtkWindow *parent, const gchar*, ...);
	gint pop_question( GtkWindow *parent, const gchar*);
	void set_cursor_busy( bool busy, GtkWidget *wid);
}

#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( __builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)			\
	(A = (Type*)(gtk_builder_get_object( B, #A)))



GtkDialog
	*wMain;
GtkLabel
	*lLabel;
GtkEntry
	*ePatientID, *eRecordingID,
	*eRecordingDate, *eRecordingTime, *eReserved;

GtkBox
	*cChannels;

GtkBuilder
	*__builder;

int ui_init();
void ui_fini();



void edf_data_to_widgets( const agh::CEDFFile&);
void widgets_to_edf_data( agh::CEDFFile&);


int
main( int argc, char **argv)
{
	int	c;
	while ( (c = getopt( argc, argv, "h")) != -1 )
		switch ( c ) {
		case 'h':
			printf( "Usage: %s file.edf\n", argv[0]);
			return 0;
		}

	g_thread_init( NULL);
	gtk_init( &argc, &argv);

	const char *fname;
	if ( optind < argc )
		fname = argv[optind];
	else {
		aghui::pop_ok_message( NULL, "Usage: %s file.edf", argv[0]);
		return 1;
	}

	if ( ui_init() ) {
		aghui::pop_ok_message( NULL, "UI failed to initialise\n");
		return 2;
	}

	try {
		auto F = agh::CEDFFile (fname, 30);
		F.no_save_extra_files = true;

		edf_data_to_widgets( F);

		if ( gtk_dialog_run( wMain) == -5 ) {
			widgets_to_edf_data( F);
		}
	} catch (invalid_argument ex) {
		aghui::pop_ok_message( NULL, ex.what());
	}

	ui_fini();

	return 0;
}



void
edf_data_to_widgets( const agh::CEDFFile& F)
{
	gtk_label_set_markup( lLabel, (string ("<b>File:</b> <i>") + fname + "</i>").c_str());
	gtk_entry_set_text( ePatientID,     strtrim( string (F.header.patient_id,     80)) . c_str());
	gtk_entry_set_text( eRecordingID,   strtrim( string (F.header.recording_id,   80)) . c_str());
	gtk_entry_set_text( eRecordingDate, strtrim( string (F.header.recording_date,  8)) . c_str());
	gtk_entry_set_text( eRecordingTime, strtrim( string (F.header.recording_time,  8)) . c_str());
	gtk_entry_set_text( eReserved,      strtrim( string (F.header.reserved,       44)) . c_str());

	for ( auto h = F.signals.begin(); h != F.signals.end(); ++h ) {
		auto label = gtk_label_new_with_mnemonic( h->channel.c_str());
		g_object_set( (GObject*)label, "visible", TRUE, NULL);
		gtk_box_pack_start( (GtkBox*)cChannels, label,
				    TRUE, TRUE, 5);
	}
}



void
widgets_to_edf_data( agh::CEDFFile& F)
{
	memcpy( F.header.patient_id,     strpad( gtk_entry_get_text( ePatientID),     80).c_str(), 80);
	memcpy( F.header.recording_id,   strpad( gtk_entry_get_text( eRecordingID),   80).c_str(), 80);
	memcpy( F.header.recording_date, strpad( gtk_entry_get_text( eRecordingDate),  8).c_str(),  8);
	memcpy( F.header.recording_time, strpad( gtk_entry_get_text( eRecordingTime),  8).c_str(),  8);
	memcpy( F.header.reserved,       strpad( gtk_entry_get_text( eReserved),      44).c_str(), 44);
}





void
bNext_clicked_cb( GtkButton *button, gpointer userdata)
{
}

void
bPrevious_clicked_cb( GtkButton *button, gpointer userdata)
{
}






int
ui_init()
{
      // load glade
	__builder = gtk_builder_new();
	if ( !gtk_builder_add_from_file( __builder, PACKAGE_DATADIR "/" PACKAGE "/ui/edf-header-editor.glade", NULL) ) {
		aghui::pop_ok_message( NULL, "Failed to load UI description file.");
		return -1;
	}

	if ( !AGH_GBGETOBJ (GtkDialog,  wMain) ||
	     !AGH_GBGETOBJ (GtkLabel,   lLabel) ||
	     !AGH_GBGETOBJ (GtkEntry,	ePatientID) ||
	     !AGH_GBGETOBJ (GtkEntry,	eRecordingID) ||
	     !AGH_GBGETOBJ (GtkEntry,	eRecordingDate) ||
	     !AGH_GBGETOBJ (GtkEntry,	eRecordingTime) ||
	     !AGH_GBGETOBJ (GtkEntry,	eReserved) ||
	     !AGH_GBGETOBJ (GtkBox,     cChannels) )
		return -1;

	gtk_builder_connect_signals( __builder, NULL);

	return 0;
}


void
ui_fini()
{
	// gtk_widget_destroy
	g_object_unref( __builder);
}


// EOF
