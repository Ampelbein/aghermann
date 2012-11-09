// ;-*-C++-*-
/*
 *       File name:  libsigfile/source-base.hh
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

#include "common/fs.hh"
#include "common/alg.hh"
#include "sigproc/sigproc.hh"
#include "channel.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigfile {

template<class T>
string
make_fname_hypnogram( const T& _filename, size_t pagesize)
{
	return agh::fs::make_fname_base( _filename, ".edf", true)
		+ "-" + to_string( (long long unsigned)pagesize) + ".hypnogram";
}

template<class T>
string
make_fname_artifacts( const T& _filename, const SChannel& channel)
{
	return agh::fs::make_fname_base( _filename, ".edf", true)
		+ "-" + channel + ".af";
}

template<class T>
string
make_fname_annotations( const T& _filename, const SChannel& channel)
{
	return agh::fs::make_fname_base( _filename, ".edf", true)
		+ "-" + channel + ".annotations";
}

template<class T>
string
make_fname_filters( const T& _filename)
{
	return agh::fs::make_fname_base( _filename, ".edf", true)
		+ ".filters";
}







struct SArtifacts {
	SArtifacts (float f_ = 0.95,
		    sigproc::TWinType dwt_ = sigproc::TWinType::welch)
	      : factor (f_),
		dampen_window_type (dwt_)
		{}

	list<agh::alg::SSpan<size_t>>
		obj;
	float	factor;
	sigproc::TWinType
		dampen_window_type;

	list<agh::alg::SSpan<size_t>>&
	operator() ()
		{
			return obj;
		}
	const list<agh::alg::SSpan<size_t>>&
	operator() () const
		{
			return obj;
		}

	void mark_artifact( size_t aa, size_t az);
	void clear_artifact( size_t aa, size_t az);
	void clear_all()
		{
			obj.clear();
		}

	float region_dirty_fraction( size_t a, size_t z) const;

	unsigned long dirty_signature() const;
};





struct SAnnotation {
	agh::alg::SSpan<size_t> span;
	string label;
	// enum class TOrigin : bool { internal, external };
	// TOrigin origin;

	SAnnotation( size_t aa, size_t az, const string& l)
	      : span {aa, az}, label (l)
//		  origin (_origin)
		{}

	bool operator==( const SAnnotation& rv) const
		{
			return span == rv.span && label == rv.label; // && origin == rv.origin;
		}
};

inline void
mark_annotation( list<SAnnotation>& annotations,
		 size_t aa, size_t az,
		 const char* label)
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

	double	high_pass_cutoff,
		low_pass_cutoff;
	unsigned
		high_pass_order,
		low_pass_order;

	enum TNotchFilter : int {
		none, at50Hz, at60Hz
	};
	TNotchFilter
		notch_filter;

	unsigned long dirty_signature() const;
};






class CSource_base {
    protected:
	string	_filename;
	int	_status;
	int	_flags;
    public:
	DELETE_DEFAULT_METHODS (CSource_base);
	CSource_base( const string& fname, int flags = 0)
	      : _filename (fname),
		_status (0),
		_flags (flags)
		{}
	CSource_base( CSource_base&& rv);
	virtual ~CSource_base()
		{}

	int status()	const { return _status; }
	int flags()	const { return _flags; }
	virtual string explain_status()			const = 0;
	virtual string details( bool channels_too = true)
							const = 0;

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

      // recording time and duration
	virtual const time_t& start_time()		const = 0;
	virtual const time_t& end_time()		const = 0;
	virtual time_t recording_time()			const = 0;

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
	virtual const SArtifacts&
	artifacts( const char*)			const = 0;
	virtual const SArtifacts&
	artifacts( int)				const = 0;

	// filters
	virtual SFilterPack&
	filters( const char*)			      = 0;
	virtual SFilterPack&
	filters( int)				      = 0;
	virtual const SFilterPack&
	filters( const char*)			const = 0;
	virtual const SFilterPack&
	filters( int)				const = 0;

	template <typename T>
	unsigned long
	dirty_signature( T id) const
		{
			return artifacts(id).dirty_signature() + filters(id).dirty_signature();
		}

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
			     float seconds_off_start,
			     float seconds_off_end) const
		{
			size_t sr = samplerate(h);
			return get_region_original(
				h,
				seconds_off_start * sr,
				seconds_off_end * sr);
		}

	template <typename T>
	valarray<TFloat>
	get_signal_original( T h) const
		{
			return get_region_original(
				h,
				0, recording_time() * samplerate(h));
		}

	// filtered
	virtual valarray<TFloat>
	get_region_filtered( const char* h, size_t, size_t) const = 0;
	virtual valarray<TFloat>
	get_region_filtered( int h, size_t, size_t) const = 0;

	template <typename T>
	valarray<TFloat>
	get_region_filtered( T h,
			     float seconds_off_start,
			     float seconds_off_end) const
		{
			size_t sr = samplerate(h);
			return get_region_filtered(
				h,
				seconds_off_start * sr,
				seconds_off_end   * sr);
		}

	template <typename T>
	valarray<TFloat>
	get_signal_filtered( T h) const
		{
			return get_region_filtered(
				h,
				0, recording_time() * samplerate(h));
		}

      // put samples
	virtual int
	put_region( int h,
		    const valarray<TFloat>& src,
		    size_t offset)		const = 0;
	virtual int
	put_region( const char* h,
		    const valarray<TFloat>& src,
		    size_t offset)		const = 0;

	int
	put_signal( int h,
		    const valarray<TFloat>& src)
		{
			return put_region( h, src, 0);
		}
	int
	put_signal( const char* h,
		    const valarray<TFloat>& src)
		{
			return put_region( h, src, 0);
		}

      // export
	virtual int
	export_original( int, const char* fname) const = 0;
	virtual int
	export_original( const char*, const char* fname) const = 0;
	virtual int
	export_filtered( int, const char* fname) const = 0;
	virtual int
	export_filtered( const char*, const char* fname) const = 0;

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
	virtual void write_ancillary_files() = 0;
};




} // namespace sigfile

#endif // _SIGFILE_SOURCE_BASE_H

// eof
