// ;-*-C++-*- *  Time-stamp: "2010-11-17 02:24:29 hmmr"

/*
 * Author: Andrei Zavada (johnhommer@gmail.com)
 *
 * License: GPL
 *
 * Initial version: 2010-04-28
 */


#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cerrno>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "misc.hh"
#include "edf.hh"

using namespace std;



string
CEDFFile::SSignal::SUnfazer::dirty_signature() const
{
	UNIQUE_CHARP(_);
	assert ( asprintf( &_, "%d:%g", h, fac) > 1 );
	return string(_);
}

size_t
CEDFFile::SSignal::dirty_signature() const
{
	string s (artifacts);
	for ( auto U = interferences.begin(); U != interferences.end(); ++U )
		s += U->dirty_signature();
	return HASHKEY(s);
}


CEDFFile::CEDFFile( const char *fname,
		    size_t scoring_pagesize,
		    TFFTWinType _af_dampen_window_type)
      : CHypnogram (scoring_pagesize, ::make_fname_hypnogram(fname, scoring_pagesize).c_str()),
	_status (0)
{
	UNIQUE_CHARP(cwd);
	cwd = getcwd(NULL, 0);
	_filename = fname;
	{
		struct stat stat0;
		int stst = stat( filename(), &stat0);
		if ( stst == -1 ) {
			UNIQUE_CHARP(_);
			if ( asprintf( &_, "No such file: \"%s/%s\"", fname, cwd) ) ;
			throw invalid_argument (_);
		}
		_fsize = stat0.st_size;
	}
	int filedes = open( fname, O_RDWR);
	if ( filedes == -1 ) {
		_status |= AGH_EDFCHK_SYSFAIL;
		throw invalid_argument (string ("Failed to open: ") + fname);
	}
	if ( (_mmapping = mmap( NULL,
				_fsize,
				PROT_READ /*|PROT_WRITE */, MAP_SHARED,
				filedes,
				0)) == (void*)-1 ) {
		close( filedes);
		UNIQUE_CHARP(_);
		if ( asprintf( &_, "Failed to mmap %zu bytes", _fsize) ) ;
		throw length_error (_);
	}

	if ( _parse_header() ) {  // creates signals list
		string st = explain_edf_status(_status);
		fprintf( stderr, "CEDFFile(\"%s\"): errors found while parsing:\n%s\n",
			 fname, st.c_str());
		UNIQUE_CHARP(_);
		if ( asprintf( &_, "Failed to parse edf header of \"%s/%s\"", fname, cwd) ) ;
		throw invalid_argument (_);
	}

	_data_offset = 256 + (NSignals * 256);

      // CHypnogram::
	size_t scorable_pages = NDataRecords*DataRecordSize / scoring_pagesize;  // with implicit floor
	if ( CHypnogram::length() != scorable_pages ) {
		if ( CHypnogram::length() > 0 )
			fprintf( stderr, "CEDFFile(\"%s\"): number of scorable pages @pagesize=%zu (%zu) differs from the number read from hypnogram file (%zu); discarding hypnogram\n",
				 fname, scoring_pagesize, scorable_pages, CHypnogram::length());
		CHypnogram::_pages.resize( scorable_pages);
	}

	//fprintf( stderr, "CEDFFile(\"%s\"): added, with details:\n", fname);
	// fprintf( stderr, "%s\n", o.c_str());

      // artifacts, per signal
	for ( size_t h = 0; h < NSignals; ++h ) {
		string &af = signals[h].artifacts;
		af.resize( length(), ' ');
		FILE *fd = fopen( make_fname_artifacts( signals[h].Channel.c_str()).c_str(), "r");
		if ( fd == NULL )
			continue;
		int v1 = -1;
		if ( fscanf( fd, "%d %g\n", &v1, &signals[h].af_factor) )
			;
		signals[h].af_dampen_window_type = (v1 < 0 || v1 > AGH_WT_WELCH) ? AGH_WT_WELCH : (TFFTWinType)v1;

		if ( fread( &af[0], af.size(), 1, fd) )
			;
		if ( af.find_first_not_of( " x") < af.size() ) {
			fprintf( stderr, "CEDFFile(\"%s\"): invalid characters in artifacts file for channel %s; discarding\n",
				 fname, signals[h].Channel.c_str());
			af.assign( af.size(), ' ');
		}
		fclose( fd);
	}

      // unfazers
	ifstream unff (make_fname_unfazer(fname).c_str());
	if ( !unff.fail() )
		while ( !unff.eof() ) {
			int a, o;
			double f;
			unff >> a >> o >> f;
			if ( unff.bad() || unff.eof() )
				break;
			if ( a >= 0 && a < (int)signals.size() && o >= 0 && o < (int)signals.size() &&
			     a != o )
				signals[a].interferences.emplace_back( o, f);
		}
}


