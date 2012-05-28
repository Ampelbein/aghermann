// ;-*-C++-*-
/*
 *       File name:  common/config-validate.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-05-27
 *
 *         Purpose:  libconfig-bound validator
 *
 *         License:  GPL
 */



#include <list>

#include "config-validate.hh"
#include "string.hh"



#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;


// eof
