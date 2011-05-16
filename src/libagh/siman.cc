// ;-*-C++-*- *  Time-stamp: "2011-05-15 02:24:10 hmmr"
/*
 *       File name:  libagh/siman.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-03
 *
 *         Purpose:  Model simulation using gsl siman facility
 *
 *         License:  GPL
 */


#include <gsl/gsl_math.h>
#include <gsl/gsl_siman.h>

#include "model.hh"

namespace agh {
using namespace std;





// create a copy of original scores; also patch Unscored and MVT
void
CModelRun::_prepare_scores2()
{
	size_t p, pp;

	_scores2.assign( _timeline.begin(), _timeline.end());

	if ( ctl_params.ScoreUnscoredAsWake ) {
		for ( p = _sim_start; p < _timeline.size(); ++p )
			if ( _scores2[p].NREM + _scores2[p].REM + _scores2[p].Wake == 0 ) { // this is Unscored
//				printf( " (patching Unscored as Wake at %d)\n", p);
				_scores2[p].Wake = 1.;
			}
	} else {  // ... as prev page
		for ( p = (_sim_start > 0) ?_sim_start :1; p < _timeline.size(); ++p )
			if ( _scores2[p].NREM + _scores2[p].REM + _scores2[p].Wake == 0 ) {
//				printf( " (patching Unscored as prev at %d)\n", p);
				pp = p-1;
				do
					_scores2[p] = _scores2[pp];
				while ( ++p < _timeline.size() &&
					_scores2[p].NREM + _scores2[p].REM + _scores2[p].Wake == 0. );
			}
	}

	if ( ctl_params.ScoreMVTAsWake ) {
		for ( p = _sim_start; p < _timeline.size(); ++p )
			if ( _scores2[p].Wake == AGH_MVT_WAKE_VALUE ) { // this is MVT
//				printf( " (patching MVT as Wake at %d)\n", p);
				_scores2[p].Wake = 1.;
			}
	} else {  // ... as prev page
		for ( p = (_sim_start > 0) ?_sim_start :1; p < _timeline.size(); ++p )
			if ( _scores2[p].Wake == AGH_MVT_WAKE_VALUE ) {
//				printf( " (patching MVT as prev at %d)\n", p);
				pp = p-1;
				do
					_scores2[p] = _scores2[pp];
				while ( ++p < _timeline.size() &&
					_scores2[p].Wake == AGH_MVT_WAKE_VALUE );
			}
	}
}





// recreate timeline REM values from _scores2, extend REM bouts per _ta_ and _tp_
void
CModelRun::_restore_scores_and_extend_rem( size_t da, size_t dz)
{
	size_t	a  =                    da,
		z  = _timeline.size() - dz,
		p, pi, di;

	for ( p = 0; p < _timeline.size(); ++p )
		_timeline[p].REM = _scores2[p].REM;

	for ( p = a; p < z; ++p )
		if ( _scores2[p].REM > 0.33 ) {  // only deal with some substantial REM
			for ( pi = p; _timeline[pi].REM > .33 && pi < z; ++pi ) {

				// pull front
				di = da;
				do  {
					if ( _scores2[pi].REM > _scores2[pi - di].REM )
						_timeline[pi - di].REM = _scores2[pi].REM;
				} while ( di-- );

				// push end
				di = dz;
				do {
					if ( _scores2[pi].REM > _scores2[pi + di].REM )
						_timeline[pi + di].REM = _scores2[pi].REM;
				} while ( di-- );

			}  // perhaps, in addition to spreading the boundary value to regions before and after existing REM,
			   // we should also bump the existing values inside it?
			p = pi;
		}
}






double
CModelRun::_cost_function( const void *xp)
{
	cur_tset = (double*)xp; // this is clandestinely overridden
//	NSSiman::_siman_print( xp);

//	printf( "AZAmendment = %d; cur_tset.size = %zu\n", AZAmendment, cur_tset.size());
	const float ppm = 60. / pagesize();
	STunableSet _tset (cur_tset);
	_tset.adjust_for_ppm( ppm);

	_restore_scores_and_extend_rem( (int)round( _tset[TTunable::ta]), (int)round( _tset[TTunable::tp]));

      // substitute S_0 and S_U, expressed in %, with abs values
	_tset[TTunable::S0] *= _SWA_100/100;
	_tset[TTunable::SU] *= _SWA_100/100;

	if ( ctl_params.DBAmendment2 )
		_timeline[_baseline_end].S = _SWA_100 * 3; // will be overwritten at completion of the first iteration

      // prime S and swa_sim
	_timeline[_sim_start].SWA_sim = _SWA_0;
	_timeline[_sim_start].S = _tset[TTunable::S0];

	double _fit = 0.;

#define CF_CYCLE_COMMON \
	{								\
		int	WT = (_timeline[p].Wake > 0.33);				\
										\
		double	pS = _timeline[p].S / _tset[TTunable::SU];	\
		_timeline[p+1].SWA_sim = _timeline[p].SWA_sim			\
			+ _tset[TTunable::rc] * _timeline[p].SWA_sim * pS * (1. - _timeline[p].SWA_sim / _timeline[p].S) \
			* (ctl_params.DBAmendment1 ?(1. - _timeline[p].Wake) :1) \
			- _tset[TTunable::fcR] * (_timeline[p].SWA_sim - _SWA_L) * _timeline[p].REM \
			- _tset[TTunable::fcW] * (_timeline[p].SWA_sim - _SWA_L) * _timeline[p].Wake; \
										\
		_timeline[p+1].S = _timeline[p].S + ( (WT && ctl_params.DBAmendment1)	\
						    ?0				\
						    :(-_which_gc(p) * _timeline[p].SWA_sim) ) \
			+ (_tset[TTunable::SU] - _timeline[p].S) * _tset[TTunable::rs]; \
										\
		if ( _timeline[p].has_swa() )					\
			_fit += gsl_pow_2( _timeline[p].SWA - _timeline[p].SWA_sim);	\
	}

	if ( ctl_params.DBAmendment2 )
		for ( size_t p = _sim_start; p < _sim_end; ++p ) {
			double edt = exp( -(24*60*ppm + _sim_start - _baseline_end) * _tset[TTunable::rs]);
			_tset[TTunable::SU] = (_timeline[_sim_start].S - _timeline[_baseline_end].S * edt) / (1. - edt);

			CF_CYCLE_COMMON;
		}
	else
		for ( size_t p = _sim_start; p < _sim_end; ++p )
			CF_CYCLE_COMMON;

#undef CF_CYCLE_COMMON

	return sqrt( _fit/_pages_with_SWA);
}












// modify the configuration xp using a random step taken from the
// generator r, up to a maximum distance of step_size
void
CModelRun::_siman_step( const gsl_rng *r, void *xp, double step_size)
{
	STunableSet
		X0 (cur_tset.size() - (size_t)TTunable::gc, (double*)xp),
		X1 = X0;

      // randomly pick a tunable
retry:
	size_t t = gsl_rng_uniform_int( r, cur_tset.size());
	if ( ctl_params.DBAmendment2 && t == TTunable::SU )
		goto retry;

	bool go_positive = (bool)gsl_rng_uniform_int( r, 2);

	double	nudge = tt.step[t],
		d;
	size_t nudges = 0;
	do {
	      // nudge it a little,
	      // prevent from going out-of-bounds
		if ( go_positive )
			if ( X1[t] + nudge < tt.hi[t] )
				X1[t] += nudge;
			else
				goto retry;
		else
			if ( X1[t] - nudge > tt.lo[t] )
				X1[t] -= nudge;
			else
				goto retry;

	      // special checks
		if ( (t == TTunable::S0 && X1[TTunable::S0] + nudge >= X1[TTunable::SU]) ||
		     (t == TTunable::SU && X1[TTunable::S0] >= X1[TTunable::SU] - nudge) )
			goto retry;

		d = X0.distance( X1, tt.step);

		if ( d > step_size && nudges == 0 ) {  // nudged too far from the outset
			nudge /= 2;
			X1[t] = X0[t];
			continue;
		}

		++nudges;

	} while ( d < step_size );

	memcpy( xp, &X1[0], cur_tset.size()*sizeof(double));

//	NSSiman::_siman_print( &X0[0]);
//	NSSiman::_siman_print( &X1[0]);
	// printf( "normalized:\n");
	// NSSiman::_siman_print( &X1.normalize(step) [0]);
	// printf( "difference of normalized, ^2:\n");
	// valarray<double> X3 (pow( X1.normalize(step) - X0.normalize(step), 2.));
	// NSSiman::_siman_print( & X3[0]);
}




// this is not reentrable!
namespace NSSiman {

