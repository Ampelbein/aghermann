// ;-*-C++-*-
/*
 *       File name:  primaries.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  experimental design primary classes: CMeasurement,
 *         	     CSubject & CExpDesign
 *
 *         License:  GPL
 */


#include <cfloat>
#include <cstdlib>
#include <string>
#include <functional>
#include <initializer_list>

#include <ftw.h>

#include "misc.hh"
#include "boost-config-validate.hh"
#include "primaries.hh"
#include "model.hh"
#include "edf.hh"


#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;
using namespace agh;


inline namespace {
	int
	mkdir_with_parents( const char *dir)
	{
		UNIQUE_CHARP(_);
		assert (asprintf( &_, "mkdir -p '%s'", dir));
		return system( _);
	}

	struct
	progress_fun_stdout_fo {
		void operator() ( const char* current, size_t n, size_t i) const
			{
				printf( "(%zu of %zu) %s\n", i, n, current);
			}
	};
}


agh::CExpDesign::TMsmtCollectProgressIndicatorFun
	agh::CExpDesign::progress_fun_stdout = progress_fun_stdout_fo();

agh::CExpDesign::CExpDesign( const string& session_dir_,
			     TMsmtCollectProgressIndicatorFun progress_fun)
      : _session_dir (session_dir_),
	__id_pool (0),
	af_dampen_window_type (SFFTParamSet::TWinType::welch),
	config_keys_g ({
		SValidator<double>("ctlparam.StepSize",		&ctl_params0.siman_params.step_size),
		SValidator<double>("ctlparam.Boltzmannk",	&ctl_params0.siman_params.k,		SValidator<double>::SVFRange( DBL_MIN, 1e9)),
		SValidator<double>("ctlparam.TInitial",		&ctl_params0.siman_params.t_initial,	SValidator<double>::SVFRange( DBL_MIN, 1e9)),
		SValidator<double>("ctlparam.DampingMu",	&ctl_params0.siman_params.mu_t,		SValidator<double>::SVFRange( DBL_MIN, 1e9)),
		SValidator<double>("ctlparam.TMin",		&ctl_params0.siman_params.t_min,	SValidator<double>::SVFRange( DBL_MIN, 1e9)),
		SValidator<double>("ctlparam.ReqScoredPC",	&ctl_params0.req_percent_scored,	SValidator<double>::SVFRange( 80., 100.)),
		SValidator<double>("fftparam.BinSize",		&fft_params.bin_size,			SValidator<double>::SVFRange( .1, 1.))
	}),
	config_keys_d ({
		SValidator<int>("fftparam.WelchWindowType",	(int*)&fft_params.welch_window_type,	SValidator<int>::SVFRange( 0, (int)SFFTParamSet::TWinType::_total - 1)),
		SValidator<int>("artifacts.DampenWindowType",	(int*)&af_dampen_window_type,		SValidator<int>::SVFRange( 0, (int)SFFTParamSet::TWinType::_total - 1)),
		SValidator<int>("ctlparam.ItersFixedT",		&ctl_params0.siman_params.iters_fixed_T,SValidator<int>::SVFRange( 1, 1000000)),
		SValidator<int>("ctlparam.NTries",		&ctl_params0.siman_params.n_tries,	SValidator<int>::SVFRange( 1, 10000)),
	}),
	config_keys_z ({
		SValidator<size_t>("ctlparam.NSWALadenPagesBeforeSWA0",	&ctl_params0.swa_laden_pages_before_SWA_0,	SValidator<size_t>::SVFRange( 1, 100)),
		SValidator<size_t>("fftparam.PageSize",			&fft_params.page_size),
	}),
	config_keys_b ({
		SValidator<bool>("ctlparam.DBAmendment1",		&ctl_params0.DBAmendment1),
		SValidator<bool>("ctlparam.DBAmendment2",		&ctl_params0.DBAmendment2),
		SValidator<bool>("ctlparam.AZAmendment1",		&ctl_params0.AZAmendment1),
		SValidator<bool>("ctlparam.AZAmendment2",		&ctl_params0.AZAmendment2),
		SValidator<bool>("ctlparam.ScoreMVTAsWake",		&ctl_params0.ScoreMVTAsWake),
		SValidator<bool>("ctlparam.ScoreUnscoredAsWake",	&ctl_params0.ScoreUnscoredAsWake),
	})
{
      // ensure this
	// if ( _session_dir.back() == '/' )
	// 	_session_dir.pop_back();
	if ( _session_dir[_session_dir.size()-1] == '/' )
		_session_dir.erase( _session_dir.size()-1, 1);

	if ( chdir( session_dir()) == -1 ) {
		fprintf( stderr, "CExpDesign::CExpDesign(): Could not cd to \"%s\"; trying to create a new directory there...", session_dir());
		if ( mkdir_with_parents( session_dir()) || chdir( session_dir()) != -1 )
			fprintf( stderr, "done\n");
		else {
			fprintf( stderr, "failed\n");
			throw init_fail;
		}
	} else {
		if ( load_settings() )
			;
		scan_tree( progress_fun);
	}
}







