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
	_recording_date.assign( b);
	strftime( b, 9, "%H.%M.%s", localtime(&s));
	_recording_time.assign( b);

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
			_fd = -1;
			throw invalid_argument (explain_status(_status)); // _status set in _parse_header()
		} else
			fprintf( stderr, "CTSVFile::CTSVFile(\"%s\") Warning: parse header failed, but proceeding anyway\n", fname_.c_str());
	}
	// channels now available

	_read_data();

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
	_subject = {"Fafa_1", "Mr. Fafa"};
	set_recording_id( "Zzz");
	set_comment( fname_);
	set_start_time( time(NULL));

	size_t hi = 0;
	for ( auto& h : channels_ ) {
		auto& H = channels[hi++];
		H.ucd = h;
	}

	resize_seconds( recording_time_);
}




CTSVFile::
CTSVFile (CTSVFile&& rv)
      : CSource (move(rv))
{
	swap( _patient_id,   rv._patient_id);
	swap( _recording_id, rv._recording_id);
	swap( _recording_date, rv._recording_date);
	swap( _recording_time, rv._recording_time);
	swap( _episode,    rv._episode);
	swap( _session,    rv._session);
	swap( _comment, rv._comment);

	swap( metadata, rv.metadata);

	_subtype    = rv._subtype;
	_start_time = rv._start_time;
	_end_time   = rv._end_time;


	swap( channels, rv.channels);
	swap( common_annotations, rv.common_annotations);

	_fd = rv._fd;
	rv._fd = -1;
}


CTSVFile::
~CTSVFile ()
{
	if ( _fd != -1 ) {
		close( _fd);

		if ( not (flags() & sigfile::CSource::no_ancillary_files) )
			write_ancillary_files();
	}
}





int
CTSVFile::
_parse_header()
{
	size_t	n_channels;
	_subtype = TSubtype::tsv;

	// deal with episode and session
	{
		// (a) parsed from RecordingID_raw
		char int_session[81], int_episode[81];
		string rec_id_isolated (trim( _recording_id));
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
_read_data()
{
	
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
			  recording_id.c_str(),
			  recording_date, 8)).c_str(),
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
// tab-width: 8
// End:
