// ;-*-C++-*-
/*
 *       File name:  libagh/source.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  generic signal source
 *
 *         License:  GPL
 */

#ifndef _AGH_SOURCE_H
#define _AGH_SOURCE_H

#include "../libedf/edf.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;



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





struct SChannel
  : public string {
      // static members
	static array<const char*, 78> system1020_channels;
	static array<const char*, 16> kemp_signal_types;
	static int compare( const char *a, const char *b) __attribute__ ((pure));
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



class agh::ISource {
    public:
      // identification
	virtual const char *filename()		const = 0;
	virtual const char *patient()		const = 0;
	virtual const char *recording_id()	const = 0;
	virtual const char *comment()		const = 0;

      // metrics
	virtual time_t start_time()		const = 0;
	virtual time_t end_time()		const = 0;
	virtual double recording_time()		const = 0;

      // channels
	virtual list<string> channel_list()	const = 0;
	virtual bool have_channel( const char*) const = 0;
	virtual size_t samplerate( const char*)	const = 0;

      // get samples
	template <class T>
	virtual valarray<T>
	get_region_original( const char* h,
			     size_t start_sample,
			     size_t end_sample)	const = 0;

	template <class T>
	virtual valarray<T>
	get_region_original( const char* h,
			     double start_time,
			     double end_time, ) const
		{
			return get_region_original<T>(
				h,
				start_time * samplerate(h),
				end_time   * samplerate(h));
		}

	template <class T>
	valarray<T>
	get_signal_original( const char* h) const
		{
			return get_region_original<T>(
				h,
				0., recording_time());
		}
      // put samples
	template <class T>
	int
	put_region( const char* h,
		    const valarray<T>& src,
		    size_t smpla, size_t smplz)	const = 0;
	template <class T>
	int
	put_signal( const char* h,
		    const valarray<T>& src)
		{
			return put_region<T>( h, src, 0, src.size());
		}
};


template <ISource::TType t>
class CGenericSource {

};

class agh::CSource
  : public CHypnogram {
	enum class TType : int {
		bin, ascii,
		edf, edfplus,
	};

	TType _type;
	union {
		CGenericSource<edf::CEDFFile>&
			edf;
		CGenericSource<edf::CEDFPlusFile>&
			edfplus;
	} _obj;

    public:
      // ctor
	CSource( const char* fname);

	TSourceType type() const
		{
			return _type;
		}

	const char *filename() const
		{
			return 
		}

};


#endif // _AGH_SOURCE_H

// eof
