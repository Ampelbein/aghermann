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
	metadata["recording_date"].assign( b);
	strftime( b, 9, "%H.%M.%s", localtime(&s));
	metadata["recording_time"].assign( b);

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
			throw invalid_argument (explain_status(_status |= CSource::TStatus::sysfail));
	}
	_f = fopen( fname_.c_str(), "r");
	if ( !_f )
		throw invalid_argument (explain_status(_status |= sysfail));

      // parse header
	if ( _parse_header() ) {  // creates channels list
		if ( not (flags_ & sigfile::CSource::no_field_consistency_check) ) {
			throw invalid_argument (explain_status(_status)); // _status set in _parse_header()
		} else
			fprintf( stderr, "CTSVFile::CTSVFile(\"%s\") Warning: parse header failed, but proceeding anyway\n", fname_.c_str());
	}
	// channels now available

	if ( _read_data() )
		throw invalid_argument (explain_status(_status)); // _status set in _parse_header()

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
	_samplerate (samplerate_),
	_line0 (nullptr)
{
	_f = fopen( fname_.c_str(), "r");
	if ( !_f ) {
		fprintf( stderr, "CTSVFile::CTSVFile(\"%s\"): Failed to open file for writing\n", fname_.c_str());
		throw invalid_argument (explain_status(_status |= CSource::TStatus::sysfail));
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
	swap( _episode,    rv._episode);
	swap( _session,    rv._session);

	swap( metadata, rv.metadata);

	_subtype    = rv._subtype;
	_start_time = rv._start_time;
	_end_time   = rv._end_time;

	swap( channels, rv.channels);
	swap( common_annotations, rv.common_annotations);

	_f = rv._f;
	rv._f = nullptr;

	_line0_mallocked_bytes = rv._line0_mallocked_bytes;
	_line0 = rv._line0;
	rv._line0 = nullptr;

}


CTSVFile::
~CTSVFile ()
{
	if ( not (flags() & sigfile::CSource::no_ancillary_files) )
		save_ancillary_files();
	if ( _line0 )
		free( (void*)_line0);
}





int
CTSVFile::
_parse_header()
{
      // 1. read metadata
	regex_t RE;
	assert (0 == regcomp( &RE, "^#\\W*([a-zA-Z_][a-zA-Z_0-9]*)\\W*(:|=)\\W*(.+)\\W*\n", REG_EXTENDED));
	regmatch_t M[1+1+2];

	_line0_mallocked_bytes = 4096;
	_line0 = (char*)malloc( _line0_mallocked_bytes);

	while ( getline( &_line0, &_line0_mallocked_bytes, _f) != -1 ) {
		if ( _line0[0] == '\n' )
			continue;
		if ( regexec( &RE, _line0, 1+1+2, M, 0) == 0 ) {
			string	K = trim( string (_line0, M[1].rm_so, (M[1].rm_eo - M[1].rm_so))),
				V = trim( string (_line0, M[3].rm_so, (M[3].rm_eo - M[3].rm_so)));
			metadata[K] = V;
		} else if ( _line0[0] != '#' )
			break; // end of header
	}

      // 2. pick essential bits
	if ( metadata.find( "recording_id") == metadata.end() ) {
		fprintf( stderr, "No session/episode in header\n");
		_status |= (nosession | noepisode);
		return -1;
	}

	if ( metadata.find( "patient_id") == metadata.end() ) {
		fprintf( stderr, "No patient_id in header\n");
		_status |= CSource::missing_patient_id;;
		return -1;
	}

	if ( metadata.find( "comment") == metadata.end() )
		;

	if ( metadata.find( "samplerate") == metadata.end() ||
	     (_samplerate = stoi( metadata["samplerate"])) > 2048 ) {
		fprintf( stderr, "Samplerate missing or too high in header\n");
		_status |= bad_header;
		return -1;
	}

	if ( metadata.find( "channels") == metadata.end() ) {
		fprintf( stderr, "No channels in header\n");
		_status |= bad_header;
		return -1;
	}
	for ( const auto& h : agh::str::tokens( metadata["channels"], " ,;\t") )
		channels.emplace_back( h);

      // 3. deal with episode and session
	int parsed_with_issues;
	tie( _session, _episode, parsed_with_issues) =
		figure_session_and_episode();
	if ( parsed_with_issues )
		_status |= (nosession | noepisode);

      // 4. are channels unique?
	for ( auto &H : channels )
		for ( auto &J : channels ) {
			if ( &J != &H && J.ucd == H.ucd ) {
				_status |= dup_channels;
				goto outer_break;
			}
		}
outer_break:

      // 4. sample one line of channel data
	if ( agh::str::tokens( _line0, "\t;, ").size() != channels.size() ) {
		fprintf( stderr, "Number of channels declared in header (%zu) different from number of columns of data\n", channels.size());
		_status |= bad_channel_count;
		return -1;
	}

	return 0;
}


int
CTSVFile::
_read_data()
{
	vector<vector<double>> c2 (channels.size());

	// _line0 contains the first row of data already (it is the
	// first line not beginning with a #)
	size_t r, ll = 0;
	do {
		for ( r = 0; r < channels.size(); ++r ) {
			double x;
			if ( 1 != fscanf( _f, "%lg", &x) )
				goto outer_break;
			c2[r].push_back( x);
		}
		++ll;
	} while ( getline( &_line0, &_line0_mallocked_bytes, _f) > 0 );

outer_break:

	if ( r != 0 && r != channels.size() ) {
		fprintf( stderr, "Number of data read (%zu) not a multiple of channel count (%zu)\n", r, channels.size());
		_status |= bad_channel_count;
		return -1;
	}

	printf( "read %zu samples in %zu channels\n", ll/channels.size(), channels.size());
	// vector -> valarray
	for ( size_t h = 0; h < channels.size(); ++h ) {
		channels[h].data.resize( ll);
		for ( size_t i = 0; i < ll; ++i )
			channels[h].data[i] = c2[h][i];
	}

	return 0;
}






int
CTSVFile::
put_region_smpl( int h, const valarray<TFloat>& V, size_t off)
{
	if ( unlikely (h > (int)channels.size() - 1) )
		throw out_of_range ("Bad channel index");
	if ( unlikely (off + V.size() > channels[h].data.size()) )
		throw out_of_range ("Bad offset");

	channels[h].data[ slice (off, V.size(), 1) ] = V[ slice (0, V.size(), 1) ];

	return 0;
}



size_t
CTSVFile::
resize_seconds( double s)
{
	assert ( s > 0. );
	for ( auto& H : channels )
		H.data.resize( s * _samplerate);
	return 0;
}


string
CTSVFile::
details( const int which) const
{
	ostringstream recv;
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
	if ( status & bad_channel_count )
		recv.emplace_back( "* Number of channels declared in header different from number of columns of data");
	return join(recv, "\n");
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
