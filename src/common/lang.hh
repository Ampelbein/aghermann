/*
 *       File name:  common/lang.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-16
 *
 *         Purpose:  language and gcc macros
 *
 *         License:  GPL
 */

#ifndef _AGH_COMMON_LANG_H
#define _AGH_COMMON_LANG_H

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

#include <unistd.h>
#include <cassert>
#include <memory>

using namespace std;

namespace agh {




typedef unsigned long hash_t;


// g++ bits

#define	MAKE_UNIQUE_CHARP(p)				\
	unique_ptr<void,void(*)(void*)> p##_pp(p,free);


#define DELETE_DEFAULT_METHODS(T)		\
	T () = delete;				\
	T (const T&) = delete;			\
	void operator=( const T&) = delete;




// gcc bits

// # define __pure		__attribute__ ((pure))
// # define __const		__attribute__ ((const))
// # define __noreturn		__attribute__ ((noreturn))
// # define __malloc		__attribute__ ((malloc))
// # define __must_check	__attribute__ ((warn_unused_result))
// # define __deprecated	__attribute__ ((deprecated))
// # define __used		__attribute__ ((used))
// # define __unused		__attribute__ ((unused))
// # define __packed		__attribute__ ((packed))
#define likely(x)	__builtin_expect (!!(x), 1)
#define unlikely(x)	__builtin_expect (!!(x), 0)


#define ASPRINTF(...) \
	assert (asprintf(__VA_ARGS__) > 0)

#define FABUF printf( __FILE__ ":%d (%s): %s\n", __LINE__, __FUNCTION__, __buf__);
#define FAFA printf( __FILE__ ":%d (%s): fafa\n", __LINE__, __FUNCTION__);

} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
