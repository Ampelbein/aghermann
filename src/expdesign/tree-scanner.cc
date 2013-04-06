/*
 *       File name:  expdesign/tree-scanner.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-19
 *
 *         Purpose:  CExpDesign tree scanner
 *
 *         License:  GPL
 */


#include <ftw.h>
#include <cassert>
#include <string>

#include "common/alg.hh"
#include "primaries.hh"


using namespace std;



namespace {
struct progress_fun_stdout_fo {
	void operator() ( const char* current, size_t n, size_t i) const
		{
			printf( "(%zu of %zu) %s\n", i, n, current);
		}
};
} // namespace

agh::CExpDesign::TMsmtCollectProgressIndicatorFun
	agh::CExpDesign::progress_fun_stdout = progress_fun_stdout_fo();



#define AGH_EPSEQADD_OVERLAP -1
#define AGH_EPSEQADD_TOOFAR  -2

int
agh::CSubject::SEpisodeSequence::
add_one( sigfile::CTypedSource&& Fmc,
	 const metrics::psd::SPPack& fft_params,
	 const metrics::swu::SPPack& swu_params,
	 const metrics::mc::SPPack& mc_params,
	 float max_hours_apart)
{
	auto Ei = find( episodes.begin(), episodes.end(),
			Fmc().episode());

	if ( Ei == episodes.end() ) {
	      // ensure the newly added episode is well-placed
		for ( auto &E : episodes )
		      // does not overlap with existing ones
			if ( agh::alg::overlap(
				     E.start_time(), E.end_time(),
				     Fmc().start_time(), Fmc().end_time()) )
				return AGH_EPSEQADD_OVERLAP;

		// or is not too far off
		if ( episodes.size() > 0 &&
		     episodes.begin()->sources.size() > 0 &&
		     fabs( difftime( (*episodes.begin()->sources.begin())().start_time(), Fmc().start_time())) / 3600 > max_hours_apart )
			return AGH_EPSEQADD_TOOFAR;

		episodes.emplace_back( move(Fmc), fft_params, swu_params, mc_params);
		episodes.sort();

	} else { // same as SEpisode() but done on an existing one
	      // check that the edf source being added has exactly the same timestamp and duration
		printf( "CSubject::SEpisodeSequence::add_one( \"%s\") try in-place\n",
			Fmc().filename());
		if ( fabs( difftime( Ei->start_time(), Fmc().start_time())) > 1 )
			return AGH_EPSEQADD_TOOFAR;
		Ei->sources.emplace_back( move(Fmc));
		auto& F = Ei->sources.back();
		auto HH = F().channel_list();
		int h = 0;
		for ( auto &H : HH )
			Ei->recordings.insert( {H, {F, h++, fft_params, swu_params, mc_params}});
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
agh::CExpDesign::
register_intree_source( sigfile::CTypedSource&& F,
			const char **reason_if_failed_p)
{
	try {
	      // parse fname (as appearing in the right place in the
	      // tree) as ./group/subject/session/episode.edf
	      // in order to validate this source wrt its placement in the tree
		string toparse (F().filename());
		if ( strncmp( F().filename(), _session_dir.c_str(), _session_dir.size()) == 0 )
			toparse.erase( 0, _session_dir.size());
		list<string> broken_path = agh::fs::path_elements( toparse);
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
		if ( j_name != F().subject() ) {
			log_message( "%s: file belongs to subject \"%s\", is misplaced here under subject \"%s\"\n",
				     F().filename(), F().subject(), j_name.c_str());
			return -1;
		}
		try {
			auto existing_group = group_of( F().subject());
			if ( g_name != existing_group ) {
				log_message( "%s: subject \"%s\" belongs to a different group (\"%s\")\n",
					     F().filename(), F().subject(), existing_group);
				return -1;
			}
		} catch (invalid_argument) {
			;
		}

		// but correct session/episode fields
		if ( d_name != F().session() ) {
			log_message( "%s: correcting embedded session \"%s\" to match placement in the tree (\"%s\")\n",
				     F().filename(), F().session(), d_name.c_str());
			F().set_session( d_name.c_str());
		}
		if ( e_name != F().episode() ) {
			log_message( "%s: correcting embedded episode \"%s\" to match file name\n",
				     F().filename(), F().episode());
			F().set_episode( e_name.c_str());
		}

		CSubject *J;
		CJGroup& G = groups[g_name];
		CJGroup::iterator Ji;
		if ( (Ji = find( G.begin(), G.end(), j_name)) == G.end() ) {
			G.emplace_back( _session_dir + '/' + g_name + '/' + j_name, _id_pool++);
			J = &G.back();
		} else
			J = &*Ji;

	      // insert/update episode observing start/end times
		printf( "\nCExpDesign::register_intree_source( file: \"%s\", J: \"%s\", E: \"%s\", D: \"%s\")\n",
			F().filename(), F().subject(), F().episode(), F().session());
		switch ( J->measurements[F().session()].add_one(
				 move(F), fft_params, swu_params, mc_params) ) {  // this will do it
		case AGH_EPSEQADD_OVERLAP:
			log_message( "%s: not added as it overlaps with existing episodes\n",
				     F().filename());
			return -1;
		case AGH_EPSEQADD_TOOFAR:
			log_message( "%s: not added as it is too far removed from the rest\n",
				     F().filename());
			return -1;
		default:
			return 0;
		}

	} catch (invalid_argument ex) {
		log_message( ex.what() + '\n');
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








namespace {

size_t
	__cur_edf_file;
agh::CExpDesign
	*__expdesign;

agh::CExpDesign::TMsmtCollectProgressIndicatorFun
	only_progress_fun;

int
edf_file_processor( const char *fname, const struct stat*, int flag, struct FTW *ftw)
{
	if ( flag == FTW_F && ftw->level == 4 ) {
		int fnlen = strlen(fname); // - ftw->base;
		if ( fnlen < 5 )
			return 0;
		if ( strcasecmp( &fname[fnlen-4], ".edf") == 0 ) {
			++__cur_edf_file;
			only_progress_fun( fname, agh::fs::__n_edf_files, __cur_edf_file);
			try {
				sigfile::CTypedSource f_tmp {fname, __expdesign->fft_params.pagesize};
				string st = f_tmp().explain_status();
				if ( not st.empty() )
					__expdesign->log_message( "%s: %s\n", fname, st.c_str());
				__expdesign -> register_intree_source( move(f_tmp));

			} catch ( invalid_argument ex) {
				__expdesign->log_message(ex.what());
			}
		}
	}
	return 0;
}



typedef pair<time_t, size_t> TTimePair;

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

} // namespace

void
agh::CExpDesign::
scan_tree( TMsmtCollectProgressIndicatorFun user_progress_fun)
{
	groups.clear();

      // glob it!
	agh::fs::__n_edf_files = 0;
	nftw( "./", agh::fs::edf_file_counter, 20, 0);
	printf( "CExpDesign::scan_tree(\"%s\"): %zu edf file(s) found\n",
		session_dir().c_str(), agh::fs::__n_edf_files);
	if ( agh::fs::__n_edf_files == 0 )
		return;

	__cur_edf_file = 0;
	only_progress_fun = user_progress_fun;
	__expdesign = this;
	nftw( "./", edf_file_processor, 10, 0);
	printf( "CExpDesign::scan_tree(): recordings collected\n");

	compute_profiles(); // in an SMP fashion
	printf( "CExpDesign::scan_tree(): all computed\n");

      // find any subjects with incomplete episode sets
	list<string> complete_episode_set = enumerate_episodes();
	size_t	n_episodes = complete_episode_set.size();

	for ( auto &G : groups )
		for ( auto &J : G.second )
		startover:
			for ( auto &D : J.measurements )
				if ( D.second.episodes.size() < n_episodes &&
				     complete_episode_set.front() != D.second.episodes.begin()->name() ) { // the baseline is missing
					log_message( "No Baseline episode in %s's %s: skip this session\n",
						     J.short_name.c_str(), D.first.c_str());
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
						E.sources.front()().start_time(),
						E.sources.front()().recording_time());
		for ( auto &D : complete_session_set )
			for ( auto &E : complete_episode_set )
				G.second.avg_episode_times[D][E] =
					avg_tm( tms[D][E]);
	}
}


void
agh::CExpDesign::
compute_profiles()
{
	TRecordingOpFun F =
		[&]( CRecording& R)
		{
			R.psd_profile.compute();
			R.swu_profile.compute();
			R.mc_profile.compute();
		};
	TRecordingReportFun G =
		[&]( const CJGroup&, const CSubject&, const string&, const CSubject::SEpisode&, const CRecording& R,
		     size_t i, size_t total)
		{
			only_progress_fun(
				(string ("Compute ") + R.F().filename() + ":"+R.F().channel_by_id(R.h())).c_str(),
				total, i);
		};
	TRecordingFilterFun filter =
		[&]( CRecording& R)
		{
			return R.signal_type() == sigfile::SChannel::TType::eeg;
		};
	for_all_recordings( F, G, filter);
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
