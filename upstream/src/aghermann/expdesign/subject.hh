/*
 *       File name:  aghermann/expdesign/subject.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-01
 *
 *         Purpose:  experimental design primary classes: CSubject
 *
 *         License:  GPL
 */


#ifndef AGH_AGHERMANN_EXPDESIGN_SUBJECT_H_
#define AGH_AGHERMANN_EXPDESIGN_SUBJECT_H_


#include <string>
#include <list>
#include <map>

#include "common/subject_id.hh"
#include "libsigfile/forward-decls.hh"
#include "libsigfile/typed-source.hh"
#include "libmetrics/forward-decls.hh"
//#include "aghermann/model/forward-decls.hh" // map needs it full
#include "aghermann/model/achermann.hh"

#include "forward-decls.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {

using namespace std;


struct SEpisode {

	SEpisode (sigfile::CTypedSource&&,
		  const metrics::psd::SPPack&,
		  const metrics::swu::SPPack&,
		  const metrics::mc::SPPack&);

	time_t start_time() const	{ return sources.front()().start_time(); }
	time_t end_time() const		{ return sources.front()().end_time();	 }
	time_t start_time()		{ return sources.front()().start_time(); }
	time_t end_time()		{ return sources.front()().end_time();	 }
	time_t	// relative to start_time
		start_rel,
		end_rel;

	typedef map<sigfile::SChannel, CRecording> TRecordingSet;
	TRecordingSet
		recordings; // one per channel, naturally

	const char*
	name() const
		{ return sources.front()().episode(); }
	bool
	operator==( const string& e) const
		{ return e == name(); }
	bool
	operator<( const SEpisode& rv) const
		{
			return sources.front()().end_time()
				< rv.sources.front()().start_time();
		}

	struct SAnnotation
	  : public sigfile::SAnnotation {
		SAnnotation (const sigfile::CSource& _si, int _hi,
			     const sigfile::SAnnotation& _a)
		      : sigfile::SAnnotation (_a),
			_source (_si), _h (_hi)
			{}
		SAnnotation( const SAnnotation&) = default;

		const sigfile::CSource& _source;
		int _h;

		bool
		operator<( const SAnnotation& rv) const
			{ return span < rv.span; }

		const char*
		channel() const
			{ return (_h == -1) ? "(embedded)" : _source.channel_by_id(_h).name(); }
		agh::alg::SSpan<float>
		page_span( size_t pagesize) const
			{ return span / (float)pagesize; }
	};
	list<SAnnotation>
	get_annotations() const;

    // private:
    // 	friend class agh::CSubject;
    // 	friend class agh::ui::SScoringFacility;
      // allow multiple sources (possibly supplying different channels)
	list<sigfile::CTypedSource>
		sources;
};


struct SEpisodeSequence {
	friend class agh::CExpDesign;
	friend class agh::CProfile;
    public:
	list<SEpisode> episodes;
	size_t
	size() const
		{ return episodes.size(); }

	list<SEpisode>::const_iterator
	episode_iter_by_name( const string& e) const
		{ return find( episodes.begin(), episodes.end(), e); }
	bool
	have_episode( const string& e) const
		{ return episode_iter_by_name(e) != episodes.cend(); }

	const SEpisode&
	operator[]( const string& e) const;
	SEpisode&
	operator[]( const string& e);

      // either construct a new episode from F, or update an
      // existing one (add F to its sources)
	int
	add_one( sigfile::CTypedSource&&,
		 const metrics::psd::SPPack&,
		 const metrics::swu::SPPack&,
		 const metrics::mc::SPPack&,
		 float max_hours_apart = 7*24.);

      // simulations rather belong here
	typedef map<SProfileParamSet,
		    map<string, // channel
			ach::CModelRun>>
		TModrunSetMap;
	TModrunSetMap
		modrun_sets;  // a bunch (from, to) per each fftable channel
};





typedef size_t sid_t;


class CSubject : public SSubjectId {

	void operator=( const CSubject&) = delete;
	CSubject () = delete;

    public:
	CSubject (const CSubject& rv)
	      : agh::SSubjectId (rv),
		_status (rv._status),
		_id (rv._id),
		_dir (rv._dir)
		{}

	CSubject (const string& dir, sid_t id)
	      : agh::SSubjectId (dir.substr( dir.rfind('/')+1)),
		_status (0),
		_id (id),
		_dir (dir)
		{}

      // identification
	const char*
	dir() const
		{ return _dir.c_str(); }

	int try_update_subject_details( const agh::SSubjectId& j)
		{ return SSubjectId::update_from( j); }

	float age( const string& d) const; // age when recordings in this session were made
	float age() const; // now
	float age_rel( time_t) const;

	bool operator==( const CSubject &o) const
		{ return id == o.id; }
	bool operator==( const string& n) const
		{ return SSubjectId::id == n; }
	bool operator==( sid_t id) const
		{ return _id == id; }

	// all episode sequences, all channels forming a session
	// using CMSessionSet = map<string, // session name
	// 			 SEpisodeSequence>;
	typedef map<string, // session name
		    SEpisodeSequence>
		CMSessionSet;
	CMSessionSet
		measurements;

	bool
	have_session( const string& d) const
		{
			return measurements.find(d) != measurements.end();
		}

    private:
	int	_status;
	sid_t	_id; // eventually to allow distinctly identifiable namesakes in different groups

	string	_dir;
};

} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
