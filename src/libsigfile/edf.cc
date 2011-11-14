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
sigfile::CEDFFile::set_patient( const char* s)
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
	return set_recording_id( _session + '/' + _episode);
}

int
sigfile::CEDFFile::set_session( const char* s)
{
	_session.assign( s);
	return set_recording_id( _session + '/' + _episode);
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
      : af_dampen_window_type (SFFTParamSet::TWinType::welch),
	no_save_extra_files (false),
	_status (TStatus::ok)
{
	_filename = fname;
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
	int filedes = open( fname, O_RDWR);
	if ( filedes == -1 ) {
		_status |= TStatus::sysfail;
		throw invalid_argument (string ("Failed to open: ") + fname);
	}
	if ( (_mmapping = mmap( NULL,
				_fsize,
				PROT_READ | PROT_WRITE, MAP_SHARED,
				filedes,
				0)) == (void*)-1 ) {
		close( filedes);
		DEF_UNIQUE_CHARP(_);
		if ( asprintf( &_, "Failed to mmap %zu bytes", _fsize) ) ;
		throw length_error (_);
	}

      // parse header
	if ( _parse_header() ) {  // creates signals list
		string st = explain_edf_status(_status);
		fprintf( stderr, "CEDFFile(\"%s\"): errors found while parsing:\n%s\n",
			 fname, st.c_str());
		throw invalid_argument (
			string ("Failed to parse edf header of \"") + fname
			+ "\": " + st);
	}

	_data_offset = 256 + (signals.size() * 256);

	//fprintf( stderr, "CEDFFile(\"%s\"): added, with details:\n", fname);
	// fprintf( stderr, "%s\n", o.c_str());

      // artifacts, per signal
	for ( size_t h = 0; h < signals.size(); ++h ) {
		ifstream thomas (make_fname_artifacts( _filename, signals[h].channel));
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
		signals[h].af_dampen_window_type = (SFFTParamSet::TWinType)wt;
		signals[h].af_factor = fac;

		while ( !thomas.eof() ) {
			size_t aa = (size_t)-1, az = (size_t)-1;
			thomas >> aa >> az;
			if ( aa == (size_t)-1 || az == (size_t)-1 )
				break;
			signals[h].artifacts.emplace_back( aa, az);
		}
	}

      // annotations, per signal
	for ( auto &I : signals ) {
		ifstream fd (make_fname_annotations( _filename, I.channel));
		if ( not fd.good() )
			continue;
		size_t aa, az;
		string an;
		while ( true ) {
			fd >> aa >> az;
			getline( fd, an, EOA);
			if ( fd.good() && !fd.eof() ) {
				I.annotations.emplace_back(
					aa, az,
					strtrim(an),
					SSignal::SAnnotation::TOrigin::file);
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
					I.low_pass_cutoff = fl;
					I.low_pass_order  = ol;
					I.high_pass_cutoff = fh;
					I.high_pass_order  = oh;
					I.notch_filter = (SSignal::TNotchFilter)nf;
				}
			}
	}
}




sigfile::CEDFFile::CEDFFile( CEDFFile&& rv)
      : CHypnogram ((CEDFFile&&)rv)
{
	swap( _filename, rv._filename);

	header = rv.header;
	n_data_records   = rv.n_data_records;
	data_record_size = rv.data_record_size;

	start_time = rv.start_time;
	end_time   = rv.end_time;

	swap( _patient, rv.patient);
	swap( _episode, rv.episode);
	swap( _session, rv.session);

	swap( signals, rv.signals);

	_data_offset = rv._data_offset;
	_fsize       = rv._fsize;
	_fld_pos     = rv._fld_pos;
	_total_samples_per_record = rv._total_samples_per_record;
	_mmapping    = rv._mmapping;

	_status = rv._status;

	rv._mmapping = (void*)-1;  // will prevent munmap in ~CEDFFile()
}


sigfile::CEDFFile::~CEDFFile()
{
	if ( _mmapping != (void*)-1 ) {
		munmap( _mmapping, _fsize);

		if ( not no_save_extra_files )
			write_ancillary_files();
	}
}

void
sigfile::CEDFFile::write_ancillary_files() const
{
	CHypnogram::save( sigfile::make_fname_hypnogram( filename(), pagesize()));

	for ( auto &I : signals ) {
		if ( I.artifacts.size() ) {
			ofstream thomas (make_fname_artifacts( I.channel), ios_base::trunc);
			if ( thomas.good() ) {
				thomas << (unsigned short)I.af_dampen_window_type << ' ' << I.af_factor << endl;
				for ( auto &A : I.artifacts )
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
			thomas << I.low_pass_cutoff << ' ' << I.low_pass_order << ' '
			       << I.high_pass_cutoff << ' ' << I.high_pass_order << ' '
			       << (int)I.notch_filter << endl;
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

		patient = strtrim( string (header.patient_id, 80));

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
				episode.assign( fn_episode);    // use RecordingID_raw as Session
				session.assign( rec_id_isolated.c_str());
			} else {
				episode.assign( int_episode);
				session.assign( int_session);
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
			start_time = mktime( &ts);
			if ( start_time == (time_t)-1 )
				_status |= (date_unparsable|time_unparsable);
			else
				end_time = start_time + n_data_records * data_record_size;
		}

		if ( n_signals > max_signals ) {
			_status |= bad_numfld;
			return -2;
		} else {
			signals.resize( n_signals);

			for ( i = 0; i < n_signals; ++i )
				signals[i].channel = strtrim( string (_get_next_field( signals[i].header.label, 16), 16));
			        // to be parsed again wrt SignalType:Channel format

			for ( i = 0; i < n_signals; ++i )
				_get_next_field( signals[i].header.transducer_type, 80);

			for ( i = 0; i < n_signals; ++i )
				_get_next_field( signals[i].header.physical_dim, 8);

			for ( i = 0; i < n_signals; ++i ) {
				_get_next_field( signals[i].header.physical_min, 8);
				if ( sscanf( signals[i].header.physical_min, "%8g",
					     &signals[i].physical_min) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}
			for ( i = 0; i < n_signals; ++i ) {
				_get_next_field( signals[i].header.physical_max, 8);
				if ( sscanf( signals[i].header.physical_max, "%8g",
					     &signals[i].physical_max) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}

			for ( i = 0; i < n_signals; ++i ) {
				_get_next_field( signals[i].header.digital_min, 8);
				if ( sscanf( signals[i].header.digital_min, "%8d",
					     &signals[i].digital_min) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}
			for ( i = 0; i < n_signals; ++i ) {
				_get_next_field( signals[i].header.digital_max, 8);
				if ( sscanf( signals[i].header.digital_max, "%8d",
					     &signals[i].digital_max) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}

			for ( i = 0; i < n_signals; ++i )
				_get_next_field( signals[i].header.filtering_info, 80);

			for ( i = 0; i < n_signals; ++i ) {
				char *tail;
				signals[i].samples_per_record =
					strtoul( strtrim( string (_get_next_field( signals[i].header.samples_per_record, 8), 8)).c_str(),
						 &tail, 10);
				if ( *tail != '\0' ) {
					_status |= bad_numfld;
					return -2;
				}
			}

			for ( i = 0; i < n_signals; ++i )
				_get_next_field( signals[i].header.reserved, 32);
		}
	} catch (TStatus ex) {
		return -1;
	} catch (invalid_argument ex) {
		_status |= bad_numfld;
		return -3;
	}

      // calculate gain
	for ( i = 0; i < n_signals; ++i )
		if ( signals[i].physical_max <= signals[i].physical_min ||
		     signals[i].digital_max  <= signals[i].digital_min  ) {
			_status |= nogain;
			return -2;
		} else
			signals[i].scale =
				(signals[i].physical_max - signals[i].physical_min) /
				(signals[i].digital_max  - signals[i].digital_min );


      // determine signal type
	for ( i = 0; i < n_signals; ++i ) {
	      // try parsing as "type channel" first
		string parsable (signals[i].channel);
		char	*_1 = strtok( &parsable[0], " :,./"),
			*_2 = strtok( NULL, " :,./");
		if ( _2 ) {
			signals[i].signal_type = _1;
			signals[i].channel = _2;  // .channel overwritten
	      // it only has a channel name
		} else {
			const char* _signal_type = SChannel::signal_type_following_kemp( signals[i].signal_type);
			if ( _signal_type )
				signals[i].signal_type = _signal_type;
			else {
				signals[i].signal_type = "(unknown type)";
				_status |= nonkemp_signaltype;
			}

			if ( not signals[i].channel.follows_system1020() ) {  // in case there are duplicate labels, rewrite
				DEF_UNIQUE_CHARP (_);
				if ( asprintf( &_, "%zu:<%s>", i, signals[i].channel.c_str()) )
					;
				signals[i].channel = _;
				_status |= non1020_channel;
			}
		}
	}

      // convenience field
	_total_samples_per_record = 0;
	for ( i = 0; i < n_signals; ++i ) {
		signals[i]._at = _total_samples_per_record;
		_total_samples_per_record += signals[i].samples_per_record;
	}

      // are channels unique?
	for ( i = 0; i < n_signals; ++i )
		for ( size_t j = 0; j < n_signals; ++j )
			if ( j != i && signals[j].channel == signals[i].channel ) {
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
			       patient.c_str(),
			       session.c_str(), episode.c_str(),
			       strtrim( string (header.recording_id, 80)).c_str(),
			       asctime( localtime( &start_time)),
			       signals.size(),
			       n_data_records,
			       data_record_size) )
			;
		recv << outp;
		free( outp);

		for ( size_t i = 0; i < signals.size(); ++i ) {
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
				       i,
				       signals[i].signal_type.c_str(),
				       signals[i].channel.c_str(),
				       strtrim( string (signals[i].header.label, 16)).c_str(),
				       signals[i].transducer_type.c_str(),
				       signals[i].physical_dim.c_str(),
				       signals[i].physical_min,
				       signals[i].physical_max,
				       signals[i].digital_min,
				       signals[i].digital_max,
				       signals[i].filtering_info.c_str(),
				       signals[i].samples_per_record,
				       signals[i].scale,
				       signals[i].reserved.c_str()) )
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
