// ;-*-C++-*-
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


namespace aghui {
	void pop_ok_message( GtkWindow *parent, const gchar*, ...);
	gint pop_question( GtkWindow *parent, const gchar*);
	void set_cursor_busy( bool busy, GtkWidget *wid);
}

#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( __builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)			\
	(A = (Type*)(gtk_builder_get_object( B, #A)))




GtkBuilder
	*__builder;

GtkDialog
	*wMain;
GtkLabel
	*lLabel,
	*lChannelsNum;

enum TEntry : int {
	PatientID, RecordingID,
	RecordingDate, RecordingTime, Reserved,
	ChannelLabel, ChannelPhysicalDim,
	ChannelPhysicalMin, ChannelPhysicalMax,
	ChannelDigitalMin, ChannelDigitalMax,
	ChannelTransducerType, ChannelFilteringInfo,
	ChannelReserved,
	ChannelSamplesPerRecord,
	_n_entries
};

GtkEntry
	*e[_n_entries];
GtkButton
	*bNext, *bPrevious,
	*bWrite;


int ui_init();
void ui_fini();



void edf_data_to_widgets( const agh::CEDFFile&);
void widgets_to_edf_data( agh::CEDFFile&);

agh::CEDFFile *Fp;


struct SChannelTmp {
	string	Label, PhysicalDim,
		PhysicalMin, PhysicalMax,
		DigitalMin, DigitalMax,
		TransducerType, FilteringInfo,
		SamplesPerRecord,
		Reserved;
	SChannelTmp( const string& iLabel, const string& iPhysicalDim,
		     const string& iPhysicalMin, const string& iPhysicalMax,
		     const string& iDigitalMin, const string& iDigitalMax,
		     const string& iTransducerType, const string& iFilteringInfo,
		     const string& iSamplesPerRecord,
		     const string& iReserved)
	      : Label (iLabel), PhysicalDim (iPhysicalDim),
		PhysicalMin (iPhysicalMin), PhysicalMax (iPhysicalMax),
		DigitalMin (iDigitalMin), DigitalMax (iDigitalMax),
		TransducerType (iTransducerType), FilteringInfo (iFilteringInfo),
		SamplesPerRecord (iSamplesPerRecord),
		Reserved (iReserved)
		{}
};

list<SChannelTmp>
	channels_tmp;
list<SChannelTmp>::iterator
	HTmpi;

void current_channel_data_to_widgets();
void widgets_to_current_channel_data();
void sensitize_channel_nav_buttons();

size_t channel_no;




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
		if ( isatty( fileno( stdin)) ) {
			printf( "Usage: %s file.edf\n", argv[0]);
			return 0;
		} else {
			// aghui::pop_ok_message( NULL, "Usage: %s file.edf", argv[0]);
			GtkWidget *f_chooser = gtk_file_chooser_dialog_new(
				"EDFHEd: Choose a file to edit",
				NULL,
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
			GtkFileFilter *file_filter = gtk_file_filter_new();
			gtk_file_filter_set_name( file_filter, "EDF recordings");
			gtk_file_filter_add_pattern( file_filter, "*.edf");
			gtk_file_chooser_add_filter( (GtkFileChooser*)f_chooser, file_filter);

			if ( gtk_dialog_run( GTK_DIALOG (f_chooser)) == GTK_RESPONSE_ACCEPT )
				fname = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (f_chooser));
			else
				return 0;
			gtk_widget_destroy( f_chooser);
		}
	}

	if ( ui_init() ) {
		aghui::pop_ok_message( NULL, "UI failed to initialise\n");
		return 2;
	}

	try {
		auto F = agh::CEDFFile (fname, 30);
		F.no_save_extra_files = true;
		channel_no = 0;
		Fp = &F;

		edf_data_to_widgets( F);
		HTmpi = channels_tmp.begin();
		current_channel_data_to_widgets();

		sensitize_channel_nav_buttons();
		if ( gtk_dialog_run( wMain) == -5 ) {
			// something edited but no Next/Prev pressed to trigger this, so do it now
			widgets_to_current_channel_data();
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
	gtk_label_set_markup( lLabel, (string ("<b>File:</b> <i>") + F.filename() + "</i>").c_str());
	gtk_entry_set_text( e[PatientID],     strtrim( string (F.header.patient_id,     80)) . c_str());
	gtk_entry_set_text( e[RecordingID],   strtrim( string (F.header.recording_id,   80)) . c_str());
	gtk_entry_set_text( e[RecordingDate], strtrim( string (F.header.recording_date,  8)) . c_str());
	gtk_entry_set_text( e[RecordingTime], strtrim( string (F.header.recording_time,  8)) . c_str());
	gtk_entry_set_text( e[Reserved],      strtrim( string (F.header.reserved,       44)) . c_str());

	for ( auto h = F.signals.begin(); h != F.signals.end(); ++h ) {
		channels_tmp.emplace_back(
			strtrim( string (h->header.label, 16)),
			strtrim( string (h->header.physical_dim, 8)),
			strtrim( string (h->header.physical_min, 8)),
			strtrim( string (h->header.physical_max, 8)),
			strtrim( string (h->header.digital_min,  8)),
			strtrim( string (h->header.digital_max,  8)),
			strtrim( string (h->header.transducer_type, 80)),
			strtrim( string (h->header.filtering_info, 80)),
			strtrim( string (h->header.samples_per_record, 8)),
			strtrim( string (h->header.reserved, 32)));
	}
}



void
widgets_to_edf_data( agh::CEDFFile& F)
{
	memcpy( F.header.patient_id,     strpad( gtk_entry_get_text( e[PatientID]),     80).c_str(), 80);
	memcpy( F.header.recording_id,   strpad( gtk_entry_get_text( e[RecordingID]),   80).c_str(), 80);
	memcpy( F.header.recording_date, strpad( gtk_entry_get_text( e[RecordingDate]),  8).c_str(),  8);
	memcpy( F.header.recording_time, strpad( gtk_entry_get_text( e[RecordingTime]),  8).c_str(),  8);
	memcpy( F.header.reserved,       strpad( gtk_entry_get_text( e[Reserved]),      44).c_str(), 44);

	auto H = channels_tmp.begin();
	for ( auto h = F.signals.begin(); h != F.signals.end(); ++h, ++H ) {
		memcpy( h->header.label,		strpad( H->Label,           16).c_str(), 16);
		memcpy( h->header.physical_dim,		strpad( H->PhysicalDim,      8).c_str(),  8);
		memcpy( h->header.physical_min,		strpad( H->PhysicalMin,      8).c_str(),  8);
		memcpy( h->header.physical_max,		strpad( H->PhysicalMax,      8).c_str(),  8);
		memcpy( h->header.digital_min,		strpad( H->DigitalMin,       8).c_str(),  8);
		memcpy( h->header.digital_max,		strpad( H->DigitalMax,       8).c_str(),  8);
		memcpy( h->header.transducer_type,	strpad( H->TransducerType,  80).c_str(), 80);
		memcpy( h->header.filtering_info,	strpad( H->FilteringInfo,   80).c_str(), 80);
		memcpy( h->header.samples_per_record,	strpad( H->SamplesPerRecord, 8).c_str(),  8);
		memcpy( h->header.reserved,		strpad( H->Reserved,        32).c_str(), 32);
	}
}





void
current_channel_data_to_widgets()
{
	static char buf[120];
	snprintf( buf, 119, "channel %zu of %zu", channel_no+1, Fp->signals.size());
	gtk_label_set_markup( lChannelsNum, buf);
	gtk_entry_set_text( e[ChannelLabel],		strtrim( HTmpi->Label      ) . c_str());
	gtk_entry_set_text( e[ChannelPhysicalDim],	strtrim( HTmpi->PhysicalDim) . c_str());
	gtk_entry_set_text( e[ChannelPhysicalMin],	strtrim( HTmpi->PhysicalMin) . c_str());
	gtk_entry_set_text( e[ChannelPhysicalMax],	strtrim( HTmpi->PhysicalMax) . c_str());
	gtk_entry_set_text( e[ChannelDigitalMin],		strtrim( HTmpi->DigitalMin ) . c_str());
	gtk_entry_set_text( e[ChannelDigitalMax],		strtrim( HTmpi->DigitalMax ) . c_str());
	gtk_entry_set_text( e[ChannelTransducerType],	strtrim( HTmpi->TransducerType) . c_str());
	gtk_entry_set_text( e[ChannelFilteringInfo],	strtrim( HTmpi->FilteringInfo)  . c_str());
	gtk_entry_set_text( e[ChannelSamplesPerRecord],	strtrim( HTmpi->SamplesPerRecord) . c_str());
	gtk_entry_set_text( e[ChannelReserved],		strtrim( HTmpi->Reserved)         . c_str());
}

void
widgets_to_current_channel_data()
{
	HTmpi->Label		= gtk_entry_get_text( e[ChannelLabel]);
	HTmpi->PhysicalDim	= gtk_entry_get_text( e[ChannelPhysicalDim]);
	HTmpi->PhysicalMin	= gtk_entry_get_text( e[ChannelPhysicalMin]);
	HTmpi->PhysicalMax	= gtk_entry_get_text( e[ChannelPhysicalMax]);
	HTmpi->DigitalMin	= gtk_entry_get_text( e[ChannelDigitalMin]);
	HTmpi->DigitalMax	= gtk_entry_get_text( e[ChannelDigitalMax]);
	HTmpi->TransducerType	= gtk_entry_get_text( e[ChannelTransducerType]);
	HTmpi->FilteringInfo	= gtk_entry_get_text( e[ChannelFilteringInfo]);
	HTmpi->SamplesPerRecord	= gtk_entry_get_text( e[ChannelSamplesPerRecord]);
	HTmpi->Reserved		= gtk_entry_get_text( e[ChannelReserved]);
}


bool
validate_all_widgets()
{
	return true;
}


bool
validate_text_entry_buffer( const char *str, size_t i)
{
	unsigned long u;
	char *tail;
	switch ( i ) {
	case PatientID:
	case RecordingID:
		return true;
	case RecordingDate:
	{
		struct tm ts;
		char *p = strptime( str, "%d.%m.%y", &ts);
		return strlen(str) == 8 && p && *p == '\0';
	}
	case RecordingTime:
	{
		struct tm ts;
		char *p = strptime( str, "%H.%M.%S", &ts);
		return strlen(str) == 8 && p && *p == '\0';
	}
	case Reserved:
	case ChannelLabel:
	case ChannelPhysicalDim:
		return true;
	case ChannelPhysicalMin:
	case ChannelPhysicalMax:
		return u = strtod( str, &tail), *tail == '\0';
	case ChannelDigitalMin:
	case ChannelDigitalMax:
		return u = strtol( str, &tail, 10), *tail == '\0';
	case ChannelTransducerType:
	case ChannelFilteringInfo:
	case ChannelReserved:
		return true;
	case ChannelSamplesPerRecord:
		return u = strtol( str, &tail, 10), *tail == '\0';
	}
	return false; // wtf is i?
}

extern "C" void
bNext_clicked_cb( GtkButton *button, gpointer userdata)
{
	if ( next(HTmpi) != channels_tmp.end() ) {
		widgets_to_current_channel_data();
		++HTmpi;
		++channel_no;
		current_channel_data_to_widgets();
	}
	sensitize_channel_nav_buttons();
}

extern "C" void
bPrevious_clicked_cb( GtkButton *button, gpointer userdata)
{
	if ( HTmpi != channels_tmp.begin() ) {
		widgets_to_current_channel_data();
		--HTmpi;
		--channel_no;
		current_channel_data_to_widgets();
	}
	sensitize_channel_nav_buttons();
}


extern "C" void
inserted_text_cb( GtkEntryBuffer *buffer,
		  guint           position,
		  gchar          *chars,
		  guint           n_chars,
		  gpointer        userdata)
{
	gtk_widget_set_sensitive( (GtkWidget*)bWrite,
				  validate_text_entry_buffer( gtk_entry_buffer_get_text( buffer),
							      (size_t)userdata));
}
extern "C" void
deleted_text_cb( GtkEntryBuffer *buffer,
		 guint           position,
		 guint           n_chars,
		 gpointer        userdata)
{
	gtk_widget_set_sensitive( (GtkWidget*)bWrite,
				  validate_text_entry_buffer( gtk_entry_buffer_get_text( buffer),
							      (size_t)userdata));
}

void
sensitize_channel_nav_buttons()
{
	gtk_widget_set_sensitive( (GtkWidget*)bNext, not (next(HTmpi) == channels_tmp.end()));
	gtk_widget_set_sensitive( (GtkWidget*)bPrevious, not (HTmpi == channels_tmp.begin()));
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
	     !AGH_GBGETOBJ (GtkEntry,	e[PatientID]) ||
	     !AGH_GBGETOBJ (GtkEntry,	e[RecordingID]) ||
	     !AGH_GBGETOBJ (GtkEntry,	e[RecordingDate]) ||
	     !AGH_GBGETOBJ (GtkEntry,	e[RecordingTime]) ||
	     !AGH_GBGETOBJ (GtkEntry,	e[Reserved]) ||
	     !AGH_GBGETOBJ (GtkLabel,   lChannelsNum) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelLabel]) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelPhysicalDim]) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelPhysicalMin]) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelPhysicalMax]) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelDigitalMin]) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelDigitalMax]) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelTransducerType]) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelFilteringInfo]) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelReserved]) ||
	     !AGH_GBGETOBJ (GtkEntry,   e[ChannelSamplesPerRecord]) ||
	     !AGH_GBGETOBJ (GtkButton,	bNext) ||
	     !AGH_GBGETOBJ (GtkButton,	bPrevious) ||
	     !AGH_GBGETOBJ (GtkButton,	bWrite) )
		return -1;

	gtk_builder_connect_signals( __builder, NULL);

	for ( int i = 0; i < _n_entries; ++i ) {
		g_signal_connect( gtk_entry_get_buffer( e[i]),
				  "deleted-text", (GCallback)deleted_text_cb,
				  (gpointer)i);
		g_signal_connect( gtk_entry_get_buffer( e[i]),
				  "inserted-text", (GCallback)inserted_text_cb,
				  (gpointer)i);
	}

	return 0;
}


void
ui_fini()
{
	// gtk_widget_destroy
	g_object_unref( __builder);
}


// EOF
