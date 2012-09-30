// ;-*-C++-*-
/*
 *       File name:  expdesign/primaries.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  experimental design primary classes: CSubject & CExpDesign
 *
 *         License:  GPL
 */


#include <stdarg.h>
#include <cassert>
#include <string>
#include <fstream>

//#include "../common/lang.hh"
#include "../common/config-validate.hh"
#include "primaries.hh"


using namespace std;
using namespace agh;



agh::CExpDesign::
CExpDesign (const string& session_dir_,
	    TMsmtCollectProgressIndicatorFun progress_fun)
      : af_dampen_window_type (sigfile::SFFTParamSet::TWinType::welch),
	af_dampen_factor (.95),
	tunables0 (tstep, tlo, thi),
	_id_pool (0),
	config_keys_g ({
		confval::SValidator<double>("ctlparam.StepSize",	&ctl_params0.siman_params.step_size),
		confval::SValidator<double>("ctlparam.Boltzmannk",	&ctl_params0.siman_params.k,			confval::SValidator<double>::SVFRangeEx( DBL_MIN, 1e9)),
		confval::SValidator<double>("ctlparam.TInitial",	&ctl_params0.siman_params.t_initial,		confval::SValidator<double>::SVFRangeEx( DBL_MIN, 1e9)),
		confval::SValidator<double>("ctlparam.DampingMu",	&ctl_params0.siman_params.mu_t,			confval::SValidator<double>::SVFRangeEx( DBL_MIN, 1e9)),
		confval::SValidator<double>("ctlparam.TMin",		&ctl_params0.siman_params.t_min,		confval::SValidator<double>::SVFRangeEx( DBL_MIN, 1e9)),
		confval::SValidator<double>("ctlparam.ReqScoredPC",	&ctl_params0.req_percent_scored,		confval::SValidator<double>::SVFRangeIn( 80., 100.)),
		confval::SValidator<double>("fftparam.BinSize",		&fft_params.binsize,				confval::SValidator<double>::SVFRangeIn( .125, 1.)),
		confval::SValidator<double>("artifacts.DampenFactor",	&af_dampen_factor,				confval::SValidator<double>::SVFRangeIn( 0., 1.)),
		confval::SValidator<double>("mcparam.mc_gain",		&mc_params.mc_gain,				confval::SValidator<double>::SVFRangeIn( 0., 100.)),
		confval::SValidator<double>("mcparam.f0fc",		&mc_params.f0fc,				confval::SValidator<double>::SVFRangeEx( 0., 80.)),
		confval::SValidator<double>("mcparam.bandwidth",	&mc_params.bandwidth,				confval::SValidator<double>::SVFRangeIn( 0.125, 2.)),
		confval::SValidator<double>("mcparam.iir_backpolate",	&mc_params.iir_backpolate,			confval::SValidator<double>::SVFRangeIn( 0., 1.)),
	}),
	config_keys_d ({
		confval::SValidator<int>("fftparam.WelchWindowType",	(int*)&fft_params.welch_window_type,		confval::SValidator<int>::SVFRangeIn( 0, (int)sigfile::SFFTParamSet::TWinType::_total - 1)),
		confval::SValidator<int>("artifacts.DampenWindowType",	(int*)&af_dampen_window_type,			confval::SValidator<int>::SVFRangeIn( 0, (int)sigfile::SFFTParamSet::TWinType::_total - 1)),
		confval::SValidator<int>("ctlparam.ItersFixedT",	&ctl_params0.siman_params.iters_fixed_T,	confval::SValidator<int>::SVFRangeIn( 1, 1000000)),
		confval::SValidator<int>("ctlparam.NTries",		&ctl_params0.siman_params.n_tries,		confval::SValidator<int>::SVFRangeIn( 1, 10000)),
		confval::SValidator<int>("ctlparam.NSWALadenPagesBeforeSWA0",
					 				(int*)&ctl_params0.swa_laden_pages_before_SWA_0,confval::SValidator<size_t>::SVFRangeIn( 1, 100)),
		confval::SValidator<int>("fftparam.PageSize",		(int*)&fft_params.pagesize,		confval::SValidator<size_t>::SVFRangeIn( 4, 120)),
		confval::SValidator<int>("mcparam.SmoothSide",		(int*)&mc_params.smooth_side,		confval::SValidator<size_t>::SVFRangeIn( 0, 5)),
	}),
	config_keys_b ({
		confval::SValidator<bool>("ctlparam.DBAmendment1",		&ctl_params0.DBAmendment1),
		confval::SValidator<bool>("ctlparam.DBAmendment2",		&ctl_params0.DBAmendment2),
		confval::SValidator<bool>("ctlparam.AZAmendment1",		&ctl_params0.AZAmendment1),
		confval::SValidator<bool>("ctlparam.AZAmendment2",		&ctl_params0.AZAmendment2),
		confval::SValidator<bool>("ctlparam.ScoreUnscoredAsWake",	&ctl_params0.ScoreUnscoredAsWake),
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

	mc_params.scope = fft_params.pagesize;

	scan_tree( progress_fun);
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
	if ( strlen(buf) && *(buf + strlen(buf)-1) != '\n' ) {
		_error_log += "\n";
		fprintf( stderr, "%s\n", buf);
	} else
		fputs( buf, stderr);

	va_end (ap);
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
#pragma omp parallel for
	for ( size_t i = 0; i < v.size(); ++i ) {
		F( *get<1>(v[i]));
#pragma omp critical
		{
			report( *get<0>(v[i]), *get<1>(v[i]),
				++global_i, v.size());
		}
	}
}


void
agh::CExpDesign::
for_all_episodes( const TEpisodeOpFun& F, const TEpisodeReportFun& report, const TEpisodeFilterFun& filter)
{
	vector<tuple<CJGroup*,
		     CSubject*,
		     const string*,
		     CSubject::SEpisode*>> v;
	for ( auto& G : groups )
		for ( auto& J : G.second )
			for ( auto& M : J.measurements )
				for ( auto& E : M.second.episodes )
					if ( filter(E) )
						v.emplace_back( make_tuple(&G.second, &J, &M.first, &E));
	size_t global_i = 0;
#pragma omp parallel for
	for ( size_t i = 0; i < v.size(); ++i ) {
		F( *get<3>(v[i]));
#pragma omp critical
		{
			report( *get<0>(v[i]), *get<1>(v[i]), *get<2>(v[i]), *get<3>(v[i]),
				++global_i, v.size());
		}
	}
}


void
agh::CExpDesign::
for_all_recordings( const TRecordingOpFun& F, const TRecordingReportFun& report, const TRecordingFilterFun& filter)
{
	vector<tuple<CJGroup*,
		     CSubject*,
		     const string*,
		     CSubject::SEpisode*,
		     CRecording*>> v;
	for ( auto& G : groups )
		for ( auto& J : G.second )
			for ( auto& M : J.measurements )
				for ( auto& E : M.second.episodes )
					for ( auto &R : E.recordings )
						if ( filter(R.second) )
							v.emplace_back( make_tuple (&G.second, &J, &M.first, &E, &R.second));
	size_t global_i = 0;
#pragma omp parallel for
	for ( size_t i = 0; i < v.size(); ++i ) {
		F( *get<4>(v[i]));
#pragma omp critical
		{
			report( *get<0>(v[i]), *get<1>(v[i]), *get<2>(v[i]), *get<3>(v[i]), *get<4>(v[i]),
				++global_i, v.size());
		}
}

void
agh::CExpDesign::
for_all_modruns( const TModelRunOpFun& F, const TModelRunReportFun& report, const TModelRunFilterFun& filter)
{
	vector<tuple<CJGroup*,
		     CSubject*,
		     const string*,
		     const sigfile::TMetricType*,
		     const string*,
		     const pair<float,float>*,
		     ach::CModelRun*>> v;
	for ( auto& G : groups )
		for ( auto& J : G.second )
			for ( auto& D : J.measurements )
				for ( auto& T : D.second.modrun_sets )
					for ( auto& H : T.second )
						for ( auto& Q : H.second )
							if ( filter(Q.second) )
								v.emplace_back(
									make_tuple (
										&G.second, &J, &D.first,
										&T.first,
										&H.first,
										&Q.first,
										&Q.second));
	size_t global_i = 0;
#pragma omp parallel for
	for ( size_t i = 0; i < v.size(); ++i ) {
#pragma omp critical
		{
			report( *get<0>(v[i]), *get<1>(v[i]), *get<2>(v[i]), *get<3>(v[i]), *get<4>(v[i]), *get<5>(v[i]), *get<6>(v[i]),
				++global_i, v.size());
		}
		F( *get<6>(v[i]));
	}
}



list<string>
agh::CExpDesign::
enumerate_groups() const
{
	list<string> recp;
	for ( auto &G : groups )
		recp.push_back( G.first);
	return recp;
}

list<string>
agh::CExpDesign::
enumerate_subjects() const
{
	list<string> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			recp.push_back( J.name());
	return recp;
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
	return recp;
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
	return recp;
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
						for ( size_t h = 0; h < F.n_channels(); ++h )
							if ( F.signal_type(h) == sigfile::SChannel::TType::eeg )
								recp.push_back( F.channel_by_id(h));
	recp.sort();
	recp.unique();
	return recp;
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
						auto Ins = F.channel_list();
						recp.insert( recp.end(),
							     Ins.begin(), Ins.end());
					}
	recp.sort();
	recp.unique();
	return recp;
}


list<size_t>
agh::CExpDesign::
used_samplerates( sigfile::SChannel::TType type) const
{
	list<size_t> recp;
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources )
						for ( size_t h = 0; h < F.n_channels(); ++h )
							if ( type == sigfile::SChannel::other or
							     type == F.signal_type(h) ) {
								recp.push_back( F.samplerate(h));
							}
	recp.sort();
	recp.unique();
	return recp;
}



