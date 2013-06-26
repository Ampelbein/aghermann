/*
 *       File name:  aghermann/print_version.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-05-28
 *
 *         Purpose:  print version (separate file for every make to always touch it)
 *
 *         License:  GPL
 */

#include <cstdio>
#include "config.h"

void
print_version()
{
	printf( PACKAGE_STRING " built " __DATE__ " " __TIME__ " by " BUILT_BY "\n");
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: nil
// tab-width: 8
// End:
