/*
 *       File name:  metrics/phasic-events.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2013-01-03
 *
 *         Purpose:  CPhasicEventEstimator and related stuff
 *
 *         License:  GPL
 */

#ifndef _METRICS_PHASIC_EVENTS_H
#define _METRICS_PHASIC_EVENTS_H

#include <list>

#include "common/alg.hh"
#include "libsigfile/source.hh"
#include "forward-decls.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {
namespace phasic {

enum TEventTypes {
	spindle,
	K_complex
};

template <typename T>
list<agh::alg::SSpan<double>>
detect_spindles( const sigfile::SNamedChannel<T>&);

template <typename T>
list<agh::alg::SSpan<double>>
detect_Kcomplexes( const sigfile::SNamedChannel<T>&);


#include "phasic-events.ii"

} // namespace phasic
} // namespace metrics


#endif // _METRICS_PHASIC_EVENTS_H

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
