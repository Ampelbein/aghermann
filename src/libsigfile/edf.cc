// ;-*-C++-*-
/*
 *       File name:  libsigfile/edf.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  EDF class methods
 *
 *         License:  GPL
 */


#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cerrno>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iterator>

#include "../misc.hh"
#include "edf.hh"
#include "source.hh"

using namespace std;


int
sigfile::CEDFFile::set_subject( const char* s)
{
	memcpy( header.patient_id, strpad( s, 80).c_str(), 80);
	return strlen(s) > 80;
}

int
sigfile::CEDFFile::set_recording_id( const char* s)
{
	memcpy( header.recording_id, strpad( s, 80).c_str(), 80);
	return strlen(s) > 80;
}

int
sigfile::CEDFFile::set_episode( const char* s)
{
	_episode.assign( s);
	return set_recording_id( (_session + '/' + _episode).c_str());
}

int
sigfile::CEDFFile::set_session( const char* s)
{
	_session.assign( s);
	return set_recording_id( (_session + '/' + _episode).c_str());
}

int
sigfile::CEDFFile::set_comment( const char *s)
{
	memcpy( header.reserved, strpad( s, 44).c_str(), 44);
	return strlen(s) > 44;
}

int
sigfile::CEDFFile::set_start_time( time_t s)
{
	char b[9];
	// set start
	strftime( b, 8, "%d.%m.%y", localtime(&s));
	memcpy( header.recording_date, b, 8);
	strftime( b, 8, "%H.%M.%s", localtime(&s));
	memcpy( header.recording_time, b, 8);

	return 0;
}











#define EOA '$'


sigfile::CEDFFile::CEDFFile( const char *fname)
      : CSource_base (fname)
{
	{
		struct stat stat0;
		int stst = stat( filename(), &stat0);
		if ( stst == -1 ) {
			DEF_UNIQUE_CHARP(_);
			if ( asprintf( &_, "No such file: \"%s\"", fname) ) ;
			throw invalid_argument (_);
		}
		_fsize = stat0.st_size;
	}
      // mmap
	_fd = open( fname, O_RDWR);
	if ( _fd == -1 ) {
		_status |= TStatus::sysfail;
		throw invalid_argument (string ("Failed to open: ") + fname);
	}
	if ( (_mmapping = mmap( NULL,
				_fsize,
				PROT_READ | PROT_WRITE, MAP_SHARED,
				_fd,
				0)) == (void*)-1 ) {
		close( _fd);
		DEF_UNIQUE_CHARP(_);
		if ( asprintf( &_, "Failed to mmap %zu bytes", _fsize) ) ;
		throw length_error (_);
	}

      // parse header
	if ( _parse_header() ) {  // creates signals list
		string st = explain_edf_status(_status);
		fprintf( stderr, "CEDFFile(\"%s\"): errors found while parsing:\n%s\n",
			 fname, st.c_str());
		close( _fd);
		throw invalid_argument (
			string ("Failed to parse edf header of \"") + fname
			+ "\": " + st);
	}
	// signals now available

	_data_offset = 256 + (signals.size() * 256);

	//fprintf( stderr, "CEDFFile(\"%s\"): added, with details:\n", fname);
	// fprintf( stderr, "%s\n", o.c_str());

      // artifacts, per signal
	for ( auto &H : signals ) {
		ifstream thomas (make_fname_artifacts( H.channel));
		if ( not thomas.good() )
			continue;
		int wt = -1;
		float fac = 0.;
		thomas >> wt >> fac;
		if ( thomas.eof()
		     || wt < 0 || wt > (int)SFFTParamSet::TWinType::welch
		     || fac == 0. ) {
			continue;
		}
		H.artifacts.dampen_window_type = (SFFTParamSet::TWinType)wt;
		H.artifacts.factor = fac;

		while ( !thomas.eof() ) {
			size_t aa = (size_t)-1, az = (size_t)-1;
			thomas >> aa >> az;
			if ( aa == (size_t)-1 || az == (size_t)-1 )
				break;
			H.artifacts.mark_artifact( aa, az);
		}
	}

      // annotations, per signal
	for ( auto &H : signals ) {
		ifstream fd (make_fname_annotations( H.channel));
		if ( not fd.good() )
			continue;
		size_t aa, az;
		string an;
		while ( true ) {
			fd >> aa >> az;
			getline( fd, an, EOA);
			if ( fd.good() && !fd.eof() ) {
				H.annotations.emplace_back(
					aa, az,
					strtrim(an));
			} else
				break;
		}
	}

      // filters
	{
		ifstream thomas (make_fname_filters(fname));
		if ( !thomas.fail() )
			for ( auto &I : signals ) {
				int ol = -1, oh = -1, nf = -1;
				float fl = 0., fh = 0.;
				thomas >> fl >> ol
				       >> fh >> oh >> nf;
				if ( ol > 0 && oh > 0 && ol < 5 && oh < 5
				     && fl >= 0. && fh >= 0.
				     && nf >= 0 && nf <= 2 ) {
					I.filters.low_pass_cutoff = fl;
					I.filters.low_pass_order  = ol;
					I.filters.high_pass_cutoff = fh;
					I.filters.high_pass_order  = oh;
					I.filters.notch_filter = (SFilterPack::TNotchFilter)nf;
				}
			}
	}
}




