// ;-*-C++-*- *  Time-stamp: "2011-01-30 20:25:45 hmmr"
/*
 *       File name:  libagh/edf.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  EDF class
 *
 *         License:  GPL
 */

#ifndef _AGH_EDF_H
#define _AGH_EDF_H


#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cassert>
#include <cinttypes>
#include <cstring>
#include <ctime>
#include <string>
#include <valarray>
#include <vector>
#include <list>
#include <stdexcept>
#include <memory>

#include "../common.h"
#include "misc.hh"
#include "page.hh"
#include "../libexstrom/signal.hh"

using namespace std;

// borrow this declaration from psd.hh
extern double (*winf[])(size_t, size_t);

namespace NEDF {


string make_fname_hypnogram( const char*, size_t);
string make_fname_artifacts( const char*, const char*);
string make_fname_unfazer( const char*);
string make_fname_filters( const char*);



#define AGH_KNOWN_CHANNELS_TOTAL 78
extern const char* const __agh_System1020_channels[];

#define AGH_KNOWN_SIGNAL_TYPES 16
extern const char* const __agh_SignalTypeByKemp[];


inline int
compare_channels_for_sort( const char *a, const char *b)
{
	size_t ai = 0, bi = 0;
	while ( __agh_System1020_channels[ai] && strcmp( a, __agh_System1020_channels[ai]) )
		++ai;
	while ( __agh_System1020_channels[bi] && strcmp( b, __agh_System1020_channels[bi]) )
		++bi;
	return (ai < bi) ? -1 : (ai > bi) ? 1 : strcmp( a, b);
}

struct SChannel : public string {
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
			return compare_channels_for_sort( c_str(), rv.c_str()) == -1;
		}
};





class CMeasurement;

class CEDFFile
  : public CHypnogram {

    friend class CRecording;
    friend class CExpDesign;

	int	_status;

	string	_filename;

	CEDFFile();

    public:
	int status() const
		{
			return _status;
		}

	const char *filename() const
		{
			return _filename.c_str();
		}

      // static fields (raw)
	char	VersionNumber_raw    [ 8+1],
		PatientID_raw        [80+1],  // maps to subject name
		RecordingID_raw      [80+1],  // maps to episode_name (session_name)
		RecordingDate_raw    [ 8+1],
		RecordingTime_raw    [ 8+1],
		HeaderLength_raw     [ 8+1],
		Reserved_raw         [44+1],
		NDataRecords_raw     [ 8+1],
		DataRecordSize_raw   [ 8+1],
		NSignals_raw         [ 4+1];
      // (relevant converted integers)
	size_t	HeaderLength,
		NDataRecords,
		DataRecordSize,
		NSignals;
	struct tm
		timestamp_struct;
	time_t	start_time,
		end_time;

      // take care of file being named 'episode-1.edf'
	string	Episode;
      // loosely/possibly also use RecordingID as session
	string	Session;

	struct SSignal {
		char	Label               [16+1],  // maps to channel
			TransducerType      [80+1],
			PhysicalDim         [ 8+1],
			PhysicalMin_raw     [ 8+1],
			PhysicalMax_raw     [ 8+1],
			DigitalMin_raw      [ 8+1],
			DigitalMax_raw      [ 8+1],
			FilteringInfo       [80+1],
			SamplesPerRecord_raw[ 8+1],
			Reserved            [32+1];

		string	SignalType;
		SChannel
			Channel;

		int	DigitalMin,
			DigitalMax;
		float	PhysicalMin,
			PhysicalMax,
			Scale;
		size_t	SamplesPerRecord,
			_at;  // offset of our chunk within record, in samples

		bool operator==( const char *h) const
			{
				return h == Channel;
			}

		struct SUnfazer {
			size_t h; // offending channel
			double fac;
			SUnfazer( int _h, double _fac = 0.)
			      : h (_h), fac (_fac)
				{}
			bool operator==( const SUnfazer& rv) const
				{
					return h == rv.h;
				}

			string dirty_signature() const;
		};
		list<SUnfazer>
			interferences;

		list<pair<size_t,size_t>>
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

		size_t dirty_signature() const;

		SSignal()
		      : af_factor (.85), af_dampen_window_type (AGH_WT_WELCH),
			high_pass_cutoff (0.), low_pass_cutoff (0.),
			high_pass_order (1), low_pass_order (1)
			{}
	};
	vector<SSignal>
		signals;

	template <class A>
	size_t samplerate( A h) const
		{
			return (*this)[h].SamplesPerRecord / DataRecordSize;
		}

	bool have_unfazers() const;

      // ctors
	CEDFFile( const char *fname,
		  size_t scoring_pagesize,
		  TFFTWinType af_dampen_window_type = AGH_WT_WELCH);
	CEDFFile( CEDFFile&& rv);
       ~CEDFFile();

      // convenience identity comparison
	bool operator==( const CEDFFile &o) const
		{
			return	!strcmp( PatientID_raw, o.PatientID_raw) &&
				!strcmp( RecordingID_raw, o.RecordingID_raw);
		}
	bool operator==( const char* fname) const
		{
			return	_filename == fname;
		}

      // size
	size_t length() const // in seconds
		{
			return NDataRecords * DataRecordSize;
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
				if ( signals[i].Channel == h )
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
			return get_region_original<Th, Tw>( h,
							    0, NDataRecords * (*this)[h].SamplesPerRecord);
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
			return get_region_filtered<Th, Tw>( h,
							    0, NDataRecords * (*this)[h].SamplesPerRecord);
		}

	template <class Th>
	int export_original( Th h, const char *fname) const;
	template <class Th>
	int export_filtered( Th h, const char *fname) const;


      // reporting & misc
	string details() const;

	string make_fname_hypnogram() const
		{
			return NEDF::make_fname_hypnogram( filename(), pagesize());
		}
	string make_fname_artifacts( const char *channel) const
		{
			return NEDF::make_fname_artifacts( filename(), channel);
		}

	int write_header();

    private:
	int _parse_header();

	size_t	_data_offset,
		_fsize,
		_fld_pos,
		_total_samples_per_record;
	int _get_next_field( char*, size_t);
	int _put_next_field( char*, size_t);

	void	*_mmapping;
};








