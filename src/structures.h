// ;-*-C-*- *  Time-stamp: "2011-02-23 23:53:19 hmmr"
/*
 *       File name:  structures.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
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

inline void
free_charp_array( char** what)
{
	if ( what ) {
		size_t i = 0;
		while ( what[i] )
			free( what[i++]);
		free( what);
	}
}



#endif

// EOF
