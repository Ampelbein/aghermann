// ;-*-C++-*- *  Time-stamp: "2011-06-30 02:29:51 hmmr"
/*
 *       File name:  libagh/primaries.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-01
 *
 *         Purpose:  experimental design primary classes: CRecording,
 *         	     CSubject & CExpDesign
 *
 *         License:  GPL
 */


#ifndef _AGH_PRIMARIES_H
#define _AGH_PRIMARIES_H


#include <cstring>
#include <cstdio>
#include <string>
#include <list>
#include <algorithm>
#include <map>
#include <stdexcept>

#include "misc.hh"
#include "boost-config-validate.hh"
#include "psd.hh"
#include "edf.hh"
#include "model.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif

namespace agh {
using namespace agh;
using namespace std;




class CRecording
  : public CBinnedPower {

    friend class CExpDesign;

    protected:
	int	_status;

	CEDFFile&
		_source;
	int	_sig_no;

	CRecording() = delete;
    public:
	const CEDFFile& F() const
		{
			return _source;
		}
	CEDFFile& F()  // although we shouldn't want to access CEDFFile writably from CRecording,
		{      // this shortcut saves us the trouble of AghCC->subject_by_x(,,,).measurements...
			return _source;  // on behalf of aghui::SChannelPresentation
		}
	size_t h() const
		{
			return _sig_no;
		}

	CRecording( CEDFFile& F, int sig_no,
		    const SFFTParamSet& fft_params)
	      : CBinnedPower (fft_params),
		_status (0),
		_source (F), _sig_no (sig_no)
		{
			if ( SChannel::signal_type_is_fftable( F[sig_no].signal_type) )
				obtain_power( F, sig_no, fft_params);
		}

	const char* subject() const      {  return _source.patient.c_str(); }
	const char* session() const      {  return _source.session.c_str(); }
	const char* episode() const      {  return _source.episode.c_str(); }
	const char* channel() const      {  return _source[_sig_no].channel.c_str(); }
	const char* signal_type() const  {  return _source[_sig_no].signal_type.c_str(); }

	bool operator<( const CRecording &o) const
		{
			return _source.end_time < o._source.start_time;
		}

	time_t start() const
		{
			return _source.start_time;
		}
	time_t end() const
		{
			return _source.end_time;
		}
};





class CSubject {

    friend class CExpDesign;

    public:
	enum class TGender : char {
		neuter = 'o', male   = 'M', female = 'F'
	};

    private:
	CSubject();

	int	_status;

	sid_type
		_id;

	string	_name;
	TGender	_gender;
	int	_age;
	string	_comment;

    public:
	sid_type           id() const	{ return _id; }
	const char      *name() const	{ return _name.c_str(); };
	int               age() const	{ return _age; }
	TGender        gender() const	{ return _gender; }
	const char   *comment() const	{ return _comment.c_str(); }

	CSubject( const char *name, TGender gender, int age,
		  const char *comment,
		  sid_type id)
	      : _status (0),
		_id (id),
		_name (name), _gender (gender), _age (age), _comment (comment)
		{}

	struct SEpisode {
	      // allow multiple sources (possibly supplying different channels)
		list<CEDFFile>
			sources;

		const time_t& start_time() const
			{
				return sources.begin()->start_time;
			}
		const time_t& end_time() const
			{
				return sources.begin()->end_time;
			}
		time_t& start_time()
			{
				return sources.begin()->start_time;
			}
		time_t& end_time()
			{
				return sources.begin()->end_time;
			}
		time_t	// relative to start_time
			start_rel,
			end_rel;

		typedef map<SChannel, CRecording> TRecordingSet;
		TRecordingSet
			recordings; // one per channel, naturally

		SEpisode( CEDFFile&& Fmc, const SFFTParamSet& fft_params);

		const char* name() const
			{
				return sources.begin()->episode.c_str();
			}
		bool operator==( const string& e) const
			{
				return e == name();
			}
		bool operator<( const SEpisode& rv) const
			{
				return sources.begin()->end_time
					< rv.sources.begin()->start_time;
			}

