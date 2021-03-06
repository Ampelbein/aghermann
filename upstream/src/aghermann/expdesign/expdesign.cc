/*
 *       File name:  aghermann/expdesign/expdesign.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  experimental design primary classes: CSubject & CExpDesign
 *
 *         License:  GPL
 */


#include <stdarg.h>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "aghermann/globals.hh"
#include "common/config-validate.hh"
#include "expdesign.hh"


using namespace std;

using agh::confval::SValidator;

const char
	*const agh::CExpDesign::FreqBandNames[metrics::TBand::TBand_total] = {
	"Delta", "Theta", "Alpha", "Beta", "Gamma",
};

double
	agh::CExpDesign::freq_bands[metrics::TBand::TBand_total][2] = {
	{  1.5,  4.0 },
	{  4.0,  8.0 },
	{  8.0, 12.0 },
	{ 15.0, 30.0 },
	{ 30.0, 40.0 },
};



agh::CExpDesign::
CExpDesign (const string& session_dir_,
	    TMsmtCollectProgressIndicatorFun progress_fun)
      : num_threads (0),
	af_dampen_window_type (sigproc::TWinType::welch),
	af_dampen_factor (.95),
	tunables0 (tstep, tlo, thi), // only references here, don't worry
	req_percent_scored (80.),
	swa_laden_pages_before_SWA_0 (3),
	strict_subject_id_checks (false),
	_id_pool (0),
	config_keys_g ({
		SValidator<double>("ctl_param.step_size",	&ctl_params0.siman_params.step_size),
		SValidator<double>("ctl_param.boltzmann_k",	&ctl_params0.siman_params.k,			SValidator<double>::SVFRangeEx( DBL_MIN, 1e9)),
		SValidator<double>("ctl_param.t_initial",	&ctl_params0.siman_params.t_initial,		SValidator<double>::SVFRangeEx( DBL_MIN, 1e9)),
		SValidator<double>("ctl_param.damping_mu",	&ctl_params0.siman_params.mu_t,			SValidator<double>::SVFRangeEx( DBL_MIN, 1e9)),
		SValidator<double>("ctl_param.t_min",		&ctl_params0.siman_params.t_min,		SValidator<double>::SVFRangeEx( DBL_MIN, 1e9)),
		SValidator<double>("profile.req_scored_pc",	&req_percent_scored,				SValidator<double>::SVFRangeIn( 80., 100.)),
		SValidator<double>("fft_param.pagesize",	&fft_params.pagesize,				SValidator<double>::SVFRangeIn( 4., 120.)),
		SValidator<double>("fft_param.binsize",		&fft_params.binsize,				SValidator<double>::SVFRangeIn( .125, 1.)),
		SValidator<double>("artifacts.dampen_factor",	&af_dampen_factor,				SValidator<double>::SVFRangeIn( 0., 1.)),
		SValidator<double>("mc_param.mc_gain",		&mc_params.mc_gain,				SValidator<double>::SVFRangeIn( 0., 100.)),
		SValidator<double>("mc_param.f0fc",		&mc_params.f0fc,				SValidator<double>::SVFRangeEx( 0., 80.)),
		SValidator<double>("mc_param.bandwidth",	&mc_params.bandwidth,				SValidator<double>::SVFRangeIn( 0.125, 2.)),
		SValidator<double>("mc_param.iir_backpolate",	&mc_params.iir_backpolate,			SValidator<double>::SVFRangeIn( 0., 1.)),
		SValidator<double>("swu_param.min_upswing_duration",
								&swu_params.min_upswing_duration,		SValidator<double>::SVFRangeIn( 0.01, 1.)),
	}),
	config_keys_d ({
		SValidator<int>("fft_param.welch_window_type",	(int*)&fft_params.welch_window_type,		SValidator<int>::SVFRangeIn( 0, (int)sigproc::TWinType_total - 1)),
		SValidator<int>("fft_param.plan_type",		(int*)&fft_params.plan_type,			SValidator<int>::SVFRangeIn( 0, (int)metrics::psd::TFFTWPlanType_total - 1)),
		SValidator<int>("artifacts.dampen_window_type",	(int*)&af_dampen_window_type,			SValidator<int>::SVFRangeIn( 0, (int)sigproc::TWinType_total - 1)),
		SValidator<int>("ctl_param.iters_fixed_t",	&ctl_params0.siman_params.iters_fixed_T,	SValidator<int>::SVFRangeIn( 1, 1000000)),
		SValidator<int>("ctl_param.n_tries",		&ctl_params0.siman_params.n_tries,		SValidator<int>::SVFRangeIn( 1, 10000)),
	}),
	config_keys_z ({
		SValidator<size_t>("smp.num_threads",		&num_threads,					SValidator<size_t>::SVFRangeIn( 0, 20)),
		SValidator<size_t>("mc_params.n_bins",		&mc_params.n_bins,				SValidator<size_t>::SVFRangeIn( 1, 100)),
		SValidator<size_t>("profile.swa_laden_pages_before_SWA_0",
								&swa_laden_pages_before_SWA_0,			SValidator<size_t>::SVFRangeIn( 1, 100)),
		SValidator<size_t>("mc_param.smooth_side",	&mc_params.smooth_side,				SValidator<size_t>::SVFRangeIn( 0, 5)),
	}),
	config_keys_b ({
		SValidator<bool>("ctl_param.DBAmendment1",	&ctl_params0.DBAmendment1),
		SValidator<bool>("ctl_param.DBAmendment2",	&ctl_params0.DBAmendment2),
		SValidator<bool>("ctl_param.AZAmendment1",	&ctl_params0.AZAmendment1),
		SValidator<bool>("ctl_param.AZAmendment2",	&ctl_params0.AZAmendment2),
		SValidator<bool>("profile.score_unscored_as_wake",
				  				&score_unscored_as_wake),
		SValidator<bool>("StrictSubjectIdChecks",	&strict_subject_id_checks),
	}),
	config_keys_s ({
		SValidator<string>("LastUsedVersion",		&last_used_version),
	})
{
	char *tmp = canonicalize_file_name(session_dir_.c_str());
	if ( !tmp ) // does not exist
		throw invalid_argument (string ("Failed to canonicalize dir: ") + session_dir_);

	if ( session_dir_ != tmp ) {
		printf( "CExpDesign::CExpDesign(): canonicalized session_dir \"%s\" to \"%s\"\n", session_dir_.c_str(), tmp);
		_session_dir.assign( tmp);
	} else
		_session_dir = session_dir_;
	free( tmp);

	if ( _session_dir.size() > 1 && _session_dir[_session_dir.size()-1] == '/' )
		_session_dir.erase( _session_dir.size()-1, 1);

	if ( fs::exists_and_is_writable( session_dir()) == false )
		throw invalid_argument (string("Experiment directory ") + _session_dir + " does not exist or is not writable");

	if ( chdir( session_dir()) == -1 )
		throw invalid_argument (string("Failed to cd to ") + _session_dir);

	load_settings();

	// that's pretty important: scope is not itself exposed to the user
	mc_params.scope = fft_params.pagesize;

#ifdef _OPENMP
	omp_set_num_threads( (num_threads == 0) ? global::num_procs : num_threads);
	printf( "SMP enabled with %d threads\n", omp_get_max_threads());
#endif
	if ( last_used_version != VERSION ) {
		printf( "Purging old files as we are upgrading from version %s to %s\n", last_used_version.c_str(), VERSION);
	}
	// last_used_version = VERSION;
	/// leave it so SExpDesignUI::populate will see it and pop the changelog

	scan_tree( progress_fun);
}