list<string>
agh::CExpDesign::enumerate_groups() const
{
	list<string> recp;
	for ( auto Gi = groups.begin(); Gi != groups.end(); ++Gi )
		recp.push_back( Gi->first);
	return recp;
}

list<string>
agh::CExpDesign::enumerate_subjects() const
{
	list<string> recp;
	for ( auto Gi = groups.begin(); Gi != groups.end(); ++Gi )
		for ( auto Ji = Gi->second.begin(); Ji != Gi->second.end(); ++Ji )
			recp.push_back( Ji->name());
	return recp;
}


list<string>
agh::CExpDesign::enumerate_sessions() const
{
	list<string> recp;
	for ( auto Gi = groups.begin(); Gi != groups.end(); ++Gi )
		for ( auto Ji = Gi->second.begin(); Ji != Gi->second.end(); ++Ji )
			for ( auto Di = Ji->measurements.begin(); Di != Ji->measurements.end(); ++Di )
				recp.push_back( Di->first);
	recp.sort();
	recp.unique();
	return recp;
}

list<string>
agh::CExpDesign::enumerate_episodes() const
{
	list<string> recp;
	for ( auto Gi = groups.begin(); Gi != groups.end(); ++Gi )
		for ( auto Ji = Gi->second.begin(); Ji != Gi->second.end(); ++Ji )
			for ( auto Di = Ji->measurements.begin(); Di != Ji->measurements.end(); ++Di )
				for ( auto Ei = Di->second.episodes.begin(); Ei != Di->second.episodes.end(); ++Ei )
					recp.push_back( Ei->name());
	recp.sort();
	recp.unique();
	return recp;
}

list<SChannel>
agh::CExpDesign::enumerate_eeg_channels() const
{
	list<SChannel> recp;
	for ( auto Gi = groups.begin(); Gi != groups.end(); ++Gi )
		for ( auto Ji = Gi->second.begin(); Ji != Gi->second.end(); ++Ji )
			for ( auto Di = Ji->measurements.begin(); Di != Ji->measurements.end(); ++Di )
				for ( auto Ei = Di->second.episodes.begin(); Ei != Di->second.episodes.end(); ++Ei )
					for ( auto Fi = Ei->sources.begin(); Fi != Ei->sources.end(); ++Fi )
						for ( size_t h = 0; h < Fi->signals.size(); ++h )
							if ( SChannel::signal_type_is_fftable( Fi->signals[h].signal_type) )
								recp.push_back( Fi->signals[h].channel);
	recp.sort();
	recp.unique();
	return recp;
}

