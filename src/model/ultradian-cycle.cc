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

	// -(exp(-r*x) * (cos(T*x) - d))

	agh::beersma::FUltradianCycle
		F (gsl_vector_get( x, 0),
		   gsl_vector_get( x, 1),
		   gsl_vector_get( x, 2));
	printf( "in %g, %g, %g\n", F.r, F.T, F.b);
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

	double	r = gsl_vector_get( x, 0),
		T = gsl_vector_get( x, 1);

	for ( size_t i = 0; i < D.n; ++i ) {
		/* Jacobian matrix J(i,j) = dfi / dxj, */
		/* where fi = (Yi - yi)/sigma[i],      */
		/*       Yi = A * exp(-lambda * i) + b  */
		/* and the xj are the parameters (A,lambda,b) */
		double	t = (i * D.pagesize) / 60.,
			s = D.sigma[i],
			g = exp(-r*t);
		gsl_matrix_set( J, i, 0, (t * g * (cos(t*T)-1)) / s);
		gsl_matrix_set( J, i, 1, (t * g *  sin(t*T)   ) / s);
		gsl_matrix_set( J, i, 2, (                   1) / s);
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
	printf ("iter: %3zu r = % 15.8f, T = % 8.2f, b = %8g "
		"|f(x)| = %g\n",
		iter,
		gsl_vector_get (s->x, 0),
		gsl_vector_get (s->x, 1),
		gsl_vector_get (s->x, 2),
		gsl_blas_dnrm2 (s->f));
}



} // inline namespace




agh::beersma::SUltradianCycle
agh::beersma::
ultradian_cycles( const agh::CRecording& M,
		  const agh::beersma::SUltradianCycleCtl& P,
		  list<SUltradianCycleDetails> *extra)
{
      // set up
	size_t	pp;
	size_t	pagesize;

	valarray<double> course;
	switch ( P.metric ) {
	case sigfile::TMetricType::Psd:
		pp = M.CBinnedPower::pages();
		pagesize = ((sigfile::CBinnedPower)M).CPageMetrics_base::pagesize();
		course = M.CBinnedPower::course<double>( P.freq_from, P.freq_upto);
	    break;
	case sigfile::TMetricType::Mc:
		pp = M.CBinnedMC::pages();
		pagesize = ((sigfile::CBinnedMC)M).CPageMetrics_base::pagesize();
		course = M.CBinnedMC::course<double>(
			min( (size_t)((P.freq_from - M.freq_from) / M.bandwidth),
			     M.CBinnedMC::bins()-1));
	    break;
	default:
		throw runtime_error ("ultradian_cycles(): Invalid profile type");
	}

	valarray<double>
		sigma (P.sigma, pp);

	SUltradianCycleWeightedData wd = {
		pp,
		&course[0],
		&sigma[0],
		pagesize
	};

      // set up (contd)
	gsl_matrix *covar = gsl_matrix_alloc( 3, 3);
	double x_init[3] = { 0.0046, 80. * M_PI, 0 }; // min^-1, physiologically plausible
	gsl_vector_view X = gsl_vector_view_array( x_init, 3);

	gsl_multifit_function_fdf F;
	F.f = &fun_f;
	F.df = &fun_df;
	F.fdf = &fun_fdf;
	F.n = pp;
	F.p = 3;
	F.params = &wd;

	gsl_multifit_fdfsolver
		*S = gsl_multifit_fdfsolver_alloc(
			gsl_multifit_fdfsolver_lmsder,
			pp, 3);
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
		double dof = pp - 3;
		double c = GSL_MAX_DBL(1, chi / sqrt(dof));

		printf("chisq/dof = %g\n",  gsl_pow_2(chi) / dof);

		printf ("r = %.5f +/- %.5f\n", FIT(0), c*ERR(0));
		printf ("T = %.5f +/- %.5f\n", FIT(1), c*ERR(1));
		printf ("b = %.5f +/- %.5f\n\n", FIT(2), c*ERR(2));
	}

	double	r = gsl_vector_get( S->x, 0),
		T = gsl_vector_get( S->x, 1),
		b = gsl_vector_get( S->x, 2);

	gsl_multifit_fdfsolver_free( S);
	gsl_matrix_free( covar);

	if ( extra )
		*extra = analyse_deeper( wd, {r, T, b});

	return {r, T, b};
}




list<agh::beersma::SUltradianCycleDetails>
agh::beersma::
analyse_deeper( const SUltradianCycleWeightedData& W, const SUltradianCycle& P)
{
	list<agh::beersma::SUltradianCycleDetails>
		ret;

	

	return ret;
}


// eof
