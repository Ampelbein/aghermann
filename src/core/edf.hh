// ;-*-C++-*- *  Time-stamp: "2010-11-09 02:21:12 hmmr"
/*
 *       File name:  edf.hh
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

using namespace std;


// borrow this declaration from psd.hh
extern double (*winf[])(size_t, size_t);


string make_fname_hypnogram( const char*);
string make_fname_artifacts( const char*, const char*);
string make_fname_unfazer( const char*);



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

		bool operator==( const char *h)
			{
				return h == Channel;
			}

		struct SUnfazer {
			int h; // offending channel
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
		string	artifacts;
		float	af_factor;
		TFFTWinType af_dampen_window_type;

		SSignal()
		      : af_factor (.85), af_dampen_window_type (AGH_WT_WELCH)
			{}

		size_t dirty_signature() const;
	};
	vector<SSignal>
		signals;

	bool have_unfazers() const;

      // ctors
	CEDFFile( const char *fname,
		  size_t scoring_pagesize,
		  TFFTWinType af_dampen_window_type = AGH_WT_WELCH);
	CEDFFile( CEDFFile&& rv);
	CEDFFile( const CEDFFile& rv)
		{
			CEDFFile( const_cast<CEDFFile&&>(rv));
		}

       ~CEDFFile();

	bool operator==( const CEDFFile &o) const
		{
			return	!strcmp( PatientID_raw, o.PatientID_raw) &&
				!strcmp( RecordingID_raw, o.RecordingID_raw);
		}
	bool operator==( const char* fname) const
		{
			return	_filename == fname;
		}

	size_t length() const // in seconds
		{
			return NDataRecords * DataRecordSize;
		}
	float n_pages() const
		{
			return (float)length() / pagesize();
		}

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

	bool have_channel( const char *h)
		{
			return find( signals.begin(), signals.end(), h) != signals.end();
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

	enum {
		ESigOK,
		ESigEBadSource,
		ESigERecordOutOfRange,
		ESigEBadChannel
	};
	template <class A, class T>  // accommodates int or const char* as A, double or float as T
	int get_signal_original( A h,
				 size_t r0, size_t nr,
				 valarray<T>& recp) const;

	template <class A, class T>
	int get_signal_filtered( A h,
				 size_t r0, size_t nr,
				 valarray<T>& recp) const;

	int assisted_score();

	string details() const;

	string make_fname_hypnogram() const
		{
			return ::make_fname_hypnogram( filename());
		}
	string make_fname_artifacts( const char *channel) const
		{
			return ::make_fname_artifacts( filename(), channel);
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






template <class A, class T> int
CEDFFile::get_signal_original( A h,
			       size_t r0, size_t nr,
			       valarray<T>& recp) const
{
	if ( _status & (AGH_EDFCHK_BAD_HEADER | AGH_EDFCHK_BAD_VERSION) ) {
		fprintf( stderr, "CEDFFile::get_signal_data(): broken source \"%s\"\n", filename());
		return ESigEBadSource;
	}
	if ( r0 + nr > NDataRecords || nr == 0 ) {
		fprintf( stderr, "CEDFFile::get_signal_data() for \"%s\": bad params r0 = %zu, nr = %zu\n",
			 filename(), r0, nr);
		return ESigERecordOutOfRange;
	}

	const SSignal& H = signals[h];
	int16_t* tmp;
	try {
		tmp = (int16_t*)malloc( nr * H.SamplesPerRecord * 2);  // 2 is sizeof(sample) sensu edf
		assert (tmp);
	} catch (out_of_range ex) {
		fprintf( stderr, "CEDFFile::get_signal_data() for \"%s\": %s\n",
			 filename(), ex.what());
		return ESigEBadChannel;
	}

	size_t r_cnt = nr;
	while ( r_cnt-- )
		memcpy( &tmp[ r_cnt * H.SamplesPerRecord ],

			(char*)_mmapping + _data_offset
			+ (r0 + r_cnt) * _total_samples_per_record * 2	// full records before
			+ H._at * 2,				// offset to our samples

			H.SamplesPerRecord * 2);	// our precious ones

	recp.resize( nr * H.SamplesPerRecord);

      // repackage for shipping
	for ( size_t s = 0; s < recp.size(); ++s )
		recp[s] = tmp[s];
      // and scale
	recp *= H.Scale;

	free( tmp);

	return ESigOK;
}


template <class A, class T> int
CEDFFile::get_signal_filtered( A h,
			       size_t r0, size_t nr,
			       valarray<T>& recp) const
{
	get_signal_original( h, r0, nr, recp);

	const SSignal& H = signals[h];

      // unfazers
	valarray<T> offending_signal;
	for ( auto Od = H.interferences.begin(); Od != H.interferences.end(); ++Od ) {
		int retval = get_signal_original( Od->h, r0, nr, offending_signal);
		if ( retval )
			return retval;
		recp -= (offending_signal * (T)Od->fac);
	}

      // artifacts
	size_t samplerate = H.SamplesPerRecord / DataRecordSize;
	for ( size_t sa = 0; sa < H.artifacts.size(); ++sa )
		if ( H.artifacts[sa] == 'x' ) {
			// find a contiguous artifact run
			size_t sz = sa + 1;
			while ( sz < H.artifacts.size() && H.artifacts[sz] == 'x' )
				++sz;
//				printf("x at %zu,%zu", sa, sz);

			valarray<T>
				W (samplerate * (sz - sa));

			// construct a vector of multipliers using an INVERTED windowing function on the
			// first and last seconds of the run
			size_t	t, t0;
			for ( t = 0; t < samplerate/2; ++t )
				W[t] = (1 - winf[H.af_dampen_window_type]( t, samplerate));
			t0 = (sz-sa-1) * samplerate;  // start of the last page but one
			for ( t = samplerate/2; t < samplerate; ++t )
				W[t0 + t] = (1 - winf[H.af_dampen_window_type]( t, samplerate));
			// AND, connect mid-first to mid-last seconds (at lowest value of the window)
			W[ slice(samplerate/2, (sz-sa-1)*samplerate, 1) ] =
				(1 - winf[H.af_dampen_window_type]( samplerate/2, samplerate));
//				printf( "  lowest = %g\n", 1 - winf[af_dampen_window_type]( samplerate/2, samplerate));
			// now gently apply the multiplier vector onto the artifacts
			recp[ slice(sa*samplerate, (sz-sa)*samplerate, 1) ] *= (W * (T)H.af_factor);

			sa = sz;
		}

	return ESigOK;
}


bool channel_follows_1020( const char*);
const char* signal_type_following_Kemp( const char* channel);

inline bool
signal_type_is_fftable( const char *signal_type)
{
	return strcmp( signal_type, "EEG") == 0;
}


string explain_edf_status( int);

#endif

// EOF
