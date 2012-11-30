// ;-*-C++-*-
/*
 *       File name:  sigproc/winfun.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-11-17
 *
 *         Purpose:  windowing functions
 *
 *         License:  GPL
 */

#ifndef _SIGPROC_WINFUN_H
#define _SIGPROC_WINFUN_H

#include <stddef.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigproc {

enum TWinType {
	bartlett,
	blackman,
	blackman_harris,
	hamming,
	hanning,
	parzen,
	square,
	welch,
	_total
};

extern const char*
	welch_window_type_names[TWinType::_total];

extern TFloat (*winf[])(size_t, size_t);

} // namespace sigproc


#endif

// eof