CEDFFile::CEDFFile( CEDFFile&& rv)
      : CHypnogram (rv)
{
	swap( _filename, rv._filename);
	strcpy( VersionNumber_raw , rv.VersionNumber_raw);
	strcpy( PatientID_raw     , rv.PatientID_raw);
	strcpy( RecordingID_raw   , rv.RecordingID_raw);
	strcpy( RecordingDate_raw , rv.RecordingDate_raw);
	strcpy( RecordingTime_raw , rv.RecordingTime_raw);
	strcpy( HeaderLength_raw  , rv.HeaderLength_raw);
	strcpy( Reserved_raw      , rv.Reserved_raw);
	strcpy( NDataRecords_raw  , rv.NDataRecords_raw);
	strcpy( DataRecordSize_raw, rv.DataRecordSize_raw);
	strcpy( NSignals_raw      , rv.NSignals_raw);

	HeaderLength   = rv.HeaderLength;
	NDataRecords   = rv.NDataRecords;
	DataRecordSize = rv.DataRecordSize;
	NSignals       = rv.NSignals;

	timestamp_struct = rv.timestamp_struct;
	start_time = rv.start_time;
	end_time   = rv.end_time;

	swap( Episode, rv.Episode);
	swap( Session, rv.Session);

	swap( signals, rv.signals);

	_data_offset = rv._data_offset;
	_fsize       = rv._fsize;
	_fld_pos     = rv._fld_pos;
	_total_samples_per_record = rv._total_samples_per_record;
	_mmapping    = rv._mmapping;

	_status = rv._status;

	rv._mmapping = (void*)-1;  // will prevent munmap in ~CEDFFile()
}


CEDFFile::~CEDFFile()
{
	if ( _mmapping != (void*)-1 ) {
		munmap( _mmapping, _fsize);
		CHypnogram::save( ::make_fname_hypnogram( _filename.c_str(), pagesize()).c_str());

		for ( size_t h = 0; h < NSignals; ++h ) {
			string &af = signals[h].artifacts;
			FILE *fd = fopen( make_fname_artifacts( signals[h].Channel.c_str()).c_str(), "w");
			if ( fd != NULL ) {
				fprintf( fd, "%d %g\n%s",
					 signals[h].af_dampen_window_type, signals[h].af_factor,
					 af.c_str());
				fclose( fd);
			}
		}

		if ( have_unfazers() ) {
			ofstream unff (make_fname_unfazer( filename()), ios_base::trunc);
			for ( size_t h = 0; h < signals.size(); ++h )
				for ( auto u = signals[h].interferences.begin(); u != signals[h].interferences.end(); ++u )
					unff << h << "\t" << u->h << "\t"<< u->fac << endl;
		} else
			if ( unlink( make_fname_unfazer( filename()).c_str()) )
				;
	}
}





bool
CEDFFile::have_unfazers() const
{
	for ( size_t h = 0; h < signals.size(); ++h )
		if ( signals[h].interferences.size() > 0 )
			return true;
	return false;
}

string
make_fname_hypnogram( const char *_filename, size_t pagesize)
{
	string	fname_ (_filename);
	if ( fname_.size() > 4 && strcasecmp( &fname_[fname_.size()-4], ".edf") == 0 )
		fname_.erase( fname_.size()-4, 4);
	return fname_.append( string("-") + to_string( (long long unsigned)pagesize) + ".hypnogram");
}

string
make_fname_unfazer( const char *_filename)
{
	string	fname_ (_filename);
	if ( fname_.size() > 4 && strcasecmp( &fname_[fname_.size()-4], ".edf") == 0 )
		fname_.erase( fname_.size()-4, 4);
	return fname_.append( ".unf");
}

