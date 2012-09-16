// ;-*-C++-*-
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

#ifndef _AGH_COMMON_LANG_H_
#define _AGH_COMMON_LANG_H_

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

#include <unistd.h>
#include <memory>

using namespace std;

// # define __pure		__attribute__ ((pure))
// # define __const		__attribute__ ((const))
// # define __noreturn		__attribute__ ((noreturn))
// # define __malloc		__attribute__ ((malloc))
// # define __must_check	__attribute__ ((warn_unused_result))
// # define __deprecated	__attribute__ ((deprecated))
// # define __used		__attribute__ ((used))
// # define __unused		__attribute__ ((unused))
// # define __packed		__attribute__ ((packed))
# define likely(x)	__builtin_expect (!!(x), 1)
# define unlikely(x)	__builtin_expect (!!(x), 0)


#define	DEF_UNIQUE_CHARP(p)				\
	char* p = nullptr;				\
	std::unique_ptr<void,void(*)(void*)> p##_pp(p,free);

#define DELETE_DEFAULT_METHODS(T)		\
	T () = delete;				\
	T (const T&) = delete;			\
	void operator=( const T&) = delete;

#endif

// eof
