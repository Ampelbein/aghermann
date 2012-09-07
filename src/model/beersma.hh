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

#include "../libsigfile/page-metrics-base.hh"
#include "../expdesign/primaries.hh"


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
	sigfile::TMetricType
		metric;
	float	freq_from,
		freq_upto;
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
	static double
		ir = 0.0001, iT =   .1 * M_PI, id =  .1, ib = .1; // the last one must be overridden depending on metric
	static double
		ur = 0.01,   uT = 120. * M_PI, ud = 60., ub = ; // the last one must be overridden depending on metric
};
inline double
distance( const SUltradianCycle& lv,
	  const SUltradianCycle& rv)
{
	return sqrt(
		gsl_pow_2( lv.r - rv.r) +
		gsl_pow_2( lv.T - rv.T) +
		gsl_pow_2( lv.d - rv.d) +
		gsl_pow_2( lv.b - rv.d));
}

struct SUltradianCycleDetails {
	time_t	start, end;
	double	max, avg;
};

struct SUltradianCycleMftCtl {
	sigfile::TMetricType
		metric;
	float	freq_from,
		freq_upto;
	double	sigma;
	size_t	iterations;
};


struct SUltradianCycleSimanCtl {
	sigfile::TMetricType
		metric;
	float	freq_from,
		freq_upto;
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
	FUltradianCycle (const FUltradianCycle&) = delete;

    public:
	FUltradianCycle (double r_, double T_, double d_, double b_)
	      : SUltradianCycle {r_, T_, d_, b_}
		{}

	double operator()( double t) const
		{
			return f(t);
		}

	// function
	double f( double t) const
		{
			auto A = (exp(-r*t) * (cos((t+d)/T) - 1)) + b;
			return (A > 0.) ? A : 0.;
		}

	// partial derivatives
	double dr( double t) const
		{
			return -t * exp(-r*t) * (cos((t+d)/T) - 1);
		}
	double dT( double t) const
		{
			return (t + d) * exp(-r*t) * sin((t+d)/T) / gsl_pow_2(T);
		}
	double dd( double t) const
		{
			return exp(-r*t) * sin((t+d)/T) / T;
		}
	double db( double t) const
		{
			return 1;
		}

};

struct SUltradianCycleWeightedData {
	size_t	n;
	double	*y,
		*sigma;
	int	pagesize;
};


SUltradianCycle
ultradian_cycles_mfit( agh::CRecording&,
		       const SUltradianCycleMftCtl&,
		       list<SUltradianCycleDetails>* extra = nullptr);
SUltradianCycle
ultradian_cycles_siman( agh::CRecording&,
			const SUltradianCycleSimanCtl&,
			list<SUltradianCycleDetails>* extra = nullptr);

list<SUltradianCycleDetails>
analyse_deeper( const SUltradianCycleWeightedData&, const SUltradianCycle&);








int
assisted_score( agh::CSubject::SEpisode&);

} // namespace beersma
} // namespace agh

#endif // _AGH_MODEL_BEERSMA_H

// eof