string
make_fname_artifacts( const char *_filename, const char *channel)
{
	string	fname_ (_filename);
	if ( fname_.size() > 4 && strcasecmp( &fname_[fname_.size()-4], ".edf") == 0 )
		fname_.erase( fname_.size()-4, 4);
	return ((fname_ += "-") += channel) += ".af";
}







#define AGH_KNOWN_CHANNELS_TOTAL 78
#define AGH_LAST_EEG 74
#define AGH_LAST_EOG 76
#define AGH_LAST_EMG 77
const char * const __agh_System1020_channels[] = {  // counted 'em all!
	"Nz",
	"Fp1", "Fpz", "Fp2",
	"AF7", "AF3", "AFz", "AF4", "AF8",
	"F9",  "F7", "F5", "F3", "F1", "Fz", "F2", "F4", "F6", "F8", "F10",
	"FT9", "FT7", "FC5", "FC3", "FC1", "FCz", "FC2", "FC4", "FC6", "FCT8", "FT10",
	"A1", "T9", "T7", "C5", "C3", "C1", "Cz", "C2", "C4", "C6", "T8", "T10", "A2",
	"TP9", "TP7", "CP5", "CP3", "CP1", "CPz", "CP2", "CP4", "CP6", "TP8", "TP10",
	"P9", "P7", "P5", "P3", "P1", "Pz", "P2", "P4", "P6", "P8", "P10",
	"PO7", "PO3", "POz", "PO4", "PO8",
	"O1", "Oz", "O2",
	"Iz",
	// plus channels of other signal types
	"Left", "Right",
	"Chin",
	NULL
};


#define AGH_KNOWN_SIGNAL_TYPES 16

const char * const __agh_SignalTypeByKemp[] = {
	"EEG", "EOG", "EMG", "ECG", "ERG",
	"NC",  "MEG", "MCG", "EP",
	"Temp", "Resp", "SaO2",
	"Light", "Sound", "Event", "Freq",
	NULL
};



const char*
signal_type_following_Kemp( const char *signal)
{
	size_t h = 0;
	for ( ; h <= AGH_LAST_EEG; ++h )
		if ( strcmp( signal, __agh_System1020_channels[h]) == 0 )
			return __agh_SignalTypeByKemp[0];  // we
	for ( ; h <= AGH_LAST_EOG; ++h )
		if ( strcmp( signal, __agh_System1020_channels[h]) == 0 )
			return __agh_SignalTypeByKemp[1];  // love
	for ( ; h <= AGH_LAST_EMG; ++h )
		if ( strcmp( signal, __agh_System1020_channels[h]) == 0 )
			return __agh_SignalTypeByKemp[2];  // plain C
	return NULL;
}

bool
channel_follows_1020( const char *channel)
{
	for ( size_t h = 0; h < AGH_KNOWN_CHANNELS_TOTAL; ++h )
		if ( !strcmp( channel, __agh_System1020_channels[h]) )
			return true;
	return false;
}




int
CEDFFile::_get_next_field( char* field, size_t fld_size)
{
	if ( _fld_pos + fld_size > _fsize )
		return -1;

	memset( (void*)field, '\0', fld_size+1);

	memcpy( field, (char*)_mmapping + _fld_pos, fld_size);
	_fld_pos += fld_size;

	size_t c = fld_size-1;
	while ( field[c] == ' '  && c )
		field[c--] = '\0';

	return 0;
}


