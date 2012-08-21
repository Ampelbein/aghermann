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

#include <cassert>
#include <algorithm>
#include <list>

#include <gsl/gsl_rng.h>

#include "tunable.hh"
#include "primaries.hh"
#include "model.hh"


using namespace std;


void
agh::SControlParamSet::
check() const
{
	if ( siman_params.n_tries < 1 ||
	     siman_params.iters_fixed_T < 1 ||
	     siman_params.step_size <= 0. ||
	     siman_params.k <= 0. ||
	     siman_params.t_initial <= 0. ||
	     siman_params.t_min <= 0. ||
	     siman_params.t_min >= siman_params.t_initial ||
	     siman_params.mu_t <= 0 ||
	     (req_percent_scored < 50. || req_percent_scored > 100. ) )
		throw invalid_argument("Bad SControlParamSet");
}

void
agh::SControlParamSet::
reset()
{
	siman_params.n_tries		=   20;
	siman_params.iters_fixed_T	=   10;
	siman_params.step_size		=    3.;
	siman_params.k			=    1.0;
	siman_params.t_initial  	=  200.;
	siman_params.mu_t		=    1.003;
	siman_params.t_min		=    1.;

	DBAmendment1		= true;
	DBAmendment2		= false;
	AZAmendment1		= false;
	AZAmendment2		= false;

	ScoreUnscoredAsWake	= true;

	req_percent_scored = 90.;
	swa_laden_pages_before_SWA_0 = 3;
}



bool
__attribute__ ((pure))
agh::SControlParamSet::
operator==( const SControlParamSet &rv) const
{
	return	memcmp( &siman_params, &rv.siman_params, sizeof(siman_params)) == 0 &&
		DBAmendment1 == rv.DBAmendment1 &&
		DBAmendment2 == rv.DBAmendment2 &&
		AZAmendment1 == rv.AZAmendment1 &&
		AZAmendment2 == rv.AZAmendment2 &&
		ScoreUnscoredAsWake == rv.ScoreUnscoredAsWake &&
		req_percent_scored == rv.req_percent_scored &&
		swa_laden_pages_before_SWA_0 == rv.swa_laden_pages_before_SWA_0;
}



agh::CSCourse::
CSCourse( const CSubject& J, const string& d, const sigfile::SChannel& h,
	  const SSCourseParamSet& params)
      : SSCourseParamSet (params),
	_status (0),
	_sim_start ((size_t)-1), _sim_end ((size_t)-1)
{
	if ( not J.have_session(d) or J.measurements.at(d).size() == 0 )
		throw invalid_argument (string(J.name()) + " has no recordings in session " + d);

	auto& EE = J.measurements.at(d).episodes;
	for ( auto &E : EE )
		_mm_list.push_back( &E.recordings.at(h));

	for ( auto Mi = _mm_list.begin(); Mi != _mm_list.end(); ++Mi ) {
		const auto& M = **Mi;
		const auto& F = M.F();

		if ( Mi == _mm_list.begin() ) {
			_0at = F.start_time();
			_pagesize = M.SFFTParamSet::pagesize;
			_pages_in_bed = 0;
		} else
			if ( _pagesize != F.pagesize() ) {
				_status |= TFlags::euneq_pagesize;
				return;  // this is really serious, so return now
			}

		int	pa = (size_t)difftime( F.start_time(), _0at) / _pagesize,
			pz = (size_t)difftime( F.end_time(), _0at) / _pagesize;
	      // anchor zero page, get pagesize from edf^W CBinnedPower^W either goes
		printf( "CSCourse::CSCourse(): adding %s of [%s, %s, %s] %zu pages (%d indeed) recorded %s",
			sigfile::metric_method(params._profile_type), F.subject(), F.session(), F.episode(),
			F.pages(), pz-pa, ctime( &F.start_time()));

		// this is not really a reportable/recoverable circumstance, so just abort
		assert (pz - pa == (int)F.pages());
		_pages_in_bed += (pz-pa);

		if ( pa < 0 ) {
			_status |= TFlags::enegoffset;
			return;
		}
		// this condition is checked against already in CSubject::SEpisodeSequence::add_one()
		// if ( _mm_bounds.size() > 0  &&  pa - _mm_bounds.back().second > 4 * 24 * 3600 ) {
		// 	_status |= (int)TSimPrepError::efarapart;
		// 	return;
		// }
		_mm_bounds.emplace_back( TBounds (pa, pz));

		_timeline.resize( pz, sigfile::SPageSimulated {0., 0., 1.});  // fill with WAKE
	}

	create_timeline();

	if ( _sim_start != (size_t)-1 )
		printf( "CSCourse::CSCourse(): sim start-end: %zu-%zu; avg SWA = %.4g (over %zu pp, or %.3g%% of all time in bed); "
			" SWA_L = %g;  SWA[%zu] = %g\n",
			_sim_start, _sim_end, _SWA_100, _pages_with_SWA, (double)_pages_with_SWA / _pages_in_bed * 100,
			_SWA_L, _sim_start, _SWA_0);
	else
		printf( "CSCourse::CSCourse(): status %xd, %s\n", _status, CSCourse::explain_status( _status).c_str());
}




