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
	CEDFFile (const char *fname, int flags = 0);
	// create new
	CEDFFile (const char *fname, TSubtype subtype_, int flags,
		  const list<pair<string, size_t>>& channels,
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
	int set_patient_id( const char*);
	int set_recording_id( const char*);
	int set_episode( const char*);
	int set_session( const char*);
	int set_reserved( const char*);
	int set_comment( const char* s)
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
				ret.push_back( H.label);
			return ret;
		}
	bool
	have_channel( const char *h) const
		{ return find( channels.cbegin(), channels.cend(), h) != channels.cend(); }
	int
	channel_id( const char *h) const
		{
			for ( size_t i = 0; i < channels.size(); i++ )
				if ( channels[i].label == h )
					return i;
			return -1;
		}
	const char*
	channel_by_id( int h) const
		{
			if ( likely (h < (int)channels.size()) )
				return channels[h].label.c_str();
			return nullptr;
		}

	SChannel::TType
	signal_type( int h) const
		{ return (*this)[h].signal_type; }
	SChannel::TType
	signal_type( const char* h) const
		{ return (*this)[h].signal_type; }

	size_t
	samplerate( int h) const
		{ return (*this)[h].samples_per_record / data_record_size; }
	size_t
	samplerate( const char* h) const
		{ return (*this)[h].samples_per_record / data_record_size; }

	list<SAnnotation>&
	annotations( int h)
		{ return (*this)[h].annotations; }
	list<SAnnotation>&
	annotations( const char* h)
		{ return (*this)[h].annotations; }
	const list<SAnnotation>&
	annotations( int h) const
		{ return (*this)[h].annotations; }
	const list<SAnnotation>&
	annotations( const char* h) const
		{ return (*this)[h].annotations; }

	// artifacts
	SArtifacts&
	artifacts( int h)
		{ return (*this)[h].artifacts; }
	SArtifacts&
	artifacts( const char* h)
		{ return (*this)[h].artifacts; }
	const SArtifacts&
	artifacts( int h) const
		{ return (*this)[h].artifacts; }
	const SArtifacts&
	artifacts( const char* h) const
		{ return (*this)[h].artifacts; }

	// filters
	SFilterPack&
	filters( int h)
		{ return (*this)[h].filters; }
	SFilterPack&
	filters( const char* h)
		{ return (*this)[h].filters; }
	const SFilterPack&
	filters( int h) const
		{ return (*this)[h].filters; }
	const SFilterPack&
	filters( const char* h) const
		{ return (*this)[h].filters; }


      // signal data extractors
	template <class Th>  // accommodates int or const char* as Th
	valarray<TFloat>
	get_region_original_( Th h, size_t smpla, size_t smplz) const;
	valarray<TFloat>
	get_region_original( int h, size_t smpla, size_t smplz) const
		{ return get_region_original_<int>( h, smpla, smplz); }
	valarray<TFloat>
	get_region_original( const char* h, size_t smpla, size_t smplz) const
		{ return get_region_original_<const char*>( h, smpla, smplz); }

	template <class Th>
	valarray<TFloat>
	get_region_original_( Th h,
			      float timea, float timez) const
		{
			size_t sr = samplerate(h);
			return get_region_original_(
				h, (size_t)(timea * sr), (size_t)(timez * sr));
		}
	valarray<TFloat>
	get_region_original( int h,
			     float timea, float timez) const
		{ return get_region_original_<int>( h, timea, timez); }
	valarray<TFloat>
	get_region_original( const char* h,
			     float timea, float timez) const
		{ return get_region_original_<const char*>( h, timea, timez); }

	template <class Th>
	valarray<TFloat>
	get_signal_original_( Th h) const
		{ return get_region_original_( h, 0, n_data_records * (*this)[h].samples_per_record); }
	valarray<TFloat>
	get_signal_original( int h) const
		{ return get_signal_original_( h); }
	valarray<TFloat>
	get_signal_original( const char* h) const
		{ return get_signal_original_( h); }

	template <class Th>
	valarray<TFloat>
	get_region_filtered_( Th h,
			      size_t smpla, size_t smplz) const;
	valarray<TFloat>
	get_region_filtered( int h,
			     size_t smpla, size_t smplz) const
		{ return get_region_filtered_( h, smpla, smplz); }
	valarray<TFloat>
	get_region_filtered( const char* h,
			     size_t smpla, size_t smplz) const
		{ return get_region_filtered_( h, smpla, smplz); }
	template <class Th>
	valarray<TFloat>
	get_region_filtered_( Th h,
			      float timea, float timez) const
		{
			size_t sr = samplerate(h);
			return get_region_filtered_(
				h, (size_t)(timea * sr), (size_t)(timez * sr));
		}
	valarray<TFloat>
	get_region_filtered( int h,
			     float timea, float timez) const
		{ return get_region_filtered_( h, timea, timez); }
	valarray<TFloat>
	get_region_filtered( const char* h,
			     float timea, float timez) const
		{ return get_region_filtered_( h, timea, timez); }
	template <class Th>
	valarray<TFloat>
	get_signal_filtered_( Th h) const
		{ return get_region_filtered_( h, 0, n_data_records * (*this)[h].samples_per_record); }

      // put signal
	template <class Th>
	int
	put_region_( Th h, const valarray<TFloat>& src, size_t offset) const;
	int
	put_region( int h, const valarray<TFloat>& src, size_t offset) const
		{ return put_region_( h, src, offset); }
	int
	put_region( const char* h, const valarray<TFloat>& src, size_t offset) const
		{ return put_region_( h, src, offset); }

	template <class Th>
	int
	put_region_( Th h, const valarray<TFloat>& src, float offset) const
		{ return put_region_( h, src, (size_t)(offset * samplerate(h))); }
	int
	put_region( int h, const valarray<TFloat>& src, float offset) const
		{ return put_region_( h, src, offset); }

	template <class Th>
	int
	put_signal_( Th h, const valarray<TFloat>& src) const;
	int
	put_signal( int h, const valarray<TFloat>& src) const
		{ return put_signal_( h, src); }
	int
	put_signal( const char* h, const valarray<TFloat>& src) const
		{ return put_signal_( h, src); }

      // signal data info
	pair<TFloat, TFloat>
	get_real_original_signal_range( const char* h) const
		{ return get_real_original_signal_range( channel_id(h)); }
	pair<TFloat, TFloat>
	get_real_original_signal_range( int h) const
		{
			auto x = get_signal_original( h);
			return {x.min(), x.max()};
		}

	pair<TFloat, TFloat>
	get_max_original_signal_range( const char* h) const
		{ return get_max_original_signal_range( channel_id(h)); }
	pair<TFloat, TFloat>
	get_max_original_signal_range( int h) const
		{ return {(TFloat)channels[h].digital_min, (TFloat)channels[h].digital_max}; }


	pair<TFloat, TFloat>
	get_real_filtered_signal_range( const char* h) const
		{ return get_real_filtered_signal_range( channel_id(h)); }
	pair<TFloat, TFloat>
	get_real_filtered_signal_range( int h) const
		{
			auto x = get_signal_filtered( h);
			return {x.min(), x.max()};
		}

	pair<TFloat, TFloat>
	get_max_filtered_signal_range( const char* h) const
		{ return get_max_filtered_signal_range( channel_id(h)); }
	pair<TFloat, TFloat>
	get_max_filtered_signal_range( int h) const
		{
			auto x = get_signal_filtered( h);
			return {x.min(), x.max()};   // an approximate
		}

      // adjust capacity
	size_t
	resize( size_t new_records);

      // export
	int
	export_original( int h, const char *fname) const
		{ return export_original_( h, fname); }
	int
	export_filtered( int h, const char *fname) const
		{ return export_filtered_( h, fname); }
	int
	export_original( const char* h, const char *fname) const
		{ return export_original_( h, fname); }
	int
	export_filtered( const char* h, const char *fname) const
		{ return export_filtered_( h, fname); }
	template <class Th>
	int export_original_( Th h, const char *fname) const;
	template <class Th>
	int export_filtered_( Th h, const char *fname) const;


      // reporting & misc
	void write_ancillary_files();

	string details( bool channels_too = true) const;

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
	size_t	data_record_size,
		n_data_records;

      // channels
	struct SSignal {
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
		string	signal_type_s; // although SChannel:: has all known types including "(unknown)", some
				       // users will freak out and want their own
		SChannel::TType
			signal_type;
		SChannel
			label;
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

		bool operator==( const char *h) const
			{
				return label == h;
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
	SSignal& operator[]( const char *h)
		{
			auto S = find( channels.begin(), channels.end(), h);
			if ( S == channels.end() )
				throw out_of_range (string ("Unknown channel ") + h);
			return *S;
		}
	const SSignal& operator[]( const char *h) const
		{
			return (*const_cast<CEDFFile*>(this)) [h];
		}


	enum TStatus : int {
		ok			 = 0,
		bad_header		 = (1 <<  0),
		bad_version		 = (1 <<  1),
		bad_numfld		 = (1 <<  2),
		bad_recording		 = (1 <<  3),
		date_unparsable		 = (1 <<  4),
		time_unparsable		 = (1 <<  5),
		nosession		 = (1 <<  6),
		noepisode		 = (1 <<  7),
		nonkemp_signaltype	 = (1 <<  8),
		non1020_channel		 = (1 <<  9),
		dup_channels		 = (1 << 11),
		nogain			 = (1 << 12),
		sysfail			 = (1 << 13),
		too_many_channels	 = (1 << 14),
		nonconforming_patient_id = (1 << 15),
		inoperable		 = (bad_header
					   | bad_version
					   | bad_numfld
					   | bad_recording
					   | date_unparsable | time_unparsable
					   | dup_channels
					   | nogain
					   | sysfail
					   | too_many_channels)
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

	size_t	header_length,
		_fsize,
		_fld_pos,
		_total_samples_per_record;
	char* _get_next_field( char*&, size_t) throw (TStatus);

	void	*_mmapping;
	int	_fd;
};




#include "edf.ii"

} // namespace sigfile


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

