// ;-*-C++-*- *  Time-stamp: "2010-08-14 01:57:58 hmmr"
/*
 *       File name:  core/siman.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-05-03
 *
 *         Purpose:  Model simulation using gsl siman facility
 *
 *         License:  GPL
 */


#include <cmath>
#include <gsl/gsl_siman.h>

#include "model.hh"

using namespace std;





// create a copy of original scores; also patch Unscored and MVT
void
CModelRun::_prepare_scores2()
{
	size_t p, pp;

	_scores2.assign( timeline.begin(), timeline.end());

	if ( ScoreUnscoredAsWake ) {
		for ( p = _sim_start; p < timeline.size(); p++ )
			if ( _scores2[p].NREM + _scores2[p].REM + _scores2[p].Wake == 0 ) { // this is Unscored
//				printf( " (patching Unscored as Wake at %d)\n", p);
				_scores2[p].Wake = 1.;
			}
	} else {  // ... as prev page
		for ( p = (_sim_start > 0) ?_sim_start :1; p < timeline.size(); p++ )
			if ( _scores2[p].NREM + _scores2[p].REM + _scores2[p].Wake == 0 ) {
//				printf( " (patching Unscored as prev at %d)\n", p);
				pp = p-1;
				do
					_scores2[p] = _scores2[pp];
				while ( ++p < timeline.size() &&
					_scores2[p].NREM + _scores2[p].REM + _scores2[p].Wake == 0. );
			}
	}

	if ( ScoreMVTAsWake ) {
		for ( p = _sim_start; p < timeline.size(); p++ )
			if ( _scores2[p].Wake == AGH_MVT_WAKE_VALUE ) { // this is MVT
//				printf( " (patching MVT as Wake at %d)\n", p);
				_scores2[p].Wake = 1.;
			}
	} else {  // ... as prev page
		for ( p = (_sim_start > 0) ?_sim_start :1; p < timeline.size(); p++ )
			if ( _scores2[p].Wake == AGH_MVT_WAKE_VALUE ) {
//				printf( " (patching MVT as prev at %d)\n", p);
				pp = p-1;
				do
					_scores2[p] = _scores2[pp];
				while ( ++p < timeline.size() &&
					_scores2[p].Wake == AGH_MVT_WAKE_VALUE );
			}
	}
}





// recreate timeline REM values from _scores2, extend REM bouts per _ta_ and _tp_
void
CModelRun::_restore_scores_and_extend_rem( size_t da, size_t dz)
{
	size_t	a  =                   da,
		z  = timeline.size() - dz,
		p, pi, di;

//	if ( fresh )
		for ( p = 0; p < timeline.size(); p++ )
			timeline[p].REM = _scores2[p].REM;

	for ( p = a; p < z; p++ )
		if ( _scores2[p].REM > 0.33 ) {  // only deal with some substantial REM
			for ( pi = p; timeline[pi].REM > .33 && pi < z; pi++ ) {

				// pull front
				di = da;
				do  {
					if ( _scores2[pi].REM > _scores2[pi - di].REM )
						timeline[pi - di].REM = _scores2[pi].REM;
					di--;
				} while ( di );

				// push end
				di = dz;
				do {
					if ( _scores2[pi].REM > _scores2[pi + di].REM )
						timeline[pi + di].REM = _scores2[pi].REM;
					di--;
				} while ( di );

			}  // perhaps, in addition to spreading the boundary value to regions before and after existing REM,
			   // we should also bump the existing values inside it?
			p = pi;
		}
}





int
CModelRun::watch_simplex_move()
{
	gsl_siman_solve( __agh_rng,
			 (void*)&cur_tset.P[0],		// void * x0_p,
			 (gsl_siman_Efunc_t) &CModelRun::_cost_function,	// gsl_siman_Efunc_t,
			 (gsl_siman_step_t)  &CModelRun::_siman_step,		// gsl_siman_step_t
			 (gsl_siman_metric_t)&CModelRun::_siman_metric,		// gsl_siman_metric_t,
			 NULL,				// gsl_siman_print_t print_position,
			 NULL, NULL, NULL,		// gsl_siman_copy_t copyfunc, gsl_siman_copy_construct_t copy_constructor, gsl_siman_destroy_t destructor,
			 cur_tset.size(),		// size_t element_size,
			 siman_params);			// gsl_siman_params_t params
	return 0;
}






