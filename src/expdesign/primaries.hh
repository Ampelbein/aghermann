/*
 *       File name:  expdesign/primaries.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-01
 *
 *         Purpose:  experimental design primary classes: CSubject & CExpDesign
 *
 *         License:  GPL
 */


#ifndef _AGH_EXPDESIGN_PRIMARIES_H
#define _AGH_EXPDESIGN_PRIMARIES_H


#include <cstring>
#include <string>
#include <list>
#include <functional>
#include <forward_list>
#include <map>
#include <stdexcept>

#include "common/config-validate.hh"
#include "common/subject_id.hh"
#include "sigproc/winfun.hh"
#include "model/achermann.hh"
#include "recording.hh"
#include "forward-decls.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {

using namespace std;


typedef size_t sid_type;


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

	CSubject (const string& dir, sid_type id)
	      : agh::SSubjectId (dir.substr( dir.rfind('/')+1)),
		_status (0),
		_id (id),
		_dir (dir)
		{}

      // identification
	const string&
	dir() const
		{ return _dir; }

	int try_update_subject_details( const agh::SSubjectId& j)
		{
			return SSubjectId::update_from( j);
		}


	float age( const string& d) const // age when recordings in this session were made
		{
			if ( measurements.find(d) != measurements.end() &&
			     measurements.at(d).episodes.size() > 0 )
				return age_rel(
					measurements.at(d).episodes.front().start_time());
			else
				return -1.;
		}
	float age() const; // now
	float age_rel( time_t) const;

	bool operator==( const CSubject &o) const
		{
			return id == o.id;
		}
	bool operator==( const string& n) const
		{
			return SSubjectId::id == n;
		}
	bool operator==( sid_type id) const
		{
			return _id == id;
		}

      // contents
	class SEpisodeSequence;
	class SEpisode {
	    public:
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

		SEpisode (sigfile::CTypedSource&& Fmc,
			  const metrics::psd::SPPack&,
			  const metrics::swu::SPPack&,
			  const metrics::mc::SPPack&);

		const char*
		name() const
			{
				return sources.front()().episode();
			}
		bool
		operator==( const string& e) const
			{
				return e == name();
			}
		bool
		operator<( const SEpisode& rv) const
			{
				return sources.front()().end_time()
					< rv.sources.front()().start_time();
			}

		struct SAnnotation
		      : public sigfile::SAnnotation<double> {
			const sigfile::CSource& _source;
			int _h;
			SAnnotation( const sigfile::CSource& _si, int _hi,
				     const sigfile::SAnnotation<double>& _a)
			      : sigfile::SAnnotation<double> (_a),
				_source (_si), _h (_hi)
				{}
			SAnnotation( const SAnnotation&) = default;

			bool
			operator<( const SAnnotation& rv) const
				{
					return span < rv.span;
				}

			const char*
			channel()
				{
					return _source.channel_by_id(_h);
				}
			agh::alg::SSpan<float>
			page_span( size_t pagesize) const
				{
					return span / (float)_source.samplerate(_h) / (float)pagesize;
				}
		};
		list<SAnnotation>
		get_annotations() const;

	    // private:
	    // 	friend class agh::CSubject;
	    // 	friend class aghui::SScoringFacility;
	      // allow multiple sources (possibly supplying different channels)
		list<sigfile::CTypedSource>
			sources;
	};

	class SEpisodeSequence {
		friend class agh::CExpDesign;
		friend class agh::CProfile;
	    public:
		list<SEpisode> episodes;
		size_t
		__attribute__ ((pure))
		size() const
			{
				return episodes.size();
			}
		list<SEpisode>::const_iterator
		episode_iter_by_name( const string& e) const
			{
				return find( episodes.begin(), episodes.end(), e);
			}
		bool
		have_episode( const string& e) const
			{
				return episode_iter_by_name(e) != episodes.cend();
			}
		const SEpisode&
		operator[]( const string& e) const
			{
				auto E = find( episodes.begin(), episodes.end(), e);
				if ( E != episodes.end() )
					return *E;
				else
					throw invalid_argument( string("no such episode: ") + e);
			}
		SEpisode&
		operator[]( const string& e)
			{
				auto E = find( episodes.begin(), episodes.end(), e);
				if ( E != episodes.end() )
					return *E;
				else // or don't throw, go and make one?
					throw invalid_argument( string("no such episode: ") + e);
				// no, let it be created in
				// CExpDesign::add_measurement, when
				// episode start/end times are known
			}
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
			try {
				measurements.at(d);
				return true;
			} catch (...) {
				return false;
			}
		}

    private:
	int	_status;
	sid_type
		_id; // eventually to allow distinctly identifiable namesakes in different groups

	string	_dir;
};




class CJGroup
      : public list<CSubject> {

	void operator=( const CJGroup&) = delete;
    public:
	map<string,  // session
		   map<string,  // episode
		       pair<float, float>>> // decimal hour
		avg_episode_times;
};





class CExpDesign {

	DELETE_DEFAULT_METHODS (CExpDesign);

    public:
      // constructor
	typedef function<void(const char*, size_t, size_t)> TMsmtCollectProgressIndicatorFun;
	CExpDesign (const string& sessiondir,
		    TMsmtCollectProgressIndicatorFun progress_fun = progress_fun_stdout);
       ~CExpDesign ()
		{
			save_settings();
		}

	int status() const
		{
			return _status;
		}

	const string& session_dir() const
		{
			return _session_dir;
		}

	string name() const // dirname
		{
			return _session_dir.substr( _session_dir.rfind( '/'));
		}

	void reset_error_log()
		{
			_error_log.clear();
		}
	const char* last_error() const
		{
			return &_error_log[_error_log.rfind("\n", _error_log.size()-1)];
		}
	const string& error_log() const
		{
			return _error_log;
		}
	void log_message( const char* fmt, ...);

      // contains
	typedef map<string, CJGroup> TJGroups;
	TJGroups
		groups;
	template <typename T>
	bool have_group( const T&) const;

	template <class T>
	CSubject& subject_by_x( const T&);

	template <class T>
	const CSubject& subject_by_x( const T&,
				      TJGroups::const_iterator *Giter_p = nullptr) const;
	template <class T>
	const char* group_of( const T&);

      // add subject to group; if he exists in another group, remove him therefrom first;
      // if he is already there, update his record
	int add_subject( const char *name, CSubject::TGender gender, int age,
			 const char *group,
			 const char *comment = "");

	template <class T>
	string subject_dir( const T& j) const
		{
			map<string, CJGroup>::const_iterator G;
			const CSubject& J = subject_by_x(j, &G);
			return _session_dir + '/' + G->first + '/' + J.SSubjectId::id;
		}

      // scan tree: build all structures
	static TMsmtCollectProgressIndicatorFun progress_fun_stdout;
	void scan_tree( TMsmtCollectProgressIndicatorFun progress_fun = progress_fun_stdout);
    private:
	void compute_profiles();

    public:
      // edf sources
	int register_intree_source( sigfile::CTypedSource&&,
				    const char **reason_if_failed_p = nullptr);

      // model runs
	int setup_modrun( const char* j, const char* d, const char* h,
			  const SProfileParamSet&,
			  ach::CModelRun**);
	void remove_all_modruns();
	void remove_untried_modruns();
	void export_all_modruns( const string& fname) const;

	void sync();

      // global info on expdesign
	list<string> enumerate_groups() const;
	list<string> enumerate_subjects() const;
	list<string> enumerate_sessions() const;
	list<string> enumerate_episodes() const;
	list<sigfile::SChannel> enumerate_all_channels() const;
	list<sigfile::SChannel> enumerate_eeg_channels() const;
	list<size_t> used_samplerates( sigfile::SChannel::TType type = sigfile::SChannel::other) const;

      // omp-enabled lists:foreach
	typedef function<void(CSubject&)>
		TSubjectOpFun;
	typedef function<void(const CJGroup&,
			      const CSubject&,
			      size_t, size_t)>
		TSubjectReportFun;
	typedef function<bool(CSubject&)>
		TSubjectFilterFun;
	void
	for_all_subjects( const TSubjectOpFun&, const TSubjectReportFun&, const TSubjectFilterFun&);

	typedef function<void(CSubject::SEpisode&)>
		TEpisodeOpFun;
	typedef function<void(const CJGroup&,
			      const CSubject&,
			      const string&,
			      const CSubject::SEpisode&,
			      size_t, size_t)>
		TEpisodeReportFun;
	typedef function<bool(CSubject::SEpisode&)>
		TEpisodeFilterFun;
	void
	for_all_episodes( const TEpisodeOpFun&, const TEpisodeReportFun&, const TEpisodeFilterFun&);

