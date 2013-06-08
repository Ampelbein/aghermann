/*
 *       File name:  aghermann/globals.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  global (gasp!) variable definitions
 *
 *         License:  GPL
 */

#ifndef _AGH_GLOBALS_H
#define _AGH_GLOBALS_H

#include <gsl/gsl_rng.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {
namespace global {

extern gsl_rng *rng;

extern int num_procs;

void init();

} // namespace global
} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