agh::CSCourse::
CSCourse( CSCourse&& rv)
      : SSCourseParamSet (rv),
	_sim_start (rv._sim_start), _sim_end (rv._sim_end),
	_baseline_end (rv._baseline_end),
	_pages_with_SWA (rv._pages_with_SWA),
	_pages_in_bed (rv._pages_in_bed),
	_SWA_L (rv._SWA_L), _SWA_0 (rv._SWA_0), _SWA_100 (rv._SWA_100),
	_0at (rv._0at),
	_pagesize (rv._pagesize)
{
	swap( _timeline,  rv._timeline);
	swap( _mm_bounds, rv._mm_bounds);
	swap( _mm_list,   rv._mm_list);
}




void
agh::CSCourse::
create_timeline()
{
	_metric_avg = 0.;
	for ( auto Mi = _mm_list.begin(); Mi != _mm_list.end(); ++Mi ) {
		const auto& M = **Mi;
		const auto& F = M.F();

		if ( F.percent_scored() < _req_percent_scored )
			_status |= TFlags::enoscore;

	      // collect M's power and scores
		valarray<TFloat>
			lumped_bins;
		switch ( _profile_type ) {
		case sigfile::TMetricType::Psd:
			lumped_bins =
				M.CBinnedPower::course<TFloat>( _freq_from, _freq_upto);
		    break;
		case sigfile::TMetricType::Mc:
			size_t b = (_freq_from - M.freq_from) / M.bandwidth;
			lumped_bins =
				M.CBinnedMC::course<TFloat>( min( b, M.CBinnedMC::bins()-1)); // make up a range of freq_from + bandwidth instead
		    break;
		}


		size_t	pa = (size_t)difftime( F.start_time(), _0at) / _pagesize,
			pz = (size_t)difftime( F.end_time(), _0at) / _pagesize;
		for ( size_t p = pa; p < pz; ++p ) {
			_timeline[p] = sigfile::SPageSimulated {F[p-pa]};
		      // fill unscored/MVT per user setting
			if ( !_timeline[p].is_scored() ) {
				if ( _ScoreUnscoredAsWake )
					_timeline[p].mark( sigfile::SPage::TScore::wake);
				else
					if ( p > 0 )
						_timeline[p] = _timeline[p-1];
			}
		      // put SWA, compute avg PSD
			_metric_avg +=
				(_timeline[p].metric = lumped_bins[p-pa]);
		}

	      // determine SWA_0
		if ( Mi == _mm_list.begin() ) {
			_baseline_end = pz;

			// require some length of swa-containing pages to happen before sim_start
			for ( size_t p = 0; p < pz; ++p ) {
				for ( size_t pp = p; pp < pz; ++pp ) {
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
				_status |= TFlags::enoswa;
			else
				_SWA_0 = _timeline[_sim_start].metric;
		}

		_sim_end = pz-1;
	}
	_metric_avg /= _pages_in_bed;

      // determine SWA metrics
	_pages_with_SWA = _pages_non_wake = 0;
	_SWA_L = _SWA_100 = 0.;

	if ( _sim_start != (size_t)-1 ) {
		size_t REM_pages_cnt = 0;
		for ( size_t p = _sim_start; p < _sim_end; ++p ) {
			auto& P = _timeline[p];
			if ( P.REM > .5 ) {
				_SWA_L += P.metric;
				++REM_pages_cnt;
			}
			if ( P.NREM > 1./3 ) {
				_SWA_100 += P.metric;
				++_pages_with_SWA;
			}
			if ( P.Wake == 0. )
				++_pages_non_wake;
		}
		if ( REM_pages_cnt )
			_SWA_L /= (REM_pages_cnt / .95);
		if ( _pages_with_SWA )
			_SWA_100 /= _pages_with_SWA;
	}
}





int
agh::CExpDesign::
setup_modrun( const char* j, const char* d, const char* h,
	      sigfile::TMetricType metric_type,
	      float freq_from, float freq_upto,
	      agh::CModelRun* &R_ref)
{
	try {
		CSubject& J = subject_by_x(j);

		if ( J.measurements[d].size() == 1 && ctl_params0.DBAmendment2 )
			return CSCourse::TFlags::eamendments_ineffective;

		if ( J.measurements[d].size() == 1 && tunables0.step[TTunable::rs] > 0. )
			return CSCourse::TFlags::ers_nonsensical;

		auto freq_idx = pair<float,float> (freq_from, freq_upto);
		J.measurements[d]
			. modrun_sets[metric_type][h].insert(
				pair<pair<float, float>, CModelRun>
				(freq_idx, agh::CModelRun (J, d, h,
							   metric_type, freq_from, freq_upto,
							   ctl_params0, tunables0)));
		R_ref = &J.measurements[d]
			. modrun_sets[metric_type][h][freq_idx];

	} catch (invalid_argument ex) { // thrown by CSCourse ctor
		fprintf( stderr, "CExpDesign::setup_modrun( %s, %s, %s): %s\n", j, d, h, ex.what());
		return -1;
	} catch (int ex) { // thrown by CModelRun ctor
		log_message( "CExpDesign::setup_modrun( %s, %s, %s): %s\n", j, d, h, CSCourse::explain_status(ex).c_str());
		return ex;
	}

	return 0;
}



string
agh::CSCourse::
explain_status( int code)
{
	list<const char*> ss;
	if ( code & TFlags::enoscore )
		ss.push_back( "insufficiently scored");
	if ( code & TFlags::efarapart )
		ss.push_back( "episodes too far apart");
	if ( code & TFlags::esigtype )
		ss.push_back( "signal is not an EEG");
	if ( code & TFlags::etoomanymsmt )
		ss.push_back( "too many episodes");
	if ( code & TFlags::enoswa )
		ss.push_back( "no SWA");
	if ( code & TFlags::eamendments_ineffective)
		ss.push_back( "inappropriate amendments");
	if ( code & TFlags::ers_nonsensical )
		ss.push_back( "too few episoded for rs");
	if ( code & TFlags::enegoffset )
		ss.push_back( "negative offset");
	if ( code & TFlags::euneq_pagesize )
		ss.push_back( "wrong page size");
	return agh::str::join( ss, "; ");
}







agh::CModelRun::
CModelRun( CSubject& subject, const string& session, const sigfile::SChannel& channel,
	   sigfile::TMetricType metric_type,
	   float freq_from, float freq_upto,
	   const SControlParamSet& _ctl_params,
	   const STunableSetFull& t0)
      : CSCourse( subject, session, channel,
		  agh::SSCourseParamSet {metric_type,
				  freq_from, freq_upto, (float)_ctl_params.req_percent_scored,
				  _ctl_params.swa_laden_pages_before_SWA_0,
				  _ctl_params.ScoreUnscoredAsWake}),
	status (0),
	ctl_params (_ctl_params),
	tt (t0, ctl_params.AZAmendment1 ? _mm_list.size() : 1),
	cur_tset (t0.value, ctl_params.AZAmendment1 ? _mm_list.size() : 1)
{
	if ( CSCourse::_status )
		throw CSCourse::_status;
	_prepare_scores2();
}

agh::CModelRun::
CModelRun( CModelRun&& rv)
      : CSCourse (move(rv)),
	status (rv.status),
	ctl_params (rv.ctl_params),
	tt (move(rv.tt)),
	cur_tset (move(rv.cur_tset))
{
	_prepare_scores2();
}





// eof
