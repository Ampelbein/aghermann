/*
 *       File name:  aghermann/rk1968/rk1968.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-11-09
 *
 *         Purpose:  scoring assistant
 *
 *         License:  GPL
 */


#include <forward_list>

#include "libsigfile/page.hh"
#include "libsigfile/typed-source.hh"
#include "aghermann/expdesign/recording.hh"
#include "aghermann/expdesign/subject.hh"
#include "aghermann/expdesign/expdesign.hh"
#include "libmetrics/bands.hh"

#include "rk1968.hh"


using namespace std;
using namespace agh::rk1968;

int
CScoreAssistant::
score( agh::SEpisode& E)
{
	forward_list<agh::CRecording*> HH;
	for ( auto &R : E.recordings )
		if ( R.second.psd_profile.have_data() )
			HH.push_front( &R.second);

	forward_list<valarray<TFloat>>
		courses_delta,
		courses_theta;
	for ( auto &H : HH ) {
		courses_delta.emplace_front(
			H->psd_profile.course(
				agh::CExpDesign::freq_bands[metrics::TBand::delta][0],
				agh::CExpDesign::freq_bands[metrics::TBand::delta][1]));
		courses_theta.emplace_front(
			H->psd_profile.course(
				agh::CExpDesign::freq_bands[metrics::TBand::theta][0],
				agh::CExpDesign::freq_bands[metrics::TBand::theta][1]));
	}

	auto& firstsource = E.sources.front();
	for ( size_t p = 0; p < firstsource.pages(); ++p ) {
		auto	Di = courses_delta.begin(),
			Ti = courses_theta.begin();
		int decision = 0;
		for ( ; Di != courses_delta.end(); ++Di, ++Ti )
			decision +=
				( (*Di)[p] > (*Ti)[p] * nrem3_delta_theta_ratio );
		if ( decision > 0 )
			firstsource[p].mark( sigfile::SPage::TScore::nrem3);
	}
	return 0;
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
