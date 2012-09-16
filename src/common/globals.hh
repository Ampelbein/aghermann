// ;-*-C++-*-
/*
 *       File name:  common/globals.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  global (gasp!) variable definitions
 *
 *         License:  GPL
 */

#ifndef _AGH_COMMON_GLOBALS_H
#define _AGH_COMMON_GLOBALS_H

#include <gsl/gsl_rng.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {
namespace global {

extern gsl_rng *rng;
void init_rng();


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

} // namespace global
} // namespace agh

#endif

// eof
