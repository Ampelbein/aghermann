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

#include "globals.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {
namespace ui {

#define snprintf_buf(...) (snprintf( __buf__, AGH_BUF_SIZE-1, __VA_ARGS__), __buf__)

void snprintf_buf_ts_d( double h);
void snprintf_buf_ts_h( double h);
void snprintf_buf_ts_m( double m);
void snprintf_buf_ts_s( double s);

}
} // namespace agh::ui

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
