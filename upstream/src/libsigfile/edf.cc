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
#include <fstream>
#include <sstream>
#include <list>
#include <stdexcept>

#include "common/lang.hh"
#include "common/string.hh"
#include "edf.hh"
#include "typed-source.hh"

using namespace std;

using agh::str::trim;
using agh::str::pad;
using agh::str::join;
using agh::str::tokens;
using agh::str::tokens_trimmed;

using sigfile::CEDFFile;


// every setter is special
int
CEDFFile::
set_patient_id( const string& s)
{
	memcpy( header.patient_id, pad( s, 80).c_str(), 80);
	_patient_id = s;
	return s.size() > 80;
}

int
CEDFFile::
set_recording_id( const string& s)
{
	memcpy( header.recording_id, pad( s, 80).c_str(), 80);
	_recording_id = s;
	// maybe let _session and _episode be assigned, too?
	return s.size() > 80;
}

int
CEDFFile::
set_episode( const string& s)
{
	_episode.assign( s);
	// aha
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
set_reserved( const string& s)
{
	fprintf( stderr, "You just voided your warranty: Writing to \"reserved\" field in EDF header:\n%s\n", s.c_str());
	_recording_id = s;
	memcpy( header.reserved, pad( s, 44).c_str(), 44);
	return s.size() > 44;
}

int
CEDFFile::
set_recording_date( const string& s)
{
	memcpy( header.recording_date, s.c_str(), 8);
	return 0;
}
int
CEDFFile::
set_recording_time( const string& s)
{
	memcpy( header.recording_time, s.c_str(), 8);
	return 0;
}











#define EOA '$'

namespace {

const char version_string[8]  = {'0',' ',' ',' ', ' ',' ',' ',' '};

}


CEDFFile::
CEDFFile (const string& fname_, const int flags_)
      : CSource (fname_, flags_)
{
	{
		struct stat stat0;
		int stst = stat( fname_.c_str(), &stat0);
		if ( stst == -1 )
			throw invalid_argument (explain_status(_status |= CSource::TStatus::sysfail));
		_fsize = stat0.st_size;
	}
	_fd = open( fname_.c_str(), O_RDWR);
	if ( _fd == -1 )
		throw invalid_argument (explain_status(_status |= sysfail));

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
			throw invalid_argument (explain_status(_status));
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
			throw invalid_argument (explain_status(_status));
		} else if ( _fsize > expected_fsize ) {
			_status |= trailing_junk;
			fprintf( stderr, "CEDFFile::CEDFFile(\"%s\") Warning: %zu bytes of trailing junk\n",
				 fname_.c_str(), _fsize - expected_fsize);
		}
	}

	_extract_embedded_annotations();

	if ( not (flags_ & CSource::no_ancillary_files) )
		load_ancillary_files();
}




