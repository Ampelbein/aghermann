// ;-*-C++-*- *  Time-stamp: "2011-04-05 02:55:47 hmmr"
/*
 *       File name:  structures.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-11-20
 *
 *         Purpose:  core structures
 *
 *         License:  GPL
 */

#include "structures.hh"

using namespace std;

CExpDesign *AghCC;


list<string>
	AghDD,
	AghGG,
	AghEE;

list<SChannel>
	AghHH,
	AghTT;


void
remove_untried_modruns()
{
	for ( auto Gi = AghCC->groups_begin(); Gi != AghCC->groups_end(); ++Gi )
		for ( auto Ji = Gi->second.begin(); Ji != Gi->second.end(); ++Ji )
			for ( auto Di = Ji->measurements.begin(); Di != Ji->measurements.end(); ++Di )
			retry_modruns:
				for ( auto RSi = Di->second.modrun_sets.begin(); RSi != Di->second.modrun_sets.end(); ++RSi ) {
				retry_this_modrun_set:
					for ( auto Ri = RSi->second.begin(); Ri != RSi->second.end(); ++Ri )
						if ( !(Ri->second.status & AGH_MODRUN_TRIED) ) {
							RSi->second.erase( Ri);
							goto retry_this_modrun_set;
						}
					if ( RSi->second.empty() ) {
						Di->second.modrun_sets.erase( RSi);
						goto retry_modruns;
					}
				}
}


// EOF