int
CEDFFile::_parse_header()
{
	_fld_pos = 0;
	if ( _get_next_field( VersionNumber_raw,  8) ||
	     _get_next_field( PatientID_raw,     80) ||
	     _get_next_field( RecordingID_raw,   80) ||
	     _get_next_field( RecordingDate_raw,  8) ||
	     _get_next_field( RecordingTime_raw,  8) ||
	     _get_next_field( HeaderLength_raw,   8) ||
	     _get_next_field( Reserved_raw,      44) ||
	     _get_next_field( NDataRecords_raw,   8) ||
	     _get_next_field( DataRecordSize_raw, 8) ||
	     _get_next_field( NSignals_raw,       4) ) {
		_status |= AGH_EDFCHK_BAD_HEADER;
		return -2;
	}

	if ( strcmp( VersionNumber_raw, "0") ) {
		_status |= AGH_EDFCHK_BAD_VERSION;
		return -2;
	}

	HeaderLength = NDataRecords = DataRecordSize = NSignals = 0;
	sscanf( HeaderLength_raw,   "%8zu", &HeaderLength);
	sscanf( NDataRecords_raw,   "%8zu", &NDataRecords);
	sscanf( DataRecordSize_raw, "%8zu", &DataRecordSize);
	sscanf( NSignals_raw,       "%4zu", &NSignals);

	if ( !HeaderLength || !NDataRecords || !DataRecordSize || !NSignals ) {
		_status |= AGH_EDFCHK_BAD_NUMFLD;
		return -2;
	}

      // deal with episode and session
	// (a) parsed from RecordingID_raw
	char int_session[81], int_episode[81];
#define T "%80[-a-zA-Z0-9 _]"
	if ( sscanf( RecordingID_raw, T", "T,    int_episode, int_session) == 2 ||
	     sscanf( RecordingID_raw, T": "T,    int_session, int_episode) == 2 ||
	     sscanf( RecordingID_raw, T" / "T,   int_session, int_episode) == 2 ||
	     sscanf( RecordingID_raw, T" ("T")", int_session, int_episode) == 2 )
		;
	else
		_status |= (AGH_EDFCHK_NOSESSION | AGH_EDFCHK_NOEPISODE);
#undef T

	// (b) identified from file name
	string fn_episode;
	size_t basename_start = _filename.rfind( '/');
	fn_episode = _filename.substr( basename_start + 1,
				       _filename.size() - basename_start - strlen(".edf")-1);
	// chip away '-1' if present
	if ( fn_episode.size() >= strlen("a-1") ) {
		size_t sz = fn_episode.size();
		if ( fn_episode[sz-2] == '-' && isdigit(fn_episode[sz-1]) )
			fn_episode.erase( sz-2, 2);
	}

	if ( _status & AGH_EDFCHK_NOEPISODE ) { // (a) failed
		Episode.assign( fn_episode);    // use RecordingID_raw as Session
		Session.assign( RecordingID_raw);
	} else {
		Episode.assign( int_episode);
		Session.assign( int_session);
	}


	if ( sscanf( RecordingDate_raw, "%2d.%2d.%2d", &timestamp_struct.tm_mday,
						       &timestamp_struct.tm_mon,
						       &timestamp_struct.tm_year) != 3 )
		_status |= AGH_EDFCHK_DATE_UNPARSABLE;
	if ( sscanf( RecordingTime_raw, "%2d.%2d.%2d", &timestamp_struct.tm_hour,
						       &timestamp_struct.tm_min,
						       &timestamp_struct.tm_sec) != 3 )
		_status |= AGH_EDFCHK_TIME_UNPARSABLE;
	if ( !(_status & (AGH_EDFCHK_TIME_UNPARSABLE | AGH_EDFCHK_DATE_UNPARSABLE)) ) {
		timestamp_struct.tm_mon--;
		if ( timestamp_struct.tm_year < 50 )
			timestamp_struct.tm_year += 100;
		start_time = mktime( &timestamp_struct);
		end_time = start_time + NDataRecords * DataRecordSize;
	}

	size_t i;
	signals.resize( NSignals);

	for ( i = 0; i < NSignals; ++i )
		if ( _get_next_field( signals[i].Label, 16) ) {
			_status |= AGH_EDFCHK_BAD_HEADER;
			return -2;
		}

	for ( i = 0; i < NSignals; ++i )
		if ( _get_next_field( signals[i].TransducerType, 80) ) {
			_status |= AGH_EDFCHK_BAD_HEADER;
			return -2;
		}

	for ( i = 0; i < NSignals; ++i )
		if ( _get_next_field( signals[i].PhysicalDim, 8) ) {
			_status |= AGH_EDFCHK_BAD_HEADER;
			return -2;
		}

	for ( i = 0; i < NSignals; ++i ) {
		signals[i].PhysicalMin = 0;
		if ( _get_next_field( signals[i].PhysicalMin_raw, 8) ||
		     !sscanf( signals[i].PhysicalMin_raw, "%8g",
			      &signals[i].PhysicalMin) ) {
			_status |= AGH_EDFCHK_BAD_NUMFLD;
			return -2;
		}
	}
	for ( i = 0; i < NSignals; ++i ) {
		signals[i].PhysicalMax = 0;
		if ( _get_next_field( signals[i].PhysicalMax_raw, 8) ||
		     !sscanf( signals[i].PhysicalMax_raw, "%8g",
			      &signals[i].PhysicalMax) ) {
			_status |= AGH_EDFCHK_BAD_NUMFLD;
			return -2;
		}
	}
	for ( i = 0; i < NSignals; ++i ) {
		signals[i].DigitalMin = 0;
		if ( _get_next_field( signals[i].DigitalMin_raw, 8) ||
		     !sscanf( signals[i].DigitalMin_raw, "%8d",
			      &signals[i].DigitalMin) ) {
			_status |= AGH_EDFCHK_BAD_NUMFLD;
			return -2;
		}
	}
	for ( i = 0; i < NSignals; ++i ) {
		signals[i].DigitalMax = 0;
		if ( _get_next_field( signals[i].DigitalMax_raw, 8) ||
		     !sscanf( signals[i].DigitalMax_raw, "%8d",
			      &signals[i].DigitalMax) ) {
			_status |= AGH_EDFCHK_BAD_NUMFLD;
			return -2;
		}
	}

	for ( i = 0; i < NSignals; ++i )
		if ( _get_next_field( signals[i].FilteringInfo, 80) ) {
			_status |= AGH_EDFCHK_BAD_NUMFLD;
			return -2;
		}

	for ( i = 0; i < NSignals; ++i ) {
		signals[i].SamplesPerRecord = 0;
		if ( _get_next_field( signals[i].SamplesPerRecord_raw, 8) ||
		     !sscanf( signals[i].SamplesPerRecord_raw, "%8zu",
			      &signals[i].SamplesPerRecord) ) {
			_status |= AGH_EDFCHK_BAD_NUMFLD;
			return -2;
		}
	}

	for ( i = 0; i < NSignals; ++i )
		if ( _get_next_field( signals[i].Reserved, 32) )  {
			_status |= AGH_EDFCHK_BAD_HEADER;
			return -2;
		}

      // calculate gain
	for ( i = 0; i < NSignals; ++i )
		if ( signals[i].PhysicalMax <= signals[i].PhysicalMin ||
		     signals[i].DigitalMax  <= signals[i].DigitalMin  ) {
			_status |= AGH_EDFCHK_NOGAIN;
			return -2;
		} else
			signals[i].Scale =
				(signals[i].PhysicalMax - signals[i].PhysicalMin) /
				(signals[i].DigitalMax  - signals[i].DigitalMin );


      // determine signal type
	for ( i = 0; i < NSignals; ++i ) {
	      // try parsing as "type channel" first
		string parsable (signals[i].Label);
		char *_1 = strtok( &parsable[0], " :,./");
		if ( _1 ) {
			signals[i].SignalType = _1;
			signals[i].Channel.assign( strtok( NULL, " :,./"));
	      // it only has a channel name
		} else {
			const char* signal_type = signal_type_following_Kemp( signals[i].Label);
			if ( signal_type )
				signals[i].SignalType = signal_type;
			else {
				signals[i].SignalType = "(unknown type)";
				_status |= AGH_EDFCHK_NONKEMP_SIGNALTYPE;
			}

			if ( channel_follows_1020( signals[i].Label) )
				signals[i].Channel.assign( signals[i].Label);
			else {  // in case there are duplicate Labels
				UNIQUE_CHARP (_);
				if ( asprintf( &_, "%zu:<%s>", i, signals[i].Label) )
					;
				signals[i].Channel.assign( _);
				_status |= AGH_EDFCHK_NON1020_CHANNEL;
			}
		}
	}


      // convenience field
	_total_samples_per_record = 0;
	for ( i = 0; i < NSignals; ++i ) {
		signals[i]._at = _total_samples_per_record;
		_total_samples_per_record += signals[i].SamplesPerRecord;
	}

      // are channels unique?
	for ( size_t i = 0; i < NSignals; ++i )
		for ( size_t j = 0; j < NSignals; j++ )
			if ( j != i && signals[i].Label && signals[j].Label &&
			     signals[j].Label == signals[i].Label ) {
				_status |= AGH_EDFCHK_DUP_CHANNELS;
				break;
			}
	return 0;
}







