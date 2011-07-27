// ;-*-C++-*- *  Time-stamp: "2011-07-27 03:01:41 hmmr"
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




GtkBuilder
	*__builder;

GtkDialog
	*wMain;
GtkLabel
	*lLabel,
	*lChannelsNum;
GtkEntry
	*ePatientID, *eRecordingID,
	*eRecordingDate, *eRecordingTime, *eReserved,
	*eChannelLabel, *eChannelPhysicalDim,
	*eChannelPhysicalMin, *eChannelPhysicalMax,
	*eChannelDigitalMin, *eChannelDigitalMax,
	*eChannelTransducerType, *eChannelFilteringInfo,
	*eChannelReserved,
	*eChannelSamplesPerRecord;
GtkButton
	*bNext, *bPrevious;


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
	gtk_entry_set_text( ePatientID,     strtrim( string (F.header.patient_id,     80)) . c_str());
	gtk_entry_set_text( eRecordingID,   strtrim( string (F.header.recording_id,   80)) . c_str());
	gtk_entry_set_text( eRecordingDate, strtrim( string (F.header.recording_date,  8)) . c_str());
	gtk_entry_set_text( eRecordingTime, strtrim( string (F.header.recording_time,  8)) . c_str());
	gtk_entry_set_text( eReserved,      strtrim( string (F.header.reserved,       44)) . c_str());

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
	memcpy( F.header.patient_id,     strpad( gtk_entry_get_text( ePatientID),     80).c_str(), 80);
	memcpy( F.header.recording_id,   strpad( gtk_entry_get_text( eRecordingID),   80).c_str(), 80);
	memcpy( F.header.recording_date, strpad( gtk_entry_get_text( eRecordingDate),  8).c_str(),  8);
	memcpy( F.header.recording_time, strpad( gtk_entry_get_text( eRecordingTime),  8).c_str(),  8);
	memcpy( F.header.reserved,       strpad( gtk_entry_get_text( eReserved),      44).c_str(), 44);

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
	gtk_entry_set_text( eChannelLabel,		strtrim( HTmpi->Label      ) . c_str());
	gtk_entry_set_text( eChannelPhysicalDim,	strtrim( HTmpi->PhysicalDim) . c_str());
	gtk_entry_set_text( eChannelPhysicalMin,	strtrim( HTmpi->PhysicalMin) . c_str());
	gtk_entry_set_text( eChannelPhysicalMax,	strtrim( HTmpi->PhysicalMax) . c_str());
	gtk_entry_set_text( eChannelDigitalMin,		strtrim( HTmpi->DigitalMin ) . c_str());
	gtk_entry_set_text( eChannelDigitalMax,		strtrim( HTmpi->DigitalMax ) . c_str());
	gtk_entry_set_text( eChannelTransducerType,	strtrim( HTmpi->TransducerType) . c_str());
	gtk_entry_set_text( eChannelFilteringInfo,	strtrim( HTmpi->FilteringInfo)  . c_str());
	gtk_entry_set_text( eChannelSamplesPerRecord,	strtrim( HTmpi->SamplesPerRecord) . c_str());
	gtk_entry_set_text( eChannelReserved,		strtrim( HTmpi->Reserved)         . c_str());
}

void
widgets_to_current_channel_data()
{
	HTmpi->Label		= gtk_entry_get_text( eChannelLabel);
	HTmpi->PhysicalDim	= gtk_entry_get_text( eChannelPhysicalDim);
	HTmpi->PhysicalMin	= gtk_entry_get_text( eChannelPhysicalMin);
	HTmpi->PhysicalMax	= gtk_entry_get_text( eChannelPhysicalMax);
	HTmpi->DigitalMin	= gtk_entry_get_text( eChannelDigitalMin);
	HTmpi->DigitalMax	= gtk_entry_get_text( eChannelDigitalMax);
	HTmpi->TransducerType	= gtk_entry_get_text( eChannelTransducerType);
	HTmpi->FilteringInfo	= gtk_entry_get_text( eChannelFilteringInfo);
	HTmpi->SamplesPerRecord	= gtk_entry_get_text( eChannelSamplesPerRecord);
	HTmpi->Reserved		= gtk_entry_get_text( eChannelReserved);
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
	     !AGH_GBGETOBJ (GtkEntry,	ePatientID) ||
	     !AGH_GBGETOBJ (GtkEntry,	eRecordingID) ||
	     !AGH_GBGETOBJ (GtkEntry,	eRecordingDate) ||
	     !AGH_GBGETOBJ (GtkEntry,	eRecordingTime) ||
	     !AGH_GBGETOBJ (GtkEntry,	eReserved) ||
	     !AGH_GBGETOBJ (GtkLabel,   lChannelsNum) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelLabel) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelPhysicalDim) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelPhysicalMin) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelPhysicalMax) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelDigitalMin) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelDigitalMax) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelTransducerType) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelFilteringInfo) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelReserved) ||
	     !AGH_GBGETOBJ (GtkEntry,   eChannelSamplesPerRecord) ||
	     !AGH_GBGETOBJ (GtkButton,	bNext) ||
	     !AGH_GBGETOBJ (GtkButton,	bPrevious) )
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
