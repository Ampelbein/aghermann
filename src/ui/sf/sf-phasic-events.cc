// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-phasic-events.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-04
 *
 *         Purpose:  scoring facility: phasic events
 *
 *         License:  GPL
 */

#include "sf.hh"
// I'd rather have these two includes in the reverse order, except
// that g++ takes an issue with namespace sigproc, which appears in
// sigproc/sigproc.hh and, independently, in sigproc/winfun.hh.
//
// This is what I get if winfun.hh comes first:
//
// In file included from sf-phasic-events.cc:14:
// sf.hh:121: error: 'SCachedLowPassCourse' in namespace 'sigproc' does not name a type
// sf.hh:123: error: 'SCachedBandPassCourse' in namespace 'sigproc' does not name a type
// sf.hh:125: error: 'SCachedEnvelope' in namespace 'sigproc' does not name a type
// sf.hh:127: error: 'SCachedDzcdf' in namespace 'sigproc' does not name a type
//
// No idea why they, being included in this order: winfun.hh, sigproc.hh,
// cause g++ do forget all declarations from the latter.
#include "metrics/phasic-events.hh"

using namespace std;

void
aghui::SScoringFacility::SChannel::
get_phasic_events()
{
	using namespace metrics::phasic;
	auto H = sigfile::SNamedChannel<int> (crecording.F(), _h);
	phasic_events[TEventTypes::spindle] =
		detect_spindles( H);
	phasic_events[TEventTypes::K_complex] =
		detect_Kcomplexes( H);
}

// eof
