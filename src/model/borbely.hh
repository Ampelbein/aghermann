/*
 *       File name:  model/borbely.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-25
 *
 *         Purpose:  Classic (exponential decay) fitting
 *
 *         License:  GPL
 */

#ifndef _AGH_MODEL_BORBELY_H
#define _AGH_MODEL_BORBELY_H

#include "../libsigfile/page-metrics-base.hh"
#include "../expdesign/forward-decls.hh"


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {
namespace borbely {


struct SClassicFitParamSet {
	double	tau,
		asymp;
};

struct SClassicFitCtlParamSet {
	sigfile::TMetricType
		metric;
	float	freq_from,
		freq_upto;
	size_t	n; // iterations
};


SClassicFitParamSet
classic_fit( const agh::CRecording&,
	     const SClassicFitCtlParamSet&);



} // namespace borbely
} // namespace agh

#endif // _AGH_MODEL_BORBELY_H

// eof
