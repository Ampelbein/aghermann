// ;-*-C++-*- *  Time-stamp: "2011-04-24 14:01:24 hmmr"
/*
 *       File name:  structures.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-11-20
 *
 *         Purpose:  core structures
 *
 *         License:  GPL
 */

#ifndef _AGH_STRUCTURES_H
#define _AGH_STRUCTURES_H

#include <list>
#include <string>
#include "libagh/edf.hh"
#include "libagh/primaries.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif

using namespace std;
using namespace agh;


extern list<string>
	AghDD,
	AghGG,
	AghEE;

extern list<SChannel>
	AghHH,
	AghTT;


extern list<SChannel>::iterator
	_AghHi,
	_AghTi;
extern list<string>::iterator
	_AghGi,
	_AghDi,
	_AghEi;


inline const char*
AghH() { return (_AghHi != AghHH.end()) ? _AghHi->c_str() : NULL; }
inline const char*
AghT() { return (_AghTi != AghTT.end()) ? _AghTi->c_str() : NULL; }
inline const char*
AghG() { return (_AghGi != AghGG.end()) ? _AghGi->c_str() : NULL; }
inline const char*
AghD() { return (_AghDi != AghDD.end()) ? _AghDi->c_str() : NULL; }
inline const char*
AghE() { return (_AghEi != AghEE.end()) ? _AghEi->c_str() : NULL; }

inline int
AghTi()
{
	int i = 0;
	for ( auto Ti = AghTT.begin(); Ti != AghTT.end(); ++Ti, ++i )
		if ( Ti == _AghTi )
			return i;
	return -1;
}
inline int
AghDi()
{
	int i = 0;
	for ( auto Di = AghDD.begin(); Di != AghDD.end(); ++Di, ++i )
		if ( Di == _AghDi )
			return i;
	return -1;
}



extern CExpDesign *AghCC;



void remove_untried_modruns();


#endif

// EOF
