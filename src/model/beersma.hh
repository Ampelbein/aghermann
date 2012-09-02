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


struct SClassicFit {
	double	r;
};

struct SClassicFitCtl {
	sigfile::TMetricType
		metric;
	float	freq_from,
		freq_upto;
	size_t	n; // iterations
};


SClassicFit
classic_fit( const agh::CRecording&,
	     const SClassicFitCtl&);





struct SUltradianCycle {
	time_t	start, end;
	double	max, avg;
};

struct SUltradianCycleCtl {
	sigfile::TMetricType
		metric;
	float	freq_from,
		freq_upto;
	enum TOption { Method1, Method2 };
	TOption	option;
};


list<SUltradianCycle>
ultradian_cycles( const agh::CRecording&,
		  const agh::beersma::SUltradianCycleCtl&);





int
assisted_score( agh::CSubject::SEpisode&);

} // namespace beersma
} // namespace agh

#endif // _AGH_MODEL_BEERSMA_H

// eof
