// ;-*-C++-*-
/*
 *       File name:  libsigfile/source.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-11
 *
 *         Purpose:  generic signal source
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_SOURCE_H
#define _SIGFILE_SOURCE_H

#include "source-base.hh"
#include "edf.hh"
//#include "other.hh"
#include "psd.hh"
#include "page.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;

namespace sigfile {



class CSource
  : public CHypnogram {
	enum class TType : int {
		unrecognised,
		bin, ascii,
		edf, edfplus,
	};

	TType	_type;  // rtti is evil
	CSource_base
		*_obj;

      // deleted
	CSource() = delete;
    public:
	CSource( const CSource&)
	      : CHypnogram (-1)
		{
			throw invalid_argument("nono");
		}
      // ctor
	CSource( const char* fname, int pagesize);
	CSource( CSource&& rv);
       ~CSource()
		{
			if ( not _obj->no_save_extra_files ) // quirky, eh?
				CHypnogram::save( make_fname_hypnogram());
			if ( _obj )
				delete _obj;
		}

	TType type() const
		{
			return _type;
		}

      // passthrough to obj
      // identification
	const char *filename() const
		{
			return _obj->filename();
		}

	const char* patient() const
		{
			return _obj->patient();
		}
	const char* recording_id() const
		{
			return _obj->recording_id();
		}
	const char* comment() const
		{
			return _obj->comment();
		}
	const char* episode() const
		{
			return _obj->episode();
		}
	const char* session() const
		{
			return _obj->session();
		}

      // metrics
	time_t start_time()
		{
			return _obj->start_time();
		}
	time_t end_time()
		{
			return _obj->end_time();
		}
	double recording_time()
		{
			return _obj->recording_time();
		}

      // channels
	list<SChannel> channel_list() const
		{
			return _obj->channel_list();
		}
	bool have_channel( const char* h)
		{
			return _obj->have_channel(h);
		}
	int channel_id( const char* h)
		{
			return _obj->channel_id(h);
		}
	const char* channel_by_id( int h)
		{
			return _obj->channel_by_id(h);
		}
	template <typename T>
	SChannel::TType	signal_type( T h)
		{
			return _obj->signal_type(h);
		}
	template <typename T>
	size_t samplerate( T h) const
		{
			return _obj->samplerate(h);
		}

      // annotations
	template <typename T>
	list<SAnnotation>&
	annotations( T h)
		{
			return _obj->annotations(h);
		}

	// artifacts
	template <typename T>
	SArtifacts&
	artifacts( T h)
		{
			return _obj->artifacts(h);
		}

	// filters
	template <typename T>
	SFilterPack&
	filters( T h)
		{
			return _obj->filters(h);
		}


      // setters
	int set_patient( const char* s)
		{
			return _obj->set_patient(s);
		}
	int set_recording_id( const char* s)
		{
			return _obj->set_recording_id(s);
		}
	int set_episode( const char* s)
		{
			return _obj->set_episode(s);
		}
	int set_session( const char* s)
		{
			return _obj->set_session(s);
		}
	int set_comment( const char* s)
		{
			return _obj->set_comment(s);
		}
	int set_start_time( time_t s)
		{
			return _obj->set_start_time(s);
		}


      // filenames
	string make_fname_hypnogram() const
		{
			return sigfile::make_fname_hypnogram( filename(), pagesize());
		}

	static TType source_file_type( const char* fname) __attribute__ ((pure));
};







// inline methods of CBinnedPower
inline size_t
CBinnedPower::n_bins() const
{
	if ( !_using_F )
		return 0;
	auto smplrt = _using_F->samplerate(_using_sig_no);
	return (smplrt * pagesize() + 1) / 2 / smplrt / bin_size;
}




} // namespace sigfile

#endif // _AGH_SOURCE_H

// eof
