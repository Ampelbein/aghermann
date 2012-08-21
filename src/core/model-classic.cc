// ;-*-C++-*-
/*
 *       File name:  core/model-classic.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-01
 *
 *         Purpose:  Classic (exponential decay) fitting for CSCourse
 *
 *         License:  GPL
 */


#include <gsl/gsl_multifit_nlin.h>
#include "model.hh"


using namespace std;


inline namespace {

// see http://www.gnu.org/software/gsl/manual/html_node/Example-programs-for-Nonlinear-Least_002dSquares-Fitting.html
struct SWeightedData {
	size_t n;
	double *y;
	double *sigma;
};

int
expb_f( const gsl_vector* x, void* data,
	gsl_vector* f)
{
	auto& D = *(SWeightedData*)data;

	double	A	= gsl_vector_get( x, 0),
		lambda	= gsl_vector_get( x, 1),
		b	= gsl_vector_get( x, 2);

	for ( size_t i = 0; i < D.n; ++i ) {
		/* Model Yi = A * exp(-lambda * i) + b */
		double t = i;
		double Yi = A * exp( -lambda * t) + b;
		gsl_vector_set( f, i, (Yi - D.y[i]) / D.sigma[i]);
	}

	return GSL_SUCCESS;
}

int
expb_df( const gsl_vector* x, void* data,
	 gsl_matrix* J)
{
	auto& D = *(SWeightedData*)data;

	double	A	= gsl_vector_get( x, 0),
		lambda	= gsl_vector_get( x, 1);

	for ( size_t i = 0; i < D.n; ++i ) {
		/* Jacobian matrix J(i,j) = dfi / dxj, */
		/* where fi = (Yi - yi)/sigma[i],      */
		/*       Yi = A * exp(-lambda * i) + b  */
		/* and the xj are the parameters (A,lambda,b) */
		double	t = (double)i,
			s = D.sigma[i],
			e = exp(-lambda * t);
		gsl_matrix_set( J, i, 0, e/s);
		gsl_matrix_set( J, i, 1, -t * A * e/s);
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


} // inline namespace

agh::CSCourse::SClassicFitParamSet
agh::CSCourse::
classic_fit( const SClassicFitCtlParamSet& P) const
{
	size_t nm = _mm_list.size();
	valarray<double>
		taus (nm),
		asymps (nm);

	for ( size_t e = 0; e < nm; ++e ) {
	      // set up
		size_t	ea = nth_episode_start_page(e),
			ez = nth_episode_end_page(e);
		valarray<double>
			vy (ez - ea),
			vsigma (ez - ea, 1.);
		for ( size_t i = 0; i < (ez-ea); ++i )
			vy[i] = _timeline[ea+i].metric;
		SWeightedData wd {
			ez - ea,
			&vy[0],
			&vsigma[0]
		};

	      // set up (contd)
		const gsl_multifit_fdfsolver_type *T;
		gsl_multifit_fdfsolver *s;
		int status;
		unsigned int iter = 0;
		const size_t p = 3;

		gsl_matrix *covar = gsl_matrix_alloc (p, p);
		gsl_multifit_function_fdf f;
		double x_init[3] = { 1.0, 0.0, 0.0 };
		gsl_vector_view x = gsl_vector_view_array (x_init, p);

		f.f = &expb_f;
		f.df = &expb_df;
		f.fdf = &expb_fdf;
		f.n = P.n;
		f.p = p;
		f.params = &wd;

		T = gsl_multifit_fdfsolver_lmsder;
		s = gsl_multifit_fdfsolver_alloc (T, P.n, p);
		gsl_multifit_fdfsolver_set (s, &f, &x.vector);

	      // find it
		do {
			++iter;
			status = gsl_multifit_fdfsolver_iterate (s);

			printf ("status = %s\n", gsl_strerror (status));

//			print_state (iter, s);

			if (status)
				break;

			status = gsl_multifit_test_delta (s->dx, s->x,
							  1e-4, 1e-4);
		} while ( status == GSL_CONTINUE && iter < 500 );

	}
	return {taus.sum()/nm, asymps.sum()/nm};
}




// eof
