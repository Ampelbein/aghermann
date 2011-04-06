// ;-*-C++-*- *  Time-stamp: "2011-04-02 18:31:47 hmmr"
/*
 *       File name:  misc.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  misc supporting functions
 *
 *         License:  GPL
 */

#ifndef _AGH_MISC_H
#define _AGH_MISC_H

#include <cstdlib>

#if HAVE_CONFIG_H
#  include "config.h"
#endif


#define	UNIQUE_CHARP(p)				\
	char* p = NULL;				\
	unique_ptr<void,void(*)(void*)> p##_pp(p,free);




typedef size_t sid_type;
typedef size_t hash_key;

#define HASHKEY(s) (hash<std::string>()(s))
#define HASHKEY_ANY (hash<std::string>()("any"))






// inline const char*
// yesno( bool val)
// {
// 	return val ?"yes" :"no";
// }



#endif

// EOF
