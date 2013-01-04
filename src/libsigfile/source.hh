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
#include "page.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;

namespace sigfile {



class CSource
  : public CHypnogram {

	void operator=( const CSource&) = delete;
	CSource () = delete;
    public:
	enum class TType : int {
		unrecognised,
		bin, ascii,
		edf, edfplus,
	};

    private:
	TType	_type;  // rtti is evil
	CSource_base
		*_obj;
    public:
	CSource (const CSource&)
	      : CHypnogram (-1)
		{
			throw invalid_argument("nono");
		}
      // ctor
	enum { no_ancillary_files = 1 };
	CSource (const char* fname, size_t pagesize, int flags = 0);
	CSource (CSource&& rv);
       ~CSource ();

	TType
	type()			const { return _type; }

      // passthrough to obj
      // identification
	int
	flags()			const { return _obj->flags(); }

	int
	status()		const { return _obj->status(); }

	string
	explain_status()	const { return _obj->explain_status(); }

	string
	details()		const { return _obj->details(); }

	const char*
	filename()		const { return _obj->filename(); }

	const char*
	subject()		const { return _obj->subject(); }

	const char*
	recording_id()		const { return _obj->recording_id(); }

	const char*
	comment()		const { return _obj->comment(); }

	const char*
	episode()		const { return _obj->episode(); }

	const char*
	session()		const { return _obj->session(); }

      // date/time
	const time_t&
	start_time()		const { return _obj->start_time(); }

	const time_t&
	end_time()		const { return _obj->end_time(); }

	double
	recording_time()	const { return _obj->recording_time(); }

      // channels
	size_t
	n_channels()		const { return _obj->n_channels(); }

	list<SChannel>
	channel_list()		const { return _obj->channel_list(); }

	bool
	have_channel( const char* h)
				const { return _obj->have_channel(h); }
	int
	channel_id( const char* h)
				const { return _obj->channel_id(h); }

	const char*
	channel_by_id( int h)	const { return _obj->channel_by_id(h); }

	template <typename T>
	SChannel::TType
	signal_type( T h)	const { return _obj->signal_type(h); }

	template <typename T>
	size_t
	samplerate( T h)	const { return _obj->samplerate(h); }

      // annotations
	template <typename T>
	list<SAnnotation>&
	annotations( T h)	      { return _obj->annotations(h); }

	template <typename T>
	const list<SAnnotation>&
	annotations( T h)	const { return _obj->annotations(h); }

	// artifacts
	template <typename T>
	SArtifacts&
	artifacts( T h)		      { return _obj->artifacts(h); }
	template <typename T>
	const SArtifacts&
	artifacts( T h)		const { return _obj->artifacts(h); }

	// filters
	template <typename T>
	SFilterPack&
	filters( T h)		      { return _obj->filters(h); }
	template <typename T>
	const SFilterPack&
	filters( T h)		const { return _obj->filters(h); }

	template <typename T>
	unsigned long
	dirty_signature( T h)   const { return _obj->dirty_signature(h); }

      // setters
	int set_subject( const char* s)
		{
			return _obj->set_subject(s);
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

      // get_
	template <typename T>
	valarray<TFloat>
	get_region_original( T h,
			     size_t start_sample,
			     size_t end_sample)	const
		{
			return _obj->get_region_original(
				h, start_sample, end_sample);
		}

	template <typename T>
	valarray<TFloat>
	get_region_original( T h,
			     float seconds_off_start,
			     float seconds_off_end) const
		{
			return _obj->get_region_original(
				h, seconds_off_start, seconds_off_end);
		}

	template <typename T>
	valarray<TFloat>
	get_signal_original( T h) const
		{
			return _obj->get_signal_original(h);
		}

	template <typename T>
	valarray<TFloat>
	get_region_filtered( T h,
			     size_t start_sample,
			     size_t end_sample)	const
		{
			return _obj->get_region_filtered(
				h, start_sample, end_sample);
		}

	template <typename T>
	valarray<TFloat>
	get_region_filtered( T h,
			     float seconds_off_start,
			     float seconds_off_end) const
		{
			return _obj->get_region_filtered(
				h, seconds_off_start, seconds_off_end);
		}

	template <typename T>
	valarray<TFloat>
	get_signal_filtered( T h) const
		{
			return _obj->get_signal_filtered(h);
		}


      // put_
	template <typename T>
	int
	put_region( T h,
		    const valarray<TFloat>& src,
		    size_t offset) const
		{
			return _obj->put_region( h, src, offset);
		}
	template <typename T>
	int
	put_signal( T h,
		    const valarray<TFloat>& src)
		{
			return _obj->put_region( h, src, (size_t)0);
		}

      // export
	template <class Th>
	int
	export_original( Th h, const char *fname) const
		{
			return _obj->export_original( h, fname);
		}
	template <class Th>
	int
	export_filtered( Th h, const char *fname) const
		{
			return _obj->export_filtered( h, fname);
		}


      // filenames
	string make_fname_hypnogram() const
		{
			return sigfile::make_fname_hypnogram( filename(), pagesize());
		}

	static TType source_file_type( const char* fname) __attribute__ ((pure));

      // misc
	void write_ancillary_files()
		{
			_obj->write_ancillary_files();
		}
};


template <typename T = int>
struct SNamedChannel {
	CSource& source;
	T sig_no;
	SNamedChannel (CSource& source_, T sig_no_)
	      : source (source_),
		sig_no (sig_no_)
		{}
	SNamedChannel (const SNamedChannel&) = default;
};



} // namespace sigfile

#endif // _AGH_SOURCE_H

// eof
