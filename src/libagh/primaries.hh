// ;-*-C++-*- *  Time-stamp: "2011-01-08 21:35:25 hmmr"
/*
 *       File name:  primaries.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-05-01
 *
 *         Purpose:  experimental design primary classes: CRecording,
 *         	     CSubject & CExpDesign
 *
 *         License:  GPL
 */


#ifndef _AGH_PRIMARIES_H
#define _AGH_PRIMARIES_H


#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cstring>
#include <cstdio>
#include <string>
#include <list>
#include <algorithm>
#include <map>
#include <stdexcept>

#include "../common.h"
#include "misc.hh"
#include "page.hh"
#include "psd.hh"
#include "edf.hh"
#include "model.hh"

using namespace std;




class CExpDesign;



// identification & matching

class CRecId {

    friend class CRecording;

    protected:
	hash_key
		_subject,
		_session,
		_episode,
		_channel;
    public:
	CRecId( hash_key J, hash_key D, hash_key E, hash_key H)
	      : _subject (J),
		_session (D),
		_episode (E),
		_channel (H)
		{}
	CRecId( const char *j, const char *d, const char *e, const char *h)
	      : _subject (HASHKEY(j)),
		_session (HASHKEY(d)),
		_episode (HASHKEY(e)),
		_channel (HASHKEY(h))
		{}

	hash_key subject() const  {  return _subject; }
	hash_key session() const  {  return _session; }
	hash_key episode() const  {  return _episode; }
	hash_key channel() const  {  return _channel; }

	bool operator==( const CRecId &o) const
		{
			return (_subject == o._subject || _subject == HASHKEY_ANY || o._subject == HASHKEY_ANY || !_subject || !o._subject) &&
			       (_session == o._session || _session == HASHKEY_ANY || o._session == HASHKEY_ANY || !_session || !o._session) &&
			       (_episode == o._episode || _episode == HASHKEY_ANY || o._episode == HASHKEY_ANY || !_episode || !o._episode) &&
			       (_channel == o._channel || _channel == HASHKEY_ANY || o._channel == HASHKEY_ANY || !_channel || !o._channel);
		}
};






class CRecording
  : public CBinnedPower {

    friend class CExpDesign;
    friend class CSCourse;

    protected:
	int	_status;

	const CEDFFile&
		_source;
	int	_sig_no;

	CRecording();
    public:
	const CEDFFile& F() const
		{
			return _source;
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
			if ( signal_type_is_fftable( F[sig_no].SignalType.c_str()) )
				obtain_power( F, sig_no, fft_params);
		}

	const char* subject() const      {  return _source.PatientID_raw; }
	const char* session() const      {  return _source.Session.c_str(); }
	const char* episode() const      {  return _source.Episode.c_str(); }
	const char* channel() const      {  return _source[_sig_no].Channel.c_str(); }
	const char* signal_type() const  {  return _source[_sig_no].SignalType.c_str(); }

	bool operator==( const CRecId &o) const
		{
			return	(HASHKEY(subject()) == o._subject || o._subject == HASHKEY_ANY) &&
				(HASHKEY(session()) == o._session || o._session == HASHKEY_ANY) &&
				(HASHKEY(episode()) == o._episode || o._episode == HASHKEY_ANY) &&
				(HASHKEY(channel()) == o._channel || o._channel == HASHKEY_ANY);
		}
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
		list<CEDFFile> sources;
		map<SChannel, CRecording> recordings; // one per channel, naturally

		SEpisode( CEDFFile&& Fmc, const SFFTParamSet& fft_params);

		const char* name() const
			{
				return sources.begin()->Episode.c_str();
			}
		bool operator==( const char* e) const
			{
				return strcmp( e, name()) == 0;
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
		const SEpisode& operator[]( const char* e) const
			{
				auto E = find( episodes.begin(), episodes.end(), e);
				if ( E != episodes.end() )
					return *E;
				else
					throw invalid_argument("no such episode");
			}
		SEpisode& operator[]( const char* e)
			{
				auto E = find( episodes.begin(), episodes.end(), e);
				if ( E != episodes.end() )
					return *E;
				else // or don't throw, go and make one?
					throw invalid_argument("no such episode");
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
		    list< pair< pair<float, float>,
				  CSimulation> > >
			modrun_sets;  // a bunch (from, to) per each fftable channel
	};
	// all episode sequences, all channels forming a session
	typedef map<string, // session name
		    SEpisodeSequence> CMSessionSet;
	CMSessionSet
		measurements;

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
	template <class T>
	bool operator!=( T id) const
		{
			return !(*this == id);
		}

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
		       pair<float, float>> > // decimal hour
		avg_episode_times;
};





#define AGH_SESSION_STATE_OK       0
#define AGH_SESSION_STATE_INITFAIL 1
#define AGH_SESSION_STATE_LOADFAIL 2

class CExpDesign {

    private:
	int	_status;
	string	_error_log;
	string
		_session_dir;

	CExpDesign();
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
	CSubject &subject_by_x( T jid,
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
	int add_subject( const char *name, TGender gender, int age,
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
			 TGender new_gender = AGH_J_GENDER_NEUTER,
			 int new_age = -1,
			 const char *new_comment = NULL);

      // inventory
	SFFTParamSet	fft_params;
	TFFTWinType	af_dampen_window_type;

	float req_percent_scored;
	size_t swa_laden_pages_before_SWA_0;

	STunableSetFull	 tunables;
	SControlParamSet control_params;

      // scan tree: build all structures
	// void (*)(const char* fname_being_processed,
	// 		 size_t total_sources_found,
	// 		 size_t now_processing)
	typedef void (*TMsmtCollectProgressIndicatorFun)
		(const char*, size_t, size_t);
	void scan_tree( TMsmtCollectProgressIndicatorFun progress_fun = NULL);
	CExpDesign( const char *sessiondir,
		    TMsmtCollectProgressIndicatorFun progress_fun = NULL);
       ~CExpDesign()
		{
			save();
		}

	int load();
	int save();


    public:
      // edf sources
	int register_intree_source( CEDFFile &&F,
			     const char **reason_if_failed_p = NULL);

      // model runs
	int setup_modrun( const char* j, const char* d, const char* h,
			  float freq_from, float freq_upto,
			  CSimulation*&);
	void reset_modrun( CSimulation&);
	// template<class T>
	// bool have_modrun( T j, T d, T h,
	// 		  float from, float upto)
	// 	{
	// 		return find( simulations.begin(), simulations.end(),
	// 			     CSimId (j, d, h, from, upto))
	// 			!= simulations.end();
	// 	}
	// template<class T>
	// CSimulation& modrun_by_jdhq( T j, T d, T h,
	// 			     float from, float upto)
	// 	{
	// 		auto Ri = find( simulations.begin(), simulations.end(),
	// 				CSimId (j, d, h, from, upto));
	// 		if ( Ri == simulations.end() )
	// 			throw invalid_argument("no such modrun");
	// 		return *Ri;
	// 	}

	// string make_fname_edf( const char* j, const char* d, const char* e);
	// // used when scanning the tree, say, if user has lost the init file
	// // (where all sources would normally be stored)

 	string make_fname_simulation( const char* j, const char* d, const char* h,
 //				      size_t start_m, size_t end_m,
 				      float from, float upto);


	size_t enumerate_groups( list<string>& recp);
	size_t enumerate_subjects( list<string>& recp);
	size_t enumerate_sessions( list<string>& recp);
	size_t enumerate_episodes( list<string>& recp);
	size_t enumerate_all_channels( list<SChannel>& recp);
	size_t enumerate_eeg_channels( list<SChannel>& recp);
};




#endif

// EOF
