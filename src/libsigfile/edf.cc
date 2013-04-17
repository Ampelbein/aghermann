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
#include <unistd.h>
#include <fcntl.h>
#include <cinttypes>
#include <cerrno>
#include <memory>
#include <fstream>
#include <sstream>
#include <list>
#include <stdexcept>
#include <iterator>

#include "common/lang.hh"
#include "common/string.hh"
#include "edf.hh"
#include "source.hh"

using namespace std;

using agh::str::trim;
using agh::str::pad;
using agh::str::join;
using agh::str::tokens;

using sigfile::CEDFFile;

template valarray<TFloat> CEDFFile::get_region_original_( int, size_t, size_t) const;
template valarray<TFloat> CEDFFile::get_region_original_( const string&, size_t, size_t) const;
template valarray<TFloat> CEDFFile::get_region_filtered_( int, size_t, size_t) const;
template valarray<TFloat> CEDFFile::get_region_filtered_( const string&, size_t, size_t) const;
template int CEDFFile::put_region_( int, const valarray<TFloat>&, size_t) const;
template int CEDFFile::put_region_( const string&, const valarray<TFloat>&, size_t) const;
template int CEDFFile::export_original_( int, const string&) const;
template int CEDFFile::export_original_( const string&, const string&) const;

int
CEDFFile::
set_patient_id( const string& s)
{
	memcpy( header.patient_id, pad( s, 80).c_str(), 80);
	return s.size() > 80;
}

int
CEDFFile::
set_recording_id( const string& s)
{
	memcpy( header.recording_id, pad( s, 80).c_str(), 80);
	return s.size() > 80;
}

int
CEDFFile::
set_episode( const string& s)
{
	_episode.assign( s);
	return set_recording_id( (_session + '/' + _episode).c_str());
}

int
CEDFFile::
set_session( const string& s)
{
	_session.assign( s);
	return set_recording_id( (_session + '/' + _episode).c_str());
}

int
CEDFFile::
set_reserved( const string&s)
{
	memcpy( header.reserved, pad( s, 44).c_str(), 44);
	return s.size() > 44;
}

int
CEDFFile::
set_start_time( time_t s)
{
	char b[9];
	// set start
	strftime( b, 9, "%d.%m.%y", localtime(&s));
	memcpy( header.recording_date, b, 8);
	strftime( b, 9, "%H.%M.%s", localtime(&s));
	memcpy( header.recording_time, b, 8);

	return 0;
}











#define EOA '$'

namespace {

const char version_string[8]  = {'0',' ',' ',' ', ' ',' ',' ',' '};

}

const char* CEDFFile::SSignal::edf_annotations_label = "EDF Annotations";