sigfile::CEDFFile::CEDFFile( CEDFFile&& rv)
      : CSource_base ((CSource_base&&)rv)
{
	header = rv.header;
	n_data_records   = rv.n_data_records;
	data_record_size = rv.data_record_size;

	_start_time = rv._start_time;
	_end_time   = rv._end_time;

	swap( _patient, rv._patient);
	swap( _episode, rv._episode);
	swap( _session, rv._session);

	swap( signals, rv.signals);

	_data_offset = rv._data_offset;
	_fsize       = rv._fsize;
	_fld_pos     = rv._fld_pos;
	_total_samples_per_record =
		       rv._total_samples_per_record;
	_mmapping    = rv._mmapping;
	_fd          = rv._fd;

	rv._mmapping = (void*)-1;  // will prevent munmap in ~CEDFFile()
}


sigfile::CEDFFile::~CEDFFile()
{
	if ( _mmapping != (void*)-1 ) {
		munmap( _mmapping, _fsize);
		close( _fd);

		if ( not no_save_extra_files )
			write_ancillary_files();
	}
}



void
sigfile::CEDFFile::write_ancillary_files()
{
	for ( auto &I : signals ) {
		if ( I.artifacts().size() ) {
			ofstream thomas (make_fname_artifacts( I.channel), ios_base::trunc);
			if ( thomas.good() ) {
				thomas << (unsigned short)I.artifacts.dampen_window_type << ' ' << I.artifacts.factor << endl;
				for ( auto &A : I.artifacts() )
					thomas << A.first << ' ' << A.second << endl;
			}
		} else
			if ( unlink( make_fname_artifacts( I.channel).c_str()) )
				;

		if ( I.annotations.size() ) {
			ofstream thomas (make_fname_annotations( I.channel), ios_base::trunc);
			for ( auto &A : I.annotations )
				thomas << A.span.first << ' ' << A.span.second << ' ' << A.label << EOA << endl;
		} else
			if ( unlink( make_fname_annotations( I.channel).c_str()) )
				;
	}
	ofstream thomas (make_fname_filters( filename()), ios_base::trunc);
	if ( thomas.good() )
		for ( auto &I : signals )
			thomas << I.filters.low_pass_cutoff << ' ' << I.filters.low_pass_order << ' '
			       << I.filters.high_pass_cutoff << ' ' << I.filters.high_pass_order << ' '
			       << (int)I.filters.notch_filter << endl;
}












char*
sigfile::CEDFFile::_get_next_field( char *&field, size_t fld_size) throw (TStatus)
{
	if ( _fld_pos + fld_size > _fsize ) {
		_status |= bad_header;
		throw bad_header;
	}

	field = (char*)_mmapping + _fld_pos;
	_fld_pos += fld_size;

	return field;
}

size_t
	sigfile::CEDFFile::max_signals = 128;

