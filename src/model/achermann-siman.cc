// ;-*-C++-*-
/*
 *       File name:  model/achermann-siman.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-03
 *
 *         Purpose:  Model simulation using gsl siman facility
 *
 *         License:  GPL
 */


#include <sys/time.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_siman.h>

#include "achermann.hh"
#include "../expdesign/recording.hh"

using namespace std;


inline namespace {

// struct SAchermannSimanPPack {
// 	agh::ach::CModelRun
// 	agh::ach::STunableSet tset;
// };


// modify the configuration xp using a random step taken from the
// generator r, up to a maximum distance of step_size
void
_siman_step( const gsl_rng *r, void *xp, double step_size)
{
	auto& R = *(agh::ach::CModelRun*)xp;
	STunableSet
		X0 (R.cur_tset),
		X1 (X0);
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

	R.cur_tset = X1;

//	siman::_siman_print( &X0[0]);
//	siman::_siman_print( &X1[0]);
	// printf( "normalized:\n");
	// siman::_siman_print( &X1.normalize(step) [0]);
	// printf( "difference of normalized, ^2:\n");
	// valarray<double> X3 (pow( X1.normalize(step) - X0.normalize(step), 2.));
	// siman::_siman_print( & X3[0]);
}




double
_cost_function( void *xp)
{
	auto& R = *(agh::ach::CModelRun*)xp;
	return R.snapshot();
}

double
_siman_metric( void *xp, void *yp)
{
	return modrun->_siman_metric( xp, yp);

	return STunableSet (tt.value.P.size() - (size_t)TTunable::gc, (double*)xp).distance(
		STunableSet (tt.value.P.size() - (size_t)TTunable::gc, (double*)yp),
		tt.step);
}

void
agh::ach::siman::
_siman_print( void *xp)
{
	ach::STunableSet _tset;
	_tset = (double*)xp;
	for ( size_t t = 0; t < _tset.size(); ++t )
		printf( "%s = %g %s  ",
			STunableSet::tunable_name(t).c_str(), _tset[t],
			STunableSet::stock[min(t, (size_t)TTunable::gc)].unit);
	printf( "\n");
}


int
agh::ach::CModelRun::
watch_simplex_move( void (*printer)(void*))
{
	if ( siman::modrun ) // occupied (should be prevented in the first instance by button press handlers)
		return 1;
	// FIXME: do it properly with atomic?

	siman::modrun = this;
	gsl_siman_solve( __agh_rng ? __agh_rng : (init_global_rng(), __agh_rng),
			 (void*)&cur_tset.P[0],	// void * x0_p,
			 siman::_cost_function,	// gsl_siman_Efunc_t,
			 siman::_siman_step,	// gsl_siman_step_t
			 siman::_siman_metric,	// gsl_siman_metric_t,
			 printer,		// gsl_siman_print_t print_position,
//			 siman::_siman_print,
			 NULL, NULL, NULL,	// gsl_siman_copy_t copyfunc, gsl_siman_copy_construct_t copy_constructor, gsl_siman_destroy_t destructor,
			 cur_tset.size() * sizeof(double),	// size_t element_size,
			 ctl_params.siman_params);		// gsl_siman_params_t params

	if ( ctl_params.DBAmendment2 ) {
		const float ppm = 60. / pagesize();
		double edt = exp( -(24*60*ppm + _sim_start - _baseline_end) * cur_tset[TTunable::rs]);
		cur_tset[TTunable::SU] = (_timeline[_sim_start].S - _timeline[_baseline_end].S * edt) / (1. - edt)
			/ (_SWA_100/100);
	}

	siman::modrun = nullptr; // kind of releasing a mutex
	status |= modrun_tried;
	return 0;
}





// eof