	typedef function<void(CRecording&)>
		TRecordingOpFun;
	typedef function<void(const CJGroup&,
			      const CSubject&,
			      const string&,
			      const CSubject::SEpisode&,
			      const CRecording&,
			      size_t, size_t)>
		TRecordingReportFun;
	typedef function<bool(CRecording&)>
		TRecordingFilterFun;
	void
	for_all_recordings( const TRecordingOpFun&, const TRecordingReportFun&, const TRecordingFilterFun&);

	typedef function<void(ach::CModelRun&)>
		TModelRunOpFun;
	typedef function<void(const CJGroup&,
			      const CSubject&,
			      const string&,
			      const SProfileParamSet&,
			      const string&,
			      const ach::CModelRun&,
			      size_t, size_t)>
		TModelRunReportFun;
	typedef function<bool(ach::CModelRun&)>
		TModelRunFilterFun;
	void
	for_all_modruns( const TModelRunOpFun&, const TModelRunReportFun&, const TModelRunFilterFun&);

      // inventory
	size_t	num_threads;
	metrics::psd::SPPack
		fft_params;
	metrics::swu::SPPack
		swu_params;
	metrics::mc::SPPack
		mc_params;
	sigproc::TWinType // such a fussy
		af_dampen_window_type;
	double	af_dampen_factor;

	ach::STunableSet<ach::TTRole::d>	tstep;
	ach::STunableSet<ach::TTRole::l>	tlo;
	ach::STunableSet<ach::TTRole::u>	thi;
	ach::STunableSetWithState		tunables0;

	ach::SControlParamSet
		ctl_params0;
	double	req_percent_scored;
	size_t	swa_laden_pages_before_SWA_0;
	bool	score_unscored_as_wake,
		strict_subject_id_checks;

	int load_settings();
	int save_settings();

	string	last_used_version;
	int purge_cached_profiles();

    private:
	enum TStateFlags {
		ok = 0,
		init_fail = 1,
		load_fail = 2, // irrelevant
	};

	int	_status;
	string	_session_dir;
	string	_error_log;

	sid_type
	        _id_pool;
      // load/save
	forward_list<confval::SValidator<double>>	config_keys_g;
	forward_list<confval::SValidator<int>>		config_keys_d;
	forward_list<confval::SValidator<size_t>>	config_keys_z;
	forward_list<confval::SValidator<bool>>		config_keys_b;
	forward_list<confval::SValidator<string>>	config_keys_s;
};



template <typename T>
bool CExpDesign::have_group( const T& g) const
{
	return groups.count(string(g)) > 0;
}

template <class T>
CSubject& CExpDesign::subject_by_x( const T& jid)
{
	for ( auto &G : groups ) {
		auto J = find( G.second.begin(), G.second.end(),
			       jid);
		if ( J != G.second.end() )
			return *J;
	}
	throw invalid_argument("no such subject");
}

template <class T>
const CSubject&
CExpDesign::subject_by_x( const T& jid,
			  CExpDesign::TJGroups::const_iterator *Giter_p) const
{
	for ( auto G = groups.cbegin(); G != groups.cend(); ++G ) {
		auto J = find( G->second.cbegin(), G->second.cend(),
			       jid);
		if ( J != G->second.cend() ) {
			if ( Giter_p )
				*Giter_p = G;
			return *J;
		}
	}
	throw invalid_argument("no such subject");
}

template <class T>
const char* CExpDesign::group_of( const T& jid)
{
	for ( auto I = groups.begin(); I != groups.end(); ++I ) {
		auto J = find( I->second.begin(), I->second.end(),
			       jid);
		if ( J != I->second.end() )
			return I->first.c_str();
	}
	throw invalid_argument("no such subject");
}

// template <class T>
// bool have_subject( T jid) const
// 	{
// 		for ( auto& I : groups )
// 			if ( find( I.second.begin(), I.second.end(), jid) != I.second.end() )
// 				return true;
// 		return false;
// 	}





namespace beersma {
int
assisted_score( agh::CSubject::SEpisode&);
} // namespace beersma


} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