int
CEDFFile::_put_next_field( char* field, size_t fld_size)
{
	if ( _fld_pos + fld_size > _fsize )
		return -1;

	memset( (void*)_fld_pos, '\0', fld_size);

	memcpy( (char*)_mmapping + _fld_pos, field, fld_size);
	_fld_pos += fld_size;

	return 0;
}

int
CEDFFile::write_header()
{
	_fld_pos = 0;
	_put_next_field( VersionNumber_raw,  8);
	_put_next_field( PatientID_raw,     80);
	_put_next_field( RecordingID_raw,   80);
	_put_next_field( RecordingDate_raw,  8);
	_put_next_field( RecordingTime_raw,  8);
	_put_next_field( HeaderLength_raw,   8);
	_put_next_field( Reserved_raw,      44);
	_put_next_field( NDataRecords_raw,   8);
	_put_next_field( DataRecordSize_raw, 8);
	_put_next_field( NSignals_raw,       4);
	return 0;
}

string
CEDFFile::details() const
{
	ostringstream recv;
	if ( _status & AGH_EDFCHK_BAD_HEADER )
		recv << "Bad header, or no file\n";
	else {
		char *outp;
		if ( asprintf( &outp,
			       "File\t: %s\n"
			       "PatientID\t: %s\n"
			       "Session\t: %s\n"
			       "Episode\t: %s\n"
			       "Timestamp\t: %s"
			       "# of signals\t: %lu\n"
			       "# of records\t: %lu\n"
			       "Record length\t: %lu sec\n",
			       filename(),
			       PatientID_raw,
			       Session.c_str(), Episode.c_str(),
			       asctime( &timestamp_struct),
			       NSignals,
			       NDataRecords,
			       DataRecordSize) )
			;
		recv << outp;
		free( outp);

		for ( size_t i = 0; i < NSignals; i++ ) {
			if ( asprintf( &outp,
				       "Signal %zu: Type: %s Channel: %s\n"
				       "  Transducer type\t: %s\n"
				       "  Physical dimension\t: %s\n"
				       "  Physical min\t: %g\n"
				       "  Physical max\t: %g\n"
				       "  Digital min\t: %d\n"
				       "  Digital max\t: %d\n"
				       "  Filtering info\t: %s\n"
				       "  Samples/rec\t: %zu\n"
				       "  Scale\t: %g\n"
				       "  (reserved)\t: %s\n",
				       i,
				       signals[i].SignalType.c_str(),
				       signals[i].Channel.size()? signals[i].Channel.c_str() :"(not specified)",
				       signals[i].TransducerType,
				       signals[i].PhysicalDim,
				       signals[i].PhysicalMin,
				       signals[i].PhysicalMax,
				       signals[i].DigitalMin,
				       signals[i].DigitalMax,
				       signals[i].FilteringInfo,
				       signals[i].SamplesPerRecord,
				       signals[i].Scale,
				       signals[i].Reserved) )
				;
			recv << outp;
			free( outp);
		}
	}

	return recv.str();
}







