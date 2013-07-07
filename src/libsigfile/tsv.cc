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
#include <regex.h>
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
	_f = fopen( fname_.c_str(), "r");
	if ( !_f )
		throw invalid_argument (explain_status(_status |= TStatus::sysfail));

      // parse header
	if ( _parse_header() ) {  // creates channels list
		if ( not (flags_ & sigfile::CSource::no_field_consistency_check) ) {
			throw invalid_argument (explain_status(_status)); // _status set in _parse_header()
		} else
			fprintf( stderr, "CTSVFile::CTSVFile(\"%s\") Warning: parse header failed, but proceeding anyway\n", fname_.c_str());
	}
	// channels now available

	_read_data();

	if ( not (flags_ & CSource::no_ancillary_files) )
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
	_f = fopen( fname_.c_str(), "r");
	if ( !_f ) {
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

	_f = rv._f;
	rv._f = nullptr;
}


CTSVFile::
~CTSVFile ()
{
	if ( not (flags() & sigfile::CSource::no_ancillary_files) )
		save_ancillary_files();
}





int
CTSVFile::
_parse_header()
{
	size_t	n_channels;

      // 1. read metadata

	regex_t RE;
	assert (0 == regcomp( &RE, "^#\\W*(\\w+)\\W*(:|=)\\", REG_EXTENDED));
	regmatch_t M[1+2];

	size_t n = 4096;
	char *line = (char*)malloc( n);
	while ( getline( &line, &n, _f) > 0 ) {
		if ( regexec( &RE, line, 1+2, M, 0) == 0 ) {
			metadata[string (line, M[1].rm_so, M[1].rm_eo)] =
				string (line, M[2].rm_so, M[2].rm_eo);
			printf( "matched metadata [%s] = %s\n", string (line, M[1].rm_so, M[1].rm_eo).c_str(), string (line, M[2].rm_so, M[2].rm_eo).c_str());
		} else
			if ( line[0] != '#' )
				break; // end of header
	}
	free( (void*)line);

      // 2. pick essential bits
	

      // 3. deal with episode and session
	int parsed_with_issues;
	tie( _session, _episode, parsed_with_issues) =
		figure_session_and_episode();
	if ( parsed_with_issues )
		_status |= (nosession | noepisode);

	// are channels unique?
	for ( auto &H : channels )
		for ( auto &J : channels ) {
			if ( &J != &H && J.ucd == H.ucd ) {
				_status |= dup_channels;
				goto outer_break;
			}
		}
outer_break:

      // 4. read one line of channel data, figure subtype and number of channels
	

	return 0;
}


int
CTSVFile::
_read_data()
{
	
	return 0;
}






int
CTSVFile::
put_region_smpl( int, const valarray<TFloat>&, size_t) const
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
		char b[20];
		recv << agh::str::sasprintf(
			"File\t: %s\n"
			" subtype\t: %s\n"
			" PatientID\t: %s\n"
			" RecordingID\t: %s\n"
			" Start time\t: %s\n"
			" Duration\t: %s\n"
			" # of channels\t: %zu\n"
			" Sample rate\t: %zu\n",
			filename(),
			subtype_s(),
			patient_id(),
			recording_id(),
			(strftime( b, 20, "%F %T", localtime(&_start_time)), b),
			agh::str::dhms( recording_time()).c_str(),
			channels.size(),
			_samplerate);

		if ( which & with_channels ) {
			size_t i = 0;
			for ( auto &H : channels )
				recv << agh::str::sasprintf(
					" Channel %zu:\n"
					"  Label\t: %s\n",
					++i,
					H.ucd.name());
		}
	}

	return recv.str();
}







string
CTSVFile::
explain_status( const int status)
{
	list<string> recv;
	if ( status & sysfail )
		recv.emplace_back( "* stat or fopen error");
	if ( status & bad_header )
		recv.emplace_back( "* Ill-formed header");
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
	if ( status & too_many_channels )
		recv.emplace_back( string("* Number of channels grearter than ") + to_string(max_channels));
	if ( status & extra_patientid_subfields )
		recv.emplace_back( "* Extra subfields in PatientId");
	return join(recv, "\n");
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
