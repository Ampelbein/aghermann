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





struct SChannel : public string {
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


class CMeasurement;



template <class T>
class agh::CSource
  : public T,
    public CHypnogram {

};


#endif // _AGH_SOURCE_H

// eof