	CModelRun *modrun;

	double
	_cost_function( void *xp)
	{
		return modrun->_cost_function( xp);
	}

	void
	_siman_step( const gsl_rng *r, void *xp, double step_size)
	{
		modrun->_siman_step( r, xp, step_size);
	}

	double
	_siman_metric( void *xp, void *yp)
	{
		return modrun->_siman_metric( xp, yp);
	}

	void
	_siman_print( void *xp)
	{
		STunableSet _tset;
		_tset = (double*)xp;
		for ( size_t t = 0; t < _tset.size(); ++t )
			printf( "%s = %g %s  ", STunableSet::stock[t].name, _tset[t], STunableSet::stock[t].unit);
		printf( "\n");
	}
};


int
CModelRun::watch_simplex_move( void (*printer)(void*))
{
	if ( NSSiman::modrun ) // occupied (should be prevented in the first instance by button press handlers)
		return 1;
	// FIXME: do it properly with atomic?

	NSSiman::modrun = this;
	gsl_siman_solve( __agh_rng ? __agh_rng : (init_global_rng(), __agh_rng),
			 (void*)&cur_tset.P[0],			// void * x0_p,
			 NSSiman::_cost_function,	// gsl_siman_Efunc_t,
			 NSSiman::_siman_step,		// gsl_siman_step_t
			 NSSiman::_siman_metric,	// gsl_siman_metric_t,
			 printer,			// gsl_siman_print_t print_position,
//			 NSSiman::_siman_print,
			 NULL, NULL, NULL,		// gsl_siman_copy_t copyfunc, gsl_siman_copy_construct_t copy_constructor, gsl_siman_destroy_t destructor,
			 cur_tset.size() * sizeof(double),		// size_t element_size,
			 ctl_params.siman_params);			// gsl_siman_params_t params

	if ( ctl_params.DBAmendment2 ) {
		const float ppm = 60. / pagesize();
		double edt = exp( -(24*60*ppm + _sim_start - _baseline_end) * cur_tset[TTunable::rs]);
		cur_tset[TTunable::SU] = (_timeline[_sim_start].S - _timeline[_baseline_end].S * edt) / (1. - edt)
			/ (_SWA_100/100);
	}

	NSSiman::modrun = NULL; // kind of releasing a mutex
	status |= AGH_MODRUN_TRIED;
	return 0;
}

}


// EOF
