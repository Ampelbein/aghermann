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

#ifndef _AGH_COMMON_MISC_H_
#define _AGH_COMMON_MISC_H_

#include <unistd.h>
#include <string>
#include <memory>

#include <gsl/gsl_rng.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {

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


#define	DEF_UNIQUE_CHARP(p)				\
	char* p = nullptr;				\
	unique_ptr<void,void(*)(void*)> p##_pp(p,free);

#define DELETE_DEFAULT_METHODS(T)		\
	T () = delete;				\
	T (const T&) = delete;			\
	void operator=( const T&) = delete;



typedef unsigned long hash_t;

#define HASHKEY(s) (hash<std::string>()(s))
#define HASHKEY_ANY (hash<std::string>()("any"))


extern gsl_rng *__agh_rng;
void init_global_rng();


// typedef std::valarray<TFloat> VAF;

// // debugging aids
// template <typename T> void
// vaf_dump( const valarray<T>& v, const string& fname, size_t size = -1)
// {
// 	if ( size == (size_t)-1 )
// 		size = v.size();
// 	int fd;
// 	if ( (fd = open( fname.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1 ||
// 	     write( fd, &v[0], size * sizeof(T)) == -1 )
// 		printf( "so broken even vaf_dump failed\n");
// 	close( fd);
// }

} // namespace agh

#endif

// eof
