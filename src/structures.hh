// ;-*-C++-*- *  Time-stamp: "2011-04-17 23:09:12 hmmr"
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



extern CExpDesign *AghCC;



void remove_untried_modruns();


#endif

// EOF
