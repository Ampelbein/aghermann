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
classic_fit( const agh::CRecording&,
	     const SClassicFitCtl&);





struct SUltradianCycle {
	double	r, T, b;
};

struct SUltradianCycleDetails {
	time_t	start, end;
	double	max, avg;
};

struct SUltradianCycleCtl {
	sigfile::TMetricType
		metric;
	float	freq_from,
		freq_upto;
	double	sigma;
	size_t	iterations;
};


class FUltradianCycle {
	FUltradianCycle() = delete;
	FUltradianCycle( const FUltradianCycle&) = delete;
    public:
	double	r, T, b;
	FUltradianCycle (double r_, double T_, double b_)
	      : r (r_), T (T_), b (b_)
		{}
	double operator()( double t) const
		{
			auto f = (-exp(-r*t) * (cos(T*t) - 1)) - b;
			return (f > 0.) ? f : 0.;
		}
};

struct SUltradianCycleWeightedData {
	size_t	n;
	double	*y,
		*sigma;
	int	pagesize;
};


SUltradianCycle
ultradian_cycles( const agh::CRecording&,
		  const SUltradianCycleCtl&,
		  list<SUltradianCycleDetails>* extra = nullptr);

list<SUltradianCycleDetails>
analyse_deeper( const SUltradianCycleWeightedData&, const SUltradianCycle&);








int
assisted_score( agh::CSubject::SEpisode&);

} // namespace beersma
} // namespace agh

#endif // _AGH_MODEL_BEERSMA_H

// eof
