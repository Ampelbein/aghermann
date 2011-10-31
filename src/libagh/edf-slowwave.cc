// ;-*-C++-*-
/*
 *       File name:  libagh/edf-slowwave.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-09-25
 *
 *         Purpose:  Slow Wave inspector
 *
 *         License:  GPL
 */


#include "page.hh"
#include "edf.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;

int
__attribute__ ((const)) agh::CEDFFile::SSignal::assess_slowwaves() const
{
	
	return 0;
}


// EOF