string
explain_edf_status( int status)
{
	ostringstream recv;
	if ( status & AGH_EDFCHK_BAD_HEADER )
		recv << "* Ill-formed header\n";
	if ( status & AGH_EDFCHK_BAD_VERSION )
		recv << "* Bad Version signature (i.e., not an EDF file)\n";
	if ( status & AGH_EDFCHK_BAD_NUMFLD )
		recv << "* Garbage in numerical fields\n";
	if ( status & AGH_EDFCHK_DATE_UNPARSABLE )
		recv << "* Date field ill-formed\n";
	if ( status & AGH_EDFCHK_TIME_UNPARSABLE )
		recv << "* Time field ill-formed\n";
	if ( status & AGH_EDFCHK_NOSESSION )
		recv << "* No session information in field RecordingID "
			"(expecting this to appear after "
			"episode designation followed by a comma)\n";
	if ( status & AGH_EDFCHK_NOCHANNEL )
		recv << "* Channel not specified (after SignalType)\n";
	if ( status & AGH_EDFCHK_NON1020_CHANNEL )
		recv << "* Channel designation not following 10-20 system\n";
	if ( status & AGH_EDFCHK_NONKEMP_SIGNALTYPE )
		recv << "* Signal type not listed in Kemp et al\n";
	if ( status & AGH_EDFCHK_DUP_CHANNELS )
		recv << "* Duplicate channel names\n";
	if ( status & AGH_EDFCHK_NOGAIN )
		recv << "* Physical or Digital Min not greater than Max\n";

	return recv.str();
}


// EOF