void
agh::CExpDesign::
log_message( const char* fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);

	char *buf0;
	int np = vasprintf( &buf0, fmt, ap);
	char *buf = buf0;
	if ( np > 0 ) {
		TLogEntryStyle style =
			(strncmp( buf, "$$", 2) == 0)
			? ((buf += 2), TLogEntryStyle::bold)
			: (strncmp( buf, "//", 2) == 0)
			? ((buf += 2), TLogEntryStyle::italic)
			: TLogEntryStyle::plain;
		fprintf( stdout, "%s\n", buf0);
		_error_log.emplace_back( buf, style);
	}
	if ( np != -1 )
		free( (void*)buf0);

	va_end (ap);
}


string
agh::CExpDesign::
error_log_serialize() const
{
	string ret;
	for ( const auto& E : _error_log )
		ret += E.first + '\n';
	return move(ret);
}



void
agh::CExpDesign::
for_all_subjects( const TSubjectOpFun& F, const TSubjectReportFun& report, const TSubjectFilterFun& filter)
{
	vector<tuple<CJGroup*,
		     CSubject*>> v;
	for ( auto& G : groups )
		for ( auto& J : G.second )
			if ( filter(J) )
				v.emplace_back( make_tuple(&G.second, &J));
	size_t global_i = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(guided)
#endif
	for ( size_t i = 0; i < v.size(); ++i ) {
#ifdef _OPENMP
#pragma omp critical
#endif
		{
			report( *get<0>(v[i]), *get<1>(v[i]),
				++global_i, v.size());
		}
		F( *get<1>(v[i]));
	}
}


