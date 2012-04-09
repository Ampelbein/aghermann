// ;-*-C++-*-
/*
 *       File name:  libsigfile/edf.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  EDF class
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

#include "../libsigproc/sigproc.hh"

#include "channel.hh"
#include "source-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;


namespace sigfile {


// borrow this declaration from psd.hh
extern TFloat (*winf[])(size_t, size_t);





class CEDFFile
  : public CSource_base {

      // deleted
	bool operator==( const CEDFFile&) const = delete;
	CEDFFile() = delete;

    public:
      // ctor
	CEDFFile( const CEDFFile&)
	      : CSource_base("invalid")
		{
			throw invalid_argument("nono");
		}
	enum { no_mmap = 4, no_field_consistency_check = 8 };
	CEDFFile( const char *fname, int flags = 0);
	CEDFFile( CEDFFile&& rv);
      // dtor
       ~CEDFFile();

      // interface
	// status
	string explain_status() const
		{
			return explain_edf_status( _status);
		}
	// identification
	const char* filename() const
		{
			return _filename.c_str();
		}
	const char* subject() const
		{
			return _patient.c_str();
		}
	const char* recording_id() const
		{
			return header.recording_id;
		}
	const char* comment() const
		{
			return header.reserved;
		}
	const char* episode() const
		{
			return _episode.c_str();
		}
	const char* session() const
		{
			return _session.c_str();
		}
	// metrics
	const time_t& start_time() const
		{
			return _start_time;
		}
	const time_t& end_time() const
		{
			return _end_time;
		}
	double recording_time() const // in seconds
		{
			return (double) (n_data_records * data_record_size);
		}

	// setters
	int set_subject( const char* s);
	int set_recording_id( const char* s);
	int set_episode( const char* s);
	int set_session( const char* s);
	int set_comment( const char *s);
	int set_start_time( time_t s);

	// channels
	size_t n_channels() const
		{
			return signals.size();
		}
	list<SChannel>
	channel_list() const
		{
			list<SChannel> ret;
			for ( auto &H : signals )
				ret.push_back( H.channel);
			return ret;
		}
	bool
	have_channel( const char *h) const
		{
			return find( signals.cbegin(), signals.cend(), h) != signals.cend();
		}
	int
	channel_id( const char *h) const
		{
			for ( size_t i = 0; i < signals.size(); i++ )
				if ( signals[i].channel == h )
					return i;
			return -1;
		}
	const char*
	channel_by_id( int h) const
		{
			if ( h < (int)signals.size() )
				return signals[h].channel.c_str();
			return NULL;
		}

	SChannel::TType
	signal_type( int h) const
		{
			return (*this)[h].signal_type;
		}
	SChannel::TType
	signal_type( const char* h) const
		{
			return (*this)[h].signal_type;
		}

	size_t
	samplerate( int h) const
		{
			return (*this)[h].samples_per_record / data_record_size;
		}
	size_t
	samplerate( const char* h) const
		{
			return (*this)[h].samples_per_record / data_record_size;
		}

	list<SAnnotation>&
	annotations( int h)
		{
			return (*this)[h].annotations;
		}
	list<SAnnotation>&
	annotations( const char* h)
		{
			return (*this)[h].annotations;
		}

	// artifacts
	SArtifacts& artifacts( int h)
		{
			return (*this)[h].artifacts;
		}
	SArtifacts& artifacts( const char* h)
		{
			return (*this)[h].artifacts;
		}

	// filters
	SFilterPack& filters( int h)
		{
			return (*this)[h].filters;
		}
	SFilterPack& filters( const char* h)
		{
			return (*this)[h].filters;
		}


      // signal data extractors
	template <class Th>  // accommodates int or const char* as Th
	valarray<TFloat>
	get_region_original_( Th h,
			     size_t smpla, size_t smplz) const;
	valarray<TFloat>
	get_region_original( int h,
			     size_t smpla, size_t smplz) const
		{
			return get_region_original_<int>( h, smpla, smplz);
		}
	valarray<TFloat>
	get_region_original( const char* h,
			     size_t smpla, size_t smplz) const
		{
			return get_region_original_<const char*>( h, smpla, smplz);
		}

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
		{
			return get_region_original_<int>( h, timea, timez);
		}
	valarray<TFloat>
	get_region_original( const char* h,
			     float timea, float timez) const
		{
			return get_region_original_<const char*>( h, timea, timez);
		}

	template <class Th>
	valarray<TFloat>
	get_signal_original_( Th h) const
		{
			return get_region_original_(
				h, 0, n_data_records * (*this)[h].samples_per_record);
		}
	valarray<TFloat>
	get_signal_original( int h) const
		{
			return get_signal_original_( h);
		}
	valarray<TFloat>
	get_signal_original( const char* h) const
		{
			return get_signal_original_( h);
		}

	template <class Th>
	valarray<TFloat>
	get_region_filtered_( Th h,
			      size_t smpla, size_t smplz) const;
	valarray<TFloat>
	get_region_filtered( int h,
			     size_t smpla, size_t smplz) const
		{
			return get_region_filtered_( h, smpla, smplz);
		}
	valarray<TFloat>
	get_region_filtered( const char* h,
			     size_t smpla, size_t smplz) const
		{
			return get_region_filtered_( h, smpla, smplz);
		}
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
		{
			return get_region_filtered_( h, timea, timez);
		}
	valarray<TFloat>
	get_region_filtered( const char* h,
			     float timea, float timez) const
		{
			return get_region_filtered_( h, timea, timez);
		}
	template <class Th>
	valarray<TFloat>
	get_signal_filtered_( Th h) const
		{
			return get_region_filtered_(
				h,
				0, n_data_records * (*this)[h].samples_per_record);
		}

      // put signal
	template <class Th>
	int
	put_region_( Th h,
		     const valarray<TFloat>& src, size_t smpla, size_t smplz) const;
	int
	put_region( int h,
		    const valarray<TFloat>& src, size_t smpla, size_t smplz) const
		{
			return put_region_( h, src, smpla, smplz);
		}
	int
	put_region( const char* h,
		    const valarray<TFloat>& src, size_t smpla, size_t smplz) const
		{
			return put_region_( h, src, smpla, smplz);
		}

	template <class Th>
	int
	put_region_( Th h,
		     const valarray<TFloat>& src, float timea, float timez) const
		{
			size_t sr = samplerate(h);
			return put_region_(
				h, src, (size_t)(timea * sr), (size_t)(timez * sr));
		}
	int
	put_region( int h,
		    const valarray<TFloat>& src, float timea, float timez) const
		{
			return put_region_( h, src, timea, timez);
		}

	template <class Th>
	int
	put_signal_( Th h,
		     const valarray<TFloat>& src) const;
	int
	put_signal( int h,
		    const valarray<TFloat>& src) const
		{
			return put_signal_( h, src);
		}
	int
	put_signal( const char* h,
		    const valarray<TFloat>& src) const
		{
			return put_signal_( h, src);
		}

      // export
	int export_original( int h, const char *fname) const
		{
			return export_original_( h, fname);
		}
	int export_filtered( int h, const char *fname) const
		{
			return export_filtered_( h, fname);
		}
	int export_original( const char* h, const char *fname) const
		{
			return export_original_( h, fname);
		}
	int export_filtered( const char* h, const char *fname) const
		{
			return export_filtered_( h, fname);
		}
	template <class Th>
	int export_original_( Th h, const char *fname) const;
	template <class Th>
	int export_filtered_( Th h, const char *fname) const;


      // reporting & misc
	void write_ancillary_files();

	string details( bool channels_too = true) const;

	SFFTParamSet::TWinType af_dampen_window_type; // master copy

      // static fields (mmapped)
	struct SEDFHeader {
		char	*version_number,   //[ 8],
			*patient_id    ,   //[80],  // maps to subject name
			*recording_id  ,   //[80],  // maps to episode_name (session_name)
			*recording_date,   //[ 8],
			*recording_time,   //[ 8],
			*header_length ,   //[ 8],
			*reserved      ,   //[44],
			*n_data_records,   //[ 8],
			*data_record_size, //[ 8],
			*n_signals       ; //[ 4];
	};
	SEDFHeader header;

      // (relevant converted integers)
	size_t	n_data_records,
		data_record_size;

      // signals
	struct SSignal {
		struct SEDFSignalHeader {
			char	*label             ,//  [16],  // maps to channel
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
			channel;
		string	transducer_type,
			physical_dim,
			filtering_info,
			reserved;

		int	digital_min,
			digital_max;
		float	physical_min,
			physical_max,
			scale;
		size_t	samples_per_record;

		bool operator==( const char *h) const
			{
				return channel == h;
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
		signals;
	static size_t max_signals;

      // signal accessors
	SSignal& operator[]( size_t i)
		{
			if ( i >= signals.size() )
				throw out_of_range ("Signal index out of range");
			return signals[i];
		}
	const SSignal& operator[]( size_t i) const
		{
			if ( i >= signals.size() )
				throw out_of_range ("Signal index out of range");
			return signals[i];
		}
	SSignal& operator[]( const char *h)
		{
			auto S = find( signals.begin(), signals.end(), h);
			if ( S == signals.end() )
				throw out_of_range (string ("Unknown channel ") + h);
			return *S;
		}
	const SSignal& operator[]( const char *h) const
		{
			return (*const_cast<CEDFFile*>(this)) [h];
		}


	enum TStatus : int {
		ok			= 0,
		bad_header		= (1 <<  0),
		bad_version		= (1 <<  1),
		bad_numfld		= (1 <<  2),
		bad_recording		= (1 <<  3),
		date_unparsable		= (1 <<  4),
		time_unparsable		= (1 <<  5),
		nosession		= (1 <<  6),
		noepisode		= (1 <<  7),
		nonkemp_signaltype	= (1 <<  8),
		non1020_channel		= (1 <<  9),
		dup_channels		= (1 << 11),
		nogain			= (1 << 12),
		sysfail			= (1 << 13),
		too_many_signals	= (1 << 14),
		inoperable		= (bad_header
					   | bad_version
					   | bad_numfld
					   | bad_recording
					   | date_unparsable | time_unparsable
					   | dup_channels
					   | nogain
					   | sysfail
					   | too_many_signals)
	};
	static string explain_edf_status( int);

    private:
	time_t	_start_time,
		_end_time;

	string	_patient,
       // take care of file being named 'episode-1.edf'
		_episode,
       // loosely/possibly also use RecordingID as session
		_session;

	int _parse_header();

	size_t	_data_offset,
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

// eof
