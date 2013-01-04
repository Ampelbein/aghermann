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

#include "metrics/phasic-events.hh"
#include "sf.hh"

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
