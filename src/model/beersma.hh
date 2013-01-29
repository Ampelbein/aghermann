/*
 *       File name:  model/beersma.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-25
 *
 *         Purpose:  Classic (exponential decay) fitting & other non-member functions
 *
 *         License:  GPL
 */

#ifndef _AGH_MODEL_BEERSMA_H
#define _AGH_MODEL_BEERSMA_H


#include <list>
#include <gsl/gsl_math.h>
#include <gsl/gsl_siman.h>
#include "metrics/page-metrics-base.hh"
#include "expdesign/profile.hh"


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {
namespace beersma {


struct SClassicFitWeightedData {
	size_t	n;
	double	*y,
		*sigma;
	int	pagesize;
	double	SWA_0, SWA_L;
};




struct SClassicFit {
	double	r;
};

struct SClassicFitCtl {
	SProfileParamSet
		P;
	double	sigma;
	size_t	iterations;
};

class FClassicFit {
	FClassicFit() = delete;
	FClassicFit( const FClassicFit&) = delete;
    public:
	double	A, r, b;
	FClassicFit (double A_, double r_, double b_)
	      : A (A_), r (r_), b (b_)
		{}
	double operator()( double t) const
		{
			return A * exp(-r*t) + b;
		}
};


SClassicFit
classic_fit( agh::CRecording&,
	     const SClassicFitCtl&);





struct SUltradianCycle {
	double	r, T, d, b;
	SUltradianCycle& operator=( const SUltradianCycle&) = default;
	static constexpr double
		ir = 0.0001,  iT =   1., id =  .1, ib = .1, // the last one is a normalized value of metric
		ur = 0.010,   uT = 130., ud =  60., ub = .01,
		lr = 0.001,   lT =  60., ld = -60., lb = .1;
	double	cf;
};

inline double
distance( const SUltradianCycle& lv,
	  const SUltradianCycle& rv)
{
	return sqrt(
		gsl_pow_2( (lv.r - rv.r)/SUltradianCycle::ir) +
		gsl_pow_2( (lv.T - rv.T)/SUltradianCycle::iT) +
		gsl_pow_2( (lv.d - rv.d)/SUltradianCycle::id) +
		gsl_pow_2( (lv.b - rv.d)/SUltradianCycle::ib));
}

struct SUltradianCycleDetails {
	time_t	start, end;
	double	max, avg;
};


struct SUltradianCycleCtl {
	agh::SProfileParamSet
		profile_params;

	double	sigma;
	gsl_siman_params_t
		siman_params;
		    // int n_tries
		    // 	The number of points to try for each step.
		    // int iters_fixed_T
		    // 	The number of iterations at each temperature.
		    // double step_size
		    // 	The maximum step size in the random walk.
		    // double k, t_initial, mu_t, t_min
};


class FUltradianCycle
  : public SUltradianCycle {
	FUltradianCycle () = delete;

    public:
	FUltradianCycle (const FUltradianCycle&) = default;
	FUltradianCycle (const SUltradianCycle& rv)
	      : SUltradianCycle (rv)
		{}

	double operator()( double t) const
		{
			return f(t);
		}

	// function
	double f( double t) const
		{
			// if ( (unlikely (r > ur || r < lr ||
			// 		T > uT || T < lT ||
			// 		d > ud || d < ld ||
			// 		b > ub || b < lb) ) )
			//      return 1e9;
			/// better let the caller do it just once
			double A = -(exp(-r*t) * (cos((t+d)/T * 2 * M_PI) - 1)) - b;
			return (A > 0.) ? A : 0.;
		}
};

// struct SUltradianCycleWeightedData {
// 	size_t	n;
// 	double	*y,
// 		*sigma;
// 	int	pagesize;
// };


SUltradianCycle
ultradian_cycles( agh::CRecording&,
		  const SUltradianCycleCtl&,
		  list<SUltradianCycleDetails>* extra = nullptr);

list<SUltradianCycleDetails>
analyse_deeper( const SUltradianCycle&,
		agh::CRecording&,
		const SUltradianCycleCtl&);

} // namespace beersma
} // namespace agh

#endif // _AGH_MODEL_BEERSMA_H

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
