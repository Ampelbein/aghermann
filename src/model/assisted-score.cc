// ;-*-C++-*-
/*
 *       File name:  model/assisted-score.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-11-09
 *
 *         Purpose:  scoring assistant
 *
 *         License:  GPL
 */


#include <forward_list>
#include <valarray>

#include "libsigfile/page.hh"
#include "libsigfile/source.hh"
#include "expdesign/primaries.hh"
#include "beersma.hh"


using namespace std;

int
agh::beersma::
assisted_score( agh::CSubject::SEpisode& E)
{
	forward_list<agh::CRecording*> HH;
	for ( auto &R : E.recordings )
		if ( R.second.metrics::psd::CProfile::have_data() )
			HH.push_front( &R.second);

	forward_list<valarray<TFloat>>
		courses_delta,
		courses_theta;
	for ( auto &H : HH ) {
		courses_delta.emplace_front( H->metrics::psd::CProfile::course<TFloat>( 2., 3.));
		courses_theta.emplace_front( H->metrics::psd::CProfile::course<TFloat>( 5., 8.));
	}

	auto& firstsource = E.sources.front();
	for ( size_t p = 0; p < firstsource.pages(); ++p ) {
		// list<valarray<double>> spectra;
		// for ( auto H = HH.begin(); H != HH.end(); ++H )
		// 	spectra.emplace_back( (*H)->power_spectrum(p));

		auto	D = courses_delta.begin(),
			T = courses_theta.begin();
		for ( ; D != courses_delta.end(); ++D, ++T )
			if ( (*D)[p] > (*T)[p] * 1.5 )
				firstsource[p].mark( sigfile::SPage::TScore::nrem3);
	}
	return 0;
}


// eof