list<SChannel>
agh::CExpDesign::enumerate_all_channels() const
{
	list<SChannel> recp;
	for ( auto Gi = groups.begin(); Gi != groups.end(); ++Gi )
		for ( auto Ji = Gi->second.begin(); Ji != Gi->second.end(); ++Ji )
			for ( auto Di = Ji->measurements.begin(); Di != Ji->measurements.end(); ++Di )
				for ( auto Ei = Di->second.episodes.begin(); Ei != Di->second.episodes.end(); ++Ei )
					for ( auto Fi = Ei->sources.begin(); Fi != Ei->sources.end(); ++Fi )
						for ( size_t h = 0; h < Fi->signals.size(); ++h )
							recp.push_back( Fi->signals[h].channel);
	recp.sort();
	recp.unique();
	return recp;
}




agh::CSubject::CSubject( const string& dir,
			 sid_type id)
  : _status (0),
    _id (id),
    _dir (dir),
    _name (dir.substr( dir.rfind('/')+1))
{
	ifstream ifs (_dir + "/.subject_info");
	char gender_char;
	if ( ifs.good() and
	     (getline( ifs, full_name, '\n'),
	      ifs >> gender_char >> age,
	      getline( ifs, comment, '\n'),
	      ifs.good()) )
		gender = (TGender)gender_char;
	else {
		full_name = _name;
		gender = TGender::neuter;
		age = 21;
		comment = "fafa";
	}
}


agh::CSubject::~CSubject()
{
	ofstream ofs (_dir + "/.subject_info");
	char gender_char = (char)gender;
	if ( ofs.good() )
		ofs << full_name << endl
		    << gender_char << endl
		    << age << endl
		    << comment << endl;
}






#define AGH_EPSEQADD_OVERLAP -1
#define AGH_EPSEQADD_TOOFAR  -2


agh::CSubject::SEpisode::SEpisode( CEDFFile&& Fmc, const SFFTParamSet& fft_params)
{
     // move it in place
	sources.emplace_back( static_cast<CEDFFile&&>(Fmc));
	CEDFFile& F = sources.back();
	for ( size_t h = 0; h < F.signals.size(); ++h )
		recordings.insert(
			{F[h].channel, CRecording (F, h, fft_params)});
}


list<agh::CSubject::SEpisode::SAnnotation>
agh::CSubject::SEpisode::get_annotations() const
{
	list<agh::CSubject::SEpisode::SAnnotation>
		ret;
	for_each( sources.begin(), sources.end(),
		  [&ret]( const agh::CEDFFile& F)
		  {
			  for ( int h = 0; h < (int)F.signals.size(); ++h )
				  for_each( F[h].annotations.begin(), F[h].annotations.end(),
					    [&ret, &F, h]( const agh::CEDFFile::SSignal::SAnnotation& A)
					    {
						    ret.emplace_back( F, h, A);
					    });
		  });
	ret.sort();
	return ret;
}