CEDFFile::
CEDFFile (const string& fname_, int flags_)
      : CSource (fname_, flags_)
{
	{
		struct stat stat0;
		int stst = stat( fname_.c_str(), &stat0);
		if ( stst == -1 ) {
			_status |= TStatus::sysfail;
			throw runtime_error (explain_edf_status(_status));
		}
		_fsize = stat0.st_size;
	}
	_fd = open( fname_.c_str(), O_RDWR);
	if ( _fd == -1 ) {
		_status |= TStatus::sysfail;
		throw runtime_error (explain_edf_status(_status));
	}

      // mmap
	_mmapping =
		mmap( NULL,
		      _fsize,
		      PROT_READ | PROT_WRITE, MAP_SHARED,
		      _fd, 0);
	if ( _mmapping == (void*)-1 ) {
		close( _fd);
		throw length_error ("CEDFFile::CEDFFile(): mmap error");
	}

      // parse header
	if ( _parse_header() ) {  // creates channels list
		if ( not (flags_ & no_field_consistency_check) ) {
			close( _fd);
			munmap( _mmapping, _fsize);
			throw runtime_error (explain_edf_status(_status));
		} else
			fprintf( stderr, "CEDFFile::CEDFFile(\"%s\") Warning: parse header failed, but proceeding anyway\n", fname_.c_str());
	}
	// channels now available

	header_length = 256 + (channels.size() * 256);

      // lest we ever access past mmapped region
	{
		size_t	total_samples_per_record = 0;
		for ( auto& H : channels )
			total_samples_per_record += H.samples_per_record;
		size_t	expected_fsize = header_length + sizeof(int16_t) * total_samples_per_record * n_data_records;
		if ( _fsize < expected_fsize ) {
			fprintf( stderr, "CEDFFile::CEDFFile(\"%s\") file size less than declared in header\n", fname_.c_str());
			close( _fd);
			munmap( _mmapping, _fsize);
			_status |= file_truncated;
			throw runtime_error (explain_edf_status(_status));
		} else if ( _fsize > expected_fsize ) {
			_status |= trailing_junk;
			fprintf( stderr, "CEDFFile::CEDFFile(\"%s\") Warning: %zu bytes of trailing junk\n",
				 fname_.c_str(), _fsize - expected_fsize);
		}
	}

	_extract_embedded_annotations();

      // ancillary files:
	if ( flags_ & sigfile::CTypedSource::no_ancillary_files )
		;
	else {
	      // 1. artifacts, per signal
		for ( auto &H : channels ) {
			ifstream thomas (make_fname_artifacts( H.label));
			if ( not thomas.good() )
				continue;

			while ( !thomas.eof() ) {
				size_t aa = (size_t)-1, az = (size_t)-1;
				thomas >> aa >> az;
				if ( aa == (size_t)-1 || az == (size_t)-1 )
					break;
				H.artifacts.mark_artifact( aa, az);
			}
		}

	      // 2. annotations, per signal
		for ( auto &H : channels ) {
			ifstream fd (make_fname_annotations( H.label));
			if ( not fd.good() )
				continue;
			int type = -1;
			size_t aa = -1, az = -1;
			string an;
			while ( fd.good() and not fd.eof() ) {
				fd >> type >> aa >> az;
				getline( fd, an, EOA);
				if ( aa < az and az < n_data_records * H.samples_per_record
				     and type < SAnnotation::TType_total and type >= 0 )
					H.annotations.emplace_back(
						aa, az,
						trim(an),
						(SAnnotation::TType)type);
				else {
					fprintf( stderr, "Bad annotation: (%d %zu %zu %50s)\n", type, aa, az, an.c_str());
					break;
				}
			}
			H.annotations.sort();
		}

	      // 3. filters
		{
			ifstream thomas (make_fname_filters(fname_));
			if ( !thomas.fail() )
				for ( auto &I : channels ) {
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
}




CEDFFile::
CEDFFile (const string& fname_, TSubtype subtype_, int flags_,
	  const list<pair<string, size_t>>& channels_,
	  size_t data_record_size_,
	  size_t n_data_records_)
      : CSource (fname_, flags_),
	data_record_size (data_record_size_),
	n_data_records (n_data_records_),
	_subtype (subtype_)
{
	_fd = open( fname_.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
	if ( _fd == -1 ) {
		_status |= TStatus::sysfail;
		throw invalid_argument ("CEDFFile::CEDFFile(): file open error");
	}

	header_length = 256 + (channels_.size() * 256);
	size_t total_samplerate = 0;
	for ( auto& H : channels_ )
		total_samplerate += H.second; // total samplerate

	_fsize = header_length + 2 * total_samplerate * data_record_size * n_data_records;
	// extend
	if ( lseek( _fd, _fsize-1, SEEK_SET) == -1 || write( _fd, "\0", 1) != 1 ) {
		_status |= TStatus::sysfail;
		throw invalid_argument ("CEDFFile::CEDFFile(): file write error");
	}

//	size_t sys_page_size = (size_t) sysconf( _SC_PAGESIZE);
	_mmapping =
		mmap( NULL,
		      _fsize,
		      PROT_READ | PROT_WRITE, MAP_SHARED,
		      _fd,
		      0);
	if ( _mmapping == (void*)-1 ) {
		close( _fd);
		throw length_error ("CEDFFile::CEDFFile(): mmap error");
	}

      // fill out some essential header fields
	channels.resize( channels_.size());
	_lay_out_header();

	strncpy( header.version_number, version_string, 8);
	_subject.id = "Fafa_1";
	set_recording_id( "Zzz");
	set_comment( fname_);
	set_start_time( time(NULL));

	strncpy( header.header_length,		pad( to_string(header_length),    8).c_str(), 8);
	strncpy( header.data_record_size,	pad( to_string(data_record_size), 8).c_str(), 8);
	strncpy( header.n_data_records,		pad( to_string(n_data_records),   8).c_str(), 8);
	strncpy( header.n_channels,		pad( to_string(channels_.size()), 4).c_str(), 4);

	_total_samples_per_record = 0;
	size_t hi = 0;
	for ( auto& h : channels_ ) {
		auto& H = channels[hi];

		strncpy( H.header.label,
			 pad( H.label = h.first, 16).c_str(), 16);

		strncpy( H.header.transducer_type,
			 pad( H.transducer_type = "no transducer info", 80).c_str(), 80);
		strncpy( H.header.physical_dim,
			 pad( H.physical_dim = "mV", 8).c_str(), 8);

		H.set_physical_range( -20, 20); // expecting these to be reset before put_signal
		H.set_digital_range( INT16_MIN, INT16_MAX);
		H.scale = (H.physical_max - H.physical_min) /
			(H.digital_max - H.digital_min );

		strncpy( H.header.filtering_info,
			 pad( H.filtering_info = "raw", 80).c_str(), 80);
		strncpy( H.header.samples_per_record,
			 pad( to_string( H.samples_per_record = h.second * data_record_size), 8).c_str(), 8);

		H._at = _total_samples_per_record;
		_total_samples_per_record += H.samples_per_record;

		++hi;
	}
}


void
CEDFFile::SSignal::
set_physical_range( double m, double M)
{
	strncpy( header.physical_min, pad( to_string( physical_min = m), 8).c_str(), 8);
	strncpy( header.physical_max, pad( to_string( physical_max = M), 8).c_str(), 8);
}


void
CEDFFile::SSignal::
set_digital_range( int16_t m, int16_t M)
{
	strncpy( header.digital_min, pad( to_string( digital_min = m), 8).c_str(), 8);
	strncpy( header.digital_max, pad( to_string( digital_max = M), 8).c_str(), 8);
}




size_t
CEDFFile::
resize( size_t new_records)
{
	size_t total_samples_per_record = 0;
	for ( auto& H : channels )
		total_samples_per_record += H.samples_per_record; // total samplerate
	size_t old_records
		= n_data_records;
	auto new_fsize
		= header_length + 2 * total_samples_per_record * (n_data_records = new_records);
	_mmapping =
		mremap( _mmapping,
			_fsize,
			new_fsize,
			0);
	if ( _mmapping == (void*)-1 ) {
		close( _fd);
		throw length_error ("CEDFFile::resize(): mmap error");
	}

	_fsize = new_fsize;
	return old_records;
}



CEDFFile::
CEDFFile (CEDFFile&& rv)
      : CSource (move(rv))
{
	header = rv.header; // no need to re-layout as we don't mremap
	n_data_records   = rv.n_data_records;
	data_record_size = rv.data_record_size;

	_subtype    = rv._subtype;
	_start_time = rv._start_time;
	_end_time   = rv._end_time;

	swap( _patient_id, rv._patient_id);
	swap( _episode,    rv._episode);
	swap( _session,    rv._session);

	swap( channels, rv.channels);

	header_length = rv.header_length;
	_fsize        = rv._fsize;
	_fld_pos      = rv._fld_pos;
	_total_samples_per_record =
		       rv._total_samples_per_record;
	_mmapping     = rv._mmapping;
	_fd           = rv._fd;

	rv._mmapping = (void*)-1;  // will prevent munmap in ~CEDFFile()
}


CEDFFile::
~CEDFFile ()
{
	if ( _mmapping != (void*)-1 ) {
		munmap( _mmapping, _fsize);
		close( _fd);

		if ( not (flags() & sigfile::CTypedSource::no_ancillary_files) )
			write_ancillary_files();
	}
}






void
CEDFFile::
write_ancillary_files()
{
	for ( auto &I : channels ) {
		if ( not I.artifacts().empty() ) {
			ofstream thomas (make_fname_artifacts( I.label), ios_base::trunc);
			if ( thomas.good() )
				for ( auto &A : I.artifacts() )
					thomas << A.a << ' ' << A.z << endl;
		} else
			if ( unlink( make_fname_artifacts( I.label).c_str()) ) {}

		if ( I.annotations.size() ) {
			ofstream thomas (make_fname_annotations( I.label), ios_base::trunc);
			for ( auto &A : I.annotations )
				thomas << (int)A.type << ' ' << A.span.a << ' ' << A.span.z << ' ' << A.label << EOA << endl;
		} else
			if ( unlink( make_fname_annotations( I.label).c_str()) ) {}
	}
	ofstream thomas (make_fname_filters( filename()), ios_base::trunc);
	if ( thomas.good() )
		for ( auto &I : channels )
			thomas << I.filters.low_pass_cutoff << ' ' << I.filters.low_pass_order << ' '
			       << I.filters.high_pass_cutoff << ' ' << I.filters.high_pass_order << ' '
			       << (int)I.filters.notch_filter << endl;
}











void
CEDFFile::
_lay_out_header()
{
	header.version_number 	 = (char*)_mmapping;               //[ 8],
	header.patient_id     	 = header.version_number   +  8;   //[80],
	header.recording_id   	 = header.patient_id       + 80;   //[80],
	header.recording_date 	 = header.recording_id     + 80;   //[ 8],
	header.recording_time 	 = header.recording_date   +  8;   //[ 8],
	header.header_length  	 = header.recording_time   +  8;   //[ 8],
	header.reserved       	 = header.header_length    +  8;   //[44],
	header.n_data_records 	 = header.reserved         + 44;   //[ 8],
	header.data_record_size  = header.n_data_records   +  8;   //[ 8],
	header.n_channels        = header.data_record_size +  8;   //[ 4];

	char *p = (char*)_mmapping + 256;
	size_t h;
	vector<SSignal>::iterator H;
#define FOR(A, C)							\
		for ( h = 0, H = channels.begin(); H != channels.end(); ++h, ++H, p += C ) H->A = p;

	FOR (header.label,			16);
	FOR (header.transducer_type,		80);
	FOR (header.physical_dim,		 8);
	FOR (header.physical_min,		 8);
	FOR (header.physical_max,		 8);
	FOR (header.digital_min,		 8);
	FOR (header.digital_max,		 8);
	FOR (header.filtering_info,		80);
	FOR (header.samples_per_record,		 8);
	FOR (header.reserved,			32);
#undef FOR
}




char*
CEDFFile::
_get_next_field( char *&field, size_t fld_size) throw (TStatus)
{
	if ( _fld_pos + fld_size > _fsize ) {
		_status |= bad_header;
		throw bad_header;
	}

	field = (char*)_mmapping + _fld_pos;
	_fld_pos += fld_size;

	return field;
}

size_t	CEDFFile::max_channels = 256;

int
CEDFFile::
_parse_header()
{
	size_t	n_channels,
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
		_get_next_field( header.n_channels,       4);

		if ( strncmp( header.version_number, version_string, 8) ) {
			_status |= (bad_version | inoperable);
			return -2;
		}

		_subtype =
			(strncasecmp( header.reserved, "edf+c", 5) == 0)
			? edfplus_c
			: (strncasecmp( header.reserved, "edf+d", 5) == 0)
			? edfplus_d
			: edf;

		size_t	header_length;

		header_length = n_data_records = data_record_size = n_channels = 0;
		sscanf( header.header_length,    "%8zu", &header_length);
		sscanf( header.n_data_records,   "%8zu", &n_data_records);
		sscanf( header.data_record_size, "%8zu", &data_record_size);
		sscanf( header.n_channels,       "%4zu", &n_channels);

		if ( !header_length || !n_data_records || !data_record_size || !n_channels ) {
			_status |= bad_numfld;
			if ( not (flags() & no_field_consistency_check) )
				return -2;
		}
		if ( n_channels == 0 )  {
			_status |= inoperable;
			return -2;
		}

		_patient_id = trim( string (header.patient_id, 80));

	      // sub-parse patient_id into SSubjectId struct
		{
			auto subfields = tokens( _patient_id, " ");
			if ( unlikely (_patient_id.empty()) ) {
				_status |= missing_patient_id;
			} else if ( subfields.size() != 4 ) {
				_subject.id = subfields.front();
				_status |= nonconforming_patient_id;
			} else {
				auto i = subfields.begin();
				_subject.id = *i++;
				_subject.gender = agh::SSubjectId::char_to_gender((*i++)[0]);
				_subject.dob = agh::SSubjectId::str_to_dob(*i++);
				_subject.name = join( tokens(*i++, "_"), " ");
				if ( not _subject.valid() )
					_status |= invalid_subject_details;
			}
		}

	      // deal with episode and session
		{
		      // (a) parsed from RecordingID_raw
			char int_session[81], int_episode[81];
			string rec_id_isolated (trim( string (header.recording_id, 80)));
#define T "%80[-a-zA-Z0-9 _]"
			if ( sscanf( rec_id_isolated.c_str(), T ", " T,     int_episode, int_session) == 2 ||
			     sscanf( rec_id_isolated.c_str(), T ": " T,     int_session, int_episode) == 2 ||
			     sscanf( rec_id_isolated.c_str(), T "/"  T,     int_session, int_episode) == 2 ||
			     sscanf( rec_id_isolated.c_str(), T " (" T ")", int_session, int_episode) == 2 )
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
				_session.assign( rec_id_isolated);
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
			string tmp (header.recording_date, 8);
			p = strptime( tmp.c_str(), "%d.%m.%y", &ts);
			if ( p == NULL || *p != '\0' ) {
				_status |= date_unparsable;
				if ( not (flags() & no_field_consistency_check) )
					return -2;
			}
			tmp = {string (header.recording_time, 8)};
			p = strptime( tmp.c_str(), "%H.%M.%S", &ts);
			if ( p == NULL || *p != '\0' ) {
				_status |= time_unparsable;
				if ( not (flags() & no_field_consistency_check) )
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

		if ( n_channels > max_channels ) {
			_status |= bad_numfld;
			if ( not (flags() & no_field_consistency_check) )
				return -2;
		} else {
			channels.resize( n_channels);

			for ( auto &H : channels )
				H.label =
					trim( string (_get_next_field( H.header.label, 16), 16));
			        // to be parsed again wrt SignalType:Channel format

			for ( auto &H : channels )
				H.transducer_type =
					trim( string (_get_next_field( H.header.transducer_type, 80), 80));

			for ( auto &H : channels )
				H.physical_dim =
					trim( string (_get_next_field( H.header.physical_dim, 8), 8));

			for ( auto &H : channels ) {
				_get_next_field( H.header.physical_min, 8);
				if ( H.label == SSignal::edf_annotations_label )
					continue;
				if ( sscanf( H.header.physical_min, "%8lg",
					     &H.physical_min) != 1 ) {
					_status |= bad_numfld;
					if ( not (flags() & no_field_consistency_check) )
						return -2;
				}
			}
			for ( auto &H : channels ) {
				_get_next_field( H.header.physical_max, 8);
				if ( H.label == SSignal::edf_annotations_label )
					continue;
				if ( sscanf( H.header.physical_max, "%8lg",
					     &H.physical_max) != 1 ) {
					_status |= bad_numfld;
					if ( not (flags() & no_field_consistency_check) )
						return -2;
				}
			}

			for ( auto &H : channels ) {
				_get_next_field( H.header.digital_min, 8);
				if ( H.label == SSignal::edf_annotations_label )
					continue;
				if ( sscanf( H.header.digital_min, "%8d",
					     &H.digital_min) != 1 ) {
					_status |= bad_numfld;
					if ( not (flags() & no_field_consistency_check) )
						return -2;
				}
			}
			for ( auto &H : channels ) {
				_get_next_field( H.header.digital_max, 8);
				if ( H.label == SSignal::edf_annotations_label )
					continue;
				if ( sscanf( H.header.digital_max, "%8d",
					     &H.digital_max) != 1 ) {
					_status |= bad_numfld;
					if ( not (flags() & no_field_consistency_check) )
						return -2;
				}
			}

			for ( auto &H : channels ) {
				if ( H.label == SSignal::edf_annotations_label )
					continue;
				H.filtering_info.assign(
					trim( string (_get_next_field( H.header.filtering_info, 80), 80)));
			}

			for ( auto &H : channels ) {
				char *tail;
				string t {trim( string (_get_next_field( H.header.samples_per_record, 8), 8))};
				H.samples_per_record =
					strtoul( t.c_str(), &tail, 10);
				if ( tail == NULL || *tail != '\0' ) {
					_status |= bad_numfld;
					if ( not (flags() & no_field_consistency_check) )
						return -2;
				}
			}

			for ( auto &H : channels )
				H.reserved.assign(
					trim( string (_get_next_field( H.header.reserved, 32), 32)));
		}
	} catch (TStatus ex) {
		return -1;
	} catch (invalid_argument ex) {
		_status |= bad_numfld;
		if ( not (flags() & no_field_consistency_check) )
			return -3;
	}

      // calculate gain
	for ( auto &H : channels )
		if ( H.label == SSignal::edf_annotations_label )
			;
		else
			if ( H.physical_max <= H.physical_min ||
			     H.digital_max  <= H.digital_min  ) {
				_status |= nogain;
				if ( not (flags() & no_field_consistency_check) )
					return -2;
			} else
				H.scale =
					(H.physical_max - H.physical_min) /
					(H.digital_max  - H.digital_min );


      // determine & validate signal types
	i = 0;
	for ( auto &H : channels ) {
		if ( H.label == SSignal::edf_annotations_label )
			H.signal_type = SChannel::TType::embedded_annotation;
		else {
		      // try parsing as "type channel" first
			string parsable (H.label);
			char	*_1 = strtok( &parsable[0], " :,./"),
				*_2 = strtok( NULL, " :,./");
			if ( _2 ) {
				H.signal_type_s = _1;
				H.signal_type = SChannel::figure_signal_type(_1);
				H.label.assign( _2);  // .channel overwritten
		      // it only has a channel name
			} else {
				H.signal_type_s = SChannel::kemp_signal_types[
					H.signal_type = SChannel::signal_type_of_channel( H.label) ];

				if ( not H.label.follows_system1020() ) {  // in case there are duplicate labels, rewrite
					DEF_UNIQUE_CHARP (_);
					if ( asprintf( &_, "%zu:<%s>", i, H.label.c_str()) ) {}
					H.label.assign( _);
					_status |= non1020_channel;
				}
			}
			if ( H.signal_type == SChannel::TType::other )
				_status |= nonkemp_signaltype;
		}
		++i;
	}

      // convenience field
	_total_samples_per_record = 0;
	for ( auto &H : channels ) {
		H._at = _total_samples_per_record;
		_total_samples_per_record += H.samples_per_record;
	}

      // are channels unique?
	for ( auto &H : channels )
		for ( auto &J : channels )
			if ( &J != &H && J.label == H.label ) {
				_status |= dup_channels;
				break;
			}
	return 0;
}








int
CEDFFile::
_extract_embedded_annotations()
{
	auto S = find( channels.begin(), channels.end(), SSignal::edf_annotations_label);
	if ( S == channels.end() )
		return 0;
	auto& AH = *S;

	// hand-picked from get_signal_original
	size_t	r_cnt = n_data_records * AH.data_record_size * 2;

	size_t alen = r_cnt * AH.samples_per_record * 2;
	char* abuf = (char*)malloc( alen);
	while ( r_cnt-- )
		memcpy( &abuf[ r_cnt * H.samples_per_record ],

			(char*)_mmapping + header_length
			+ r_cnt * _total_samples_per_record * 2	// full records before
			+ H._at,				// offset to our samples

			H.samples_per_record * 2);	// our precious ones

	// walk it and pick up annotations
	size_t ai = 0;
//	while ( index( abuf+ai, 20) )
	// for ( size_t i = 0; i < alen; ++i )
	// 	for ( size_t k = i; k < alen-1; ++k )
	// 		if ( abuf[k  ] == (char)21 &&
	// 		     abuf[i+1] == (char)20 )
	// 			;

	free(tmp);

	return 0;
}







string
CEDFFile::
details( bool channels_too) const
{
	ostringstream recv;
	if ( _status & bad_header )
		recv << "Bad header, or no file\n";
	else {
		char *outp;
		ASPRINTF( &outp,
			  "File\t: %s\n"
			  " subtype\t: %s\n"
			  " PatientID\t: %s\n"
			  " RecordingID\t: %s\n"
			  " Date\t: %s\n"
			  " Time\t: %s\n"
			  " # of channels\t: %zu\n"
			  " # of records\t: %zu\n"
			  " Record length\t: %zu sec\n",
			  filename(),
			  subtype_s(),
			  patient_id(),
			  trim( string (header.recording_id, 80)).c_str(),
			  trim( string (header.recording_date, 8)).c_str(),
			  trim( string (header.recording_time, 8)).c_str(),
			  // asctime( localtime( &_start_time)),
			  channels.size(),
			  n_data_records,
			  data_record_size);
		recv << outp;
		free( outp);

		if ( channels_too ) {
			size_t i = 0;
			for ( auto &H : channels ) {
				ASPRINTF( &outp,
					  " Channel %zu:\n"
					  "  Label\t: %s\n"
					  "  Transducer type\t: %s\n"
					  "  Physical dimension\t: %s\n"
					  "  Physical min\t: % g\n"
					  "  Physical max\t: % g\n"
					  "  Digital min\t: % d\n"
					  "  Digital max\t: % d\n"
					  "  Filtering info\t: %s\n"
					  "  Samples/rec\t: %zu\n"
					  "  Scale\t: %g\n"
					  "  (reserved)\t: %s\n",
					  ++i,
					  trim( string (H.header.label, 16)).c_str(),
					  H.transducer_type.c_str(),
					  H.physical_dim.c_str(),
					  H.physical_min,
					  H.physical_max,
					  H.digital_min,
					  H.digital_max,
					  H.filtering_info.c_str(),
					  H.samples_per_record,
					  H.scale,
					  H.reserved.c_str());
				recv << outp;
				free( outp);
			}
		}
	}

	return recv.str();
}







string
CEDFFile::
explain_edf_status( int status)
{
	list<string> recv;
	if ( status & sysfail )
		recv.emplace_back( "* stat or fopen error");
	if ( status & bad_header )
		recv.emplace_back( "* Ill-formed header");
	if ( status & bad_version )
		recv.emplace_back( "* Bad Version signature (i.e., not an EDF file)");
	if ( status & missing_patient_id )
		recv.emplace_back( "* Missing PatientId");
	if ( status & bad_numfld )
		recv.emplace_back( "* Garbage in numerical fields");
	if ( status & date_unparsable )
		recv.emplace_back( "* Date field ill-formed");
	if ( status & time_unparsable )
		recv.emplace_back( "* Time field ill-formed");
	if ( status & nosession )
		recv.emplace_back( "* No session information in field RecordingID");
	if ( status & non1020_channel )
		recv.emplace_back( "* Channel designation not following the 10-20 system");
	if ( status & nonconforming_patient_id )
		recv.emplace_back( "* PatientId not conforming to section 2.1.3.3 of EDF spec");
	if ( status & invalid_subject_details )
		recv.emplace_back( "* PatientId has incomplete or ill-formed subject details");
	if ( status & nonkemp_signaltype )
		recv.emplace_back( "* Signal type not listed in Kemp et al");
	if ( status & dup_channels )
		recv.emplace_back( "* Duplicate channel names");
	if ( status & nogain )
		recv.emplace_back( "* Physical or Digital Min value greater than Max");
	if ( status & too_many_channels )
		recv.emplace_back( string("* Number of channels grearter than ") + to_string(max_channels));
	if ( status & file_truncated )
		recv.emplace_back( "* File truncated");
	if ( status & trailing_junk )
		recv.emplace_back( "* File has trailing junk");
	return join(recv, "\n");
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