		int assisted_score();
	};
	struct SEpisodeSequence {
		list<SEpisode> episodes;
		size_t size() const
			{
				return episodes.size();
			}
		bool have_episode( const string& e) const
			{
				auto E = find( episodes.begin(), episodes.end(), e);
				return E != episodes.end();
			}
		const SEpisode& operator[]( const string& e) const
			{
				auto E = find( episodes.begin(), episodes.end(), e);
				if ( E != episodes.end() )
					return *E;
				else
					throw invalid_argument( string("no such episode: ") + e);
			}
		SEpisode& operator[]( const string& e)
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
		int add_one( CEDFFile&& Fmc, const SFFTParamSet& fft_params,
			     float max_hours_apart = 96.);

	      // simulations rather belong here
		map<string, // channel
		    list< pair< pair<float, float>,  // frequency range
				  CSimulation>>>
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

	bool have_session( const string& d) const
		{
			try {
				measurements.at(d);
				return true;
			} catch (...) {
				return false;
			}
		}

	bool operator==( const CSubject &o) const
		{
			return strcmp( name(), o.name()) == 0;
		}
	bool operator==( const char *n) const
		{
			return strcmp( name(), n) == 0;
		}
	bool operator==( sid_type id) const
		{
			return _id == id;
		}
	// template <class T>
	// bool operator!=( T id) const
	// 	{
	// 		return !(*this == id);
	// 	}

	void rename( const char *new_name)	{  _name = new_name;		}
	void set_age( int new_age)		{  _age = new_age;		}
	void set_gender( TGender new_gender)	{  _gender = new_gender;	}
	void set_comment( const char *new_cmt)	{  _comment = new_cmt;		}
};




class CJGroup
      : public list<CSubject> {
    public:
	map<string,  // session
		   map<string,  // episode
		       pair<float, float>>> // decimal hour
		avg_episode_times;
};





class CExpDesign {

	enum TStateFlags {
		ok = 0,
		init_fail = 1,
		load_fail = 2,
	};
    private:
	int	_status;
	string	_session_dir;
	string	_error_log;

	CExpDesign() = delete;
    public:
	int status() const
		{
			return _status;
		}

	void reset_error_log()
		{
			_error_log.clear();
		}
	const char* last_error() const
		{
			return &_error_log[_error_log.rfind("\n", _error_log.size()-1)];
		}
	const char* error_log() const
		{
			return _error_log.c_str();
		}
	template <class T>
	void log_message( T msg)
		{
			_error_log += msg;
		}

	const char* session_dir() const
		{
			return _session_dir.c_str();
		}

	string name;

    private:
	map<string, CJGroup>
		groups;
    public:
      // access groups
	map<string, CJGroup>::iterator groups_begin()
		{
			return groups.begin();
		}
	map<string, CJGroup>::iterator groups_end()
		{
			return groups.end();
		}
	size_t n_groups() const
		{
			return groups.size();
		}

	CJGroup& group_by_name( const char *g)
		{
			return groups.at(g);
		}

	bool have_group( const char* g) const
		{
			return groups.count(string(g)) > 0;
		}
	bool have_group( const string& g) const
		{
			return groups.count(g) > 0;
		}

      // access subjects, groups and independently
	list<CSubject>::iterator subject_in_group_begin( map<string, CJGroup>::iterator G)
		{
			return G->second.begin();
		}
	list<CSubject>::iterator subject_in_group_end( map<string, CJGroup>::iterator G)
		{
			return G->second.end();
		}