int
agh::CSubject::SEpisodeSequence::add_one( CEDFFile&& Fmc, const SFFTParamSet& fft_params,
					  float max_hours_apart)
{
	auto Ei = find( episodes.begin(), episodes.end(),
			Fmc.episode.c_str());

	if ( Ei == episodes.end() ) {
	      // ensure the newly added episode is well-placed
		for ( auto E = episodes.begin(); E != episodes.end(); ++E ) {
		      // does not overlap with existing ones
			if ( (E->start_time() < Fmc.start_time && Fmc.start_time < E->end_time()) ||
			     (E->start_time() < Fmc.end_time   && Fmc.end_time < E->end_time()) ||
			     (Fmc.start_time < E->start_time() && E->start_time() < Fmc.end_time) ||
			     (Fmc.start_time < E->end_time()   && E->end_time() < Fmc.end_time) )
				return AGH_EPSEQADD_OVERLAP;
		}
		// or is not too far off
		if ( episodes.size() > 0 &&
		     episodes.begin()->sources.size() > 0 &&
		     fabs( difftime( episodes.begin()->sources.begin()->start_time, Fmc.start_time)) / 3600 > max_hours_apart )
			return AGH_EPSEQADD_TOOFAR;

		episodes.emplace_back( static_cast<CEDFFile&&>(Fmc), fft_params);
		episodes.sort();

	} else { // same as SEpisode() but done on an existing one
	      // check that the edf source being added has exactly the same timestamp and duration
		if ( fabs( difftime( Ei->start_time(), Fmc.start_time)) > 1 )
			return AGH_EPSEQADD_TOOFAR;
		Ei->sources.emplace_back( static_cast<CEDFFile&&>(Fmc));
		CEDFFile& F = Ei->sources.back();
		for ( size_t h = 0; h < F.signals.size(); ++h )
			Ei->recordings.insert(
				SEpisode::TRecordingSet::value_type (const_cast<const SChannel&>(F[h].channel),
								     CRecording (F, h, fft_params)));
		// no new episode added: don't sort
	}

      // compute start_rel and end_rel
	// do it for all episodes over again (necessary if the newly added episode becomes the new first)
	SEpisode &e0 = episodes.front();
	struct tm t0;
	time_t start_time_tmp = e0.start_time();
	memcpy( &t0, localtime( &start_time_tmp), sizeof(struct tm));
	t0.tm_year = 101;
	t0.tm_mon = 10;
	t0.tm_mday = 1 + (t0.tm_hour < 12);
	t0.tm_isdst = 0; // must clear this, else a wall clock hour of
			 // 23:00 in summer will become 22:00 in
			 // October, which makes no sense in circadian
			 // context
	e0.start_rel = mktime( &t0);
	// printf( "E0 %s: ", e0.name());
	// puts( asctime( localtime(&e0.start_time())));
	// puts( asctime( localtime(&e0.start_rel)));
	// printf( "--\n");
	double shift = difftime( e0.start_rel, e0.start_time());
	e0.end_rel   = e0.end_time() + shift;


	for_each( next( episodes.begin()), episodes.end(),
		  [&shift] ( SEpisode& E )
		  {
			  E.start_rel	= E.start_time() + shift;
			  E.end_rel	= E.end_time()   + shift;
			  // printf("E %s: ", E.name());
			  // puts( asctime( localtime(&E.start_time())));
			  // puts( asctime( localtime(&E.start_rel)));
			  // printf( "--\n");
		  });

	return episodes.size();
}






// create new session/episode as necessary
int
agh::CExpDesign::register_intree_source( CEDFFile&& F,
					 const char **reason_if_failed_p)
{
	try {
//		CEDFFile F (fname, fft_params.page_size);
	      // parse fname (as appearing in the right place in the
	      // tree) as ./group/subject/session/episode.edf
	      // (We couldn't make life easier for us by passing these
	      // as parameters due to us being called from nftw()).
		string toparse (F.filename());
		if ( strncmp( F.filename(), _session_dir.c_str(), _session_dir.size()) == 0 )
			toparse.erase( 0, _session_dir.size());
		const char
			*g_name = strtok(&toparse[2], "/"),  // skip "./"
			*j_id   = strtok(NULL, "/"),
			*d_name = strtok(NULL, "/");
			//*e_name = F.Episode.c_str();  // except for this, which if of the form episode-1.edf,
							// will still result in 'episode' (handled in CEDFFile(fname))
			// all handled in add_one
		if ( F.patient != j_id ) {
			fprintf( stderr, "CExpDesign::register_intree_source(\"%s\"): file belongs to subject \"%s\", is misplaced here (\"%s\")\n",
				 F.filename(), F.patient.c_str(), j_id);
			return -1;
		}
		if ( F.session != d_name ) {
			fprintf( stderr, "CExpDesign::register_intree_source(\"%s\"): embedded session identifier \"%s\" does not match its session as placed in the tree; using \"%s\"\n",
				 F.filename(), F.session.c_str(), d_name);
			F.session = d_name;
		}

		CSubject *J;
		CJGroup& G = groups[g_name];
		CJGroup::iterator Ji;
		if ( (Ji = find( G.begin(), G.end(), j_id)) == G.end() ) {
			G.emplace_back( _session_dir + '/' + g_name + '/' + j_id, __id_pool++);
			J = &G.back();
		} else
			J = &*Ji;

	      // insert/update episode observing start/end times
		switch ( J->measurements[F.session].add_one( (CEDFFile&&)F, fft_params) ) {  // this will do it
		case AGH_EPSEQADD_OVERLAP:
			fprintf( stderr, "CExpDesign::register_intree_source(\"%s\"): not added as it overlaps with existing episodes\n",
				 F.filename());
			log_message( string(F.filename()) + " not added as it overlaps with existing episodes\n");
			return -1;
		case AGH_EPSEQADD_TOOFAR:
			fprintf( stderr, "CExpDesign::register_intree_source(\"%s\"): not added as it is too far removed from the rest\n",
				 F.filename());
			log_message( string(F.filename()) + " not added as it is too far removed from the rest\n");
			return -1;
		default:
			return 0;
		}
//		fprintf( stderr, "CExpDesign::register_intree_source(\"%s\"): ok\n", toparse());

	} catch (invalid_argument ex) {
		log_message( ex.what() + '\n');
		fprintf( stderr, "CExpDesign::register_intree_source(\"%s\") failed: %s\n", F.filename(), ex.what());
		if ( reason_if_failed_p )
			*reason_if_failed_p = ex.what();
		return -1;
	} catch (int status) {
		log_message( "Bad edf header or data\n");
		if ( reason_if_failed_p )
			*reason_if_failed_p = "Bad edf header or data";
		return -1;
	}
	return 0;
}













