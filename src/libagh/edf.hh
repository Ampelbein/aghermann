// ;-*-C++-*- *  Time-stamp: "2011-05-16 23:38:58 hmmr"
/*
 *       File name:  libagh/edf.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  EDF class
 *
 *         License:  GPL
 */

#ifndef _AGH_EDF_H
#define _AGH_EDF_H

#include <cinttypes>
#include <cstring>
#include <ctime>
#include <string>
#include <valarray>
#include <vector>
#include <list>
#include <map>
#include <array>
#include <stdexcept>
#include <memory>

#include "misc.hh"
#include "enums.hh"
#include "page.hh"
#include "../libexstrom/signal.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;


namespace agh {


// borrow this declaration from psd.hh
extern double (*winf[])(size_t, size_t);




template<class T>
string
make_fname__common( const T& _filename, bool hidden)
{
	string	fname_ (_filename);
	if ( fname_.size() > 4 && strcasecmp( &fname_[fname_.size()-4], ".edf") == 0 )
		fname_.erase( fname_.size()-4, 4);
	if ( hidden ) {
		size_t slash_at = fname_.rfind('/');
		if ( slash_at < fname_.size() )
			fname_.insert( slash_at+1, ".");
	}
	return fname_;
}

template<class T>
string
make_fname_hypnogram( const T& _filename, size_t pagesize)
{
	return make_fname__common( _filename, true)
		+ "-" + to_string( (long long unsigned)pagesize) + ".hypnogram";
}

template<class T>
string
make_fname_unfazer( const T& _filename)
{
	return make_fname__common( _filename, true)
		+ ".unf";
}

template<class T>
string
make_fname_artifacts( const T& _filename, const string& channel)
{
	return make_fname__common( _filename, true)
		+ "-" + channel + ".af";
}

template<class T>
string
make_fname_annotations( const T& _filename, const T& channel)
{
	return make_fname__common( _filename, true)
		+ "-" + channel + ".annotations";
}

template<class T>
string
make_fname_filters( const T& _filename)
{
	return make_fname__common( _filename, true)
		+ ".filters";
}







struct SChannel : public string {
      // static members
	static array<const char*, 78> system1020_channels;
	static array<const char*, 16> kemp_signal_types;
	static int compare( const char *a, const char *b);
	static bool channel_follows_system1020( const string& channel);
		// {
		// 	return find( system1020_channels.begin(), system1020_channels.end(), channel.c_str())
		// 		!= system1020_channels.end();
		// }
	static const char* signal_type_following_kemp( const string& signal);
	static bool signal_type_is_fftable( const string& signal_type)
		{
			return signal_type == "EEG";
		}


      // bound members
	bool follows_system1020() const
		{
			return channel_follows_system1020( *this);
		}

	SChannel( const char *v = "")
	      : string (v)
		{}
	SChannel( const string& v)
	      : string (v)
		{}
	SChannel( const SChannel& v)
	      : string (v.c_str())
		{}

	bool operator<( const SChannel& rv) const
		{
			return compare( c_str(), rv.c_str()) < 0;
		}

	// bool operator==( const SChannel& rv) const
	// 	{
	// 		return strcmp( c_str(), rv.c_str()) == 0;
	// 	}
};




class CMeasurement;

class CEDFFile
   : public CHypnogram {

    public:
	static string explain_edf_status( int);

    private:
	friend class CRecording;
	friend class CExpDesign;

	int 	_status;
	string	_filename;

	CEDFFile() = delete;

     public:
	TEdfStatus status() const
		{
			return (TEdfStatus)_status;
		}

	const char *filename() const
		{
			return _filename.c_str();
		}

      // static fields (raw)
	struct SEDFHeader {
		char	version_number   [ 8+1],
			patient_id       [80+1],  // maps to subject name
			recording_id     [80+1],  // maps to episode_name (session_name)
			recording_date   [ 8+1],
			recording_time   [ 8+1],
			header_length    [ 8+1],
			reserved         [44+1],
			n_data_records   [ 8+1],
			data_record_size [ 8+1],
			n_signals        [ 4+1];
	};

    // private:
    // 	SEDFHeader _header;

       // (relevant converted integers)
	size_t	n_data_records,
		data_record_size;
	time_t	start_time,
		end_time;

	string	patient,
       // take care of file being named 'episode-1.edf'
		episode,
       // loosely/possibly also use RecordingID as session
		session;

	struct SSignal {
		struct SEDFSignalHeader {
			char	label               [16+1],  // maps to channel
				transducer_type     [80+1],
				physical_dim        [ 8+1],
				physical_min        [ 8+1],
				physical_max        [ 8+1],
				digital_min         [ 8+1],
				digital_max         [ 8+1],
				filtering_info      [80+1],
				samples_per_record  [ 8+1],
				reserved            [32+1];
		};
	    // private:
	    // 	SEDFSignalHeader
	    // 		_header;
		string	signal_type;
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
				return h == channel;
			}

		// struct SUnfazer {
		// 	double fac;
		// 	SUnfazer( int _h, double _fac = 0.)
		// 	      : h (_h), fac (_fac)
		// 		{}
		// 	bool operator==( const SUnfazer& rv) const
		// 		{
		// 			return h == rv.h;
		// 		}

		// 	string dirty_signature() const;
		// };
		map<size_t, double>
			interferences;

//		using TRegion = pair<size_t, size_t>;  // come gcc 4.6, come!
		typedef pair<size_t, size_t> TRegion;
		list<TRegion>
			artifacts;
		float	af_factor;
		TFFTWinType af_dampen_window_type;
		void mark_artifact( size_t aa, size_t az);
		void clear_artifact( size_t aa, size_t az);

		float	high_pass_cutoff,
			low_pass_cutoff;
		unsigned
			high_pass_order,
			low_pass_order;

		struct SAnnotation {
			TRegion span;
			string text;
			enum class TOrigin : bool { edf, file };
			TOrigin origin;
			SAnnotation( size_t aa, size_t az, const string& an, TOrigin _origin)
			      : span (aa, az), text (an),
				origin (_origin)
				{}
		};
		list<SAnnotation>
			annotations;

		size_t dirty_signature() const;

		SSignal()
		      : af_factor (.85), af_dampen_window_type (TFFTWinType::welch),
			high_pass_cutoff (0.), low_pass_cutoff (0.),
			high_pass_order (1), low_pass_order (1)
			{}
	    private:
		friend class CEDFFile;
		size_t	_at;  // offset of our chunk within record, in samples
	};

