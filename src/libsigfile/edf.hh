/*
 *       File name:  libsigfile/edf.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  EDF class, also accommodating EDF+
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_EDF_H
#define _SIGFILE_EDF_H

#include <cinttypes>
#include <cstring>
#include <ctime>
#include <string>
#include <valarray>
#include <vector>
#include <list>
#include <map>
#include <stdexcept>

#include "sigproc/sigproc.hh"
#include "channel.hh"
#include "source-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;


namespace sigfile {



class CEDFFile
  : public CSource {

      // deleted
	bool operator==( const CEDFFile&) const = delete;
	CEDFFile() = delete;

    public:
	// subtype
	enum TSubtype {
		edf,
		edfplus_c,  // continuous
		edfplus_d   // discontinuous
	};
	TSubtype subtype() const
		{ return _subtype; }
	static const char*
	subtype_s( TSubtype t)
		{
			switch (t) {
			case edf:       return "edf";
			case edfplus_c: return "edf+c";
			case edfplus_d: return "edf+d";
			default:        return "(invalid)";
			}
		}
	const char*
	subtype_s() const
		{ return subtype_s( _subtype); }

      // ctor
	CEDFFile( const CEDFFile&)
	      : CSource("")
		{
			throw invalid_argument("nono");
		}
	enum TFlags {
		no_mmap				= 1<<3,
		no_cache			= 1<<4, // just considering
		no_field_consistency_check	= 1<<5,
	};
	// open existing
	CEDFFile (const string& fname, int flags = 0);
	// create new
	CEDFFile (const string& fname, TSubtype subtype_, int flags,
		  const list<pair<SChannel, size_t>>& channels,
		  size_t data_record_size = 1,
		  size_t n_data_records = 0);
	CEDFFile (CEDFFile&& rv);
      // dtor
       ~CEDFFile ();

      // interface
	// status
	string explain_status() const
		{ return explain_edf_status( _status); }

	// identification
	const char* filename() const
		{ return _filename.c_str(); }
	const char* patient_id() const
		{ return _patient_id.c_str(); }
	const char* recording_id() const
		{ return header.recording_id; }
	const char* comment() const
		{ return header.reserved; }
	const char* episode() const
		{ return _episode.c_str(); }
	const char* session() const
		{ return _session.c_str(); }

	// times
	time_t start_time() const
		{ return _start_time; }
	time_t end_time() const
		{ return _end_time; }
	time_t recording_time() const // in seconds
		{ return n_data_records * data_record_size; }

	// setters
	int set_patient_id( const string&);
	int set_recording_id( const string&);
	int set_episode( const string&);
	int set_session( const string&);
	int set_reserved( const string&);
	int set_comment( const string& s)
		{ return set_reserved( s); }

	int set_start_time( time_t);
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
		{ return (*this)[h].ucd.type(); }

	size_t
	samplerate( const int h) const
		{ return (*this)[h].samples_per_record / data_record_size; }

	list<SAnnotation>&
	annotations( const int h)
		{ return (*this)[h].annotations; }
	const list<SAnnotation>&
	annotations( const int h) const
		{ return (*this)[h].annotations; }

	list<SAnnotation>&
	annotations()
		{ return common_annotations; }
	const list<SAnnotation>&
	annotations() const
		{ return common_annotations; }

	// artifacts
	SArtifacts&
	artifacts( int h)
		{ return (*this)[h].artifacts; }
	const SArtifacts&
	artifacts( int h) const
		{ return (*this)[h].artifacts; }

	// filters
	SFilterPack&
	filters( const int h)
		{ return (*this)[h].filters; }
	const SFilterPack&
	filters( const int h) const
		{ return (*this)[h].filters; }


      // signal data extractors
	valarray<TFloat>
	get_region_original_smpl( int, size_t, size_t) const;

	valarray<TFloat>
	get_signal_original( const int h) const // there is a CSource::get_signal_original already, but this one is a little better
		{ return get_region_original_smpl(
				h, 0, n_data_records * (*this)[h].samples_per_record); }

	valarray<TFloat>
	get_region_filtered_smpl( int, size_t, size_t) const;

	valarray<TFloat>
	get_signal_filtered( const int h) const
		{ return get_region_filtered_smpl(
				h, 0, n_data_records * (*this)[h].samples_per_record); }

      // put signal
	int
	put_region_smpl( int, const valarray<TFloat>&, size_t) const;
	int
	put_region_sec( const int h, const valarray<TFloat>& src, const float offset) const
		{ return put_region_smpl( h, src, (size_t)(offset * samplerate(h))); }

	int
	put_signal( const int h, const valarray<TFloat>& src) const
		{ return put_region_smpl( h, src, 0); }

      // signal data info
	pair<TFloat, TFloat>
	get_real_original_signal_range( const int h) const
		{
			auto x = get_signal_original( h);
			return {x.min(), x.max()};
		}

	pair<TFloat, TFloat>
	get_max_original_signal_range( const int h) const
		{ return {(TFloat)channels[h].digital_min, (TFloat)channels[h].digital_max}; }


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
	resize( size_t new_records);

      // export
	int
	export_original( int h, const string& fname) const;
	int
	export_filtered( int h, const string& fname) const;

	int
	export_original_( int h, const string& fname) const;
	int
	export_filtered_( int h, const string& fname) const;


      // reporting & misc
	void write_ancillary_files();

	enum TEdfDetails { with_channels = 1, with_annotations = 2 };
	string details( int which) const;

	sigproc::TWinType af_dampen_window_type; // master copy

      // static fields (mmapped)
	struct SEDFHeader {
		char	*version_number	 ,   //[ 8],
			*patient_id    	 ,   //[80],  // maps to subject name
			*recording_id  	 ,   //[80],  // maps to episode_name (session_name)
			*recording_date	 ,   //[ 8],
			*recording_time	 ,   //[ 8],
			*header_length 	 ,   //[ 8],
			*reserved      	 ,   //[44],
			*n_data_records	 ,   //[ 8],
			*data_record_size,   //[ 8],
			*n_channels      ;   //[ 4];
	};
	SEDFHeader header;

      // (relevant converted integers)
	double	data_record_size;
	size_t	n_data_records;

      // channels
	struct SSignal {
		static const char* edf_annotations_label;
		struct SEDFSignalHeader {
			char	*label             ,//  [16],
				*transducer_type   ,//  [80],
				*physical_dim      ,//  [ 8],
				*physical_min      ,//  [ 8],
				*physical_max      ,//  [ 8],
				*digital_min       ,//  [ 8],
				*digital_max       ,//  [ 8],
				*filtering_info    ,//  [80],
				*samples_per_record,//  [ 8],
				*reserved          ;//  [32];
		};
		SEDFSignalHeader
	    		header;
		SChannel
			ucd; // Universal Channel Designation, епта
		string	transducer_type,
			physical_dim,
			filtering_info,
			reserved;

		int	digital_min,
			digital_max;
		double	physical_min,
			physical_max,
			scale;
		void set_physical_range( double, double);
		void set_digital_range( int16_t, int16_t);
		size_t	samples_per_record;

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
	    private:
		friend class CEDFFile;
		size_t	_at;  // offset of our chunk within record, in samples
	};
	vector<SSignal>
		channels;
	static size_t max_channels;

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
		ok			  = 0,
		bad_header		  = (1 <<  0),
		bad_version		  = (1 <<  1),
		bad_numfld		  = (1 <<  2),
		bad_recording		  = (1 <<  3),
		date_unparsable		  = (1 <<  4),
		time_unparsable		  = (1 <<  5),
		nosession		  = (1 <<  6),
		noepisode		  = (1 <<  7),
		nonkemp_signaltype	  = (1 <<  8),
		non1020_channel		  = (1 <<  9),
		dup_channels		  = (1 << 10),
		nogain			  = (1 << 11),
		sysfail			  = (1 << 12),
		too_many_channels	  = (1 << 13),
		nonconforming_patient_id  = (1 << 14),
		missing_patient_id        = (1 << 15),
		invalid_subject_details   = (1 << 16),
		file_truncated            = (1 << 17),
		trailing_junk             = (1 << 18),
		extra_patientid_subfields = (1 << 19),
		recognised_channel_conflicting_type = (1 << 20),

		inoperable		 = (bad_header
					   | bad_version
					   | bad_numfld
					   | bad_recording
					   | date_unparsable | time_unparsable
					   | dup_channels
					   | nogain
					   | sysfail
					   | too_many_channels
					   | file_truncated)
	};
	static string explain_edf_status( int);

    private:
	TSubtype _subtype;

	time_t	_start_time,
		_end_time;

	string	_patient_id, // this is trimmed, raw; parsed into SSubjectId fields
       // take care of file being named 'episode-1.edf'
		_episode,
       // loosely/possibly also use RecordingID as session
		_session;

	void _lay_out_header();
	int _parse_header();
	int _extract_embedded_annotations();

	size_t	header_length,
		_fsize,
		_fld_pos,
		_total_samples_per_record;
	char* _get_next_field( char*&, size_t) throw (TStatus);

	void	*_mmapping;
	int	_fd;

	vector<double>
		_record_offsets;
};


} // namespace sigfile


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

