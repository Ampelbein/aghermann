// ;-*-C++-*-
/*
 *       File name:  model/ultradian-cycle.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-20
 *
 *         Purpose:  Detect NREM-REM cycle
 *
 *         License:  GPL
 */

#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_blas.h>

#include "../libsigfile/psd.hh"
#include "../libsigfile/mc.hh"
#include "../expdesign/recording.hh"
#include "beersma.hh"

using namespace std;



inline namespace {

// see http://www.gnu.org/software/gsl/manual/html_node/Example-programs-for-Nonlinear-Least_002dSquares-Fitting.html



int
fun_f( const gsl_vector* x, void* data,
       gsl_vector* f)
{
	auto& D = *(agh::beersma::SUltradianCycleWeightedData*)data;

	// -(exp(-r*t) * (cos((t+d)/T) + b))

	agh::beersma::FUltradianCycle
		F (gsl_vector_get( x, 0),
		   gsl_vector_get( x, 1),
		   gsl_vector_get( x, 2),
		   gsl_vector_get( x, 3));
	printf( "in %g, %g, %g, %g\n", F.r, F.T, F.d, F.b);
	for ( size_t i = 0; i < D.n; ++i ) {
		double	t = (i * D.pagesize) / 60.;

//	if ( i % 30 == 0 ) printf( "Y[%zu] = %g\n", i, Yi);
		gsl_vector_set( f, i, (F(t) - D.y[i]) / D.sigma[i]);
	}

	return GSL_SUCCESS;
}

int
fun_df( const gsl_vector* x, void* data,
	gsl_matrix* J)
{
	auto& D = *(agh::beersma::SUltradianCycleWeightedData*)data;

	agh::beersma::FUltradianCycle
		F (gsl_vector_get( x, 0),
		   gsl_vector_get( x, 1),
		   gsl_vector_get( x, 2),
		   gsl_vector_get( x, 3));

	for ( size_t i = 0; i < D.n; ++i ) {
		/* Jacobian matrix J(i,j) = dfi / dxj, */
		/* where fi = (Yi - yi)/sigma[i],      */
		/*       Yi = A * exp(-lambda * i) + b  */
		/* and the xj are the parameters (A,lambda,b) */
		double	t = (i * D.pagesize) / 60.,
			s = D.sigma[i];
		gsl_matrix_set( J, i, 0, F.dr(t) / s);
		gsl_matrix_set( J, i, 1, F.dT(t) / s);
		gsl_matrix_set( J, i, 2, F.dd(t) / s);
		gsl_matrix_set( J, i, 3, F.db(t) / s);
	}
	return GSL_SUCCESS;
}

int
fun_fdf( const gsl_vector* x, void *data,
	  gsl_vector* f, gsl_matrix* J)
{
	fun_f( x, data, f);
	fun_df( x, data, J);

	return GSL_SUCCESS;
}


void
print_state( size_t iter, gsl_multifit_fdfsolver* s)
{
	printf ("iter: %3zu r = % 15.8f, T = % 8.2f, d = %8g , b = %8g "
		"|f(x)| = %g\n",
		iter,
		gsl_vector_get (s->x, 0),
		gsl_vector_get (s->x, 1),
		gsl_vector_get (s->x, 2),
		gsl_vector_get (s->x, 3),
		gsl_blas_dnrm2 (s->f));
}



} // inline namespace




agh::beersma::SUltradianCycle
agh::beersma::
ultradian_cycles_mfit( agh::CRecording& M,
		       const agh::beersma::SUltradianCycleMftCtl& P,
		       list<SUltradianCycleDetails> *extra)
{
      // set up
	auto  course = M.cached_course<double>( P.metric, P.freq_from, P.freq_upto);
	const size_t	pp = course.size();
	const size_t	pagesize = M.pagesize();

	valarray<double>
		sigma (P.sigma, pp);

	SUltradianCycleWeightedData wd = {
		pp,
		&course[0],
		&sigma[0],
		(int)pagesize
	};

      // set up (contd)
	gsl_matrix *covar = gsl_matrix_alloc( 4, 4);
	double x_init[4] = { 0.0046, 80. * M_PI, 0., 0. }; // min^-1, physiologically plausible
	gsl_vector_view X = gsl_vector_view_array( x_init, 4);

	gsl_multifit_function_fdf F;
	F.f = &fun_f;
	F.df = &fun_df;
	F.fdf = &fun_fdf;
	F.n = pp;
	F.p = 4;
	F.params = &wd;

	gsl_multifit_fdfsolver
		*S = gsl_multifit_fdfsolver_alloc(
			gsl_multifit_fdfsolver_lmsder,
			pp, 4);
	gsl_multifit_fdfsolver_set( S, &F, &X.vector);

      // find it
	int status;
	unsigned int iter = 0;
	do {
		++iter;
		status = gsl_multifit_fdfsolver_iterate( S);

		print_state( iter, S);
		if ( status ) {
			printf ("status = %s\n", gsl_strerror( status));
			break;
		}

		status = gsl_multifit_test_delta( S->dx, S->x,
						  1e-4, 1e-4);
	} while ( iter < P.iterations );

	gsl_multifit_covar( S->J, 0.0, covar);

#define FIT(i) gsl_vector_get( S->x, i)
#define ERR(i) sqrt(gsl_matrix_get( covar, i, i))
	{
		double chi = gsl_blas_dnrm2(S->f);
		double dof = pp - 4;
		double c = GSL_MAX_DBL(1, chi / sqrt(dof));

		printf("chisq/dof = %g\n",  gsl_pow_2(chi) / dof);

		printf ("r = %.5f +/- %.5f\n", FIT(0), c*ERR(0));
		printf ("T = %.5f +/- %.5f\n", FIT(1), c*ERR(1));
		printf ("d = %.5f +/- %.5f\n", FIT(2), c*ERR(2));
		printf ("b = %.5f +/- %.5f\n\n", FIT(3), c*ERR(3));
	}

	double	r = gsl_vector_get( S->x, 0),
		T = gsl_vector_get( S->x, 1),
		d = gsl_vector_get( S->x, 2),
		b = gsl_vector_get( S->x, 3);

	gsl_multifit_fdfsolver_free( S);
	gsl_matrix_free( covar);

	if ( extra )
		*extra = analyse_deeper( wd, {r, T, d, b});

	return {r, T/M_PI, d, b};
}






