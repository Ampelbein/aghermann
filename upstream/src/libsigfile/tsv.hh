/*
 *       File name:  libsigfile/tsv.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-06-22
 *
 *         Purpose:  plain ascii (tsv) data source
 *
 *         License:  GPL
 */

#ifndef AGH_SIGFILE_TSV_H_
#define AGH_SIGFILE_TSV_H_

#include <cinttypes>
#include <cstring>
#include <ctime>
#include <string>
#include <valarray>
#include <vector>
#include <list>
#include <map>
#include <stdexcept>
#include <fstream>

#include "libsigproc/sigproc.hh"
#include "channel.hh"
#include "source-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;


namespace sigfile {



class CTSVFile
  : public CSource {

      // deleted
	bool operator==( const CTSVFile&) const = delete;
	CTSVFile() = delete;

    public:
	// subtype
	enum class TSubtype {
		invalid,
		csv,
		tsv,
	};
	TSubtype subtype() const
		{ return _subtype; }
	static const char*
	subtype_s( TSubtype t)
		{
			switch (t) {
			case TSubtype::csv: return "csv";
			case TSubtype::tsv: return "tsv";
			default:  return "(invalid)";
			}
		}
	const char*
	subtype_s() const
		{ return subtype_s( _subtype); }

      // ctor
	CTSVFile (const CTSVFile&)
	      : CSource("")
		{
			throw invalid_argument("nono");
		}
	// open existing
	CTSVFile (const string& fname, int flags = 0);
	// create new
	CTSVFile (const string& fname, TSubtype, int flags,
		  const list<SChannel>&,
		  const size_t samplerate_,
		  const double recording_time_);
	CTSVFile (CTSVFile&&);
      // dtor
       ~CTSVFile ();

      // interface
	// status
	string explain_status() const
		{ return explain_status( _status); }

	// identification
	const char* patient_id() const
		{
			const auto I = metadata.find("patient_id");
			return (I == metadata.end()) ? "" : I->second.c_str();
		}
	const char* recording_id() const
		{
			const auto I = metadata.find("recording_id");
			return (I == metadata.end()) ? "" : I->second.c_str();
		}
	const char* comment() const
		{
			const auto I = metadata.find("comment");
			return (I == metadata.end()) ? "" : I->second.c_str();
		}
	const char* episode() const
		{ return _episode.c_str(); }
	const char* session() const
		{ return _session.c_str(); }

	// times
	double recording_time() const // in seconds
		{ return (double)channels.front().data.size() / _samplerate; } // all channels have the same sr, obviously
	int set_recording_date( const string&);
	int set_recording_time( const string&);

	// setters
	int set_patient_id( const string& s)
		{
			metadata["patient_id"] = s;
			return 0;
		}
	int set_recording_id( const string& s)
		{
			metadata["recording_id"] = s;
			return 0;
		}
	int set_comment( const string& s)
		{
			metadata["comment"] = s;
			return 0;
		}

	int set_episode( const string& s) // assigning to _episode or _session directly won't have a lasting effect; think again.
		{
			_episode = s;
			return 0;
		}
	int set_session( const string& s)
		{
			_session = s;
			return 0;
		}

	// channels
	size_t n_channels() const
		{ return channels.size(); }
	list<SChannel>
	channel_list() const
		{
			list<SChannel> ret;
			for ( auto &H : channels )
				ret.push_back( H.ucd);
			return ret;
		}
	bool
	have_channel( const SChannel& h) const
		{ return find( channels.cbegin(), channels.cend(), h) != channels.cend(); }
	int
	channel_id( const SChannel& h) const
		{
			for ( size_t i = 0; i < channels.size(); ++i )
				if ( channels[i].ucd == h )
					return i;
			return -1;
		}
	const SChannel&
	channel_by_id( const int h) const
		{ return channels[h].ucd; }

	SChannel::TType
	signal_type( const int h) const
		{ return operator[](h).ucd.type(); }

	size_t
	samplerate( int) const
		{ return _samplerate; }

	list<SAnnotation>&
	annotations( const int h)
		{ return operator[](h).annotations; }
	const list<SAnnotation>&
	annotations( const int h) const
		{ return operator[](h).annotations; }

	list<SAnnotation>&
	annotations()
		{ return common_annotations; }
	const list<SAnnotation>&
	annotations() const
		{ return common_annotations; }

	// artifacts
	SArtifacts&
	artifacts( int h)
		{ return operator[](h).artifacts; }
	const SArtifacts&
	artifacts( int h) const
		{ return operator[](h).artifacts; }

	// filters
	SFilterPack&
	filters( const int h)
		{ return operator[](h).filters; }
	const SFilterPack&
	filters( const int h) const
		{ return operator[](h).filters; }


      // signal data extractors
	valarray<TFloat>
	get_region_original_smpl( const int h, const size_t sa, const size_t sz) const
		{ return operator[](h).data[ slice (sa, sz-sa, 1) ];}

	valarray<TFloat>
	get_signal_original( const int h) const // there is a CSource::get_signal_original already, but this one is a little better
		{ return get_region_original_smpl( h, 0, channels.front().data.size()-1); }

	valarray<TFloat>
	get_signal_filtered( const int h) const
		{ return get_region_filtered_smpl( h, 0, channels.front().data.size()-1); }

      // put signal
	int put_region_smpl( int, const valarray<TFloat>&, size_t);
	int put_region_sec( const int h, const valarray<TFloat>& src, const float offset)
		{ return put_region_smpl( h, src, (size_t)(offset * _samplerate)); }

	int put_signal( const int h, const valarray<TFloat>& src)
		{ return put_region_smpl( h, src, 0); }

      // signal data info
	pair<TFloat, TFloat>
	get_real_original_signal_range( const int h) const
		{
			auto x = get_signal_original( h);
			return {x.min(), x.max()};
		}

	pair<TFloat, TFloat>
	get_real_filtered_signal_range( const int h) const
		{
			auto x = get_signal_filtered( h);
			return {x.min(), x.max()};
		}

	pair<TFloat, TFloat>
	get_max_filtered_signal_range( const int h) const
		{
			auto x = get_signal_filtered( h);
			return {x.min(), x.max()};   // an approximate
		}

      // adjust capacity
	size_t
	resize_seconds( double);

      // reporting & misc
	string details( int which_details) const;

	sigproc::TWinType af_dampen_window_type; // master copy

	map<string,string>
		metadata;

      // channels
	struct SSignal {
		SSignal (const SChannel& ch)
		      : ucd (ch)
			{}

		SChannel
			ucd; // Universal Channel Designation, епта

		double	scale;

		valarray<TFloat>
			data;

		bool operator==( const SChannel& h) const
			{
				return ucd == h;
			}
		bool operator==( const string& h) const
			{
				return ucd.name() == h;
			}

		list<SAnnotation>
			annotations;
		SArtifacts
			artifacts;
		SFilterPack
			filters;
	};
	vector<SSignal>
		channels;

	list<SAnnotation> // timepoints in seconds
		common_annotations;

      // signal accessors
	SSignal& operator[]( size_t i)
		{
			if ( unlikely (i >= channels.size()) )
				throw out_of_range ("Signal index out of range");
			return channels[i];
		}
	const SSignal& operator[]( size_t i) const
		{
			if ( unlikely( i >= channels.size()) )
				throw out_of_range ("Signal index out of range");
			return channels[i];
		}

	SSignal& operator[]( const SChannel& h)
		{
			auto S = find( channels.begin(), channels.end(), h);
			if ( S == channels.end() )
				throw out_of_range (string ("Unknown channel ") + h.name());
			return *S;
		}
	const SSignal& operator[]( const SChannel& h) const
		{
			auto S = find( channels.begin(), channels.end(), h);
			if ( S == channels.end() )
				throw out_of_range (string ("Unknown channel ") + h.name());
			return *S;
		}


	enum TStatus : int_least32_t {
		bad_channel_count        = (1 << (COMMON_STATUS_BITS + 1)),
		bad_offset		 = (1 << (COMMON_STATUS_BITS + 2)),
		offsets_not_incteasing	 = (1 << (COMMON_STATUS_BITS + 3)),
		inoperable		 = (bad_header
					   | bad_numfld
					   | bad_datetime
					   | dup_channels
					   | sysfail
					   | too_many_channels
					   | bad_offset
					   | offsets_not_incteasing)
	};
	static string explain_status( int);

    private:
      // header...
	string	_episode,
		_session;

	TSubtype _subtype;

	size_t	_samplerate;

	FILE	*_f;
	char	*_line0;
	size_t	_line0_mallocked_bytes;

	int _parse_header();
	int _read_data();
};


} // namespace sigfile


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
