// ;-*-C++-*-
/*
 *       File name:  core/primaries.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  experimental design primary classes: CMeasurement,
 *         	     CSubject & CExpDesign
 *
 *         License:  GPL
 */


#include <stdarg.h>
#include <limits.h>
#include <cfloat>
#include <cstdlib>
#include <string>
#include <functional>
#include <initializer_list>

#include <ftw.h>

#include "../common/misc.hh"
#include "boost-config-validate.hh"
#include "primaries.hh"
#include "model.hh"


using namespace std;
using namespace agh;


agh::CRecording::CRecording( sigfile::CSource& F, int sig_no,
			     const sigfile::SFFTParamSet& fft_params,
			     const sigfile::SMCParamSet& mc_params)
      : CBinnedPower (F, sig_no, fft_params),
	CBinnedMC (F, sig_no, mc_params),
	_status (0),
	_source (F), _sig_no (sig_no)
{
	if ( F.signal_type(sig_no) == sigfile::SChannel::TType::eeg ) {
		CBinnedPower::compute();
		CBinnedMC::compute();
	}
}


inline namespace {
int
mkdir_with_parents( const char *dir)
{
	DEF_UNIQUE_CHARP(_);
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
} // inline namespace


agh::CExpDesign::TMsmtCollectProgressIndicatorFun
	agh::CExpDesign::progress_fun_stdout = progress_fun_stdout_fo();

agh::CExpDesign::CExpDesign( const string& session_dir_,
			     TMsmtCollectProgressIndicatorFun progress_fun)
      : _session_dir (session_dir_),
	__id_pool (0),
	fft_params ({30, sigfile::SFFTParamSet::TWinType::welch, .5}),
	mc_params ({30, 9, -9, 10, .5, -2000, 30000, .2, 10., 7., 1, 2., .5, 1., 1.8, 1.5, 1./60}),
	af_dampen_window_type (sigfile::SFFTParamSet::TWinType::welch),
	af_dampen_factor (.95),
	config_keys_g ({
		SValidator<double>("ctlparam.StepSize",		&ctl_params0.siman_params.step_size),
		SValidator<double>("ctlparam.Boltzmannk",	&ctl_params0.siman_params.k,			SValidator<double>::SVFRange( DBL_MIN, 1e9)),
		SValidator<double>("ctlparam.TInitial",		&ctl_params0.siman_params.t_initial,		SValidator<double>::SVFRange( DBL_MIN, 1e9)),
		SValidator<double>("ctlparam.DampingMu",	&ctl_params0.siman_params.mu_t,			SValidator<double>::SVFRange( DBL_MIN, 1e9)),
		SValidator<double>("ctlparam.TMin",		&ctl_params0.siman_params.t_min,		SValidator<double>::SVFRange( DBL_MIN, 1e9)),
		SValidator<double>("ctlparam.ReqScoredPC",	(double*)&ctl_params0.req_percent_scored,	SValidator<double>::SVFRange( 80., 100.)),
			SValidator<double>("fftparam.BinSize",		&fft_params.binsize,			SValidator<double>::SVFRange( .1, 1.)),
		SValidator<double>("artifacts.DampenFactor",	&af_dampen_factor,				SValidator<double>::SVFRange( 0., 1.)),
		SValidator<double>("mcparam.iir_backpolate",	&mc_params.iir_backpolate,			SValidator<double>::SVFRange( 0., 1.)),
		SValidator<double>("mcparam.pib_peak_width",	&mc_params.pib_peak_width,			SValidator<double>::SVFRange( 0., 1.)),
		SValidator<double>("mcparam.mc_gain",		&mc_params.mc_gain,				SValidator<double>::SVFRange( 0., 100.)),
		SValidator<double>("mcparam.art_max_secs",	&mc_params.art_max_secs,			SValidator<double>::SVFRange( 0., 999.)),
		SValidator<double>("mcparam.mc_event_reject",	&mc_params.mc_event_reject,			SValidator<double>::SVFRange( 0., 100.)),
		SValidator<double>("mcparam.mc_jump_find",	&mc_params.mc_jump_find,			SValidator<double>::SVFRange( 0., 100.)),
		SValidator<double>("mcparam.f0",		&mc_params.f0,					SValidator<double>::SVFRange( 0., 80.)),
		SValidator<double>("mcparam.fc",		&mc_params.fc,					SValidator<double>::SVFRange( 0., 80.)),
		SValidator<double>("mcparam.band_width",	&mc_params.band_width,				SValidator<double>::SVFRange( 0., 5.)),
		SValidator<double>("mcparam.smooth_rate",	&mc_params.smooth_rate,				SValidator<double>::SVFRange( 0., 99.)),
	}),
	config_keys_d ({
		SValidator<int>("fftparam.WelchWindowType",	(int*)&fft_params.welch_window_type,		SValidator<int>::SVFRange( 0, (int)sigfile::SFFTParamSet::TWinType::_total - 1)),
		SValidator<int>("artifacts.DampenWindowType",	(int*)&af_dampen_window_type,			SValidator<int>::SVFRange( 0, (int)sigfile::SFFTParamSet::TWinType::_total - 1)),
		SValidator<int>("ctlparam.ItersFixedT",		&ctl_params0.siman_params.iters_fixed_T,	SValidator<int>::SVFRange( 1, 1000000)),
		SValidator<int>("ctlparam.NTries",		&ctl_params0.siman_params.n_tries,		SValidator<int>::SVFRange( 1, 10000)),
		SValidator<int>("mcparam.xpi_bplus",		&mc_params.xpi_bplus,				SValidator<int>::SVFRange( 0, 99)),
		SValidator<int>("mcparam.xpi_bminus",		&mc_params.xpi_bminus,				SValidator<int>::SVFRange( -99, 0)),
		SValidator<int>("mcparam.xpi_bzero",		&mc_params.xpi_bzero,				SValidator<int>::SVFRange( 0, 99)),
		SValidator<int>("mcparam.ss_su_min",		&mc_params.ss_su_min,				SValidator<int>::SVFRange( -INT_MAX, 0)),
		SValidator<int>("mcparam.ss_su_max",		&mc_params.ss_su_min,				SValidator<int>::SVFRange( 0, +INT_MAX)),
	}),
	config_keys_z ({
		SValidator<size_t>("ctlparam.NSWALadenPagesBeforeSWA0",	&ctl_params0.swa_laden_pages_before_SWA_0,	SValidator<size_t>::SVFRange( 1, 100)),
		SValidator<size_t>("fftparam.PageSize",			&fft_params.pagesize,				SValidator<size_t>::SVFRange( 4, 120)),
		SValidator<size_t>("mcparam.mc_event_duration",		&mc_params.mc_event_duration,			SValidator<double>::SVFRange( 0, 999)),
	}),
	config_keys_b ({
		SValidator<bool>("ctlparam.DBAmendment1",		&ctl_params0.DBAmendment1),
		SValidator<bool>("ctlparam.DBAmendment2",		&ctl_params0.DBAmendment2),
		SValidator<bool>("ctlparam.AZAmendment1",		&ctl_params0.AZAmendment1),
		SValidator<bool>("ctlparam.AZAmendment2",		&ctl_params0.AZAmendment2),
		SValidator<bool>("ctlparam.ScoreUnscoredAsWake",	&ctl_params0.ScoreUnscoredAsWake),
	})
{
      // ensure this
	// if ( _session_dir.back() == '/' )
	// 	_session_dir.pop_back();
	if ( _session_dir[_session_dir.size()-1] == '/' )
		_session_dir.erase( _session_dir.size()-1, 1);

	if ( chdir( session_dir()) == -1 ) {
		printf( "CExpDesign::CExpDesign(): Could not cd to \"%s\"; trying to create a new directory there...", session_dir());
		if ( mkdir_with_parents( session_dir()) || chdir( session_dir()) != -1 )
			printf( "done\n");
		else {
			printf( "failed\n");
			throw init_fail;
		}
	} else {
		if ( load_settings() )
			;
		scan_tree( progress_fun);
	}
}


void
agh::CExpDesign::
log_message( const char* fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);

	DEF_UNIQUE_CHARP(buf);
	assert (vasprintf( &buf, fmt, ap) > 0);

	_error_log += buf;
	va_end (ap);
}