// string
// CExpDesign::make_fname_simulation( const char* j, const char* d, const char* h,
// 				   float from, float upto)
// {
// 	UNIQUE_CHARP (x);
// 	if ( asprintf( &x,
// 		       "%s/%s/%s/SIMULATIONS/"
// 		       "B:%g W:%c H:%s F:%g-%g DB1:%s DB2:%s AZ:%s "
// 		       "R:%s%s%s%s%s%s%s%s%s.S",
// 		       group_of(j), j, d,
// 		       fft_params.bin_size,
// 		       'a' + fft_params.welch_window_type,
// 		       h,
// 		       from, upto,
// 		       yesno( ctl_params0.DBAmendment1),
// 		       yesno( ctl_params0.DBAmendment2),
// 		       yesno( ctl_params0.AZAmendment),
// 		       (tunables.step.P[_gc_ ] > 0.)?"g" :"",
// 		       (tunables.step.P[_rs_ ] > 0.)?"r" :"",
// 		       (tunables.step.P[_rc_ ] > 0.)?"c" :"",
// 		       (tunables.step.P[_fcR_] > 0.)?"R" :"",
// 		       (tunables.step.P[_fcW_] > 0.)?"W" :"",
// 		       (tunables.step.P[_S0_ ] > 0.)?"0" :"",
// 		       (tunables.step.P[_SU_ ] > 0.)?"U" :"",
// 		       (tunables.step.P[_ta_ ] > 0.)?"a" :"",
// 		       (tunables.step.P[_tp_ ] > 0.)?"p" :"") )
// 		;
// 	return string (x);
// }





static size_t
	__n_edf_files,
	__cur_edf_file;
static CExpDesign* __expdesign;
static int
edf_file_counter( const char *fname, const struct stat *st, int flag, struct FTW *ftw)
{
	if ( flag == FTW_F && ftw->level == 4 ) {
		int fnlen = strlen(fname); // - ftw->base;
		if ( fnlen < 5 )
			return 0;
		if ( strcasecmp( &fname[fnlen-4], ".edf") == 0 ) {
			printf( "...found %s\n", fname);
			++__n_edf_files;
		}
	}
	return 0;
}

