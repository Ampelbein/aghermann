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
#include "../libsigfile/edf.hh"
#include "../libsigfile/source.hh"


void
pop_ok_message( GtkWindow *parent, const char *str, ...)
{
	va_list ap;
	va_start (ap, str);

	static GString *buf = NULL;
	if ( buf == NULL )
		buf = g_string_new("");

	g_string_vprintf( buf, str, ap);
	va_end (ap);

	GtkWidget *msg =
		gtk_message_dialog_new_with_markup(
			parent,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			buf->str, NULL);
	gtk_dialog_run( (GtkDialog*)msg);
	gtk_widget_destroy( msg);
}


#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( __builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)			\
	(A = (Type*)(gtk_builder_get_object( B, #A)))

//#include "../ui/globals.hh"



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



static void edf_data_to_widgets( const sigfile::CEDFFile&);
static void widgets_to_edf_data( sigfile::CEDFFile&);

sigfile::CEDFFile *Fp;


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

static void current_channel_data_to_widgets();
static void widgets_to_current_channel_data();
static void sensitize_channel_nav_buttons();

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
				"edfhed-gtk: Choose a file to edit",
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
		pop_ok_message( NULL, "UI failed to initialise\n");
		return 2;
	}

	try {
		auto F = sigfile::CEDFFile (fname,
					    sigfile::CSource::no_ancillary_files |
					    sigfile::CEDFFile::no_field_consistency_check);

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
		pop_ok_message( NULL, ex.what());
	}

	ui_fini();

	return 0;
}



static void
edf_data_to_widgets( const sigfile::CEDFFile& F)
{
	gtk_label_set_markup( lLabel, (string ("<b>File:</b> <i>") + F.filename() + "</i>").c_str());
	gtk_entry_set_text( e[PatientID],     agh::str::trim( string (F.header.patient_id,     80)) . c_str());
	gtk_entry_set_text( e[RecordingID],   agh::str::trim( string (F.header.recording_id,   80)) . c_str());
	gtk_entry_set_text( e[RecordingDate], agh::str::trim( string (F.header.recording_date,  8)) . c_str());
	gtk_entry_set_text( e[RecordingTime], agh::str::trim( string (F.header.recording_time,  8)) . c_str());
	gtk_entry_set_text( e[Reserved],      agh::str::trim( string (F.header.reserved,       44)) . c_str());

	for ( auto &h : F.channels ) {
		channels_tmp.emplace_back(
			agh::str::trim( string (h.header.label, 16)),
			agh::str::trim( string (h.header.physical_dim, 8)),
			agh::str::trim( string (h.header.physical_min, 8)),
			agh::str::trim( string (h.header.physical_max, 8)),
			agh::str::trim( string (h.header.digital_min,  8)),
			agh::str::trim( string (h.header.digital_max,  8)),
			agh::str::trim( string (h.header.transducer_type, 80)),
			agh::str::trim( string (h.header.filtering_info, 80)),
			agh::str::trim( string (h.header.samples_per_record, 8)),
			agh::str::trim( string (h.header.reserved, 32)));
	}
}



static void
widgets_to_edf_data( sigfile::CEDFFile& F)
{
	memcpy( F.header.patient_id,     agh::str::pad( gtk_entry_get_text( e[PatientID]),     80).c_str(), 80);
	memcpy( F.header.recording_id,   agh::str::pad( gtk_entry_get_text( e[RecordingID]),   80).c_str(), 80);
	memcpy( F.header.recording_date, agh::str::pad( gtk_entry_get_text( e[RecordingDate]),  8).c_str(),  8);
	memcpy( F.header.recording_time, agh::str::pad( gtk_entry_get_text( e[RecordingTime]),  8).c_str(),  8);
	memcpy( F.header.reserved,       agh::str::pad( gtk_entry_get_text( e[Reserved]),      44).c_str(), 44);

	auto H = channels_tmp.begin();
	for ( auto& h : F.channels ) {
		memcpy( h.header.label,			agh::str::pad( H->Label,           16).c_str(), 16);
		memcpy( h.header.physical_dim,		agh::str::pad( H->PhysicalDim,      8).c_str(),  8);
		memcpy( h.header.physical_min,		agh::str::pad( H->PhysicalMin,      8).c_str(),  8);
		memcpy( h.header.physical_max,		agh::str::pad( H->PhysicalMax,      8).c_str(),  8);
		memcpy( h.header.digital_min,		agh::str::pad( H->DigitalMin,       8).c_str(),  8);
		memcpy( h.header.digital_max,		agh::str::pad( H->DigitalMax,       8).c_str(),  8);
		memcpy( h.header.transducer_type,	agh::str::pad( H->TransducerType,  80).c_str(), 80);
		memcpy( h.header.filtering_info,	agh::str::pad( H->FilteringInfo,   80).c_str(), 80);
		memcpy( h.header.samples_per_record,	agh::str::pad( H->SamplesPerRecord, 8).c_str(),  8);
		memcpy( h.header.reserved,		agh::str::pad( H->Reserved,        32).c_str(), 32);
		++H;
	}
}





static void
current_channel_data_to_widgets()
{
	GString *tmp = g_string_new("");
	size_t i = 0;
	for ( auto& H : channels_tmp ) {
		gchar *escaped = g_markup_escape_text( agh::str::trim( H.Label).c_str(), -1);
		if ( i++ == channel_no )
			g_string_append_printf( tmp, "  <b>%s</b>  ", escaped);
		else
			g_string_append_printf( tmp, "  %s  ", escaped);
		g_free( escaped);
	}
	gtk_label_set_markup( lChannelsNum, tmp->str);
	g_string_free( tmp, TRUE);
	gtk_entry_set_text( e[ChannelLabel],		agh::str::trim( HTmpi->Label      ) . c_str());
	gtk_entry_set_text( e[ChannelPhysicalDim],	agh::str::trim( HTmpi->PhysicalDim) . c_str());
	gtk_entry_set_text( e[ChannelPhysicalMin],	agh::str::trim( HTmpi->PhysicalMin) . c_str());
	gtk_entry_set_text( e[ChannelPhysicalMax],	agh::str::trim( HTmpi->PhysicalMax) . c_str());
	gtk_entry_set_text( e[ChannelDigitalMin],	agh::str::trim( HTmpi->DigitalMin ) . c_str());
	gtk_entry_set_text( e[ChannelDigitalMax],	agh::str::trim( HTmpi->DigitalMax ) . c_str());
	gtk_entry_set_text( e[ChannelTransducerType],	agh::str::trim( HTmpi->TransducerType) . c_str());
	gtk_entry_set_text( e[ChannelFilteringInfo],	agh::str::trim( HTmpi->FilteringInfo)  . c_str());
	gtk_entry_set_text( e[ChannelSamplesPerRecord],	agh::str::trim( HTmpi->SamplesPerRecord) . c_str());
	gtk_entry_set_text( e[ChannelReserved],		agh::str::trim( HTmpi->Reserved)         . c_str());
}

static void
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


static bool
validate_all_widgets()
{
	const char *str, *p;
	struct tm ts;
	str = gtk_entry_get_text( e[RecordingDate]);
	p = strptime( str, "%d.%m.%y", &ts);
	if ( strlen(str) != 8 || p == NULL || *p != '\0' )
		return false;
	str = gtk_entry_get_text( e[RecordingTime]);
	p = strptime( str, "%H.%M.%S", &ts);
	if ( strlen(str) != 8 || p == NULL || *p != '\0' )
		return false;

	char *tail;
	double p_min, p_max;
	int d_min, d_max;
	if ( (p_min = strtod( gtk_entry_get_text( e[ChannelPhysicalMin]), &tail), *tail != '\0') )
		return false;
	if ( (p_max = strtod( gtk_entry_get_text( e[ChannelPhysicalMax]), &tail), *tail != '\0') )
		return false;
	if ( (d_min = strtoul( gtk_entry_get_text( e[ChannelDigitalMin]), &tail, 10), *tail != '\0') )
		return false;
	if ( (d_max = strtoul( gtk_entry_get_text( e[ChannelDigitalMax]), &tail, 10), *tail != '\0') )
		return false;
	if ( p_min >= p_max || d_min >= d_max )
		return false;

	return true;
}



extern "C" void
bNext_clicked_cb( GtkButton*, gpointer)
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
bPrevious_clicked_cb( GtkButton*, gpointer)
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
inserted_text_cb( GtkEntryBuffer *,
		  guint           ,
		  gchar          *,
		  guint           ,
		  gpointer        )
{
	gtk_widget_set_sensitive( (GtkWidget*)bWrite, validate_all_widgets());
}
extern "C" void
deleted_text_cb( GtkEntryBuffer *,
		 guint           ,
		 guint           ,
		 gpointer        )
{
	gtk_widget_set_sensitive( (GtkWidget*)bWrite, validate_all_widgets());
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
		pop_ok_message( NULL, "Failed to load UI description file.");
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
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
		g_signal_connect( gtk_entry_get_buffer( e[i]),
				  "deleted-text", (GCallback)deleted_text_cb,
				  (gpointer)i);
		g_signal_connect( gtk_entry_get_buffer( e[i]),
				  "inserted-text", (GCallback)inserted_text_cb,
				  (gpointer)i);
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
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