list<string>
agh::CExpDesign::enumerate_groups() const
{
	list<string> recp;
	for ( auto &G : groups )
		recp.push_back( G.first);
	return recp;
}

list<string>
agh::CExpDesign::enumerate_subjects() const
{
	list<string> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			recp.push_back( J.name());
	return recp;
}


list<string>
agh::CExpDesign::enumerate_sessions() const
{
	list<string> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				recp.push_back( D.first);
	recp.sort();
	recp.unique();
	return recp;
}

list<string>
agh::CExpDesign::enumerate_episodes() const
{
	list<string> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					recp.push_back( E.name());
	recp.sort();
	recp.unique();
	return recp;
}

list<sigfile::SChannel>
agh::CExpDesign::enumerate_eeg_channels() const
{
	list<sigfile::SChannel> recp;
// 	list<sigfile::SChannel> recp = enumerate_all_channels();
// 	recp.remove_if(
// //		bind( sigfile::SChannel::channel_is_fftable, _1));
// 		[] ( const sigfile::SChannel& H) { return not sigfile::SChannel::channel_is_fftable(H); });
/// sigfile::SChannel will rightly not count oddly named channels
/// which, still, were additionally qualified as EEG in the EDF
/// header, so we'd better walk it again and look at signal type
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources )
						for ( size_t h = 0; h < F.n_channels(); ++h )
							if ( F.signal_type(h) == sigfile::SChannel::TType::eeg )
								recp.push_back( F.channel_by_id(h));
	recp.sort();
	recp.unique();
	return recp;
}