static agh::CExpDesign::TMsmtCollectProgressIndicatorFun only_progress_fun;
static int
edf_file_processor( const char *fname, const struct stat *st, int flag, struct FTW *ftw)
{
	if ( flag == FTW_F && ftw->level == 4 ) {
		int fnlen = strlen(fname); // - ftw->base;
		if ( fnlen < 5 )
			return 0;
		if ( strcasecmp( &fname[fnlen-4], ".edf") == 0 ) {
			++__cur_edf_file;
			only_progress_fun( fname, __n_edf_files, __cur_edf_file);
			try {
				CEDFFile f_tmp (fname, __expdesign->fft_params.page_size);
				string st = CEDFFile::explain_edf_status( f_tmp.status());
				if ( st.size() )
					__expdesign->log_message( string (fname) + ": "+ st + '\n');
				if ( __expdesign -> register_intree_source( (CEDFFile&&)f_tmp) )
					;

			} catch ( invalid_argument ex) {
				UNIQUE_CHARP (_);
				if ( asprintf( &_, "collect_msmts_from_tree(): edf source \"%s\" could not be read (%s)\n",
					       fname, ex.what()) )
					;
				fprintf( stderr, "%s", _);
				__expdesign->log_message(_);
			}
		}
	}
	return 0;
}





inline namespace {
	typedef pair <time_t, size_t> TTimePair;
	pair<float, float> avg_tm( vector<TTimePair>&);
}

void
agh::CExpDesign::scan_tree( TMsmtCollectProgressIndicatorFun user_progress_fun)
{
	groups.clear();

      // glob it!
	__n_edf_files = 0;
	nftw( "./", edf_file_counter, 20, 0);
	fprintf( stderr, "CExpDesign::scan_tree(\"%s\"): %zu edf file(s) found\n",
		 session_dir(), __n_edf_files);
	if ( __n_edf_files == 0 )
		return;

	__cur_edf_file = 0;
	only_progress_fun = user_progress_fun;
	__expdesign = this;
	nftw( "./", edf_file_processor, 10, 0);

	fprintf( stderr, "CExpDesign::scan_tree() completed\n");

      // find any subjects with incomplete episode sets
	list<string> complete_episode_set = enumerate_episodes();
	size_t	n_episodes = complete_episode_set.size();

startover:
	for ( auto G = groups.begin(); G != groups.end(); ++G )
		for ( auto J = subject_in_group_begin(G); J != subject_in_group_end(G); ++J )
			try {
				for ( auto D = J->measurements.begin(); D != J->measurements.end(); ++D )
					if ( D->second.episodes.size() < n_episodes &&
					     *complete_episode_set.begin() != D->second.episodes.begin()->name() )  // the baseline is missing
						throw "no baseline";
			} catch ( const char *ex) {
				fprintf( stderr, "Subject %s has their Baseline episode missing and will not be included\n", J->name());
				log_message( string("Missing Baseline episode in subject ") + J->name() + ": subject will not be included");
				G->second.erase(J);
				goto startover;
			}

	list<string> complete_session_set = enumerate_sessions();
      // calculate average episode times
	for ( auto Gi = groups.begin(); Gi != groups.end(); ++Gi ) {
		map <string, map<string, vector <TTimePair>>> tms;
		for ( auto Ji = subject_in_group_begin(Gi); Ji != subject_in_group_end(Gi); ++Ji )
			for ( auto Di = Ji->measurements.begin(); Di != Ji->measurements.end(); ++Di )
				for ( auto Ei = Di->second.episodes.begin(); Ei != Di->second.episodes.end(); ++Ei )
					tms[Di->first][Ei->name()].emplace_back(
						Ei->sources.begin() -> start_time,
						Ei->sources.begin() -> length());
		for ( auto Dj = complete_session_set.begin(); Dj != complete_session_set.end(); ++Dj )
			for ( auto Ej = complete_episode_set.begin(); Ej != complete_episode_set.end(); ++ Ej )
				Gi->second.avg_episode_times[*Dj][*Ej] =
					avg_tm( tms[*Dj][*Ej]);
	}
}

