/*
 *       File name:  aghermann/ui/misc.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  misc non-gtk bits
 *
 *         License:  GPL
 */


#ifndef AGH_AGHERMANN_UI_MISC_H_
#define AGH_AGHERMANN_UI_MISC_H_

#include <stdio.h>
#include <stdarg.h>
#include "globals.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
# include "config.h"
#endif

using namespace std;

namespace agh {
namespace ui {

inline const char*
snprintf_buf( const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));

inline const char*
snprintf_buf( const char* fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);

	vsnprintf( global::buf, AGH_BUF_SIZE-1, fmt, ap);
	va_end (ap);

	return global::buf;
}


const char* snprintf_buf_ts_d( double h);
const char* snprintf_buf_ts_h( double h);
const char* snprintf_buf_ts_m( double m);
const char* snprintf_buf_ts_s( double s);

}
} // namespace agh::ui

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
