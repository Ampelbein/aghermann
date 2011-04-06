// ;-*-C++-*- *  Time-stamp: "2011-04-02 17:59:56 hmmr"
/*
 *       File name:  libagh/model.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-01
 *
 *         Purpose:  simulation model classes definitions
 *
 *         License:  GPL
 */

#include <functional>
#include <algorithm>

#include "tunable.hh"
#include "primaries.hh"
#include "model.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;

namespace agh {

TSimPrepError
CSCourse::layout_measurements( const TMsmtPtrList& MM,
			       float freq_from, float freq_upto,
			       float req_percent_scored,
			       size_t swa_laden_pages_before_SWA_0,
			       bool ScoreMVTAsWake, bool ScoreUnscoredAsWake)
{
	timeline.clear();
	mm_bounds.clear();

	for ( auto Mi = MM.begin(); Mi != MM.end(); ++Mi ) {
		const CRecording& M = **Mi;
		const CEDFFile& F = M.F();

		if ( F.percent_scored() < req_percent_scored )
			return TSimPrepError::enoscore;

	      // anchor zero page, get pagesize from edf^W CBinnedPower^W either goes
		if ( Mi == MM.begin() ) {
			_0at = F.start_time;
			_pagesize = M.pagesize();
			_pages_in_bed = 0;
		} else
			if ( _pagesize != F.pagesize() )
				return TSimPrepError::euneq_pagesize;

		size_t	pa = (size_t)difftime( F.start_time, _0at) / _pagesize,
			pz = (size_t)difftime( F.end_time, _0at) / _pagesize;
		assert ( pz - pa == M.F().CHypnogram::length());
		_pages_in_bed += (pz-pa);

		if ( pa < 0 )
			return TSimPrepError::enegoffset;
		if ( mm_bounds.size() > 0  &&  pa - mm_bounds.back().second > 4 * 24 * 3600 )
			return TSimPrepError::efarapart;
		mm_bounds.emplace_back( TBounds (pa,pz));

		timeline.resize( pz, SPageSimulated(0., 0., 1.));  // fill with WAKE

	      // collect M's power and scores
		valarray<double>
			lumped_bins = M.power_course( freq_from, freq_upto);

		for ( size_t p = pa; p < pz; ++p ) {
			timeline[p] = SPageSimulated (F.nth_page(p-pa));
		      // fill unscored/MVT per user setting
			if ( timeline[p].Wake == AGH_MVT_WAKE_VALUE ) {
				if ( ScoreMVTAsWake )
					timeline[p].mark( TScore::wake);
				else
					if ( p > 0 )
						timeline[p] = timeline[p-1];
			} else if ( !timeline[p].is_scored() ) {
				if ( ScoreUnscoredAsWake )
					timeline[p].mark( TScore::wake);
				else
					if ( p > 0 )
						timeline[p] = timeline[p-1];
			}
		      // put SWA
			timeline[p].SWA = lumped_bins[p-pa];
		}

		fprintf( stderr,
			 "CSCourse::layout_measurements(): added [%s, %s, %s] recorded %s",
			 F.PatientID_raw, F.Session.c_str(), F.Episode.c_str(),
			 ctime( &F.start_time));

	      // determine SWA_0
		if ( Mi == MM.begin() ) {
			_baseline_end = pz;

			// require some length of swa-containing pages to happen before sim_start
			for ( size_t p = 0; p < pz; ++p ) {
				for ( size_t pp = p; pp < pz; ++pp ) {
					if ( timeline[pp].NREM < 1./3 ) {
						p = pp;
						goto outer_continue;
					}
					if ( (pp-p) >= swa_laden_pages_before_SWA_0 ) {
						_sim_start = pp;
						goto outer_break;
					}
				}
			outer_continue:
				;
			}
		outer_break:

			if ( _sim_start == (size_t)-1 )
				return TSimPrepError::enoswa;
			_SWA_0 = timeline[_sim_start].SWA;
		}

		_sim_end = pz-1;
	}


      // determine SWA_L
	_pages_with_SWA = 0;
	_SWA_L = _SWA_100 = 0.;

	size_t REM_pages_cnt = 0;
	for ( size_t p = _sim_start; p < _sim_end; ++p ) {
		SPageSimulated& P = timeline[p];
		if ( P.REM > .5 ) {
			_SWA_L += P.SWA;
			++REM_pages_cnt;
		}
		if ( P.NREM > 1./3 ) {
			_SWA_100 += P.SWA;
			++_pages_with_SWA;
		}
	}
	_SWA_L /= (REM_pages_cnt / .95);
	_SWA_100 /= _pages_with_SWA;


	subject = MM.front()->subject();
	session = MM.front()->session();
	channel = MM.front()->channel();

	fprintf( stderr,
		 "CSCourse::layout_measurements(): sim start-end: %zu-%zu; avg SWA = %.4g (over %zu pp, or %.3g%% of all time in bed); "
		 " SWA_L = %g;  SWA[%zu] = %g\n",
		 _sim_start, _sim_end, _SWA_100, _pages_with_SWA, (double)_pages_with_SWA / _pages_in_bed * 100,
		 _SWA_L, _sim_start, _SWA_0);

	return TSimPrepError::ok;
}








TSimPrepError
CExpDesign::setup_modrun( const char* j, const char* d, const char* h,
			  float freq_from, float freq_upto,
			  CSimulation* &R_ref) throw (int) // logic_error
{
	try {
		CSubject& J = subject_by_x(j);
		// string	sim_fname (make_fname_simulation( J.name(), d, h,
		// 					  //0, J.measurements.size(),
		// 					  freq_from, freq_upto).c_str());

		// list<CSimulation>::iterator R = find_if( simulations.begin(), simulations.end(),
		//  					 bind( &CSimulation::matches, j, h, d, freq_from, freq_upto, control_params));
		// ниасилил!

		if ( J.measurements[d].size() == 1 && ctl_params0.DBAmendment2 )
			return TSimPrepError::eamendments_ineffective;

		if ( J.measurements[d].size() == 1 && tunables0.step[TTunable::rs] > 0. )
			return TSimPrepError::ers_nonsensical;

		// collect measurements in requested session and channel
		CSCourse::TMsmtPtrList MM;
		for ( auto E = J.measurements[d].episodes.begin(); E != J.measurements[d].episodes.end(); ++E )
			MM.push_back( &(E->recordings.at(h)));

		J.measurements[d]
			. modrun_sets[h]
			. emplace_back( //pair< pair<float, float>, CSimulation>
				pair< pair<float, float>, CSimulation> (
					pair< float, float> (freq_from, freq_upto),
					CSimulation (MM, freq_from, freq_upto,
						     ctl_params0, tunables0)));
		R_ref = &J.measurements[d]
			. modrun_sets[h].rbegin()->second;

	} catch (TSimPrepError ex) {
		log_message( string("CExpDesign::setup_modrun( ")+j+", "+d+", "+h+"): " + simprep_perror(ex)+'\n');
		throw;
	}

	return TSimPrepError::ok;
}




gsl_rng *__agh_rng = NULL;

void
init_global_rng()
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

}


// EOF
