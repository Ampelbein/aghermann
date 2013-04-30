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
#include "common/subject_id.hh"
#include "sigproc/winfun.hh"
#include "expdesign/forward-decls.hh"
#include "channel.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigfile {

inline string
make_fname_hypnogram( const string& _filename, size_t pagesize)
{
	return agh::fs::make_fname_base( _filename, ".edf", true)
		+ "-" + to_string( (long long unsigned)pagesize) + ".hypnogram";
}

inline string
make_fname_artifacts( const string& _filename, const SChannel& channel)
{
	return agh::fs::make_fname_base( _filename, ".edf", true)
		+ "-" + channel.name() + ".af";
}

inline string
make_fname_annotations( const string& _filename, const SChannel& channel)
{
	return agh::fs::make_fname_base( _filename, ".edf", true)
		+ "-" + channel.name() + ".annotations";
}

inline string
make_fname_filters( const string& _filename)
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

	list<agh::alg::SSpan<double>>
		obj;
	float	factor;
	sigproc::TWinType
		dampen_window_type;

	list<agh::alg::SSpan<double>>&
	operator() ()
		{
			return obj;
		}
	const list<agh::alg::SSpan<double>>&
	operator() () const
		{
			return obj;
		}

	size_t total() const
		{
			size_t q = 0;
			for ( auto& A : obj )
				q += (A.z - A.a);
			return q;
		}

	void mark_artifact( double aa, double az);
	void clear_artifact( double aa, double az);
	void clear_all()
		{
			obj.clear();
		}

	float region_dirty_fraction( double a, double z) const;

	unsigned long dirty_signature() const;
};





struct SAnnotation {
	agh::alg::SSpan<double> span;
	string label;
	enum TType {
		plain,
		phasic_event_spindle,
		phasic_event_K_complex,
		eyeblink,
		TType_total
	};
	TType type;

	SAnnotation( double aa, double az, const string& l, TType t = TType::plain)
	      : span {aa, az},
		label (l),
		type (t)
		{}

	bool operator==( const SAnnotation& rv) const
		{
			return span == rv.span && label == rv.label && type == rv.type;
		}
	bool operator<( const SAnnotation& rv) const
		{
			return span < rv.span;
		}
};




inline void
mark_annotation( list<SAnnotation>& annotations,
		 double aa, double az,
		 const string& label,
		 SAnnotation::TType t = SAnnotation::TType::plain)
{
	annotations.emplace_back( aa, az, label, t);
	annotations.sort();
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




class CSource {
	friend class CTypedSource;
	friend class agh::CSubject;
	friend class agh::CExpDesign;
    protected:
	string	_filename;
	int	_status;
	int	_flags;
	agh::SSubjectId
		_subject;
    public:
	DELETE_DEFAULT_METHODS (CSource);
	CSource (const string& fname_, int flags_ = 0)
	      : _filename (fname_),
		_status (0),
		_flags (flags_)
		{}
	CSource( CSource&&);
	virtual ~CSource()
		{}

	int status()	const { return _status; }
	int flags()	const { return _flags; }
	virtual string explain_status()			const = 0;
	virtual string details( int which_details)	const = 0;

      // identification
	const char* filename() const
		{
			return _filename.c_str();
		}
	const agh::SSubjectId& subject() const
		{
			return _subject;
		}
	virtual const char* patient_id()		const = 0;
	virtual const char* recording_id()		const = 0;
	virtual const char* comment()			const = 0;
	// probably parsed out of recording_id
	virtual const char* episode()			const = 0;
	virtual const char* session()			const = 0;

      // recording time and duration
	virtual time_t start_time()			const = 0;
	virtual time_t end_time()			const = 0;
	virtual time_t recording_time()			const = 0;

      // channels
	virtual size_t n_channels()			const = 0;
	virtual list<SChannel> channel_list()		const = 0;
	virtual bool have_channel( const SChannel&) 	const = 0;
	virtual int channel_id( const SChannel&)	const = 0;
	virtual const SChannel& channel_by_id( int)	const = 0;
	virtual SChannel::TType
	signal_type( int)				const = 0;
	virtual size_t samplerate( int)			const = 0;

	// the following methods are pass-through:
	// 1. annotations
	// (a) per-channel
	virtual list<SAnnotation>&
	annotations( int)			      = 0;
	virtual const list<SAnnotation>&
	annotations( int) const			      = 0;

	// (b) common
	virtual list<SAnnotation>&
	annotations()				      = 0;
	virtual const list<SAnnotation>&
	annotations()				const = 0;

	// artifacts
	virtual SArtifacts&
	artifacts( int)				      = 0;
	virtual const SArtifacts&
	artifacts( int)				const = 0;

	// filters
	virtual SFilterPack&
	filters( int)				      = 0;
	virtual const SFilterPack&
	filters( int)				const = 0;

	template <typename T>
	unsigned long
	dirty_signature( T id) const
		{
			return artifacts(id).dirty_signature() + filters(id).dirty_signature();
		}

      // setters
	virtual int set_patient_id( const string&)    = 0;
	virtual int set_recording_id( const string&)  = 0;
	virtual int set_episode( const string&)	      = 0;
	virtual int set_session( const string&)	      = 0;
	virtual int set_comment( const string&)	      = 0;
	virtual int set_start_time( time_t)	      = 0;

      // get samples
	// original
	virtual valarray<TFloat>
	get_region_original( int, size_t, size_t) const = 0;

	valarray<TFloat>
	get_region_original( const int h,
			     const float seconds_off_start,
			     const float seconds_off_end) const
		{
			size_t sr = samplerate(h);
			return get_region_original(
				h,
				seconds_off_start * sr,
				seconds_off_end * sr);
		}

	valarray<TFloat>
	get_signal_original( const int h) const
		{
			return get_region_original(
				h,
				0, recording_time() * samplerate(h));
		}

	// filtered
	virtual valarray<TFloat>
	get_region_filtered( int h, size_t, size_t) const = 0;

	valarray<TFloat>
	get_region_filtered( const int h,
			     const float seconds_off_start,
			     const float seconds_off_end) const
		{
			size_t sr = samplerate(h);
			return get_region_filtered(
				h,
				seconds_off_start * sr,
				seconds_off_end   * sr);
		}

	valarray<TFloat>
	get_signal_filtered( const int h) const
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

	int
	put_signal( int h,
		    const valarray<TFloat>& src)
		{
			return put_region( h, src, 0);
		}

      // signal data info
	virtual pair<TFloat, TFloat>
	get_real_original_signal_range( int) const = 0;

	virtual pair<TFloat, TFloat>
	get_max_original_signal_range( int) const = 0;

	virtual pair<TFloat, TFloat>
	get_real_filtered_signal_range( int) const = 0;

	virtual pair<TFloat, TFloat>
	get_max_filtered_signal_range( int) const = 0;

      // export
	virtual int
	export_original( int, const string& fname) const = 0;
	virtual int
	export_filtered( int, const string& fname) const = 0;

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

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