// siman method


inline namespace {

struct SUltradianCycleSimanPPack {
	agh::beersma::FUltradianCycle F;
	agh::CRecording* M;
	const agh::beersma::SUltradianCycleSimanCtl* P;

	SUltradianCycleSimanPPack () = delete;
	SUltradianCycleSimanPPack (const SUltradianCycleSimanPPack&) = delete;
};



double
uc_cost_function( const void *xp)
{
	auto& X = *(SUltradianCycleSimanPPack*)(xp);

      // set up
	auto  course = X.M->cached_course<double>( X.P->metric, X.P->freq_from, X.P->freq_upto);
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
	auto& X = *(SUltradianCycleSimanPPack*)(xp);

retry:
	double *pip, *ipip;
	switch ( gsl_rng_uniform_int( r, 4) ) {
	case 0:  pip = &X.F->r; ipip = &X.F->ir; break;
	case 1:  pip = &X.F->T; ipip = &X.F->iT; break;
	case 2:  pip = &X.F->d; ipip = &X.F->id; break;
	case 3:
	default: pip = &X.F->b; ipip = &X.F->ib; break;
	}

	bool go_positive = (bool)gsl_rng_uniform_int( r, 2);

	double	nudge = *ipip,
		d;
	size_t nudges = 0;
	do {
		// nudge it a little,
		// prevent from going out-of-bounds
		if ( go_positive )
			if ( *pip + *ipip < tt.hi[t] )
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

}



double
uc_siman_metric( void *xp, void *yp)
{
	return agh::beersma::distance(
		((SUltradianCycleSimanPPack*)xp) -> F,
		((SUltradianCycleSimanPPack*)yp) -> F);
}

void
uc_siman_print( void *xp)
{
	auto& X = (SUltradianCycleSimanPPack*)(xp);
	auto& F = *X.F;
	printf( "F r = %g, T = %g, d = %g, b = %g\n",
		F.r, F.T, F.d, F.b);
}

} // inline namespace




agh::beersma::SUltradianCycle
agh::beersma::
ultradian_cycles_siman( agh::CRecording& M,
			const agh::beersma::SUltradianCycleSimanCtl& P,
			list<SUltradianCycleDetails> *extra)
{
	SUltradianCycleSimanPPack
		X {{ 0.0046, 80. * M_PI, 0., 0. },
			&M, &P};
	gsl_siman_solve( __agh_rng ? __agh_rng : (init_global_rng(), __agh_rng),
			 (void*)&X,		//
			 uc_cost_function,	// gsl_siman_Efunc_t,
			 uc_siman_step,		// gsl_siman_step_t
			 uc_siman_metric,	// gsl_siman_metric_t,
			 printer,		// gsl_siman_print_t print_position,
//			 siman::_siman_print,
			 NULL, NULL, NULL,	// gsl_siman_copy_t copyfunc, gsl_siman_copy_construct_t copy_constructor, gsl_siman_destroy_t destructor,
			 sizeof(SUltradianCycleSimanPPack),	// size_t element_size,
			 P.siman_params);		// gsl_siman_params_t params

	if ( extra )
		*extra = analyse_deeper( wd, {r, T, d, b});

	return {r, T/M_PI, d, b};
}


// lil helper

list<agh::beersma::SUltradianCycleDetails>
agh::beersma::
analyse_deeper( const SUltradianCycleWeightedData& W, const SUltradianCycle& P)
{
	list<agh::beersma::SUltradianCycleDetails>
		ret;

	

	return ret;
}


// eof