int
sigfile::CEDFFile::_parse_header()
{
	size_t	n_signals,
		i;
	try {
		_fld_pos = 0;
		_get_next_field( header.version_number,   8);
		_get_next_field( header.patient_id,      80);
		_get_next_field( header.recording_id,    80);
		_get_next_field( header.recording_date,   8);
		_get_next_field( header.recording_time,   8);
		_get_next_field( header.header_length,    8);
		_get_next_field( header.reserved,        44);
		_get_next_field( header.n_data_records,   8);
		_get_next_field( header.data_record_size, 8);
		_get_next_field( header.n_signals,        4);

		if ( strncmp( header.version_number, "0       ", 8) ) {
			_status |= bad_version;
			return -2;
		}

		size_t	header_length;

		header_length = n_data_records = data_record_size = n_signals = 0;
		sscanf( header.header_length,    "%8zu", &header_length);
		sscanf( header.n_data_records,   "%8zu", &n_data_records);
		sscanf( header.data_record_size, "%8zu", &data_record_size);
		sscanf( header.n_signals,        "%4zu", &n_signals);

		if ( !header_length || !n_data_records || !data_record_size || !n_signals ) {
			_status |= bad_numfld;
			return -2;
		}

		_patient = strtrim( string (header.patient_id, 80));

	      // deal with episode and session
		{
		      // (a) parsed from RecordingID_raw
			char int_session[81], int_episode[81];
			string rec_id_isolated (strtrim( string (header.recording_id, 80)));
#define T "%80[-a-zA-Z0-9 _]"
			if ( sscanf( rec_id_isolated.c_str(), T", "T,    int_episode, int_session) == 2 ||
			     sscanf( rec_id_isolated.c_str(), T": "T,    int_session, int_episode) == 2 ||
			     sscanf( rec_id_isolated.c_str(), T"/"T,     int_session, int_episode) == 2 ||
			     sscanf( rec_id_isolated.c_str(), T" ("T")", int_session, int_episode) == 2 )
				;
			else
				_status |= (nosession | noepisode);
#undef T
		      // (b) identified from file name
			string fn_episode;
			size_t basename_start = _filename.rfind( '/');
			fn_episode =
				_filename.substr(
					basename_start + 1,
					_filename.size() - basename_start - 4 /* strlen(".edf") */ - 1);
			// chip away '-1' if present
			if ( fn_episode.size() >= 3 /* strlen("a-1") */ ) {
				size_t sz = fn_episode.size();
				if ( fn_episode[sz-2] == '-' && isdigit(fn_episode[sz-1]) )
					fn_episode.erase( sz-2, 2);
			}

			if ( _status & noepisode ) { // (a) failed
				_episode.assign( fn_episode);    // use RecordingID_raw as Session
				_session.assign( rec_id_isolated.c_str());
			} else {
				_episode.assign( int_episode);
				_session.assign( int_session);
			}
		}

		{
			struct tm ts;
			char *p;
			//memset( &ts, 0, sizeof(struct tm));
			ts.tm_isdst = 0;  // importantly
			p = strptime( string (header.recording_date, 8).c_str(), "%d.%m.%y", &ts);
			if ( p == NULL || *p != '\0' ) {
				_status |= date_unparsable;
				return -2;
			}
			p = strptime( string (header.recording_time, 8).c_str(), "%H.%M.%S", &ts);
			if ( p == NULL || *p != '\0' ) {
				_status |= time_unparsable;
				return -2;
			}

			// if ( ts.tm_year < 50 )
			// 	ts.tm_year += 100;
			_start_time = mktime( &ts);
			if ( _start_time == (time_t)-1 )
				_status |= (date_unparsable|time_unparsable);
			else
				_end_time = _start_time + n_data_records * data_record_size;
		}

		if ( n_signals > max_signals ) {
			_status |= bad_numfld;
			return -2;
		} else {
			signals.resize( n_signals);

			for ( auto &H : signals )
				H.channel.assign( strtrim( string (_get_next_field( H.header.label, 16), 16)));
			        // to be parsed again wrt SignalType:Channel format

			for ( auto &H : signals )
				_get_next_field( H.header.transducer_type, 80);

			for ( auto &H : signals )
				_get_next_field( H.header.physical_dim, 8);

			for ( auto &H : signals ) {
				_get_next_field( H.header.physical_min, 8);
				if ( sscanf( H.header.physical_min, "%8g",
					     &H.physical_min) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}
			for ( auto &H : signals ) {
				_get_next_field( H.header.physical_max, 8);
				if ( sscanf( H.header.physical_max, "%8g",
					     &H.physical_max) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}

			for ( auto &H : signals ) {
				_get_next_field( H.header.digital_min, 8);
				if ( sscanf( H.header.digital_min, "%8d",
					     &H.digital_min) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}
			for ( auto &H : signals ) {
				_get_next_field( H.header.digital_max, 8);
				if ( sscanf( H.header.digital_max, "%8d",
					     &H.digital_max) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}

			for ( auto &H : signals )
				_get_next_field( H.header.filtering_info, 80);

			for ( auto &H : signals ) {
				char *tail;
				H.samples_per_record =
					strtoul( strtrim( string (_get_next_field( H.header.samples_per_record, 8), 8)).c_str(),
						 &tail, 10);
				if ( *tail != '\0' ) {
					_status |= bad_numfld;
					return -2;
				}
			}

			for ( auto &H : signals )
				_get_next_field( H.header.reserved, 32);
		}
	} catch (TStatus ex) {
		return -1;
	} catch (invalid_argument ex) {
		_status |= bad_numfld;
		return -3;
	}

      // calculate gain
	for ( auto &H : signals )
		if ( H.physical_max <= H.physical_min ||
		     H.digital_max  <= H.digital_min  ) {
			_status |= nogain;
			return -2;
		} else
			H.scale =
				(H.physical_max - H.physical_min) /
				(H.digital_max  - H.digital_min );


      // determine signal type
	for ( auto &H : signals ) {
	      // try parsing as "type channel" first
		string parsable (H.channel);
		char	*_1 = strtok( &parsable[0], " :,./"),
			*_2 = strtok( NULL, " :,./");
		if ( _2 ) {
			H.signal_type_s = _1;
			H.signal_type = SChannel::figure_signal_type(_1);
			H.channel.assign( _2);  // .channel overwritten
	      // it only has a channel name
		} else {
			H.signal_type_s = SChannel::kemp_signal_types[
				H.signal_type = SChannel::signal_type_of_channel( H.channel) ];

			if ( not H.channel.follows_system1020() ) {  // in case there are duplicate labels, rewrite
				DEF_UNIQUE_CHARP (_);
				if ( asprintf( &_, "%zu:<%s>", i, H.channel.c_str()) )
					;
				H.channel.assign( _);
				_status |= non1020_channel;
			}
		}
		if ( H.signal_type == SChannel::TType::other )
			_status |= nonkemp_signaltype;
	}

      // convenience field
	_total_samples_per_record = 0;
	for ( auto &H : signals ) {
		H._at = _total_samples_per_record;
		_total_samples_per_record += H.samples_per_record;
	}

      // are channels unique?
	for ( auto &H : signals )
		for ( auto &J : signals )
			if ( &J != &H && J.channel == H.channel ) {
				_status |= dup_channels;
				break;
			}
	return 0;
}







// int
// sigfile::CEDFFile::_put_next_field( char* field, size_t fld_size)
// {
// 	if ( _fld_pos + fld_size > _fsize )
// 		return -1;

// 	memset( (void*)_fld_pos, '\0', fld_size);

// 	memcpy( (char*)_mmapping + _fld_pos, field, fld_size);
// 	_fld_pos += fld_size;

// 	return 0;
// }


string
sigfile::CEDFFile::details() const
{
	ostringstream recv;
	if ( _status & bad_header )
		recv << "Bad header, or no file\n";
	else {
		char *outp;
		if ( asprintf( &outp,
			       "File\t: %s\n"
			       "PatientID\t: %s\n"
			       "Session\t: %s\n"
			       "Episode\t: %s\n"
			       "(RecordingID: \"%s\")\n"
			       "Timestamp\t: %s"
			       "# of signals\t: %zu\n"
			       "# of records\t: %zu\n"
			       "Record length\t: %zu sec\n",
			       filename(),
			       subject(),
			       session(), episode(),
			       strtrim( string (header.recording_id, 80)).c_str(),
			       asctime( localtime( &_start_time)),
			       signals.size(),
			       n_data_records,
			       data_record_size) )
			;
		recv << outp;
		free( outp);

		size_t i = 0;
		for ( auto &H : signals ) {
			if ( asprintf( &outp,
				       "Signal %zu: Type: %s Channel: %s\n"
				       " (label: \"%s\")\n"
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
				       ++i,
				       H.signal_type_s.c_str(),
				       H.channel.c_str(),
				       strtrim( string (H.header.label, 16)).c_str(),
				       H.transducer_type.c_str(),
				       H.physical_dim.c_str(),
				       H.physical_min,
				       H.physical_max,
				       H.digital_min,
				       H.digital_max,
				       H.filtering_info.c_str(),
				       H.samples_per_record,
				       H.scale,
				       H.reserved.c_str()) )
				;
			recv << outp;
			free( outp);
		}
	}

	return recv.str();
}







string
sigfile::CEDFFile::explain_edf_status( int status)
{
	ostringstream recv;
	if ( status & bad_header )
		recv << "* Ill-formed header\n";
	if ( status & bad_version )
		recv << "* Bad Version signature (i.e., not an EDF file)\n";
	if ( status & bad_numfld )
		recv << "* Garbage in numerical fields\n";
	if ( status & date_unparsable )
		recv << "* Date field ill-formed\n";
	if ( status & time_unparsable )
		recv << "* Time field ill-formed\n";
	if ( status & nosession )
		recv << "* No session information in field RecordingID "
			"(expecting this to appear after "
			"episode designation followed by a comma)\n";
	if ( status & non1020_channel )
		recv << "* Channel designation not following the 10-20 system\n";
	if ( status & nonkemp_signaltype )
		recv << "* Signal type not listed in Kemp et al\n";
	if ( status & dup_channels )
		recv << "* Duplicate channel names\n";
	if ( status & nogain )
		recv << "* Physical or Digital Min value greater than Max\n";
	if ( status & too_many_signals )
		recv << "* Number of signals grearter than " << max_signals << "\n";

	return recv.str();
}






// eof
