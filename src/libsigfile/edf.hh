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
//#include <array>
#include <stdexcept>
//#include <memory>

#include "../libexstrom/signal.hh"

#include "channel.hh"
#include "source-base.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;


namespace sigfile {


// borrow this declaration from psd.hh
extern double (*winf[])(size_t, size_t);





class CEDFFile
  : public CSource_base {

      // deleted
	bool operator==( const CEDFFile &o) const = delete;
	CEDFFile() = delete;

    public:
      // ctor
	CEDFFile( const CEDFFile&)
	      : CSource_base("invalid")
		{
			throw invalid_argument("nono");
		}
	CEDFFile( const char *fname);
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
	const char* patient() const
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
	time_t start_time() const
		{
			return _start_time;
		}
	time_t end_time() const
		{
			return _end_time;
		}
	double recording_time() const // in seconds
		{
			return (double) (n_data_records * data_record_size);
		}

	// setters
	int set_patient( const char* s);
	int set_recording_id( const char* s);
	int set_episode( const char* s);
	int set_session( const char* s);
	int set_comment( const char *s);
	int set_start_time( time_t s);

	// channels
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
	template <class T>
	SChannel::TType
	signal_type( T h) const
		{
			return (*this)[h].signal_type;
		}
	SChannel::TType signal_type( int h) const;
	SChannel::TType signal_type( const char* h) const;

	template <class T>
	size_t
	samplerate( T h) const
		{
			return (*this)[h].samples_per_record / data_record_size;
		}
	size_t samplerate( int) const;
	size_t samplerate( const char*) const;

	template <class T>
	list<SAnnotation>&
	annotations( T h)
		{
			return (*this)[h].annotations;
		}
	list<SAnnotation>& annotations( int);
	list<SAnnotation>& annotations( const char*);

	// artifacts
	template <class T>
	SArtifacts&
	artifacts( T h)
		{
			return (*this)[h].artifacts;
		}
	SArtifacts& artifacts( int);
	SArtifacts& artifacts( const char*);

	// filters
	template <typename T>
	SFilterPack&
	filters( T h)
		{
			return (*this)[h].filters;
		}
	SFilterPack& filters( int);
	SFilterPack& filters( const char*);


      // signal data extractors
	template <class Th>  // accommodates int or const char* as Th
	valarray<TFloat>
	get_region_original( Th h,
			     size_t smpla, size_t smplz) const;
	template <class Th>
	valarray<TFloat>
	get_region_original( Th h,
			     float timea, float timez) const
		{
			size_t sr = samplerate(h);
			return get_region_original(
				h, (size_t)(timea * sr), (size_t)(timez * sr));
		}
	template <class Th>
	valarray<TFloat>
	get_signal_original( Th h) const
		{
			return get_region_original(
				h, 0, n_data_records * (*this)[h].samples_per_record);
		}

	template <class Th>
	valarray<TFloat>
	get_region_filtered( Th h,
			     size_t smpla, size_t smplz) const;
	template <class Th>
	valarray<TFloat>
	get_region_filtered( Th h,
			     float timea, float timez) const
		{
			size_t sr = samplerate(h);
			return get_region_filtered(
				h, (size_t)(timea * sr), (size_t)(timez * sr));
		}
	template <class Th>
	valarray<TFloat>
	get_signal_filtered( Th h) const
		{
			return get_region_filtered(
				h,
				0, n_data_records * (*this)[h].samples_per_record);
		}

      // put signal
	template <class Th>
	void
	put_region( Th h,
		    const valarray<TFloat>& src, size_t smpla, size_t smplz) const;

	template <class Th>
	void
	put_region( Th h,
		    const valarray<TFloat>& src, float timea, float timez) const
		{
			size_t sr = samplerate(h);
			return put_region(
				h, src, (size_t)(timea * sr), (size_t)(timez * sr));
		}
	template <class Th>
	void
	put_signal( Th h,
		    const valarray<TFloat>& src) const;


    private:
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
	time_t	_start_time,
		_end_time;

	string	_patient,
       // take care of file being named 'episode-1.edf'
		_episode,
       // loosely/possibly also use RecordingID as session
		_session;

	struct SSignal {
		struct SEDFSignalHeader {
			char	*label             ,//  [16+1],  // maps to channel
				*transducer_type   ,//  [80+1],
				*physical_dim      ,//  [ 8+1],
				*physical_min      ,//  [ 8+1],
				*physical_max      ,//  [ 8+1],
				*digital_min       ,//  [ 8+1],
				*digital_max       ,//  [ 8+1],
				*filtering_info    ,//  [80+1],
				*samples_per_record,//  [ 8+1],
				*reserved          ;//  [32+1];
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

	SFFTParamSet::TWinType af_dampen_window_type; // master copy

	bool no_save_extra_files;
	void write_ancillary_files() const;

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


      // export
	template <class Th>
	int export_original( Th h, const char *fname) const;
	template <class Th>
	int export_filtered( Th h, const char *fname) const;


      // reporting & misc
	string details() const;

    private:
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

	int _parse_header();

	size_t	_data_offset,
		_fsize,
		_fld_pos,
		_total_samples_per_record;
	char* _get_next_field( char*&, size_t) throw (TStatus);
	int _put_next_field( char*, size_t);

	void	*_mmapping;

	void write_ancillary_files();
};


//template SChannel::TType sigfile::CEDFFile::signal_type( int);
//template SChannel::TType sigfile::CEDFFile::signal_type( const char*);




template <class A>
valarray<TFloat>
CEDFFile::get_region_original( A h,
			       size_t sa, size_t sz) const
{
	valarray<TFloat> recp;
	if ( unlikely (_status & (TStatus::bad_header | TStatus::bad_version)) ) {
		fprintf( stderr, "CEDFFile::get_region_original(): broken source \"%s\"\n", filename());
		return recp;
	}
	if ( sa >= sz || sz > samplerate(h) * recording_time() ) {
		fprintf( stderr, "CEDFFile::get_region_original() for \"%s\": bad region (%zu, %zu)\n",
			 filename(), sa, sz);
		return recp;
	}

	const SSignal& H = (*this)[h];
	size_t	r0    =                        (   sa) / H.samples_per_record,
		r_cnt = (size_t) ceilf( (float)(sz-sa) / H.samples_per_record);

	int16_t* tmp;
	tmp = (int16_t*)malloc( r_cnt * H.samples_per_record * 2);  // 2 is sizeof(sample) sensu edf

	while ( r_cnt-- )
		memcpy( &tmp[ r_cnt * H.samples_per_record ],

			(char*)_mmapping + _data_offset
			+ (r0 + r_cnt) * _total_samples_per_record * 2	// full records before
			+ H._at * 2,				// offset to our samples

			H.samples_per_record * 2);	// our precious ones

	recp.resize( sz - sa);

      // repackage for shipping
	size_t sa_off = sa - r0 * H.samples_per_record;
	for ( size_t s = 0; s < recp.size(); ++s )
		recp[s] = tmp[sa_off + s];

      // and scale
	recp *= H.scale;

	free( tmp);

	return recp;
}



template <class Th>
valarray<TFloat>
CEDFFile::get_region_filtered( Th h,
			       size_t smpla, size_t smplz) const
{
	valarray<TFloat> recp =
		get_region_original( h, smpla, smplz);
	if ( recp.size() == 0 )
		return valarray<TFloat> (0);
	// and zeromean
       	recp -= (recp.sum() / recp.size());

	const SSignal& H = (*this)[h];

      // artifacts
	size_t this_samplerate = H.samples_per_record / data_record_size;
	for ( auto &A : H.artifacts ) {
		size_t	run = A->second - A->first,
			window = min( run, this_samplerate),
			t;
		valarray<TFloat>
			W (run);

		if ( run > window ) {
			// construct a vector of multipliers using an INVERTED windowing function on the
			// first and last windows of the run
			size_t	t0;
			for ( t = 0; t < window/2; ++t )
				W[t] = (1 - winf[(size_t)H.artifacts.dampen_window_type]( t, window));
			t0 = run-window;  // start of the last window but one
			for ( t = window/2; t < window; ++t )
				W[t0 + t] = (1 - winf[(size_t)H.artifacts.dampen_window_type]( t, window));
			// AND, connect mid-first to mid-last windows (at lowest value of the window)
			TFloat minimum = winf[(size_t)H.artifacts.dampen_window_type]( window/2, window);
			W[ slice(window/2, run-window, 1) ] =
				(1. - minimum);
		} else  // run is shorter than samplerate (1 sec)
			for ( t = 0; t < window; ++t )
				W[t] = (1 - winf[(size_t)H.artifacts.dampen_window_type]( t, window));

		// now gently apply the multiplier vector onto the artifacts
		recp[ slice(A->first, run, 1) ] *= (W * (TFloat)H.artifacts.factor);
	}

      // filters
	if ( H.filters.low_pass_cutoff > 0. && H.filters.high_pass_cutoff > 0. ) {
		auto tmp (exstrom::band_pass( recp, this_samplerate,
					      H.filters.high_pass_cutoff, H.filters.low_pass_cutoff,
					      H.filters.low_pass_order, true));
		recp = tmp;
	} else {
		if ( H.filters.low_pass_cutoff > 0. ) {
			auto tmp (exstrom::low_pass( recp, this_samplerate,
						     H.filters.low_pass_cutoff, H.filters.low_pass_order, true));
			recp = tmp;
		}
		if ( H.filters.high_pass_cutoff > 0. ) {
			auto tmp (exstrom::high_pass( recp, this_samplerate,
						      H.filters.high_pass_cutoff, H.filters.high_pass_order, true));
			recp = tmp;
		}
	}

	switch ( H.filters.notch_filter ) {
	case SFilterPack::TNotchFilter::at50Hz:
		recp = exstrom::band_stop( recp, this_samplerate,
					   48, 52, 1, true);
	    break;
	case SFilterPack::TNotchFilter::at60Hz:
		recp = exstrom::band_stop( recp, this_samplerate,
					   58, 62, 1, true);
	    break;
	case SFilterPack::TNotchFilter::none:
	    break;
	}

	return recp;
}





template <class A>
void
CEDFFile::put_region( A h,
		      const valarray<TFloat>& src, size_t sa, size_t sz) const
{
	if ( unlikely (_status & (TStatus::bad_header | TStatus::bad_version)) ) {
		fprintf( stderr, "CEDFFile::put_region(): broken source \"%s\"\n", filename());
		return;
	}
	if ( sa >= sz || sz > samplerate(h) * recording_time() ) {
		fprintf( stderr, "CEDFFile::get_region_original() for \"%s\": bad region (%zu, %zu)\n",
			 filename(), sa, sz);
		return;
	}

	const SSignal& H = (*this)[h];
	size_t	r0    =                        (   sa) / H.samples_per_record,
		r_cnt = (size_t) ceilf( (float)(sz-sa) / H.samples_per_record);

	valarray<TFloat> src_copy = src / H.scale;
	valarray<int16_t> tmp (r_cnt * H.samples_per_record);  // 2 is sizeof(sample) sensu edf
	for ( size_t i = 0; i < (sz - sa); ++i )
		tmp[i] = src_copy[sa+i];

	size_t r;
	for ( r = 0; r < r_cnt - 1; ++r ) // minus one
		memcpy( (char*)_mmapping + _data_offset
			+ (r0 + r) * _total_samples_per_record * 2	// full records before
			+ H._at * 2,				// offset to our samples

			&tmp[ r * H.samples_per_record ],

			H.samples_per_record * 2);	// our precious ones
	// last record is underfull
	memcpy( (char*)_mmapping + _data_offset
		+ (r0 + r) * _total_samples_per_record * 2
		+ H._at * 2,

		&tmp[ r * H.samples_per_record ],

		(sz - r * H.samples_per_record) * 2);
}



template <class Th>
void
CEDFFile::put_signal( Th h,
		      const valarray<TFloat>& src) const
{
	size_t src_expected_size = n_data_records * (*this)[h].samples_per_record;
	if ( src.size() > src_expected_size )
		fprintf( stderr,
			 "put_signal: Source vector size (%zu) > n_samples in "
			 "EDF channel (%zu): truncating source\n", src.size(), src_expected_size);
	else if ( src.size() < src_expected_size )
		fprintf( stderr,
			 "put_signal: Source vector size (%zu) < n_samples in "
			 "EDF channel (%zu): remainder possibly stale\n", src.size(), src_expected_size);
	return put_region(
		h, src, 0, min(src.size(), src_expected_size));
}


template <class Th>
int
CEDFFile::export_original( Th h, const char *fname) const
{
	valarray<double> signal = get_signal_original<Th, double>( h);
	FILE *fd = fopen( fname, "w");
	if ( fd ) {
		for ( size_t i = 0; i < signal.size(); ++i )
			fprintf( fd, "%g\n", signal[i]);
		fclose( fd);
		return 0;
	} else
		return -1;
}


template <class Th>
int
CEDFFile::export_filtered( Th h, const char *fname) const
{
	valarray<double> signal = get_signal_filtered<Th, double>( h);
	FILE *fd = fopen( fname, "w");
	if ( fd ) {
		for ( size_t i = 0; i < signal.size(); ++i )
			fprintf( fd, "%g\n", signal[i]);
		fclose( fd);
		return 0;
	} else
		return -1;
}




} // namespace sigfile


#endif

// eof
