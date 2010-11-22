// ;-*-C-*- *  Time-stamp: "2010-11-20 21:19:05 hmmr"
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

#include "structures.h"

int	AghDi, AghDs,
	AghHi, AghHs,  // all channels
	AghTi, AghTs,  // eeg channels
	AghGi, AghGs,  // groups
	AghEi, AghEs;  // going deprecated?

char	**AghDD,
	**AghHH, // all channels
	**AghTT, // eeg channels
	**AghGG,
	**AghEE;

struct SExpDesign agh_cc;


// EOF