template <class A, class Tw>
valarray<Tw>
CEDFFile::get_region_original( A h,
			       size_t sa, size_t sz) const
{
	valarray<Tw> recp;
	if ( _status & (AGH_EDFCHK_BAD_HEADER | AGH_EDFCHK_BAD_VERSION) ) {
		fprintf( stderr, "CEDFFile::get_region_original(): broken source \"%s\"\n", filename());
		return recp;
	}
	if ( sa >= sz ) {
		fprintf( stderr, "CEDFFile::get_region_original() for \"%s\": bad region (%zu, %zu)\n",
			 filename(), sa, sz);
		return recp;
	}

	const SSignal& H = (*this)[h];
	size_t	r0    =                        (   sa) / H.SamplesPerRecord,
		r_cnt = (size_t) ceilf( (float)(sz-sa) / H.SamplesPerRecord);

	int16_t* tmp;
	tmp = (int16_t*)malloc( r_cnt * H.SamplesPerRecord * 2);  // 2 is sizeof(sample) sensu edf
	assert (tmp);

	while ( r_cnt-- )
		memcpy( &tmp[ r_cnt * H.SamplesPerRecord ],

			(char*)_mmapping + _data_offset
			+ (r0 + r_cnt) * _total_samples_per_record * 2	// full records before
			+ H._at * 2,				// offset to our samples

			H.SamplesPerRecord * 2);	// our precious ones

	recp.resize( sz - sa);

      // repackage for shipping
	size_t sa_off = sa - r0 * H.SamplesPerRecord;
	for ( size_t s = 0; s < recp.size(); ++s )
		recp[s] = tmp[sa_off + s];

      // and zeromean
	recp -= (recp.sum() / recp.size());
      // and scale
	recp *= H.Scale;

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
		valarray<Tw> offending_signal = get_region_original<size_t, Tw>( Od->h, smpla, smplz);
		if ( _status ) {
			fprintf( stderr, "CEDFFile::get_region_filtered(): bad offending_signal %zu\n", Od->h);
			return valarray<Tw> (0);
		}
		recp -= (offending_signal * (Tw)Od->fac);
	}

      // artifacts
	size_t this_samplerate = H.SamplesPerRecord / DataRecordSize;
	for ( auto A = H.artifacts.begin(); A != H.artifacts.end(); ++A ) {
		size_t	run = A->second - A->first,
			window = run < this_samplerate ? run : this_samplerate,
			t;
		valarray<Tw>
			W (run);

		if ( run > window ) {
			// construct a vector of multipliers using an INVERTED windowing function on the
			// first and last windows of the run
			size_t	t0;
			for ( t = 0; t < window/2; ++t )
				W[t] = (1 - winf[H.af_dampen_window_type]( t, window));
			t0 = run-window;  // start of the last window but one
			for ( t = window/2; t < window; ++t )
				W[t0 + t] = (1 - winf[H.af_dampen_window_type]( t, window));
			// AND, connect mid-first to mid-last windows (at lowest value of the window)
			Tw minimum = winf[H.af_dampen_window_type]( window/2, window);
			W[ slice(window/2, run-window, 1) ] =
				(1. - minimum);
		} else  // run is shorter than samplerate (1 sec)
			for ( t = 0; t < window; ++t )
				W[t] = (1 - winf[H.af_dampen_window_type]( t, window));

		// now gently apply the multiplier vector onto the artifacts
		recp[ slice(A->first, run, 1) ] *= (W * (Tw)H.af_factor);
	}

      // filters
	if ( H.low_pass_cutoff > 0. && H.high_pass_cutoff > 0. ) {
		auto tmp (NExstrom::band_pass( recp, this_samplerate,
					       H.high_pass_cutoff, H.low_pass_cutoff, H.high_pass_order, true));
		recp = tmp;
	} else {
		if ( H.low_pass_cutoff > 0. ) {
			auto tmp (NExstrom::low_pass( recp, this_samplerate,
						      H.low_pass_cutoff, H.low_pass_order, true));
			recp = tmp;
		}
		if ( H.high_pass_cutoff > 0. ) {
			auto tmp (NExstrom::high_pass( recp, this_samplerate,
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




bool channel_follows_1020( const char*);
const char* signal_type_following_Kemp( const char* channel);

inline bool
signal_type_is_fftable( const char *signal_type)
{
	return strcmp( signal_type, "EEG") == 0;
}
inline bool
signal_type_is_fftable( const string& signal_type)
{
	return signal_type == "EEG";
}


string explain_edf_status( int);


} // namespace NEDF


#endif

// EOF