CEDFFile::
CEDFFile (const string& fname_, const TSubtype subtype_, const int flags_,
	  const list<pair<SChannel, size_t>>& channels_,
	  const size_t data_record_size_,
	  const size_t n_data_records_)
      : CSource (fname_, flags_),
	data_record_size (data_record_size_),
	n_data_records (n_data_records_),
	_subtype (subtype_)
{
	_fd = open( fname_.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
	if ( _fd == -1 )
		throw invalid_argument (explain_status(_status |= CSource::TStatus::sysfail));

	header_length = 256 + (channels_.size() * 256);
	size_t total_samplerate = 0;
	for ( auto& H : channels_ )
		total_samplerate += H.second; // total samplerate

	_fsize = header_length + 2 * total_samplerate * data_record_size * n_data_records;
	// extend
	if ( lseek( _fd, _fsize-1, SEEK_SET) == -1 || write( _fd, "\0", 1) != 1 )
		throw invalid_argument (explain_status(_status |= sysfail));

//	size_t sys_page_size = (size_t) sysconf( _SC_PAGESIZE);
	_mmapping =
		mmap( NULL,
		      _fsize,
		      PROT_READ | PROT_WRITE, MAP_SHARED,
		      _fd,
		      0);
	if ( _mmapping == (void*)-1 ) {
		close( _fd);
		throw invalid_argument (explain_status(_status |= TStatus::mmap_error));
	}

      // fill out some essential header fields
	channels.resize( channels_.size());
	_lay_out_header();

	strncpy( header.version_number, version_string, 8);

	_subject = {"Fafa_1", "Mr. Fafa"};
	set_patient_id( _subject.make_recording_id_edf_style());

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

		H.ucd = h.first;
		strncpy( H.header.label,
			 pad( H.ucd.name(), 16).c_str(), 16);

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
set_physical_range( const double m, const double M)
{
	strncpy( header.physical_min, pad( to_string( physical_min = m), 8).c_str(), 8);
	strncpy( header.physical_max, pad( to_string( physical_max = M), 8).c_str(), 8);
}


void
CEDFFile::SSignal::
set_digital_range( const int16_t m, const int16_t M)
{
	strncpy( header.digital_min, pad( to_string( digital_min = m), 8).c_str(), 8);
	strncpy( header.digital_max, pad( to_string( digital_max = M), 8).c_str(), 8);
}




// uncomment on demand (also un-dnl AC_CHECK_FUNCS(mremap,,) in configure.ac)
/*
size_t
CEDFFile::
resize_records( const size_t new_records)
{
	size_t total_samples_per_record = 0;
	for ( auto& H : channels )
		total_samples_per_record += H.samples_per_record; // total samplerate
	size_t old_records
		= n_data_records;
	auto new_fsize
		= header_length + 2 * total_samples_per_record * (n_data_records = new_records);

#if !HAVE_MREMAP
	_mmapping =
		mremap( _mmapping,
			_fsize,
			new_fsize,
			0|MREMAP_MAYMOVE);
#else
	void *_m2 =
		mmap( NULL,
		      new_fsize,
		      PROT_READ | PROT_WRITE, MAP_SHARED,
		      _fd,
		      0);
	memmove( _m2, _mmapping, _fsize);
	munmap( _mmapping, _fsize);
	_mmapping = _m2;
#endif

	if ( _mmapping == (void*)-1 ) {
		close( _fd);
		throw length_error ("CEDFFile::resize(): mmap error");
	}

	_fsize = new_fsize;
	return old_records;
}

*/

CEDFFile::
CEDFFile (CEDFFile&& rv)
      : CSource (move(rv))
{
	header = rv.header; // no need to re-layout as we don't mremap
	n_data_records   = rv.n_data_records;
	data_record_size = rv.data_record_size;

	_subtype    = rv._subtype;

	swap( _patient_id,   rv._patient_id);
	swap( _recording_id, rv._recording_id);
	swap( _reserved,     rv._reserved);

	swap( _episode,    rv._episode);
	swap( _session,    rv._session);

	swap( channels, rv.channels);
	swap( common_annotations, rv.common_annotations);

	header_length = rv.header_length;
	_fsize        = rv._fsize;
	_fld_pos      = rv._fld_pos;
	_total_samples_per_record =
		       rv._total_samples_per_record;
	_mmapping     = rv._mmapping;
	_fd           = rv._fd;

	rv._fd = -1; // for propriety's sake
	rv._mmapping = (void*)-1;  // will prevent munmap in ~CEDFFile()
}


CEDFFile::
~CEDFFile ()
{
	if ( not (_flags & no_ancillary_files) )
		save_ancillary_files();

	if ( _mmapping != (void*)-1 ) {
		munmap( _mmapping, _fsize);
		close( _fd);
	}
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
_get_next_field( char *&field, const size_t fld_size) throw (TStatus)
{
	if ( _fld_pos + fld_size > _fsize ) {
		_status |= bad_header;
		throw bad_header;
	}

	field = (char*)_mmapping + _fld_pos;
	_fld_pos += fld_size;

	return field;
}

int
CEDFFile::
_parse_header()
{
	size_t	n_channels;
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
			? TSubtype::edfplus_c
			: (strncasecmp( header.reserved, "edf+d", 5) == 0)
			? TSubtype::edfplus_d
			: TSubtype::edf;

		size_t	header_length;

		header_length = n_data_records = data_record_size = n_channels = 0;
		sscanf( header.header_length,    "%8zu", &header_length);
		sscanf( header.n_data_records,   "%8zu", &n_data_records);
		sscanf( header.data_record_size, "%8lg", &data_record_size); // edf+ supports fractions
		sscanf( header.n_channels,       "%4zu", &n_channels);

		if ( !header_length || !n_data_records || !data_record_size || !n_channels ) {
			_status |= bad_numfld;
			if ( not (_flags & sigfile::CSource::no_field_consistency_check) )
				return -2;
		}
		if ( n_channels == 0 )  {
			_status |= inoperable;
			return -2;
		}

		_patient_id = trim( string (header.patient_id, 80));
		_recording_id = trim( string (header.recording_id, 80));

	      // sub-parse patient_id into SSubjectId struct
		if ( unlikely (_patient_id.empty()) )
			_status |= missing_patient_id;
		_status |=
			_subject.parse_recording_id_edf_style( _patient_id);

	      // times
		figure_times(
			string (header.recording_date, 8),
			string (header.recording_time, 8),
			TAcceptTimeFormat::edf_strict);
		if ( _status & bad_datetime && !(_flags & CSource::TFlags::no_field_consistency_check) )
			return -2;
		_end_time = _start_time + (time_t)recording_time();

	      // deal with episode and session
		tie (_session, _episode) =
			figure_session_and_episode();

	      // assign "reserved"
		_reserved = trim( string (header.reserved, 44));

		if ( n_channels > max_channels ) {
			_status |= bad_numfld;
			if ( not (_flags & sigfile::CSource::no_field_consistency_check) )
				return -2;
		} else {
			channels.resize( n_channels);

		      // determine & validate signal types
			for ( auto &H : channels ) {
				_get_next_field( H.header.label, 16);
				string isolated_label = trim( string (H.header.label, 16));

				if ( isolated_label == sigfile::edf_annotations_label )
					H.ucd = {sigfile::SChannel::TType::embedded_annotation, 0};
				else {
					auto tt = agh::str::tokens( isolated_label, " ");
					// parse legacy pre 0.9 specs ("EEG F3" etc)
					if ( tt.size() > 1 ) {
						string suggested_type = tt.front();
						H.ucd = {(tt.pop_front(), agh::str::join( tt, " "))};
						if ( suggested_type != H.ucd.type_s() )
							_status |= conflicting_channel_type;
					} else {
						H.ucd = sigfile::SChannel (isolated_label);

						if ( H.ucd.type() == sigfile::SChannel::TType::eeg &&
						     H.ucd.idx()  == sigfile::EEG::custom )
							_status |= non1020_channel;
						if ( H.ucd.type() == SChannel::SChannel::TType::other )
							_status |= nonkemp_signaltype;
					}
				}
			}
			for ( auto &H : channels )
				H.transducer_type =
					trim( string (_get_next_field( H.header.transducer_type, 80), 80));

			for ( auto &H : channels )
				H.physical_dim =
					trim( string (_get_next_field( H.header.physical_dim, 8), 8));

			for ( auto &H : channels ) {
				_get_next_field( H.header.physical_min, 8);
				if ( H.ucd.type() == sigfile::SChannel::TType::embedded_annotation )
					continue;
				if ( sscanf( H.header.physical_min, "%8lg",
					     &H.physical_min) != 1 ) {
					_status |= bad_numfld;
					if ( not (_flags & sigfile::CSource::no_field_consistency_check) )
						return -2;
				}
			}
			for ( auto &H : channels ) {
				_get_next_field( H.header.physical_max, 8);
				if ( H.ucd.type() == sigfile::SChannel::TType::embedded_annotation )
					continue;
				if ( sscanf( H.header.physical_max, "%8lg",
					     &H.physical_max) != 1 ) {
					_status |= bad_numfld;
					if ( not (_flags & sigfile::CSource::no_field_consistency_check) )
						return -2;
				}
			}

			for ( auto &H : channels ) {
				_get_next_field( H.header.digital_min, 8);
				if ( H.ucd.type() == sigfile::SChannel::TType::embedded_annotation )
					continue;
				if ( sscanf( H.header.digital_min, "%8d",
					     &H.digital_min) != 1 ) {
					_status |= bad_numfld;
					if ( not (_flags & sigfile::CSource::no_field_consistency_check) )
						return -2;
				}
			}
			for ( auto &H : channels ) {
				_get_next_field( H.header.digital_max, 8);
				if ( H.ucd.type() == sigfile::SChannel::TType::embedded_annotation )
					continue;
				if ( sscanf( H.header.digital_max, "%8d",
					     &H.digital_max) != 1 ) {
					_status |= bad_numfld;
					if ( not (_flags & sigfile::CSource::no_field_consistency_check) )
						return -2;
				}
			}

			for ( auto &H : channels )
				H.filtering_info.assign(
					trim( string (_get_next_field( H.header.filtering_info, 80), 80)));

			for ( auto &H : channels ) {
				char *tail;
				string t {trim( string (_get_next_field( H.header.samples_per_record, 8), 8))};
				H.samples_per_record =
					strtoul( t.c_str(), &tail, 10);
				if ( tail == NULL || *tail != '\0' ) {
					_status |= bad_numfld;
					if ( not (_flags & sigfile::CSource::no_field_consistency_check) )
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
		if ( not (_flags & sigfile::CSource::no_field_consistency_check) )
			return -3;
	}

      // calculate gain
	for ( auto &H : channels )
		if ( H.ucd.type() != sigfile::SChannel::TType::embedded_annotation ) {
			if ( H.physical_max <= H.physical_min ||
			     H.digital_max  <= H.digital_min  )
				_status |= nogain;
			H.scale =
				(H.physical_max - H.physical_min) /
				(H.digital_max  - H.digital_min );
		}


      // convenience field
	_total_samples_per_record = 0;
	for ( auto &H : channels ) {
		H._at = _total_samples_per_record;
		_total_samples_per_record += H.samples_per_record;
	}

      // are channels unique?
	for ( auto &H : channels )
		for ( auto &J : channels ) {
			if ( &J != &H && J.ucd == H.ucd ) {
				_status |= dup_channels;
				goto outer_break;
			}
		}
outer_break:

	return 0;
}








int
CEDFFile::
_extract_embedded_annotations()
{
	auto S = find( channels.begin(), channels.end(), sigfile::edf_annotations_label);
	if ( S == channels.end() )
		return 0;
	auto& AH = *S;

	size_t alen = AH.samples_per_record * 2;

	for ( size_t r = 0; r < n_data_records; ++r ) {
		char   *this_a =
			(char*)_mmapping + header_length
			+ r * _total_samples_per_record * 2	// full records before
			+ AH._at * 2;				// offset to our samples

		if ( (this_a[0] == '+'   || this_a[0] == '-') &&
		     (isdigit(this_a[1]) || this_a[1] == '.') ) {

			string	abuf (this_a, alen); // NULL-terminated, possibly at pos <alen

			float	offset,
				duration;
			const char
				*offset_p = abuf.c_str(),
				*duration_p,
				*tals_p;

			while ( (tals_p = strchr( offset_p, 20)) ) {
				// determine if we have duration
				try {
					if ( (duration = 0.,
					      (duration_p = strchr( offset_p, 21))) &&
					     duration_p < tals_p ) {
						offset = stof( string (offset_p, duration_p - offset_p));
						if ( *duration_p != 20 )
							duration = stof( string (duration_p, tals_p - duration_p));
					} else {
						offset = stof( string (offset_p, tals_p - offset_p));
					}
				} catch (...) {
					break;
				}

				if ( offset_p == this_a && *tals_p == 20 ) // no TALs, it's an explicit record timestamp, not an annotation
					_record_offsets.push_back( offset);

				else {
					auto tals = tokens_trimmed( tals_p, (char)20);
					for ( auto& t : tals )
						if ( not t.empty() ) {
							common_annotations.emplace_back(
								offset,
								offset + duration,
								t,
								SAnnotation::TType::plain);
						}
				}

				offset_p = tals_p + strlen(tals_p) + 1;
			}
		}
	}

	return 0;
}







string
CEDFFile::
details( const int which) const
{
	ostringstream recv;
	if ( _status & bad_header )
		recv << "Bad header, or no file\n";
	else {
		size_t	n_dicontinuities = 0;
		double	prev_offset = NAN, cur_offset;
		for ( size_t r = 1; r < _record_offsets.size(); ++r ) {
			cur_offset = _record_offsets[r] - _record_offsets[r-1];
			if ( isfinite(prev_offset) and cur_offset != prev_offset )
				++n_dicontinuities;
			prev_offset = cur_offset;
		}
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
			  " Record size\t: %g sec\n"
			  " # of discontinuities\t: %zu\n"
			  " # of embedded annotations\t: %zu\n",
			  agh::str::homedir2tilda( filename()).c_str(),
			  subtype_s(),
			  patient_id(),
			  trim( string (header.recording_id, 80)).c_str(),
			  trim( string (header.recording_date, 8)).c_str(),
			  trim( string (header.recording_time, 8)).c_str(),
			  channels.size(),
			  n_data_records,
			  data_record_size,
			  n_dicontinuities,
			  common_annotations.size());
		recv << outp;
		free( outp);

		if ( which & with_channels ) {
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

		if ( which & with_annotations ) {
			recv << "Embedded annotations (" << common_annotations.size() << "):\n";
			for ( auto &A : common_annotations )
				recv << ' '
				     << A.span.a << '\t'
				     << A.span.z << '\t'
				     << A.label << endl;
		}
	}

	return recv.str();
}







string
CEDFFile::
explain_status( const int status)
{
	list<string> recv;
	if ( status & bad_version )
		recv.emplace_back( "Bad Version signature (i.e., not an EDF file)");
	if ( status & nonconforming_patient_id )
		recv.emplace_back( "PatientId not conforming to section 2.1.3.3 of EDF spec");
	if ( status & file_truncated )
		recv.emplace_back( "File truncated");
	if ( status & trailing_junk )
		recv.emplace_back( "File has trailing junk");
	if ( status & extra_patientid_subfields )
		recv.emplace_back( "Extra subfields in PatientId");
	if ( status & mmap_error )
		recv.emplace_back( "mmap error");

	return CSource::explain_status(status) + (recv.empty() ? "" : (join(recv, "\n") + '\n'));
}





int
agh::SSubjectId::
parse_recording_id_edf_style( const string& s)
{
	using namespace agh::str;
	int_least32_t status = 0;
	auto subfields = tokens( s, " ");
	if ( subfields.size() < 4 ) {
		id = subfields.front();
		status |= sigfile::CEDFFile::nonconforming_patient_id;
	} else {
		if ( subfields.size() > 4 )
			status |= sigfile::CEDFFile::extra_patientid_subfields;
		auto i = subfields.begin();
		id = *i++;
		gender = agh::SSubjectId::char_to_gender((*i++)[0]);
		dob = agh::SSubjectId::str_to_dob(*i++);
		name = join( tokens(*i++, "_"), " ");
		if ( not valid() )
			status |= sigfile::CSource::invalid_subject_details;
	}
	return status;
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
