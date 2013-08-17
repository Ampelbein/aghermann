/*
 *       File name:  aghermann/model/ultradian-cycle.cc
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

#include "aghermann/globals.hh"
#include "aghermann/expdesign/recording.hh"
#include "libsigproc/sigproc.hh"
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

namespace {

struct SUltradianCyclePPack {
	double X[4];
	valarray<TFloat>* coursep;
	size_t pagesize;
	const agh::beersma::SUltradianCycleCtl* P;

	// SUltradianCyclePPack () = delete;
	// SUltradianCyclePPack (const SUltradianCyclePPack&) = default;
	// SUltradianCyclePPack (const agh::beersma::SUltradianCycle& X_,
	// 		      agh::CRecording* M_,
	// 		      const agh::beersma::SUltradianCycleCtl* P_)
	//       : X {X_.r, X_.T, X_.d, X_.b}, M (M_), P (P_)
	// 	{}
};



double
uc_cost_function( void *xp)
{
	auto& P = *(SUltradianCyclePPack*)(xp);

	agh::beersma::FUltradianCycle F ({P.X[0], P.X[1], P.X[2], P.X[3]});
	// if ( (unlikely (F.r > F.ur || F.r < F.lr ||
	// 		F.T > F.uT || F.T < F.lT ||
	// 		F.d > F.ud || F.d < F.ld ||
	// 		F.b > F.ub || F.b < F.lb) ) )
	// 	return 1e9;
	double	cf = 0.;
	for ( size_t p = 0; p < P.coursep->size(); ++p ) {
		cf += gsl_pow_2( F(p*P.pagesize/60.) - (*P.coursep)[p]);
	}
//	printf( "CF = %g\n\n", sqrt(cf));
	return sqrt(cf);
}

void
uc_siman_step( const gsl_rng *r, void *xp, double step_size)
{
	auto&	P = *(SUltradianCyclePPack*)(xp);
	agh::beersma::SUltradianCycle
		Xm = {P.X[0], P.X[1], P.X[2], P.X[3]},
		X0 = Xm;

retry:
	double *pip;
	const double *ipip, *upip, *lpip;
	switch ( gsl_rng_uniform_int( r, 4) ) {
	case 0:  pip = &Xm.r; ipip = &Xm.ir; lpip = &Xm.lr; upip = &Xm.ur; break;
	case 1:  pip = &Xm.T; ipip = &Xm.iT; lpip = &Xm.lT; upip = &Xm.uT; break;
	case 2:  pip = &Xm.d; ipip = &Xm.id; lpip = &Xm.ld; upip = &Xm.ud; break;
	case 3:
	default: pip = &Xm.b; ipip = &Xm.ib; lpip = &Xm.lb; upip = &Xm.ub; break;
	}

	bool go_positive = (bool)gsl_rng_uniform_int( r, 2);

	double	d,
		nudge = *ipip;
	size_t nudges = 0;
	do {
		// nudge it a little,
		// prevent from going out-of-bounds
		if ( go_positive )
			if ( likely (*pip + nudge < *upip) )
				*pip += nudge;
			else
				goto retry;
		else
			if ( likely (*pip - nudge > *lpip) )
				*pip -= nudge;
			else
				goto retry;

		d = agh::beersma::distance( X0, Xm);
		// printf( "  r = %g, T = %g, d = %g, b = %g; distance = %g\n",
		// 	Xm.r, Xm.T, Xm.d, Xm.b, d);

		if ( d > step_size && nudges == 0 ) {  // nudged too far from the outset
			nudge /= 2;
			Xm = X0;
			continue;
		}

		++nudges;

	} while ( d < step_size );

	P.X[0] = Xm.r;
	P.X[1] = Xm.T;
	P.X[2] = Xm.d;
	P.X[3] = Xm.b;
}



double
uc_siman_metric( void *xp, void *yp)
{
	auto& P1 = *(SUltradianCyclePPack*)xp;
	auto& P2 = *(SUltradianCyclePPack*)yp;
	agh::beersma::SUltradianCycle
		X1 = {P1.X[0], P1.X[1], P1.X[2], P1.X[3]},
		X2 = {P2.X[0], P2.X[1], P2.X[2], P2.X[3]};
	return agh::beersma::distance( X1, X2);
}

// void
// uc_siman_print( void *xp)
// {
// 	auto& P = *(SUltradianCyclePPack*)(xp);
// 	printf( "F r = %g, T = %g, d = %g, b = %g\n",
// 		P.X[0], P.X[1], P.X[2], P.X[3]);
// }

} // namespace




agh::beersma::SUltradianCycle
agh::beersma::
ultradian_cycles( agh::CRecording& M,
		  const agh::beersma::SUltradianCycleCtl& C,
		  list<agh::beersma::SUltradianCycleDetails> *extra)
{
	// normalize please
	auto course = M.course( C.profile_params);
	sigproc::smooth( course, 5u);
	//auto avg = course.sum()/course.size();
	course /= course.max() / 2; // because ultradian cycle function has a range of 2

	SUltradianCyclePPack
		P {{0.0046, 60., 0.2, 0.01},
		   &course,
		   M.pagesize(),
		   &C};
	gsl_siman_solve( agh::global::rng,
			 (void*)&P,		//
			 uc_cost_function,	// gsl_siman_Efunc_t,
			 uc_siman_step,		// gsl_siman_step_t
			 uc_siman_metric,	// gsl_siman_metric_t,
			 NULL, //uc_siman_print,	// gsl_siman_print_t print_position,
			 NULL, NULL, NULL,	// gsl_siman_copy_t copyfunc, gsl_siman_copy_construct_t copy_constructor, gsl_siman_destroy_t destructor,
			 sizeof(SUltradianCyclePPack),	// size_t element_size,
			 C.siman_params);		// gsl_siman_params_t

	agh::beersma::SUltradianCycle
		X = {P.X[0], P.X[1], P.X[2], P.X[3]};

	{
		FUltradianCycle F (X);
		X.cf = 0.;
		for ( size_t p = 0; p < course.size(); ++p ) {
			X.cf += gsl_pow_2( F(p*M.pagesize()/60.) - course[p]);
		}
		X.cf = sqrt(X.cf);
	}


	if ( extra )
		*extra = analyse_deeper( X, M, C);

	return X;
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


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
