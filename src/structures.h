// ;-*-C-*- *  Time-stamp: "2010-11-27 02:04:54 hmmr"
/*
 *       File name:  structures.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-11-20
 *
 *         Purpose:  snapshot of core structures in C
 *
 *         License:  GPL
 */

#ifndef _AGH_STRUCTURES_H
#define _AGH_STRUCTURES_H

#include "libagh/iface.h"

extern int
	AghHs,
	AghTs,
	AghGs,
	AghDs,
	AghEs;

extern char
	**AghDD,
	**AghHH,
	**AghTT,
	**AghGG,
	**AghEE;


extern struct SExpDesign agh_cc;

#endif

// EOF