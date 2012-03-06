// ;-*-C++-*-
/*
 *       File name:  core/model.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-01
 *
 *         Purpose:  simulation model classes definitions
 *
 *         License:  GPL
 */

#include <algorithm>
#include <list>

#include <sys/time.h>

#include "tunable.hh"
#include "primaries.hh"
#include "model.hh"


using namespace std;



agh::CSCourse::CSCourse( CSubject& J, const string& d, const sigfile::SChannel& h,
			 const SSCourseParamSet& params)
      : SSCourseParamSet (params),
	_status (0),
	_sim_start ((size_t)-1), _sim_end ((size_t)-1)
{
	if ( not J.have_session(d) )
		throw invalid_argument (string(J.name()) + " has no recordings in session " + d);

	auto& EE = J.measurements[d].episodes;
	for ( auto &E : EE )
		_mm_list.push_back( &E.recordings.at(h));

	for ( auto Mi = _mm_list.begin(); Mi != _mm_list.end(); ++Mi ) {
		const auto& M = **Mi;
		const auto& F = M.F();

		printf( "CSCourse::CSCourse(): adding [%s, %s, %s] recorded %s",
			 F.subject(), F.session(), F.episode(),
			 ctime( &F.start_time()));

		if ( F.percent_scored() < _req_percent_scored )
			_status |= (int)TSimPrepError::enoscore;

	      // anchor zero page, get pagesize from edf^W CBinnedPower^W either goes
		if ( Mi == _mm_list.begin() ) {
			_0at = F.start_time();
			_pagesize = M.SFFTParamSet::pagesize;
			_pages_in_bed = 0;
		} else
			if ( _pagesize != F.pagesize() ) {
				_status |= (int)TSimPrepError::euneq_pagesize;
				return;  // this is really serious, so return now
			}

		size_t	pa = (size_t)difftime( F.start_time(), _0at) / _pagesize,
			pz = (size_t)difftime( F.end_time(), _0at) / _pagesize;
		// this is not really a reportable/corrigible circumstance, so just abort
		assert (pz - pa == M.F().pages());
		_pages_in_bed += (pz-pa);

		if ( pa < 0 ) {
			_status |= (int)TSimPrepError::enegoffset;
			return;
		}
		// this condition is checked against already in CSubject::SEpisodeSequence::add_one()
		// if ( _mm_bounds.size() > 0  &&  pa - _mm_bounds.back().second > 4 * 24 * 3600 ) {
		// 	_status |= (int)TSimPrepError::efarapart;
		// 	return;
		// }
		_mm_bounds.emplace_back( TBounds (pa, pz));

		_timeline.resize( pz, sigfile::SPageSimulated {0., 0., 1.});  // fill with WAKE

	      // collect M's power and scores
		valarray<double>
			lumped_bins = (_profile_type == sigfile::TProfileType::psd)
			? M.CBinnedPower::course<double>( _freq_from, _freq_upto)
			: M.CBinnedMicroConty::course<double>();
//		printf( "_freq %g - %g; binsize %f; n_bins %zu\n", _freq_from, _freq_upto, M.binsize(), M.n_bins());
//		assert (lumped_bins.sum() > 0.);

		for ( size_t p = pa; p < pz; ++p ) {
			_timeline[p] = sigfile::SPageSimulated {F[p-pa]};
		      // fill unscored/MVT per user setting
			if ( _timeline[p].Wake == sigfile::SPage::mvt_wake_value ) {
				if ( _ScoreMVTAsWake )
					_timeline[p].mark( sigfile::SPage::TScore::wake);
				else
					if ( p > 0 )
						_timeline[p] = _timeline[p-1];
			} else if ( !_timeline[p].is_scored() ) {
				if ( _ScoreUnscoredAsWake )
					_timeline[p].mark( sigfile::SPage::TScore::wake);
				else
					if ( p > 0 )
						_timeline[p] = _timeline[p-1];
			}
		      // put SWA
			_timeline[p].SWA = lumped_bins[p-pa];
		}

	      // determine SWA_0
		if ( Mi == _mm_list.begin() ) {
			_baseline_end = pz;

			// require some length of swa-containing pages to happen before sim_start
			for ( size_t p = 0; p < pz; ++p ) {
				for ( size_t pp = p; pp < pz; ++pp ) {
//					printf( "NREM[%zu] = %f\n", pp, _timeline[pp].NREM);
					if ( _timeline[pp].NREM < 1./3 ) {
						p = pp;
						goto outer_continue;
					}
					if ( (pp-p) >= _swa_laden_pages_before_SWA_0 ) {
						_sim_start = pp;
						goto outer_break;
					}
				}
			outer_continue:
				;
			}
		outer_break:

			if ( _sim_start == (size_t)-1 )
				_status |= (int)TSimPrepError::enoswa;
			else
				_SWA_0 = _timeline[_sim_start].SWA;
		}

		_sim_end = pz-1;
	}


      // determine SWA_L
	_pages_with_SWA = 0;
	_SWA_L = _SWA_100 = 0.;

	if ( _sim_start != (size_t)-1 ) {
		size_t REM_pages_cnt = 0;
		for ( size_t p = _sim_start; p < _sim_end; ++p ) {
			auto& P = _timeline[p];
			if ( P.REM > .5 ) {
				_SWA_L += P.SWA;
				++REM_pages_cnt;
			}
			if ( P.NREM > 1./3 ) {
				_SWA_100 += P.SWA;
				++_pages_with_SWA;
			}
		}
		if ( REM_pages_cnt )
			_SWA_L /= (REM_pages_cnt / .95);
		if ( _pages_with_SWA )
			_SWA_100 /= _pages_with_SWA;
	}

	if ( _sim_start != (size_t)-1 )
		printf( "CSCourse::CSCourse(): sim start-end: %zu-%zu; avg SWA = %.4g (over %zu pp, or %.3g%% of all time in bed); "
			" SWA_L = %g;  SWA[%zu] = %g\n",
			_sim_start, _sim_end, _SWA_100, _pages_with_SWA, (double)_pages_with_SWA / _pages_in_bed * 100,
			_SWA_L, _sim_start, _SWA_0);
	else
		printf( "CSCourse::CSCourse(): status %xd, %s\n", _status, CSCourse::explain_status( _status).c_str());
}