inline namespace {
	pair<float, float> // as fractions of day
	avg_tm( vector<TTimePair>& tms)
	{
		float avg_start = 0., avg_end = 0.;
		for ( auto T = tms.begin(); T != tms.end(); ++T ) {
			struct tm
				t0 = *localtime( &T->first);
			if ( t0.tm_hour > 12 )
				t0.tm_hour -= 24;  // go negative if we must
			t0.tm_hour += 24;   // pull back into positive
			float this_j_start = (t0.tm_hour/24. + t0.tm_min/24./60. + t0.tm_sec/24./60./60.);
			avg_start += this_j_start;
			avg_end   += (this_j_start + T->second/3600./24.);
		}

		return pair<float, float> (avg_start / tms.size(), avg_end / tms.size());
	}
}


void
agh::CExpDesign::remove_untried_modruns()
{
	for ( auto Gi = groups_begin(); Gi != groups_end(); ++Gi )
		for ( auto Ji = Gi->second.begin(); Ji != Gi->second.end(); ++Ji )
			for ( auto Di = Ji->measurements.begin(); Di != Ji->measurements.end(); ++Di )
			retry_modruns:
				for ( auto RSi = Di->second.modrun_sets.begin(); RSi != Di->second.modrun_sets.end(); ++RSi ) {
				retry_this_modrun_set:
					for ( auto Ri = RSi->second.begin(); Ri != RSi->second.end(); ++Ri )
						if ( !(Ri->second.status & CModelRun::modrun_tried) ) {
							RSi->second.erase( Ri);
							goto retry_this_modrun_set;
						}
					if ( RSi->second.empty() ) {
						Di->second.modrun_sets.erase( RSi);
						goto retry_modruns;
					}
				}
}


void
agh::CExpDesign::export_all_modruns( const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return;

	auto t = TTunable::rs;
	fprintf( f, "#");
	for ( ; t < TTunable::_all_tunables; ++t )
		fprintf( f, "\t%s", STunableSet::tunable_name(t).c_str());
	fprintf( f, "\n");

	for ( auto Gi = groups.cbegin(); Gi != groups.cend(); ++Gi )
		for ( auto Ji = Gi->second.cbegin(); Ji != Gi->second.cend(); ++Ji )
			for ( auto Di = Ji->measurements.cbegin(); Di != Ji->measurements.cend(); ++Di )
				for ( auto RSi = Di->second.modrun_sets.cbegin(); RSi != Di->second.modrun_sets.cend(); ++RSi )
					for ( auto Ri = RSi->second.cbegin(); Ri != RSi->second.cend(); ++Ri )
						if ( Ri->second.status & CModelRun::modrun_tried ) {
							fprintf( f, "# ----- Subject: %s;  Session: %s;  Channel: %s;  Range: %g-%g Hz\n",
								 Ri->second.subject(), Ri->second.session(), Ri->second.channel(),
								 Ri->second.freq_from(), Ri->second.freq_upto());
							t = TTunable::rs;
							do {
								fprintf( f, "%g%s", Ri->second.cur_tset[t] * STunableSet::stock[(TTunable_underlying_type)t].display_scale_factor,
									 (t == Ri->second.cur_tset.last()) ? "\n" : "\t");
							} while ( ++t != (TTunable)Ri->second.cur_tset.size() );
						}

	fclose( f);
}






void
agh::CExpDesign::sync() const
{
	for ( auto Gi = groups.cbegin(); Gi != groups.cend(); ++Gi )
		for ( auto Ji = Gi->second.cbegin(); Ji != Gi->second.cend(); ++Ji )
			for ( auto Di = Ji->measurements.cbegin(); Di != Ji->measurements.cend(); ++Di )
				for ( auto Ei = Di->second.episodes.cbegin(); Ei != Di->second.episodes.cend(); ++Ei )
					for ( auto Fi = Ei->sources.cbegin(); Fi != Ei->sources.cend(); ++Fi )
						Fi->write_ancillry_files();
}


// EOF