double
CModelRun::_cost_function( void *xp)
{
	cur_tset = (double*)xp; // this is clandestinely overridden

	const float ppm = 60. / pagesize();
	STunableSet _tset (cur_tset);
	_tset.adjust_for_ppm( ppm);

	_restore_scores_and_extend_rem( (int)round( _tset.P[_ta_]), (int)round( _tset.P[_tp_]));

      // substitute S_0 and S_U, expressed in %, with abs values
	_tset[_S0_] *= _SWA_100/100;
	_tset[_SU_] *= _SWA_100/100;

	if ( DBAmendment2 )
		timeline[_baseline_end].S = _SWA_100 * 3; // will be overwritten at completion of the first iteration

      // prime S and swa_sim
	timeline[_sim_start].SWA_sim = _SWA_0;
	timeline[_sim_start].S = _tset[_S0_];

	double _fit = 0.;

#define CF_CYCLE_COMMON \
	{								\
		int	WT = (timeline[p].Wake > 0.33);				\
										\
		double	pS = timeline[p].S / _tset[_SU_];			\
		timeline[p+1].SWA_sim = timeline[p].SWA_sim			\
			+ _tset[_rc_] * timeline[p].SWA_sim * pS * (1. - timeline[p].SWA_sim / timeline[p].S) \
			* (DBAmendment1 ?(1. - timeline[p].Wake) :1) \
			- _tset[_fcR_] * (timeline[p].SWA_sim - _SWA_L) * timeline[p].REM \
			- _tset[_fcW_] * (timeline[p].SWA_sim - _SWA_L) * timeline[p].Wake; \
										\
		timeline[p+1].S = timeline[p].S + ( (WT && DBAmendment1)	\
						    ?0				\
						    :(-_which_gc(p) * timeline[p].SWA_sim) ) \
			+ (_tset[_SU_] - timeline[p].S) * _tset[_rs_];		\
										\
		if ( timeline[p].has_swa() )					\
			_fit += pow( timeline[p].SWA - timeline[p].SWA_sim, 2);	\
	}

	if ( DBAmendment2 )
		for ( size_t p = _sim_start; p < _sim_end; ++p ) {
			double edt = exp( -(24*60*ppm + _sim_start - _baseline_end) * _tset.P[_rs_]);
			_tset.P[_SU_] = (timeline[_sim_start].S - timeline[_baseline_end].S * edt) / (1. - edt);

			CF_CYCLE_COMMON;
		}
	else
		for ( size_t p = _sim_start; p < _sim_end; ++p )
			CF_CYCLE_COMMON;

#undef CF_CYCLE_COMMON

	return _fit;
}












// modify the configuration xp using a random step taken from the
// generator r, up to a maximum distance of step_size
void
CModelRun::_siman_step( const gsl_rng *r, void *xp, double step_size)
{
	STunableSet
		X0 (cur_tset.size() - (_gc_ + 1), (double*)xp),
		X1 = X0;

      // randomly pick a tunable
retry:
	size_t t = gsl_rng_uniform_int( r, cur_tset.size());
	if ( DBAmendment2 && t == _SU_ )
		goto retry;

	bool go_positive = (bool)gsl_rng_uniform_int( r, 2);

	double nudge = step[t];
	size_t nudges = 0;
	do {
	      // nudge it a little
		if ( go_positive )
			X1[t] += nudge;
		else
			X1[t] -= nudge;

	      // need perhaps to check for tunables going out-of-bounds
		;

		if ( X0 - X1 > step_size && nudges == 0 ) {  // nudged too far from the outset
			nudge /= 2;
			X1[t] = X0[t];
			continue;
		}

		nudges++;

	} while ( X0 - X1 < step_size );

      // revert the last nudge that caused X1 to go overboard
	if ( go_positive )
		X1[t] -= nudge;
	else
		X1[t] += nudge;
}




// EOF