const char*
__attribute__ ((const))
agh::CSubject::
gender_sign( TGender g)
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

agh::CSubject::
CSubject (const string& dir,
	  sid_type id)
  : full_name (""),
    gender (TGender::neuter),
    age (21),
    _status (0),
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
	else
		full_name = _name;
}


agh::CSubject::
~CSubject ()
{
	ofstream ofs (_dir + "/.subject_info");
	char gender_char = (char)gender;
	if ( ofs.good() )
		ofs << full_name << endl
		    << gender_char << endl
		    << age << endl
		    << comment << endl;
}







agh::CSubject::SEpisode::
SEpisode (sigfile::CSource&& F_,
	  const sigfile::SFFTParamSet& fft_params,
	  const sigfile::SMCParamSet& mc_params)
{
      // move it in place
	sources.emplace_back( move(F_));
	auto& F = sources.back();
	auto HH = F.channel_list();
	printf( "CSubject::SEpisode::SEpisode( \"%s\"): %s\n",
		F.filename(), agh::str::join(HH, ", ").c_str());
	int h = 0;
	for ( auto& H : HH ) {
		recordings.insert( {H, {F, h++, fft_params, mc_params}});
	}
}


list<agh::CSubject::SEpisode::SAnnotation>
agh::CSubject::SEpisode::
get_annotations() const
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