int
agh::CExpDesign::setup_modrun( const char* j, const char* d, const char* h,
			       float freq_from, float freq_upto,
			       agh::CSimulation* &R_ref)
{
	try {
		CSubject& J = subject_by_x(j);

		if ( J.measurements[d].size() == 1 && ctl_params0.DBAmendment2 )
			return (int)TSimPrepError::eamendments_ineffective;

		if ( J.measurements[d].size() == 1 && tunables0.step[TTunable::rs] > 0. )
			return (int)TSimPrepError::ers_nonsensical;

		auto freq_idx = pair<float,float> (freq_from, freq_upto);
		J.measurements[d]
			. modrun_sets[h][freq_idx] =
			CSimulation (J, d, h, freq_from, freq_upto,
				     ctl_params0, tunables0);
		R_ref = &J.measurements[d]
			. modrun_sets[h][freq_idx];

	} catch (int ex) {
		log_message( string("CExpDesign::setup_modrun( ")+j+", "+d+", "+h+"): " + CSCourse::explain_status(ex)+'\n');
		return ex;
	}

	return 0;
}



string
agh::CSCourse::explain_status( int code)
{
	list<const char*> ss;
	if ( code & (int)TSimPrepError::enoscore )
		ss.push_back( "insufficiently scored");
	if ( code & (int)TSimPrepError::efarapart )
		ss.push_back( "episodes too far apart");
	if ( code & (int)TSimPrepError::esigtype )
		ss.push_back( "signal is not an EEG");
	if ( code & (int)TSimPrepError::etoomanymsmt )
		ss.push_back( "too many episodes");
	if ( code & (int)TSimPrepError::enoswa )
		ss.push_back( "no SWA");
	if ( code & (int)TSimPrepError::eamendments_ineffective)
		ss.push_back( "inappropriate amendments");
	if ( code & (int)TSimPrepError::ers_nonsensical )
		ss.push_back( "too few episoded for rs");
	if ( code & (int)TSimPrepError::enegoffset )
		ss.push_back( "negative offset");
	if ( code & (int)TSimPrepError::euneq_pagesize )
		ss.push_back( "wrong page size");
	string acc;
	for_each( ss.begin(), ss.end(),
		  [&acc] ( const char* s)
		  {
			  acc += (acc.size() ? string("; ") + s : s);
		  });
	return acc;
}





gsl_rng *agh::__agh_rng = NULL;

void
agh::init_global_rng()
{
	const gsl_rng_type *T;
	gsl_rng_env_setup();
	T = gsl_rng_default;
	if ( gsl_rng_default_seed == 0 ) {
		struct timeval tp = { 0L, 0L };
		gettimeofday( &tp, NULL);
		gsl_rng_default_seed = tp.tv_usec;
	}
	__agh_rng = gsl_rng_alloc( T);
}




// EOF
