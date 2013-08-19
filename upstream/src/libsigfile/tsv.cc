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
#include "typed-source.hh"

using namespace std;

using agh::str::trim;
using agh::str::pad;
using agh::str::join;
using agh::str::tokens;
using agh::str::tokens_trimmed;

using sigfile::CTSVFile;




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
	_subtype =
		(strcasecmp( &fname_[fname_.size()-4], ".csv") == 0)
		? TSubtype::csv
		: (strcasecmp( &fname_[fname_.size()-4], ".tsv") == 0) ? TSubtype::tsv
		: TSubtype::invalid;

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
	if ( not (_flags & sigfile::CSource::no_ancillary_files) )
		save_ancillary_files();
	if ( _line0 )
		free( (void*)_line0);
}



int
CTSVFile::
set_recording_date( const string& s)
{
	metadata["recording_date"] = s;
	return 0;
}
int
CTSVFile::
set_recording_time( const string& s)
{
	metadata["recording_time"] = s;
	return 0;
}


int
CTSVFile::
_parse_header()
{
      // 1. read metadata
	regex_t RE;
	regcomp( &RE, "^#\\W*([a-zA-Z_][a-zA-Z_0-9]*)\\W*(:|=)\\W*(.+)\\W*\n", REG_EXTENDED);
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
		_status |= bad_session_or_episode;
		return -1;
	}

	if ( metadata.find( "patient_id") == metadata.end() ) {
		fprintf( stderr, "No patient_id in header\n");
		_status |= CSource::missing_patient_id;;
		return -1;
	}
	_status |=
		_subject.parse_recording_id_edf_style( metadata["patient_id"]);

	if ( metadata.find( "recording_date") == metadata.end() ||
	     metadata.find( "recording_time") == metadata.end() ) {
		fprintf( stderr, "No recording_date in header\n");
		_status |= CSource::bad_datetime;
		return -1;
	}

	figure_times(
		metadata["recording_date"],
		metadata["recording_time"],
		TAcceptTimeFormat::any);
	if ( _status & bad_datetime && !(_flags & CSource::TFlags::no_field_consistency_check) )
		return -1;

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
	tie( _session, _episode) =
		figure_session_and_episode();

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
	// _line0 contains the first row of data already (it is the
	// first line not beginning with a #)

	vector<vector<double>> c2 (channels.size());
	size_t r, ll = 0;

	if ( metadata["series"] == "irregular" ) {
		vector<unsigned long> offsets;
		do {
			double ts;
			if ( 1 != sscanf( _line0, "%lg%*[,\t]", &ts) ) {
				fprintf( stderr, "Bad offset (at data line %zu)\n", ll);
				_status |= bad_offset;
				return -1;
			}
			if ( ll > 0 && ts * _samplerate <= offsets.back() ) {
				fprintf( stderr, "Offsets not increasing (at data line %zu)\n", ll);
				_status |= offsets_not_incteasing;
				return -1;
			}

			offsets.push_back( ts * _samplerate);

			for ( r = 0; r < channels.size(); ++r ) {
				double x;
				if ( 1 != sscanf( _line0, "%lg%*[,\t]", &x) )
					goto outer_break1;
				c2[r].push_back( x);
			}
			++ll;
		} while ( getline( &_line0, &_line0_mallocked_bytes, _f) > 0 );
	outer_break1:

		if ( r != 0 && r != channels.size() ) {
			fprintf( stderr, "Number of data read (%zu) not a multiple of channel count (%zu)\n", r, channels.size());
			_status |= bad_channel_count;
			return -1;
		}

	      // interpolate and resample
		for ( size_t h = 0; h < channels.size(); ++h ) {
			valarray<TFloat> interpolated =
				sigproc::interpolate<TFloat>(
					offsets,
					_samplerate,
					c2[h],
					1./_samplerate);
			channels[h].data.resize( interpolated.size());
			channels[h].data = interpolated;
		}


	} else {
		do {
			for ( r = 0; r < channels.size(); ++r ) {
				double x;
				if ( 1 != sscanf( _line0, "%lg%*[,\t]", &x) )
					goto outer_break2;
				c2[r].push_back( x);
			}
			++ll;
		} while ( getline( &_line0, &_line0_mallocked_bytes, _f) > 0 );
	outer_break2:

		if ( r != 0 && r != channels.size() ) {
			fprintf( stderr, "Number of data read (%zu) not a multiple of channel count (%zu)\n", r, channels.size());
			_status |= bad_channel_count;
			return -1;
		}

		// vector -> valarray
		for ( size_t h = 0; h < channels.size(); ++h ) {
			channels[h].data.resize( ll);
			for ( size_t i = 0; i < ll; ++i )
				channels[h].data[i] = c2[h][i];
		}
	}


	// only now as late
	_end_time = _start_time + (time_t)recording_time();

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
		agh::str::homedir2tilda( filename()).c_str(),
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
	if ( status & bad_channel_count )
		recv.emplace_back( "Number of channels declared in header different from number of columns of data");
	if ( status & bad_offset )
		recv.emplace_back( "Bad offset");
	if ( status & offsets_not_incteasing )
		recv.emplace_back( "Offsets in an irregular-series data not increasing");
	return CSource::explain_status(status) + (recv.empty() ? "" : (join(recv, "\n") + '\n'));
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
