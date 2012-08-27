// ;-*-C++-*-
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

#include "borbely.hh"
#include "../libsigfile/psd.hh"
#include "../libsigfile/mc.hh"
#include "../expdesign/recording.hh"

using namespace std;


inline namespace {

// see http://www.gnu.org/software/gsl/manual/html_node/Example-programs-for-Nonlinear-Least_002dSquares-Fitting.html
struct SWeightedData {
	size_t	n;
	double	*y,
		*sigma;
};


int
expb_f( const gsl_vector* x, void* data,
	gsl_vector* f)
{
	auto& D = *(SWeightedData*)data;

	double	lambda	= gsl_vector_get( x, 0),
		b	= gsl_vector_get( x, 1);
	printf( "{lambda, b} = %g, %g, D.n = %zu\n", lambda, b, D.n);
	for ( size_t i = 0; i < D.n; ++i ) {
		/* Model Yi = A * exp(-lambda * i) + b */
		double t = i;
		double Yi = 1 * exp( -lambda * t) + b;
		//printf( "Y[%zu] = %g / %g\n", i, Yi, D.sigma[i]);
		gsl_vector_set( f, i, (Yi - D.y[i]) / D.sigma[i]);
	}

	return GSL_SUCCESS;
}

int
expb_df( const gsl_vector* x, void* data,
	 gsl_matrix* J)
{
	auto& D = *(SWeightedData*)data;

	double	lambda	= gsl_vector_get( x, 0);

	for ( size_t i = 0; i < D.n; ++i ) {
		/* Jacobian matrix J(i,j) = dfi / dxj, */
		/* where fi = (Yi - yi)/sigma[i],      */
		/*       Yi = A * exp(-lambda * i) + b  */
		/* and the xj are the parameters (A,lambda,b) */
		double	t = (double)i,
			s = D.sigma[i],
			e = exp(-lambda * t);
		gsl_matrix_set( J, i, 0, e/s);
		gsl_matrix_set( J, i, 1, -t * 1 * e/s);
		gsl_matrix_set( J, i, 2, 1/s);
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
	printf ("iter: %3zu x = % 15.8f % 15.8f "
		"|f(x)| = %g\n",
		iter,
		gsl_vector_get (s->x, 0),
		gsl_vector_get (s->x, 1),
		gsl_blas_dnrm2 (s->f));
}

} // inline namespace

agh::borbely::SClassicFitParamSet
agh::borbely::
classic_fit( const agh::CRecording& M,
	     const agh::borbely::SClassicFitCtlParamSet& P)
{
      // set up
	size_t	pp;
	size_t	pagesize,
		samplerate;

	valarray<double> course;
	switch ( P.metric ) {
	case sigfile::TMetricType::Psd:
		pp = M.CBinnedPower::pages();
		pagesize = ((sigfile::CBinnedPower)M).CPageMetrics_base::pagesize();
		samplerate = ((sigfile::CBinnedPower)M).CPageMetrics_base::samplerate();
		course = M.CBinnedPower::course<double>( P.freq_from, P.freq_upto);
	    break;
	case sigfile::TMetricType::Mc:
		pp = M.CBinnedMC::pages();
		pagesize = ((sigfile::CBinnedMC)M).CPageMetrics_base::pagesize();
		samplerate = ((sigfile::CBinnedMC)M).CPageMetrics_base::samplerate();
		course = M.CBinnedMC::course<double>(
			min( (size_t)((P.freq_from - M.freq_from) / M.bandwidth),
			     M.CBinnedMC::bins()-1));
	    break;
	default:
		throw runtime_error ("classic_fit(): Invalid profile type");
	}

	valarray<double>
		sigma (1., pp);
	{
	// use artifacts for weights
		auto	af = M.CBinnedPower::artifacts_in_samples(); // for common methods reaching to _using_F, one would fancy some disambiguating shortcut, no?
		for ( size_t i = 0; i < pp; ++i )
			sigma[i] = 1. - (agh::SSpan<size_t> (i, i+1) * pagesize * samplerate)
				. dirty( af);
	}
	printf( "course.size = %zu\n", course.size());

	SWeightedData wd {
		pp,
		&course[0],
		&sigma[0]
	};

      // set up (contd)
	gsl_matrix *covar = gsl_matrix_alloc( 2, 2);
	double x_init[2] = { 0.0, 0.0 };
	gsl_vector_view X = gsl_vector_view_array( x_init, 2);

	gsl_multifit_function_fdf F;
	F.f = &expb_f;
	F.df = &expb_df;
	F.fdf = &expb_fdf;
	F.n = pp;
	F.p = 3;
	F.params = &wd;

	gsl_multifit_fdfsolver
		*S = gsl_multifit_fdfsolver_alloc(
			gsl_multifit_fdfsolver_lmsder,
			pp, 2);
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
	} while ( status == GSL_CONTINUE && iter < P.n );

	gsl_multifit_covar( S->J, 0.0, covar);

#define FIT(i) gsl_vector_get( S->x, i)
#define ERR(i) sqrt(gsl_matrix_get( covar, i, i))
	{
		double chi = gsl_blas_dnrm2(S->f);
		double dof = pp - 2;
		double c = GSL_MAX_DBL(1, chi / sqrt(dof));

		printf("chisq/dof = %g\n",  pow(chi, 2.0) / dof);

		printf ("lambda = %.5f +/- %.5f\n", FIT(1), c*ERR(0));
		printf ("b      = %.5f +/- %.5f\n", FIT(2), c*ERR(1));
	}

	double	rate = 1./gsl_vector_get( S->x, 0),
		asymp = gsl_vector_get( S->x, 1);
	printf( "this episode rate = %g, asymp = %g\n\n", rate, asymp);

	gsl_multifit_fdfsolver_free( S);
	gsl_matrix_free( covar);

	return {rate, asymp};

}


// eof