list<sigfile::SChannel>
agh::CExpDesign::enumerate_all_channels() const
{
	list<sigfile::SChannel> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources ) {
						auto Ins = F.channel_list();
						recp.insert( recp.end(),
							     Ins.begin(), Ins.end());
					}
	recp.sort();
	recp.unique();
	return recp;
}



const char*
agh::CSubject::gender_sign( TGender g)
{
	switch ( g ) {
	case TGender::male:
		return "M";
	case TGender::female:
		return "F";
	case TGender::neuter:
		return "o";
	default:
		return "??";
	}
}

agh::CSubject::CSubject( const string& dir,
			 sid_type id)
  : _status (0),
    _id (id),
    _dir (dir),
    _name (dir.substr( dir.rfind('/')+1)),
    full_name (_name),
    gender (TGender::neuter),
    age (-1)
{
	ifstream ifs (_dir + "/.subject_info");
	char gender_char;
	if ( ifs.good() and
	     (getline( ifs, full_name, '\n'),
	      ifs >> gender_char >> age,
	      getline( ifs, comment, '\n'),
	      ifs.good()) )
		gender = (TGender)gender_char;
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







agh::CSubject::SEpisode::SEpisode( sigfile::CSource&& Fmc,
				   const sigfile::SFFTParamSet& fft_params,
				   const sigfile::SMCParamSet& mc_params)
{
      // move it in place
	sources.emplace_back( static_cast<sigfile::CSource&&>(Fmc));
	auto& F = sources.back();
	auto HH = F.channel_list();
	printf( "CSubject::SEpisode::SEpisode( \"%s\"): %s\n",
		F.filename(), string_join(HH, ", ").c_str());
	int h = 0;
	for ( auto &H : HH )
		recordings.insert(
			{H, CRecording (F, h++, fft_params, mc_params)});
}


list<agh::CSubject::SEpisode::SAnnotation>
agh::CSubject::SEpisode::get_annotations() const
{
	list<agh::CSubject::SEpisode::SAnnotation>
		ret;
	for ( auto &F : sources ) {
		auto HH = F.channel_list();
		for ( size_t h = 0; h < HH.size(); ++h ) {
			auto &AA = F.annotations(h);
			for ( auto &A : AA )
				ret.emplace_back( F, h, A);
		}
	}
	ret.sort();
	return ret;
}




#define AGH_EPSEQADD_OVERLAP -1
#define AGH_EPSEQADD_TOOFAR  -2

int
agh::CSubject::SEpisodeSequence::
add_one( sigfile::CSource&& Fmc,
	 const sigfile::SFFTParamSet& fft_params,
	 const sigfile::SMCParamSet& mc_params,
	 float max_hours_apart)
{
	auto Ei = find( episodes.begin(), episodes.end(),
			Fmc.episode());

	if ( Ei == episodes.end() ) {
	      // ensure the newly added episode is well-placed
		for ( auto &E : episodes ) {
		      // does not overlap with existing ones
			if ( (E.start_time() < Fmc.start_time() && Fmc.start_time() < E.end_time()) ||
			     (E.start_time() < Fmc.end_time()   && Fmc.end_time() < E.end_time()) ||
			     (Fmc.start_time() < E.start_time() && E.start_time() < Fmc.end_time()) ||
			     (Fmc.start_time() < E.end_time()   && E.end_time() < Fmc.end_time()) )
				return AGH_EPSEQADD_OVERLAP;
		}
		// or is not too far off
		if ( episodes.size() > 0 &&
		     episodes.begin()->sources.size() > 0 &&
		     fabs( difftime( episodes.begin()->sources.begin()->start_time(), Fmc.start_time())) / 3600 > max_hours_apart )
			return AGH_EPSEQADD_TOOFAR;

		printf( "CSubject::SEpisodeSequence::add_one( \"%s\")\n",
			Fmc.filename());
		episodes.emplace_back( static_cast<sigfile::CSource&&>(Fmc), fft_params, mc_params);
		episodes.sort();

	} else { // same as SEpisode() but done on an existing one
	      // check that the edf source being added has exactly the same timestamp and duration
		printf( "CSubject::SEpisodeSequence::add_one( \"%s\") try in-place\n",
			Fmc.filename());
		if ( fabs( difftime( Ei->start_time(), Fmc.start_time())) > 1 )
			return AGH_EPSEQADD_TOOFAR;
		Ei->sources.emplace_back( static_cast<sigfile::CSource&&>(Fmc));
		auto& F = Ei->sources.back();
		auto HH = F.channel_list();
		int h = 0;
		for ( auto &H : HH )
			Ei->recordings.insert(
				{H, {F, h++, fft_params, mc_params}});
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
agh::CExpDesign::register_intree_source( sigfile::CSource&& F,
					 const char **reason_if_failed_p)
{
	try {
	      // parse fname (as appearing in the right place in the
	      // tree) as ./group/subject/session/episode.edf
	      // in order to validate this source wrt its placement in the tree
		string toparse (F.filename());
		if ( strncmp( F.filename(), _session_dir.c_str(), _session_dir.size()) == 0 )
			toparse.erase( 0, _session_dir.size());
		list<string> broken_path = fs::path_elements( toparse);
		assert ( broken_path.size() == 5 );
		list<string>::iterator pe = broken_path.begin();
		string& g_name = (pe = next(pe), *pe),
			j_name = (pe = next(pe), *pe),
			d_name = (pe = next(pe), *pe),
			e_name = fs::make_fname_base(*next(pe), ".edf", false);
		// take care of the case of episode-2.edf
		if ( e_name.size() >= 3 /* strlen("a-1") */ ) {
			size_t sz = e_name.size();
			if ( e_name[sz-2] == '-' && isdigit(e_name[sz-1]) )
				e_name.erase( sz-2, 2);
		}

		// refuse to register sources of wrong subjects
		if ( j_name != F.subject() ) {
			fprintf( stderr, "CExpDesign::register_intree_source(\"%s\"): file belongs to subject \"%s\", is misplaced here (\"%s\")\n",
				 F.filename(), F.subject(), j_name.c_str());
			log_message( "%s: file belongs to subject \"%s\", is misplaced here under subject \"%s\"\n",
				     F.filename(), F.subject(), j_name.c_str());
			return -1;
		}
		try {
			auto existing_group = group_of( F.subject());
			if ( g_name != existing_group ) {
				fprintf( stderr, "CExpDesign::register_intree_source(\"%s\"): subject \"%s\" belongs to a different group (\"%s\")\n",
					 F.filename(), F.subject(), existing_group);
				log_message( "%s: subject \"%s\" belongs to a different group (\"%s\")\n",
					     F.filename(), F.subject(), existing_group);
				return -1;
			}
		} catch (invalid_argument) {
			;
		}

		// but correct session/episode fields
		if ( d_name != F.session() ) {
			log_message( "%s: correcting embedded session \"%s\" to match placement in the tree (\"%s\")\n",
				     F.filename(), F.session(), d_name.c_str());
			printf( "CExpDesign::register_intree_source(\"%s\"): correcting embedded session \"%s\" to match placement in the tree (\"%s\")\n",
				F.filename(), F.session(), d_name.c_str());
			F.set_session( d_name.c_str());
		}
		if ( e_name != F.episode() ) {
			log_message( "%s: correcting embedded episode \"%s\" to match file name\n",
				     F.filename(), F.episode());
			printf( "CExpDesign::register_intree_source(\"%s\"): correcting embedded episode \"%s\" to match file name\n",
				F.filename(), F.episode());
			F.set_episode( e_name.c_str());
		}

		CSubject *J;
		CJGroup& G = groups[g_name];
		CJGroup::iterator Ji;
		if ( (Ji = find( G.begin(), G.end(), j_name)) == G.end() ) {
			G.emplace_back( _session_dir + '/' + g_name + '/' + j_name, __id_pool++);
			J = &G.back();
		} else
			J = &*Ji;

	      // insert/update episode observing start/end times
		// printf( "\nCExpDesign::register_intree_source( file: \"%s\", J: \"%s\", E: \"%s\", D: \"%s\")\n",
		// 	   F.filename(), F.subject(), F.episode(), F.session());
		switch ( J->measurements[F.session()].add_one(
				 (sigfile::CSource&&)F, fft_params, mc_params) ) {  // this will do it
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
				sigfile::CSource f_tmp {fname, __expdesign->fft_params.pagesize};
				string st = f_tmp.explain_status();
				if ( not st.empty() )
					__expdesign->log_message( string (fname) + ":\n"+ st + '\n');
				if ( __expdesign -> register_intree_source( (sigfile::CSource&&)f_tmp) )
					;

			} catch ( invalid_argument ex) {
				DEF_UNIQUE_CHARP (_);
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
	printf( "CExpDesign::scan_tree(\"%s\"): %zu edf file(s) found\n",
		session_dir(), __n_edf_files);
	if ( __n_edf_files == 0 )
		return;

	__cur_edf_file = 0;
	only_progress_fun = user_progress_fun;
	__expdesign = this;
	nftw( "./", edf_file_processor, 10, 0);

	printf( "CExpDesign::scan_tree() completed\n");

      // find any subjects with incomplete episode sets
	list<string> complete_episode_set = enumerate_episodes();
	size_t	n_episodes = complete_episode_set.size();

	for ( auto &G : groups )
		for ( auto &J : G.second )
		startover:
			for ( auto &D : J.measurements )
				if ( D.second.episodes.size() < n_episodes &&
				     complete_episode_set.front() != D.second.episodes.begin()->name() ) { // the baseline is missing
					fprintf( stderr,
						 "No Baseline episode in %s's %s: skip this session\n",
						 J.name(), D.first.c_str());
					log_message( string("Missing Baseline episode in ") + D.first + " for "
						     + G.first + '/' + J.name()
						     + ": skipping this session\n");
					J.measurements.erase(D.first);
					goto startover;
				}

	list<string> complete_session_set = enumerate_sessions();
      // calculate average episode times
	for ( auto &G : groups ) {
		map <string, map<string, vector <TTimePair>>> tms;
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					tms[D.first][E.name()].emplace_back(
						E.sources.front().start_time(),
						E.sources.front().recording_time());
		for ( auto &D : complete_session_set )
			for ( auto &E : complete_episode_set )
				G.second.avg_episode_times[D][E] =
					avg_tm( tms[D][E]);
	}
}

inline namespace {
pair<float, float> // as fractions of day
avg_tm( vector<TTimePair>& tms)
{
	float avg_start = 0., avg_end = 0.;
	for ( auto &T : tms ) {
		struct tm
			t0 = *localtime( &T.first);
		if ( t0.tm_hour > 12 )
			t0.tm_hour -= 24;  // go negative if we must
		t0.tm_hour += 24;   // pull back into positive
		float this_j_start = (t0.tm_hour/24. + t0.tm_min/24./60. + t0.tm_sec/24./60./60.);
		avg_start += this_j_start;
		avg_end   += (this_j_start + T.second/3600./24.);
	}

	return pair<float, float> (avg_start / tms.size(), avg_end / tms.size());
}
}


void
agh::CExpDesign::remove_untried_modruns()
{
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
			retry_modruns:
				for ( auto RSi = D.second.modrun_sets.begin(); RSi != D.second.modrun_sets.end(); ++RSi ) {
				retry_this_modrun_set:
					for ( auto Ri = RSi->second.begin(); Ri != RSi->second.end(); ++Ri ) {
						// printf( "#----- check Subject: %s;  Session: %s;  Channel: %s;  Range: %g-%g Hz (%d)\n",
						// 	Ri->second.subject(), Ri->second.session(), Ri->second.channel(),
						// 	Ri->second.freq_from(), Ri->second.freq_upto(),
						// 	Ri->second.status);
						if ( !(Ri->second.status & CModelRun::modrun_tried) ) {
							RSi->second.erase( Ri);
							goto retry_this_modrun_set;
						}
					}
					if ( RSi->second.empty() ) {
						D.second.modrun_sets.erase( RSi);
						goto retry_modruns;
					}
				}
}

void
agh::CExpDesign::remove_all_modruns()
{
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				D.second.modrun_sets.clear();
}


void
agh::CExpDesign::export_all_modruns( const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return;

	size_t t = (size_t)TTunable::rs;
	fprintf( f, "#");
	for ( ; t < TTunable::_all_tunables; ++t )
		fprintf( f, "%s%s", (t == 0) ? "" : "\t", STunableSet::tunable_name(t).c_str());
	fprintf( f, "\n");

	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &RS : D.second.modrun_sets )
					for ( auto &R : RS.second )
						if ( R.second.status & CModelRun::modrun_tried ) {
							fprintf( f, "# ----- Subject: %s;  Session: %s;  Channel: %s;  Range: %g-%g Hz\n",
								 R.second.subject(), R.second.session(), R.second.channel(),
								 R.second.freq_from(), R.second.freq_upto());
							t = TTunable::rs;
							do {
								fprintf( f, "%g%s", R.second.cur_tset[t] * STunableSet::stock[t].display_scale_factor,
									 (t < R.second.cur_tset.last()) ? "\t" : "\n");
							} while ( t++ < R.second.cur_tset.last() );
						}

	fclose( f);
}






void
agh::CExpDesign::sync()
{
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources )
						F.write_ancillary_files();
}


// eof
