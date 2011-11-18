// ;-*-C++-*-
/*
 *       File name:  libsigfile/source-iface.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-12
 *
 *         Purpose:  generic signal source (interface class)
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_SOURCE_BASE_H
#define _SIGFILE_SOURCE_BASE_H

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include "channel.hh"
#include "psd.hh"

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
make_fname_artifacts( const T& _filename, const SChannel& channel)
{
	return make_fname__common( _filename, true)
		+ "-" + channel + ".af";
}

template<class T>
string
make_fname_annotations( const T& _filename, const SChannel& channel)
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







// using TRegion = class pair<size_t, size_t>;  // come gcc 4.7, come!
typedef pair<size_t, size_t> TRegion;

struct SArtifacts {
	SArtifacts()
	      : factor (.95),
		dampen_window_type (SFFTParamSet::TWinType::welch)
		{}

	list<TRegion>
		obj;
	float	factor;
	SFFTParamSet::TWinType
		dampen_window_type;

	list<TRegion>&
	operator() ()
		{
			return obj;
		}
	const list<TRegion>&
	operator() () const
		{
			return obj;
		}

	void mark_artifact( size_t aa, size_t az);
	void clear_artifact( size_t aa, size_t az);
	size_t dirty_signature() const;
};





struct SAnnotation {
	TRegion span;
	string label;
	// enum class TOrigin : bool { internal, external };
	// TOrigin origin;

	SAnnotation( size_t aa, size_t az, const string& l)
		: span (aa, az), label (l)
//		  origin (_origin)
		{}

	bool operator==( const SAnnotation& rv) const
		{
			return span == rv.span && label == rv.label; // && origin == rv.origin;
		}
};

inline void
mark_annotation( list<SAnnotation>& annotations, size_t aa, size_t az, const char* label)
{
	annotations.emplace_back( aa, az, label);
}





struct SFilterPack {
	SFilterPack()
	      : high_pass_cutoff (0.),
		low_pass_cutoff (0.),
		high_pass_order (0),
		low_pass_order (0),
		notch_filter (TNotchFilter::none)
		{}

	bool have_filters() const
	{
		return low_pass_cutoff > 0. || high_pass_cutoff > 0. ||
			notch_filter != SFilterPack::TNotchFilter::none;
	}

	float	high_pass_cutoff,
		low_pass_cutoff;
	unsigned
		high_pass_order,
		low_pass_order;

	enum TNotchFilter : int {
		none, at50Hz, at60Hz
	};
	TNotchFilter
		notch_filter;
};






class CSource_base {
    protected:
	string	_filename;
	int	_status;
    public:
	CSource_base() = delete;
	CSource_base( const CSource_base&) = delete;
	CSource_base( const string& fname)
	      : _filename (fname),
		_status (0),
		no_save_extra_files (false)
		{}
	CSource_base( CSource_base&& rv);

	int status() const
		{
			return _status;
		}
	virtual string explain_status()			const = 0;
	virtual string details()			const = 0;

      // identification
	const char* filename() const
		{
			return _filename.c_str();
		}
	virtual const char* subject()			const = 0;
	virtual const char* recording_id()		const = 0;
	virtual const char* comment()			const = 0;
	// probably parsed out of recording_id
	virtual const char* episode()			const = 0;
	virtual const char* session()			const = 0;

      // metrics
	virtual const time_t& start_time()		const = 0;
	virtual const time_t& end_time()		const = 0;
	virtual double recording_time()			const = 0;

      // channels
	virtual size_t n_channels()			const = 0;
	virtual list<SChannel> channel_list()		const = 0;
	virtual bool have_channel( const char*) 	const = 0;
	virtual int channel_id( const char*)		const = 0;
	virtual const char* channel_by_id( int)		const = 0;
	virtual SChannel::TType
	signal_type( const char*)			const = 0;
	virtual SChannel::TType
	signal_type( int)				const = 0;
	virtual size_t samplerate( const char*)		const = 0;
	virtual size_t samplerate( int)			const = 0;

	// the following methods are pass-through:
	// annotations
	virtual list<SAnnotation>&
	annotations( const char*)		      = 0;
	virtual list<SAnnotation>&
	annotations( int)			      = 0;

	// artifacts
	virtual SArtifacts&
	artifacts( const char*)			      = 0;
	virtual SArtifacts&
	artifacts( int)				      = 0;

	// filters
	virtual SFilterPack&
	filters( const char*)			      = 0;
	virtual SFilterPack&
	filters( int)				      = 0;

      // setters
	virtual int set_subject( const char*)	      = 0;
	virtual int set_recording_id( const char*)    = 0;
	virtual int set_episode( const char*)	      = 0;
	virtual int set_session( const char*)	      = 0;
	virtual int set_comment( const char*)	      = 0;
	virtual int set_start_time( time_t)	      = 0;

      // get samples
	// original
	virtual valarray<TFloat>
	get_region_original( const char* h, size_t, size_t) const = 0;
	virtual valarray<TFloat>
	get_region_original( int h, size_t, size_t) const = 0;

	template <typename T>
	valarray<TFloat>
	get_region_original( T h,
			     double seconds_off_start,
			     double seconds_off_end) const
		{
			return get_region_original(
				h,
				start_time() * samplerate(h),
				end_time()   * samplerate(h));
		}

	template <typename T>
	valarray<TFloat>
	get_signal_original( T h) const
		{
			return get_region_original(
				h,
				0., recording_time());
		}

	// filtered
	virtual valarray<TFloat>
	get_region_filtered( const char* h, size_t, size_t) const = 0;
	virtual valarray<TFloat>
	get_region_filtered( int h, size_t, size_t) const = 0;

	template <typename T>
	valarray<TFloat>
	get_region_filtered( T h,
			     double seconds_off_start,
			     double seconds_off_end) const
		{
			return get_region_filtered(
				h,
				start_time() * samplerate(h),
				end_time()   * samplerate(h));
		}

	template <typename T>
	valarray<TFloat>
	get_signal_filtered( T h) const
		{
			return get_region_filtered(
				h,
				0., recording_time());
		}

      // put samples
	virtual int
	put_region( int h,
		    const valarray<TFloat>& src,
		    size_t smpla, size_t smplz)	const = 0;
	virtual int
	put_region( const char* h,
		    const valarray<TFloat>& src,
		    size_t smpla, size_t smplz)	const = 0;

	int
	put_signal( int h,
		    const valarray<TFloat>& src)
		{
			return put_region( h, src, 0, src.size());
		}
	int
	put_signal( const char* h,
		    const valarray<TFloat>& src)
		{
			return put_region( h, src, 0, src.size());
		}

      // filenames
	string make_fname_artifacts( const SChannel& channel) const
		{
			return sigfile::make_fname_artifacts( filename(), channel);
		}
	string make_fname_annotations( const SChannel& channel) const
		{
			return sigfile::make_fname_annotations( filename(), channel);
		}

      // misc useful bits
	bool	no_save_extra_files;
	virtual void write_ancillary_files() = 0;
};




} // namespace sigfile

#endif // _SIGFILE_SOURCE_BASE_H

// eof
