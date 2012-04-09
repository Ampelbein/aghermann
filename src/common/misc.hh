// ;-*-C++-*-
/*
 *       File name:  common/misc.hh
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
#include <string>
#include <memory>
#include <valarray>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

#if __GNUC__ >= 3
// # define __pure		__attribute__ ((pure))
// # define __const	__attribute__ ((const))
// # define __noreturn	__attribute__ ((noreturn))
// # define __malloc	__attribute__ ((malloc))
// # define __must_check	__attribute__ ((warn_unused_result))
// # define __deprecated	__attribute__ ((deprecated))
// # define __used		__attribute__ ((used))
// # define __unused	__attribute__ ((unused))
// # define __packed	__attribute__ ((packed))
# define likely(x)	__builtin_expect (!!(x), 1)
# define unlikely(x)	__builtin_expect (!!(x), 0)
#else
// # define inline		/* no inline */
// # define __pure		/* no pure */
// # define __const	/* no const */
// # define __noreturn	/* no noreturn */
// # define __malloc	/* no malloc */
// # define __must_check	/* no warn_unused_result */
// # define __deprecated	/* no deprecated */
// # define __used		/* no used */
// # define __unused	/* no unused */
// # define __packed	/* no packed */
# define likely(x)	(x)
# define unlikely(x)	(x)
#endif

typedef std::valarray<TFloat> VAF;



#define	DEF_UNIQUE_CHARP(p)			\
	char* p = NULL;				\
	unique_ptr<void,void(*)(void*)> p##_pp(p,free);


typedef unsigned long hash_t;

#define HASHKEY(s) (hash<std::string>()(s))
#define HASHKEY_ANY (hash<std::string>()("any"))


template <typename T>
inline
void pod_swap( T& a, T& b)
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


inline void
ensure_within( TFloat& v, const TFloat& l, const TFloat& h)
{
	if ( v < l )
		v = l;
	else if ( v > h )
		v = h;
}

inline TFloat
value_within( TFloat v, const TFloat& l, const TFloat& h)
{
	if ( v < l )
		v = l;
	else if ( v > h )
		v = h;
	return v;
}



#endif

// eof
