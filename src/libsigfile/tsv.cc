/*
 *       File name:  libsigfile/tsv.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-06-22
 *
 *         Purpose:  TSV source
 *
 *         License:  GPL
 */


#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <list>
#include <stdexcept>

#include "common/lang.hh"
#include "common/string.hh"
#include "tsv.hh"
#include "source.hh"

using namespace std;

using agh::str::trim;
using agh::str::pad;
using agh::str::join;
using agh::str::tokens;
using agh::str::tokens_trimmed;

using sigfile::CTSVFile;

int
CTSVFile::
set_start_time( time_t s)
{
	char b[9];
	strftime( b, 9, "%d.%m.%y", localtime(&s));
	header.recording_date.assign( b);
	strftime( b, 9, "%H.%M.%s", localtime(&s));
	header.recording_time.assign( b);

	return 0;
}




CTSVFile::
CTSVFile (const string& fname_, const int flags_)
      : CSource (fname_, flags_)
{
	{
		struct stat stat0;
		int stst = stat( fname_.c_str(), &stat0);
		if ( stst == -1 )
			throw invalid_argument (explain_status(_status |= TStatus::sysfail));
	}
	_fd = open( fname_.c_str(), O_RDWR);
	if ( _fd == -1 )
		throw invalid_argument (explain_status(_status |= TStatus::sysfail));

      // parse header
	if ( _parse_header() ) {  // creates channels list
		if ( not (flags_ & sigfile::CSource::no_field_consistency_check) ) {
			close( _fd);
			throw invalid_argument (explain_status(_status)); // _status set in _parse_header()
		} else
			fprintf( stderr, "CTSVFile::CTSVFile(\"%s\") Warning: parse header failed, but proceeding anyway\n", fname_.c_str());
	}
	// channels now available

      // ancillary files:
	if ( not (flags_ & sigfile::CSource::no_ancillary_files) )
		load_ancillary_files();
}




CTSVFile::
CTSVFile (const string& fname_, const TSubtype subtype_, const int flags_,
	  const list<SChannel>& channels_,
	  const size_t samplerate_,
	  const double recording_time_)
      : CSource (fname_, flags_),
	_subtype (subtype_),
	_samplerate (samplerate_)
{
	_fd = open( fname_.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
	if ( _fd == -1 ) {
		fprintf( stderr, "CTSVFile::CTSVFile(\"%s\"): Failed to open file for writing\n", fname_.c_str());
		throw invalid_argument (explain_status(_status |= TStatus::sysfail));
	}

      // fill out some essential header fields
	resize_seconds( recording_time_);

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
CTSVFile::SSignal::
set_physical_range( const double m, const double M)
{
	strncpy( header.physical_min, pad( to_string( physical_min = m), 8).c_str(), 8);
	strncpy( header.physical_max, pad( to_string( physical_max = M), 8).c_str(), 8);
}


void
CTSVFile::SSignal::
set_digital_range( const int16_t m, const int16_t M)
{
	strncpy( header.digital_min, pad( to_string( digital_min = m), 8).c_str(), 8);
	strncpy( header.digital_max, pad( to_string( digital_max = M), 8).c_str(), 8);
}




// uncomment on demand (also un-dnl AC_CHECK_FUNCS(mremap,,) in configure.ac)
/*
size_t
CTSVFile::
resize( const size_t new_records)
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
		throw length_error ("CTSVFile::resize(): mmap error");
	}

	_fsize = new_fsize;
	return old_records;
}

*/

CTSVFile::
CTSVFile (CTSVFile&& rv)
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
	swap( common_annotations, rv.common_annotations);

	header_length = rv.header_length;
	_fsize        = rv._fsize;
	_fld_pos      = rv._fld_pos;
	_total_samples_per_record =
		       rv._total_samples_per_record;
	_mmapping     = rv._mmapping;
	_fd           = rv._fd;

	rv._mmapping = (void*)-1;  // will prevent munmap in ~CTSVFile()
}


CTSVFile::
~CTSVFile ()
{
	if ( _mmapping != (void*)-1 ) {
		munmap( _mmapping, _fsize);
		close( _fd);

		if ( not (flags() & sigfile::CTypedSource::no_ancillary_files) )
			write_ancillary_files();
	}
}






void
CTSVFile::
write_ancillary_files()
{
	for ( auto &I : channels ) {
		if ( not I.artifacts().empty() ) {
			ofstream thomas (make_fname_artifacts( I.ucd), ios_base::trunc);
			if ( thomas.good() )
				for ( auto &A : I.artifacts() )
					thomas << A.a << ' ' << A.z << endl;
		} else
			if ( unlink( make_fname_artifacts( I.ucd).c_str()) ) {}

		if ( not I.annotations.empty() ) {
			ofstream thomas (make_fname_annotations( I.ucd), ios_base::trunc);
			for ( auto &A : I.annotations )
				thomas << (int)A.type << ' ' << A.span.a << ' ' << A.span.z << ' ' << A.label << EOA << endl;
		} else
			if ( unlink( make_fname_annotations( I.ucd).c_str()) ) {}
	}
	ofstream thomas (make_fname_filters( filename()), ios_base::trunc);
	if ( thomas.good() )
		for ( auto &I : channels )
			thomas << I.filters.low_pass_cutoff << ' ' << I.filters.low_pass_order << ' '
			       << I.filters.high_pass_cutoff << ' ' << I.filters.high_pass_order << ' '
			       << (int)I.filters.notch_filter << endl;
}










int
CTSVFile::
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
			? edfplus_c
			: (strncasecmp( header.reserved, "edf+d", 5) == 0)
			? edfplus_d
			: edf;

		size_t	header_length;

		header_length = n_data_records = data_record_size = n_channels = 0;
		sscanf( header.header_length,    "%8zu", &header_length);
		sscanf( header.n_data_records,   "%8zu", &n_data_records);
		sscanf( header.data_record_size, "%8lg", &data_record_size); // edf+ supports fractions
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
			} else if ( subfields.size() < 4 ) {
				_subject.id = subfields.front();
				_status |= nonconforming_patient_id;
			} else {
				if ( subfields.size() > 4 )
					_status |= extra_patientid_subfields;
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
							_status |= recognised_channel_conflicting_type;
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
					if ( not (flags() & no_field_consistency_check) )
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
					if ( not (flags() & no_field_consistency_check) )
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
					if ( not (flags() & no_field_consistency_check) )
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
					if ( not (flags() & no_field_consistency_check) )
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
CTSVFile::
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
CTSVFile::
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
			  filename(),
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
CTSVFile::
explain_edf_status( const int status)
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
	if ( status & extra_patientid_subfields )
		recv.emplace_back( "* Extra subfields in PatientId");
	if ( status & recognised_channel_conflicting_type )
		recv.emplace_back( "* Explicitly specified signal type does not match type of known channel name");
	return join(recv, "\n");
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