	vector<SSignal>
		signals;

	template <class A>
	size_t samplerate( A h) const
		{
			return (*this)[h].samples_per_record / data_record_size;
		}

	bool have_unfazers() const;

      // ctors
	CEDFFile( const char *fname,
		  size_t scoring_pagesize,
		  TFFTWinType af_dampen_window_type = TFFTWinType::welch);
	CEDFFile( CEDFFile&& rv);
       ~CEDFFile();

       // convenience identity comparison
	bool operator==( const CEDFFile &o) const
		{
			return	patient == o.patient &&
				session == o.session &&
				episode == o.episode;
		}
	bool operator==( const char* fname) const
		{
			return	_filename == fname;
		}

       // size
	size_t length() const // in seconds
		{
			return n_data_records * data_record_size;
		}
	float n_pages() const
		{
			return (float)length() / pagesize();
		}

       // signal accessors
	SSignal& operator[]( size_t i)
		{
			if ( i >= signals.size() ) {
				UNIQUE_CHARP(_);
				if ( asprintf( &_, "Signal index %zu out of range", i) )
					;
				throw out_of_range (_);
			}
			return signals[i];
		}
	const SSignal& operator[]( size_t i) const
		{
			if ( i >= signals.size() ) {
				UNIQUE_CHARP(_);
				if ( asprintf( &_, "Signal index %zu out of range", i) )
					;
				throw out_of_range (_);
			}
			return signals[i];
		}
	SSignal& operator[]( const char *h)
		{
			auto S = find( signals.begin(), signals.end(), h);
			if ( S == signals.end() )
				throw out_of_range (string ("Unknown channel") + h);
			return *S;
		}
	const SSignal& operator[]( const char *h) const
		{
			return (*const_cast<CEDFFile*>(this)) [h];
		}

	bool have_channel( const char *h) const
		{
			return find( signals.cbegin(), signals.cend(), h) != signals.cend();
		}
	bool have_channel( int h) const
		{
			return h >= 0 && (size_t)h < signals.size();
		}
	int which_channel( const char *h) const
		{
			for ( size_t i = 0; i < signals.size(); i++ )
				if ( signals[i].channel == h )
					return i;
			return -1;
		}

       // signal data extractors
	template <class Th, class Tw>  // accommodates int or const char* as Th, double or float as Tw
	valarray<Tw> get_region_original( Th h,
					  size_t smpla, size_t smplz) const;
	template <class Th, class Tw>
	valarray<Tw> get_region_original( Th h,
					  float timea, float timez) const
		{
			size_t sr = samplerate(h);
			return get_region_original<Th, Tw>( h,
							    (size_t)(timea * sr),
							    (size_t)(timez * sr));
		}
	template <class Th, class Tw>
	valarray<Tw> get_signal_original( Th h) const
		{
			return get_region_original<Th, Tw>(
				h,
				0, n_data_records * (*this)[h].samples_per_record);
		}