void
agh::CExpDesign::
for_all_episodes( const TEpisodeOpFun& F, const TEpisodeReportFun& report, const TEpisodeFilterFun& filter)
{
	vector<tuple<CJGroup*,
		     CSubject*,
		     const string*,
		     SEpisode*>> v;
	for ( auto& G : groups )
		for ( auto& J : G.second )
			for ( auto& M : J.measurements )
				for ( auto& E : M.second.episodes )
					if ( filter(E) )
						v.emplace_back( make_tuple(&G.second, &J, &M.first, &E));
	size_t global_i = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(guided)
#endif
	for ( size_t i = 0; i < v.size(); ++i ) {
#ifdef _OPENMP
#pragma omp critical
#endif
		{
			report( *get<0>(v[i]), *get<1>(v[i]), *get<2>(v[i]), *get<3>(v[i]),
				++global_i, v.size());
		}
		F( *get<3>(v[i]));
	}
}


void
agh::CExpDesign::
for_all_recordings( const TRecordingOpFun& F, const TRecordingReportFun& report, const TRecordingFilterFun& filter)
{
	vector<tuple<CJGroup*,
		     CSubject*,
		     const string*,
		     SEpisode*,
		     CRecording*>> v;
	for ( auto& G : groups )
		for ( auto& J : G.second )
			for ( auto& D : J.measurements )
				for ( auto& E : D.second.episodes )
					for ( auto &R : E.recordings )
						if ( filter(R.second) )
							v.emplace_back(
								make_tuple (&G.second, &J, &D.first,
									    &E,
									    &R.second));
	size_t global_i = 0;
#ifdef _OPENMP
// read that man, bro
#pragma omp parallel for schedule(guided)
#endif
	for ( size_t i = 0; i < v.size(); ++i ) {
#ifdef _OPENMP
#pragma omp critical
#endif
		{
			report( *get<0>(v[i]), *get<1>(v[i]), *get<2>(v[i]), *get<3>(v[i]), *get<4>(v[i]),
				++global_i, v.size());
		}
		F( *get<4>(v[i]));
	}
}

void
agh::CExpDesign::
for_all_modruns( const TModelRunOpFun& F, const TModelRunReportFun& report, const TModelRunFilterFun& filter)
{
	vector<tuple<CJGroup*,
		     CSubject*,
		     const string*,
		     const SProfileParamSet*,
		     const string*,
		     ach::CModelRun*>> v;
	for ( auto& G : groups )
		for ( auto& J : G.second )
			for ( auto& D : J.measurements )
				for ( auto& T : D.second.modrun_sets )
					for ( auto& H : T.second )
						if ( filter(H.second) )
							v.emplace_back(
								make_tuple (
									&G.second, &J, &D.first,
									&T.first,
									&H.first,
									&H.second));
	size_t global_i = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(guided)
#endif
	for ( size_t i = 0; i < v.size(); ++i ) {
#ifdef _OPENMP
#pragma omp critical
#endif
		{
			report( *get<0>(v[i]), *get<1>(v[i]), *get<2>(v[i]), *get<3>(v[i]), *get<4>(v[i]), *get<5>(v[i]),
				++global_i, v.size());
		}
		F( *get<5>(v[i]));
	}
}



list<string>
agh::CExpDesign::
enumerate_groups() const
{
	list<string> recp;
	for ( auto &G : groups )
		recp.push_back( G.first);
	return move(recp);
}

list<string>
agh::CExpDesign::
enumerate_subjects() const
{
	list<string> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			recp.push_back( J.id);
	return move(recp);
}


list<string>
agh::CExpDesign::
enumerate_sessions() const
{
	list<string> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				recp.push_back( D.first);
	recp.sort();
	recp.unique();
	return move(recp);
}

list<string>
agh::CExpDesign::
enumerate_episodes() const
{
	list<string> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					recp.push_back( E.name());
	recp.sort();
	recp.unique();
	return move(recp);
}

list<sigfile::SChannel>
agh::CExpDesign::
enumerate_eeg_channels() const
{
	list<sigfile::SChannel> recp;
/// sigfile::SChannel will rightly not count oddly named channels
/// which, still, were additionally qualified as EEG in the EDF
/// header, so we'd better walk it again and look at signal type
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources )
						for ( size_t h = 0; h < F().n_channels(); ++h )
							if ( F().signal_type(h) == sigfile::SChannel::TType::eeg )
								recp.push_back( F().channel_by_id(h));
	recp.sort();
	recp.unique();
	return move(recp);
}

list<sigfile::SChannel>
agh::CExpDesign::
enumerate_all_channels() const
{
	list<sigfile::SChannel> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources ) {
						auto Ins = F().channel_list();
						recp.insert( recp.end(),
							     Ins.begin(), Ins.end());
					}
	recp.sort();
	recp.unique();
	return move(recp);
}


