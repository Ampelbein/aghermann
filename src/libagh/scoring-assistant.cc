// ;-*-C++-*- *  Time-stamp: "2010-12-16 01:54:40 hmmr"
/*
 * Author: Andrei Zavada (johnhommer@gmail.com)
 *
 * License: GPL
 *
 * Initial version: 2010-11-09
 */


#include "misc.hh"
#include "page.hh"
#include "edf.hh"
#include "primaries.hh"

using namespace std;





int
CSubject::SEpisode::assisted_score()
{
	list<CRecording*> HH;
	for ( auto I = recordings.begin(); I != recordings.end(); ++I )
		if ( I->second.have_power() )
			HH.push_back( &I->second);
//	printf( "assisted_score(): %d usable channels\n", HH.size());

	list<valarray<double>>
		courses_delta,
		courses_theta;
	for ( auto H = HH.begin(); H != HH.end(); ++H ) {
		courses_delta.emplace_back( (*H)->power_course( 3., 4.));
		courses_theta.emplace_back( (*H)->power_course( 5., 8.));
	}

	for ( size_t p = 0; p < sources.begin()->CHypnogram::length(); ++p ) {
		SPage &P = sources.begin()->nth_page(p);

		// list<valarray<double>> spectra;
		// for ( auto H = HH.begin(); H != HH.end(); ++H )
		// 	spectra.emplace_back( (*H)->power_spectrum(p));

		auto	D = courses_delta.begin(),
			T = courses_theta.begin();
		for ( ; D != courses_delta.end(); ++D, ++T )
			if ( (*D)[p] > (*T)[p] * 1.5 )
				P.mark( AGH_SCORE_NREM3);
	}
	return 0;
}

// EOF
