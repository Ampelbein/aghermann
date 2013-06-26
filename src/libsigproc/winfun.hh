/*
 *       File name:  libsigproc/winfun.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-11-17
 *
 *         Purpose:  windowing functions
 *
 *         License:  GPL
 */

#ifndef AGH_LIBSIGPROC_WINFUN_H_
#define AGH_LIBSIGPROC_WINFUN_H_

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
	TWinType_total
};

extern const char*
	welch_window_type_names[TWinType::TWinType_total];

extern TFloat (*winf[])(size_t, size_t);

} // namespace sigproc


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