vector<size_t>
agh::CExpDesign::
used_samplerates( sigfile::SChannel::TType type) const
{
	vector<size_t> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources )
						for ( size_t h = 0; h < F().n_channels(); ++h )
							if ( type == sigfile::SChannel::TType::other or
							     type == F().signal_type(h) ) {
								recp.push_back( F().samplerate(h));
							}
	sort(recp.begin(), recp.end());
	unique(recp.begin(), recp.end());
	return move(recp);
}











int
agh::CExpDesign::
setup_modrun( const string& j, const string& d, const string& h,
	      const SProfileParamSet& profile_params0,
	      agh::ach::CModelRun** Rpp)
{
	try {
		CSubject& J = subject_by_x(j);

		if ( J.measurements[d].size() == 1 && ctl_params0.DBAmendment2 )
			return CProfile::TFlags::eamendments_ineffective;

		if ( J.measurements[d].size() == 1 && tstep[ach::TTunable::rs] > 0. )
			return CProfile::TFlags::ers_nonsensical;

		J.measurements[d].modrun_sets[profile_params0].insert(
			pair<string, ach::CModelRun> (
				h,
				ach::CModelRun (
					J, d, h,
					profile_params0,
					ctl_params0,
					tunables0))
			);
		if ( Rpp )
			*Rpp = &J.measurements[d]
				. modrun_sets[profile_params0][h];

	} catch (invalid_argument ex) { // thrown by CProfile ctor
		fprintf( stderr, "CExpDesign::setup_modrun( %s, %s, %s): %s\n", j.c_str(), d.c_str(), h.c_str(), ex.what());
		return -1;
	} catch (out_of_range ex) {
		fprintf( stderr, "CExpDesign::setup_modrun( %s, %s, %s): %s\n", j.c_str(), d.c_str(), h.c_str(), ex.what());
		return -1;
	} catch (int ex) { // thrown by CModelRun ctor
		log_message( "CExpDesign::setup_modrun( %s, %s, %s): %s", j.c_str(), d.c_str(), h.c_str(), CProfile::explain_status(ex).c_str());
		return ex;
	}

	return 0;
}


void
agh::CExpDesign::
prune_untried_modruns()
{
	for ( auto& G : groups )
		for ( auto& J : G.second )
			for ( auto& D : J.measurements )
			retry_modruns:
				for ( auto RSt = D.second.modrun_sets.begin(); RSt != D.second.modrun_sets.end(); ++RSt )
					for ( auto Ri = RSt->second.begin(); Ri != RSt->second.end(); ++Ri ) {
						if ( !(Ri->second.status & ach::CModelRun::modrun_tried) ) {
							RSt->second.erase( Ri);
							if ( RSt->second.empty() )
								D.second.modrun_sets.erase(RSt);
							goto retry_modruns;
						}
					}
}

void
agh::CExpDesign::
remove_all_modruns()
{
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				D.second.modrun_sets.clear();
}


void
agh::CExpDesign::
export_all_modruns( const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return;

	size_t t = (size_t)ach::TTunable::rs;
	fprintf( f, "#");
	for ( ; t < ach::TTunable::_all_tunables; ++t )
		fprintf( f, "%s%s", (t == 0) ? "" : "\t", ach::tunable_name(t).c_str());
	fprintf( f, "\n");

	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &RS : D.second.modrun_sets )
					for ( auto &R : RS.second )
						if ( R.second.status & ach::CModelRun::modrun_tried ) {
							auto& M = R.second;
							string extra;
							switch ( M.P().metric ) {
							case metrics::TType::psd:
								extra = agh::str::sasprintf( "%g-%g Hz", M.P().P.psd.freq_from, M.P().P.psd.freq_upto);
								break;
							case metrics::TType::swu:
								extra = agh::str::sasprintf( "%g Hz", M.P().P.swu.f0);
								break;
							case metrics::TType::mc:
								extra = agh::str::sasprintf( "%g Hz", M.P().P.mc.f0);
								break;
							default:
								throw runtime_error ("What metric?");
							}
							fprintf( f, "# ----- Subject: %s;  Session: %s;  Channel: %s;  Metric: %s (%s)\n",
								 M.subject(), M.session(), M.channel(),
								 M.P().metric_name(), extra.c_str());
							t = ach::TTunable::rs;
							do {
								fprintf( f, "%g%s", M.tx[t] * ach::stock[t].display_scale_factor,
									 (t < M.tx.size()-1) ? "\t" : "\n");
							} while ( t++ < M.tx.size()-1 );
						}

	fclose( f);
}






void
agh::CExpDesign::
sync()
{
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources )
						F().save_ancillary_files();
}



int
agh::CExpDesign::
purge_cached_profiles()
{
	return system(
		agh::str::sasprintf(
			"find '%s' \\( -name '.*.psd' -or -name '.*.mc' -or -name '.*.swu' \\) -delete",
			session_dir())
		.c_str());
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
