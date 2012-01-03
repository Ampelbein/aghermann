// ;-*-C++-*- *
/*
 *       File name:  print_version.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-03
 *
 *         Purpose: print version (single function, for easy touch and
 *                  recompile to ensure every rebuild has an
 *                  up-to-date build time)
 *
 *         License:  GPL
 */

#include <cstdio>
#include "config.h"

namespace agh {

void
print_version()
{
	printf( PACKAGE_STRING " compiled " __DATE__ " " __TIME__ " " BUILT_BY "\n");
}

}

// eof
