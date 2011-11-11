// ;-*-C++-*-
/*
 *       File name:  libsigfile/source-iface.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-11
 *
 *         Purpose:  generic signal source
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_SOURCE_IFACE_H
#define _SIGFILE_SOURCE_IFACE_H

#include <string>
#include <valarray>

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;

namespace sigfile {


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






class ISource {
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
	virtual int channel_no( const char*)	const = 0;
	virtual size_t samplerate( const char*)	const = 0;

      // get samples
	// template <class T>
	// valarray<T>
	// get_region_original( const char* h,
	// 		     size_t start_sample,
	// 		     size_t end_sample)	const;
	template <class T>
	valarray<T>
	get_region_original( const char* h,
			     double start_time,
			     double end_time) const
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
		    size_t smpla, size_t smplz)	const;
	template <class T>
	int
	put_signal( const char* h,
		    const valarray<T>& src)
		{
			return put_region<T>( h, src, 0, src.size());
		}
};


} // namespace sigfile

#endif

// eof
