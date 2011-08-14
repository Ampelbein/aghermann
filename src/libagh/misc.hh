// ;-*-C++-*-
/*
 *       File name:  libagh/misc.hh
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
#include <cstring>
#include <string>

#if HAVE_CONFIG_H
#  include "config.h"
#endif

using namespace std;

#define	UNIQUE_CHARP(p)				\
	char* p = NULL;				\
	unique_ptr<void,void(*)(void*)> p##_pp(p,free);




typedef size_t sid_type;
typedef size_t hash_key;

#define HASHKEY(s) (hash<std::string>()(s))
#define HASHKEY_ANY (hash<std::string>()("any"))


template <class T>
inline
void swap_pod( T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

template <class T>
inline bool
overlap( const T& a, const T& b,
	 const T& c, const T& d)
{
	return not ((a < c && b < c) || (a > d && b > d));
}




inline string
strtrim( const string& r0)
{
	string r (r0);
	auto rsize = r.size();
	if ( rsize == 0 )
		return r;
	while (r[rsize-1] == ' ')
		--rsize;
	r.resize( rsize);
	return r;
}

inline string
strpad( const string& r0, size_t to)
{
	string r (to, ' ');
	memcpy( (void*)r.data(), (const void*)r0.data(), r0.size());
	return r;
}


#endif

// EOF
