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

#include "common/globals.hh"
#include "expdesign/recording.hh"
#include "achermann.hh"

using namespace std;


inline namespace {

struct SAchermannSimanPPack {
	agh::ach::CModelRun *R;
	// STunableSet is probably not a POD after all
	double t[agh::ach::TTunable::_all_tunables];
};


// modify the configuration xp using a random step taken from the
// generator r, up to a maximum distance of step_size
void
_siman_step( const gsl_rng *r, void *xp, double step_size)
{
	using namespace agh::ach;

	auto& P = *(SAchermannSimanPPack*)xp;
	STunableSet<TTRole::v>
		X0 (P.R->tx.n_egc),
		X  (P.R->tx.n_egc);
	memcpy( X0.P, P.t, sizeof(double) * X.size());
	memcpy( X .P, P.t, sizeof(double) * X.size());

      // randomly pick a tunable
retry:
	size_t t = gsl_rng_uniform_int( r, X.size());
	if ( P.R->ctl_params.DBAmendment2 && t == TTunable::SU )
		goto retry;

	bool go_positive = (bool)gsl_rng_uniform_int( r, 2);

	double	nudge = P.R->tstep[t],
		d;
	size_t nudges = 0;
	do {
	      // nudge it a little,
	      // prevent from going out-of-bounds
		if ( go_positive )
			if ( likely (X[t] + nudge < P.R->thi[t]) )
				X[t] += nudge;
			else
				goto retry;
		else
			if ( likely (X[t] - nudge > P.R->tlo[t]) )
				X[t] -= nudge;
			else
				goto retry;

	      // special checks
		if ( (t == TTunable::S0 && X[TTunable::S0] + nudge >= X[TTunable::SU]) ||
		     (t == TTunable::SU && X[TTunable::S0] >= X[TTunable::SU] - nudge) )
			goto retry;

		d = distance( X, X0, P.R->tstep.P);

		if ( d > step_size && nudges == 0 ) {  // nudged too far from the outset
			nudge /= 2;
			X[t] = X0[t];
			continue;
		}

		++nudges;

	} while ( d < step_size );

	// are there ever multiple copies of t in existence?
	// can we just always use R->tx?
	memcpy( P.t, X.P, sizeof(double) * X.size());
}




double
_cost_function( void *xp)
{
	auto& P = *(SAchermannSimanPPack*)xp;
	memcpy( P.R->tx.P, P.t, sizeof(double) * P.R->tx.size());
	return P.R->snapshot();
}

double
_siman_metric( void *xp, void *yp)
{
	auto& P1 = *(SAchermannSimanPPack*)xp;
	auto& P2 = *(SAchermannSimanPPack*)yp;
	agh::ach::STunableSet<agh::ach::TTRole::v>
		X1 (P1.R->tx.n_egc),
		X2 (P2.R->tx.n_egc);
	memcpy( X1.P, P1.t, sizeof(double) * X1.size());
	memcpy( X2.P, P2.t, sizeof(double) * X2.size());

	printf( "metric %g\n", agh::ach::distance( X1, X2, P1.R->tstep.P));
	return agh::ach::distance( X1, X2, P1.R->tstep.P);
}

// void
// _siman_print( void *xp)
// {
// 	auto& P = *(SAchermannSimanPPack*)xp;
// 	for ( size_t t = 0; t < P.R->tx.size(); ++t )
// 		printf( "%s = %g %s  ",
// 			agh::ach::tunable_name(t).c_str(), P.t[t],
// 			agh::ach::tunable_unit(t).c_str());
// 	printf( "\n");
// }

} // inline namespace



int
agh::ach::CModelRun::
watch_simplex_move( void (*printer)(void*))
{
	SAchermannSimanPPack x0;
	x0.R = this;
	memcpy( x0.t, tx.P, sizeof(double) * tx.size());
	gsl_siman_solve( agh::global::rng,
			 (void*)&x0,	// void * x0_p,
			 _cost_function,	// gsl_siman_Efunc_t,
			 _siman_step,	// gsl_siman_step_t
			 _siman_metric,	// gsl_siman_metric_t,
			 printer,		// gsl_siman_print_t print_position,
//			 _siman_print,
			 NULL, NULL, NULL,	// gsl_siman_copy_t copyfunc, gsl_siman_copy_construct_t copy_constructor, gsl_siman_destroy_t destructor,
			 sizeof(SAchermannSimanPPack),	// size_t element_size,
			 ctl_params.siman_params);		// gsl_siman_params_t params

	if ( ctl_params.DBAmendment2 ) {
		const float ppm = 60. / pagesize();
		double edt = exp( -(24*60*ppm + _sim_start - _baseline_end) * tx[TTunable::rs]);
		tx[TTunable::SU] = (_timeline[_sim_start].S - _timeline[_baseline_end].S * edt) / (1. - edt)
			/ (_SWA_100/100);
	}

	status |= modrun_tried;
	return 0;
}





// eof
