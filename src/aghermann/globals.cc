/*
 *       File name:  aghermann/globals.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-06-09
 *
 *         Purpose:  global variables, all two of them
 *
 *         License:  GPL
 */


#include <sys/time.h>
#include <gsl/gsl_rng.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "globals.hh"


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

gsl_rng *agh::global::rng = nullptr;

int agh::global::num_procs = 1;


void
agh::global::
init()
{
      // 1. init rng
	{
		const gsl_rng_type *T;
		gsl_rng_env_setup();
		T = gsl_rng_default;
		if ( gsl_rng_default_seed == 0 ) {
			struct timeval tp = { 0L, 0L };
			gettimeofday( &tp, NULL);
			gsl_rng_default_seed = tp.tv_usec;
		}
		rng = gsl_rng_alloc( T);
	}

      // 2. omp
	{
#ifdef _OPENMP
		agh::global::num_procs = omp_get_max_threads();
		// if ( agh::global::num_procs > 1 )
		// 	printf( "This host is SMP-capable (omp_get_max_threads() returns %d)\n", agh::global::num_procs);
		// else
		// 	printf( "This host is not SMP-capable\n");
#endif

	}
}


void
agh::global::
fini()
{
	gsl_rng_free( rng);
	rng = nullptr;
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
