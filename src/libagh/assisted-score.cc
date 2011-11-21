// ;-*-C++-*-
/*
 *       File name:  libagh/assisted-score.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-11-09
 *
 *         Purpose:  scoring assistant
 *
 *         License:  GPL
 */


#include "../libsigfile/page.hh"
#include "../libsigfile/source.hh"
#include "primaries.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;

int
agh::CSubject::SEpisode::assisted_score()
{
	list<CRecording*> HH;
	for ( auto &R : recordings )
		if ( R.second.have_power() )
			HH.push_back( &R.second);
//	printf( "assisted_score(): %d usable channels\n", HH.size());

	list<valarray<TFloat>>
		courses_delta,
		courses_theta;
	for ( auto &H : HH ) {
		courses_delta.emplace_back( H->power_course<TFloat>( 3., 4.));
		courses_theta.emplace_back( H->power_course<TFloat>( 5., 8.));
	}

	auto& firstsource = sources.front();
	for ( size_t p = 0; p < firstsource.CHypnogram::length(); ++p ) {
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


// EOF
