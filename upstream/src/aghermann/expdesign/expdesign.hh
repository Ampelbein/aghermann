/*
 *       File name:  aghermann/expdesign/expdesign.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-01
 *
 *         Purpose:  experimental design primary classes: CExpDesign
 *
 *         License:  GPL
 */


#ifndef AGH_AGHERMANN_EXPDESIGN_PRIMARIES_H_
#define AGH_AGHERMANN_EXPDESIGN_PRIMARIES_H_


#include <string>
#include <list>
#include <forward_list>
#include <map>

#include "common/config-validate.hh"
#include "common/containers.hh"
#include "common/subject_id.hh"
#include "libsigproc/winfun.hh"
#include "libmetrics/bands.hh"
#include "aghermann/model/achermann.hh"

#include "forward-decls.hh"
#include "subject.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {

using namespace std;


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
	typedef function<void(const string&, size_t, size_t)> TMsmtCollectProgressIndicatorFun;
	CExpDesign (const string& sessiondir,
		    TMsmtCollectProgressIndicatorFun progress_fun = progress_fun_stdout);
       ~CExpDesign ()
		{
			save_settings();
		}

	int status() const
		{ return _status; }

	const char*
	session_dir() const
		{ return _session_dir.c_str(); }

	string
	name() const // dirname
		{ return _session_dir.substr( _session_dir.rfind( '/')); }

      // error log
	enum class TLogEntryStyle { plain, bold, italic };
	const list<pair<string, TLogEntryStyle>>&
	error_log() const
		{ return _error_log; }
	void reset_error_log()
		{ _error_log.clear(); }
	const char* last_error() const
		{ return _error_log.back().first.c_str(); }
	string error_log_serialize() const;
	size_t error_log_n_messages() const
		{ return _error_log.size(); }
	void log_message( const char* fmt, ...)  __attribute__ (( format (printf, 2, 3) ));

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
	static bool is_supported_source( sigfile::CTypedSource&);

      // model runs
	int setup_modrun( const string& j, const string& d, const string& h,
			  const SProfileParamSet&,
			  ach::CModelRun**);
	void remove_all_modruns();
	void prune_untried_modruns();
	void export_all_modruns( const string& fname) const;

	void sync();

      // global info on expdesign
	list<string> enumerate_groups() const;
	list<string> enumerate_subjects() const;
	list<string> enumerate_sessions() const;
	list<string> enumerate_episodes() const;
	list<sigfile::SChannel> enumerate_all_channels() const;
	list<sigfile::SChannel> enumerate_eeg_channels() const;
	vector<size_t> used_samplerates( sigfile::SChannel::TType type = sigfile::SChannel::TType::other) const;

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

	typedef function<void(SEpisode&)>
		TEpisodeOpFun;
	typedef function<void(const CJGroup&,
			      const CSubject&,
			      const string&,
			      const SEpisode&,
			      size_t, size_t)>
		TEpisodeReportFun;
	typedef function<bool(SEpisode&)>
		TEpisodeFilterFun;
	void
	for_all_episodes( const TEpisodeOpFun&, const TEpisodeReportFun&, const TEpisodeFilterFun&);

	typedef function<void(CRecording&)>
		TRecordingOpFun;
	typedef function<void(const CJGroup&,
			      const CSubject&,
			      const string&,
			      const SEpisode&,
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

	static double
		freq_bands[metrics::TBand::TBand_total][2];
	static const char
		*const FreqBandNames[metrics::TBand::TBand_total];

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
	list<pair<string, TLogEntryStyle>>
		_error_log;

	sid_t	_id_pool;
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




} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