	template <class Th, class Tw>
	valarray<Tw> get_region_filtered( Th h,
					  size_t smpla, size_t smplz) const;
	template <class Th, class Tw>
	valarray<Tw> get_region_filtered( Th h,
					  float timea, float timez) const
		{
			size_t sr = samplerate(h);
			return get_region_filtered<Th, Tw>( h,
							    (size_t)(timea * sr),
							    (size_t)(timez * sr));
		}
	template <class Th, class Tw>
	valarray<Tw> get_signal_filtered( Th h) const
		{
			return get_region_filtered<Th, Tw>(
				h,
				0, n_data_records * (*this)[h].samples_per_record);
		}

	template <class Th>
	int export_original( Th h, const char *fname) const;
	template <class Th>
	int export_filtered( Th h, const char *fname) const;


       // reporting & misc
	string details() const;

	string make_fname_hypnogram() const
		{
			return agh::make_fname_hypnogram( _filename, pagesize());
		}
	string make_fname_artifacts( const string& channel) const
		{
			return agh::make_fname_artifacts( _filename, channel);
		}
	string make_fname_annotations( const string& channel) const
		{
			return agh::make_fname_annotations( _filename, channel);
		}

//	int write_header();

     private:
	int _parse_header();

	size_t	_data_offset,
		_fsize,
		_fld_pos,
		_total_samples_per_record;
	char* _get_next_field( char*, size_t) throw (TEdfStatus);
	int _put_next_field( char*, size_t);

	void	*_mmapping;
};








template <class A, class Tw>
valarray<Tw>
CEDFFile::get_region_original( A h,
			       size_t sa, size_t sz) const
{
	valarray<Tw> recp;
	if ( _status & (TEdfStatus::bad_header | TEdfStatus::bad_version) ) {
		fprintf( stderr, "CEDFFile::get_region_original(): broken source \"%s\"\n", filename());
		return recp;
	}
	if ( sa >= sz ) {
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

      // and zeromean
	recp -= (recp.sum() / recp.size());
      // and scale
	recp *= H.scale;

	free( tmp);

	return recp;
}



template <class Th, class Tw>
valarray<Tw>
CEDFFile::get_region_filtered( Th h,
			       size_t smpla, size_t smplz) const
{
	valarray<Tw> recp =
		get_region_original<Th, Tw>( h, smpla, smplz);
	if ( recp.size() == 0 )
		return valarray<Tw> (0);

	const SSignal& H = (*this)[h];

      // unfazers
	for ( auto Od = H.interferences.begin(); Od != H.interferences.end(); ++Od ) {
		//	for ( auto Od : H.interferences ) {
		valarray<Tw> offending_signal = get_region_original<size_t, Tw>( Od->first, smpla, smplz);
		if ( _status ) {
			fprintf( stderr, "CEDFFile::get_region_filtered(): bad offending_signal index %zu\n", Od->first);
			return valarray<Tw> (0);
		}
		recp -= (offending_signal * (Tw)Od->second);
	}

      // artifacts
	size_t this_samplerate = H.samples_per_record / data_record_size;
	for ( auto A = H.artifacts.begin(); A != H.artifacts.end(); ++A ) {
		size_t	run = A->second - A->first,
			window = min( run, this_samplerate),
			t;
		valarray<Tw>
			W (run);

		if ( run > window ) {
			// construct a vector of multipliers using an INVERTED windowing function on the
			// first and last windows of the run
			size_t	t0;
			for ( t = 0; t < window/2; ++t )
				W[t] = (1 - winf[(size_t)H.af_dampen_window_type]( t, window));
			t0 = run-window;  // start of the last window but one
			for ( t = window/2; t < window; ++t )
				W[t0 + t] = (1 - winf[(size_t)H.af_dampen_window_type]( t, window));
			// AND, connect mid-first to mid-last windows (at lowest value of the window)
			Tw minimum = winf[(size_t)H.af_dampen_window_type]( window/2, window);
			W[ slice(window/2, run-window, 1) ] =
				(1. - minimum);
		} else  // run is shorter than samplerate (1 sec)
			for ( t = 0; t < window; ++t )
				W[t] = (1 - winf[(size_t)H.af_dampen_window_type]( t, window));

		// now gently apply the multiplier vector onto the artifacts
		recp[ slice(A->first, run, 1) ] *= (W * (Tw)H.af_factor);
	}

      // filters
	if ( H.low_pass_cutoff > 0. && H.high_pass_cutoff > 0. ) {
		auto tmp (exstrom::band_pass( recp, this_samplerate,
					      H.high_pass_cutoff, H.low_pass_cutoff, H.high_pass_order, true));
		recp = tmp;
	} else {
		if ( H.low_pass_cutoff > 0. ) {
			auto tmp (exstrom::low_pass( recp, this_samplerate,
						     H.low_pass_cutoff, H.low_pass_order, true));
			recp = tmp;
		}
		if ( H.high_pass_cutoff > 0. ) {
			auto tmp (exstrom::high_pass( recp, this_samplerate,
						      H.high_pass_cutoff, H.high_pass_order, true));
			recp = tmp;
		}
	}

	return recp;
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






} // namespace agh


#endif

// EOF
