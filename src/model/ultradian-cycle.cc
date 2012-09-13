// ;-*-C++-*-
/*
 *       File name:  model/ultradian-cycle.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-08
 *
 *         Purpose:  Detect NREM-REM cycle (simulation annealing method)
 *
 *         License:  GPL
 */

#include <gsl/gsl_vector.h>
#include <gsl/gsl_siman.h>
#include <gsl/gsl_blas.h>

#include "../expdesign/recording.hh"
#include "beersma.hh"

using namespace std;


// see http://www.gnu.org/software/gsl/manual/html_node/Example-programs-for-Nonlinear-Least_002dSquares-Fitting.html

namespace agh {
namespace beersma {
constexpr double
	SUltradianCycle::ir, SUltradianCycle::iT, SUltradianCycle::id, SUltradianCycle::ib, // the last one is a normalized value of metric
	SUltradianCycle::ur, SUltradianCycle::uT, SUltradianCycle::ud, SUltradianCycle::ub,
	SUltradianCycle::lr, SUltradianCycle::lT, SUltradianCycle::ld, SUltradianCycle::lb;
}
}

inline namespace {

struct SUltradianCyclePPack {
	agh::beersma::FUltradianCycle F;
	agh::CRecording* M;
	const agh::beersma::SUltradianCycleCtl* P;

	SUltradianCyclePPack () = delete;
	SUltradianCyclePPack (const SUltradianCyclePPack&) = default;
	SUltradianCyclePPack (const agh::beersma::FUltradianCycle& F_,
			      agh::CRecording* M_,
			      const agh::beersma::SUltradianCycleCtl* P_)
	      : F (F_), M (M_), P (P_)
		{}
};



double
uc_cost_function( void *xp)
{
	auto& X = *(SUltradianCyclePPack*)(xp);

      // set up
	auto course = X.M->course<double>( X.P->metric, X.P->freq_from, X.P->freq_upto);
	const size_t	pp = course.size();
	const size_t	pagesize = X.M->pagesize();

	double	cf = 0.;
	for ( size_t p = 0; p < pp; ++p )
		cf += gsl_pow_2( X.F(p*pagesize/60.) - course[p]);
	return cf;
}

void
uc_siman_step( const gsl_rng *r, void *xp, double step_size)
{
	auto&	X  = *(SUltradianCyclePPack*)(xp),
		Xm (X);

retry:
	double *pip;
	const double *ipip, *upip, *lpip;
	switch ( gsl_rng_uniform_int( r, 4) ) {
	case 0:  pip = &Xm.F.r; ipip = &Xm.F.ir; lpip = &Xm.F.lr; upip = &Xm.F.ur; break;
	case 1:  pip = &Xm.F.T; ipip = &Xm.F.iT; lpip = &Xm.F.lT; upip = &Xm.F.uT; break;
	case 2:  pip = &Xm.F.d; ipip = &Xm.F.id; lpip = &Xm.F.ld; upip = &Xm.F.ud; break;
	case 3:
	default: pip = &Xm.F.b; ipip = &Xm.F.ib; lpip = &Xm.F.lb; upip = &Xm.F.ub; break;
	}

	bool go_positive = (bool)gsl_rng_uniform_int( r, 2);

	double	d,
		nudge = *ipip;
	size_t nudges = 0;
	do {
		// nudge it a little,
		// prevent from going out-of-bounds
		if ( go_positive )
			if ( *pip + nudge < *upip )
				*pip += nudge;
			else
				goto retry;
		else
			if ( *pip - nudge > *lpip )
				*pip -= nudge;
			else
				goto retry;

		d = agh::beersma::distance( X.F, Xm.F);
		// printf( "  r = %g, T = %g, d = %g, b = %g; d = %g\n",
		// 	Xm.F.r, Xm.F.T, Xm.F.d, Xm.F.b, d);

		if ( d > step_size && nudges == 0 ) {  // nudged too far from the outset
			nudge /= 2;
			Xm = X;
			continue;
		}

		++nudges;

	} while ( d < step_size );

	X = Xm;
}



double
uc_siman_metric( void *xp, void *yp)
{
	return agh::beersma::distance(
		((SUltradianCyclePPack*)xp) -> F,
		((SUltradianCyclePPack*)yp) -> F);
}

void
uc_siman_print( void *xp)
{
	auto& X = *(SUltradianCyclePPack*)(xp);
	printf( "F r = %g, T = %g, d = %g, b = %g\n",
		X.F.r, X.F.T, X.F.d, X.F.b);
}

} // inline namespace




agh::beersma::SUltradianCycle
agh::beersma::
ultradian_cycles( agh::CRecording& M,
		  const agh::beersma::SUltradianCycleCtl& P,
		  list<agh::beersma::SUltradianCycleDetails> *extra)
{
	SUltradianCyclePPack
		X (FUltradianCycle ({0.0046, 80. * M_PI, 0.2, 0.}), &M, &P);
	gsl_siman_solve( __agh_rng ? __agh_rng : (init_global_rng(), __agh_rng),
			 (void*)&X,		//
			 uc_cost_function,	// gsl_siman_Efunc_t,
			 uc_siman_step,		// gsl_siman_step_t
			 uc_siman_metric,	// gsl_siman_metric_t,
			 uc_siman_print,	// gsl_siman_print_t print_position,
			 NULL, NULL, NULL,	// gsl_siman_copy_t copyfunc, gsl_siman_copy_construct_t copy_constructor, gsl_siman_destroy_t destructor,
			 sizeof(SUltradianCyclePPack),	// size_t element_size,
			 P.siman_params);		// gsl_siman_params_t

	if ( extra )
		*extra = analyse_deeper( X.F, M, P);

	return M.uc_params = X.F;
}


// lil helper

list<agh::beersma::SUltradianCycleDetails>
agh::beersma::
analyse_deeper( const SUltradianCycle& C,
		agh::CRecording& M,
		const agh::beersma::SUltradianCycleCtl& P)
{
	list<agh::beersma::SUltradianCycleDetails>
		ret;
	

	return ret;
}


// eof
