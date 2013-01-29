/*
 *       File name:  model/borbely.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-20
 *
 *         Purpose:  Classic (exponential decay) fitting
 *
 *         License:  GPL
 */


#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_blas.h>

#include "common/alg.hh"
#include "metrics/psd.hh"
#include "metrics/mc.hh"
#include "expdesign/recording.hh"
#include "beersma.hh"

using namespace std;


inline namespace {

// see http://www.gnu.org/software/gsl/manual/html_node/Example-programs-for-Nonlinear-Least_002dSquares-Fitting.html


int
expb_f( const gsl_vector* x, void* data,
	gsl_vector* f)
{
	auto& D = *(agh::beersma::SClassicFitWeightedData*)data;

	agh::beersma::FClassicFit
		F (D.SWA_0,
		   gsl_vector_get( x, 0),
		   D.SWA_L);
	for ( size_t i = 0; i < D.n; ++i ) {
		/* Model Yi = A * exp(-lambda * i) + b */
		double t = (i * D.pagesize) / 60.;

//	if ( i % 30 == 0 ) printf( "Y[%zu] = %g\n", i, Yi);
		gsl_vector_set( f, i, (F(t) - D.y[i]) / D.sigma[i]);
	}

	return GSL_SUCCESS;
}

int
expb_df( const gsl_vector* x, void* data,
	 gsl_matrix* J)
{
	auto& D = *(agh::beersma::SClassicFitWeightedData*)data;

	double	r = gsl_vector_get( x, 0);

	for ( size_t i = 0; i < D.n; ++i ) {
		/* Jacobian matrix J(i,j) = dfi / dxj, */
		/* where fi = (Yi - yi)/sigma[i],      */
		/*       Yi = A * exp(-lambda * i) + b  */
		/* and the xj are the parameters (A,lambda,b) */
		// except that we have A = 1 here...
		double	t = (i * D.pagesize) / 60.,
			s = D.sigma[i],
			e = exp(-r * t);
//		gsl_matrix_set( J, i, 0, e/s);
		gsl_matrix_set( J, i, 0, -t * D.SWA_0 * e/s);
//		gsl_matrix_set( J, i, 1, 1/s);
	}
	return GSL_SUCCESS;
}

int
expb_fdf( const gsl_vector* x, void *data,
	  gsl_vector* f, gsl_matrix* J)
{
	expb_f( x, data, f);
	expb_df( x, data, J);

	return GSL_SUCCESS;
}


void
print_state( size_t iter, gsl_multifit_fdfsolver* s)
{
	printf ("iter: %3zu x = % 15.8f "
		"|f(x)| = %g\n",
		iter,
		gsl_vector_get (s->x, 0),
//		gsl_vector_get (s->x, 1),
		gsl_blas_dnrm2 (s->f));
}

} // inline namespace

agh::beersma::SClassicFit
agh::beersma::
classic_fit( agh::CRecording& M,
	     const agh::beersma::SClassicFitCtl& P)
{
      // set up
	auto	course = agh::alg::to_vad( M.course( P.P));
	auto	pp = course.size(),
		pagesize = M.pagesize();


      // determine A (SWA_0) and b (SWA_L)
	double	SWA_0, SWA_L;
	{
		// this one doesn't throw
		agh::CProfile tmp (M, P.P);
		SWA_0 = tmp.SWA_0();
		SWA_L = tmp.SWA_L();
	}

	valarray<double>
		sigma (P.sigma, pp);
	// {
	// // use artifacts for weights
	// 	auto	af = M.CBinnedPower::artifacts_in_samples(); // for common methods reaching to _using_F, one would fancy some disambiguating shortcut, no?
	// 	for ( size_t i = 0; i < pp; ++i )
	// 		sigma[i] = 1. - (agh::SSpan<size_t> (i, i+1) * pagesize * samplerate)
	// 			. dirty( af);
	// }

	SClassicFitWeightedData wd = {
		pp,
		&course[0],
		&sigma[0],
		(int)pagesize,
		// and also:
		SWA_0, SWA_L,
	};

      // set up (contd)
	gsl_matrix *covar = gsl_matrix_alloc( 1, 1);
	double x_init[1] = { 0.0046 }; // min^-1, physiologically plausible
	gsl_vector_view X = gsl_vector_view_array( x_init, 1);

	gsl_multifit_function_fdf F;
	F.f = &expb_f;
	F.df = &expb_df;
	F.fdf = &expb_fdf;
	F.n = pp;
	F.p = 1;
	F.params = &wd;

	gsl_multifit_fdfsolver
		*S = gsl_multifit_fdfsolver_alloc(
			gsl_multifit_fdfsolver_lmsder,
			pp, 1);
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
		double dof = pp - 1;
		double c = GSL_MAX_DBL(1, chi / sqrt(dof));

		printf("chisq/dof = %g\n",  gsl_pow_2(chi) / dof);

		printf ("r = %.5f +/- %.5f\n\n", FIT(0), c*ERR(0));
	}

	double	rate = gsl_vector_get( S->x, 0);

	gsl_multifit_fdfsolver_free( S);
	gsl_matrix_free( covar);

	return {rate};

}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