	template <class T>
	CSubject& subject_by_x( const T& jid,
				map<string, CJGroup>::iterator *Giter_p = NULL,
				CJGroup::iterator *Jiter_p = NULL)
		{
			for ( auto G = groups.begin(); G != groups.end(); ++G ) {
				auto J = find( G->second.begin(), G->second.end(),
					       jid);
				if ( J != G->second.end() ) {
					if ( Giter_p )	*Giter_p = G;
					if ( Jiter_p )	*Jiter_p = J;
					return *J;
				}
			}
			throw invalid_argument("no such subject");
		}
	template <class T>
	const CSubject& subject_by_x( const T& jid,
				      map<string, CJGroup>::const_iterator *Giter_p = NULL) const
		{
			for ( auto G = groups.cbegin(); G != groups.cend(); ++G ) {
				auto J = find( G->second.cbegin(), G->second.cend(),
					       jid);
				if ( J != G->second.cend() ) {
					if ( Giter_p )	*Giter_p = G;
					return *J;
				}
			}
			throw invalid_argument("no such subject");
		}
	template <class T>
	const char* group_of( const T& jid)
		{
			for ( auto I = groups.begin(); I != groups.end(); ++I ) {
				auto J = find( I->second.begin(), I->second.end(),
					       jid);
				if ( J != I->second.end() )
					return I->first.c_str();
			}
			throw invalid_argument("no such subject");
		}
	template <class T>
	bool have_subject( T jid) const
		{
			for ( auto I = groups.begin(); I != groups.end(); ++I )
				if ( find( I->second.begin(), I->second.end(), jid) != I->second.end() )
					return true;
			return false;
		}

      // add subject to group; if he exists in another group, remove him therefrom first;
      // if he is already there, update his record
    private:
	sid_type
	        __id_pool;
    public:
	int add_subject( const char *name, CSubject::TGender gender, int age,
			 const char *group,
			 const char *comment = "");

	template <class T>
	void delete_subject( T jid)
		{
			for ( auto I = groups.begin(); I != groups.end(); ++I ) {
				auto J = find( I->second.begin(), I->second.end(), jid);
				if ( J != I->second.end() ) {
					I->second.erase( J);
					return;
				}
			}
		}

	int mod_subject( const char *jwhich,
			 const char *new_name,
			 CSubject::TGender new_gender = CSubject::TGender::neuter,
			 int new_age = -1,
			 const char *new_comment = NULL);

      // inventory
	SFFTParamSet	fft_params;
	SFFTParamSet::TWinType
		af_dampen_window_type;

	STunableSetFull	 tunables0;
	SControlParamSet ctl_params0;

      // scan tree: build all structures
	// void (*)(const char* fname_being_processed,
	// 		 size_t total_sources_found,
	// 		 size_t now_processing)
	typedef void (*TMsmtCollectProgressIndicatorFun)
		(const char*, size_t, size_t);
	void scan_tree( TMsmtCollectProgressIndicatorFun progress_fun = NULL);

      // constructor
	CExpDesign( const char *sessiondir,
		    TMsmtCollectProgressIndicatorFun progress_fun = NULL);
       ~CExpDesign()
		{
			save_settings();
		}

      // load/save
    private:
	vector<SValidator<unsigned int>>	config_keys_u;
	vector<SValidator<double>>		config_keys_g;
	vector<SValidator<bool>>		config_keys_b;
    public:
	int load_settings();
	int save_settings() const;

      // edf sources
	int register_intree_source( CEDFFile &&F,
				    const char **reason_if_failed_p = NULL);

      // model runs
	enum TModrunFlags { modrun_tried = 1 };
	int setup_modrun( const char* j, const char* d, const char* h,
			  float freq_from, float freq_upto,
			  CSimulation*&);
//	void reset_modrun( CSimulation&);
	void remove_untried_modruns();

	// string make_fname_edf( const char* j, const char* d, const char* e);
	// // used when scanning the tree, say, if user has lost the init file
	// // (where all sources would normally be stored)

	template <class T>
	string subject_dir( const T& j) const
		{
			map<string, CJGroup>::const_iterator G;
			const CSubject& J = subject_by_x(j, &G);
			return _session_dir + '/' + G->first + '/' + J._name;
		}
 // 	string make_fname_simulation( const char* j, const char* d, const char* h,
 // //				      size_t start_m, size_t end_m,
 // 				      float from, float upto);


	list<string> enumerate_groups();
	list<string> enumerate_subjects();
	list<string> enumerate_sessions();
	list<string> enumerate_episodes();
	list<SChannel> enumerate_all_channels();
	list<SChannel> enumerate_eeg_channels();
};


inline const char* CSCourse::subject() const { return _mm_list.front()->subject(); }
inline const char* CSCourse::session() const { return _mm_list.front()->session(); }
inline const char* CSCourse::channel() const { return _mm_list.front()->channel(); }


}

#endif

// EOF