void
agh::CExpDesign::
remove_untried_modruns()
{
	for ( auto &G : groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
			retry_modruns:
				for ( auto RSt = D.second.modrun_sets.begin(); RSt != D.second.modrun_sets.end(); ++RSt )
					for ( auto RSi = RSt->second.begin(); RSi != RSt->second.end(); ++RSi ) {
					retry_this_modrun_set:
						for ( auto Ri = RSi->second.begin(); Ri != RSi->second.end(); ++Ri ) {
							// printf( "#----- check Subject: %s;  Session: %s;  Channel: %s;  Range: %g-%g Hz (%d)\n",
							// 	Ri->second.subject(), Ri->second.session(), Ri->second.channel(),
							// 	Ri->second.freq_from(), Ri->second.freq_upto(),
							// 	Ri->second.status);
							if ( !(Ri->second.status & ach::CModelRun::modrun_tried) ) {
								RSi->second.erase( Ri);
								goto retry_this_modrun_set;
							}
						}
						if ( RSi->second.empty() ) {
							D.second.modrun_sets.erase( RSt);
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
					for ( auto &Q : RS.second )
						for ( auto &R : Q.second )
							if ( R.second.status & ach::CModelRun::modrun_tried ) {
								fprintf( f, "# ----- Subject: %s;  Session: %s;  Channel: %s;  Range: %g-%g Hz\n",
									 R.second.subject(), R.second.session(), R.second.channel(),
									 R.second.freq_from(), R.second.freq_upto());
								t = ach::TTunable::rs;
								do {
									fprintf( f, "%g%s", R.second.tx[t] * ach::stock[t].display_scale_factor,
										 (t < R.second.tx.size()-1) ? "\t" : "\n");
								} while ( t++ < R.second.tx.size()-1 );
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
						F.write_ancillary_files();
}


// eof
