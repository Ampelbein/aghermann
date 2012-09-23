// ;-*-C++-*-
/*
 *       File name:  ui/globals.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  misc non-gtk bits
 *
 *         License:  GPL
 */


#ifndef _AGHUI_MISC_H
#define _AGHUI_MISC_H

#include "globals.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace aghui {

#define snprintf_buf(...) snprintf( __buf__, AGH_BUF_SIZE-1, __VA_ARGS__)

void snprintf_buf_ts_d( double h);
void snprintf_buf_ts_h( double h);
void snprintf_buf_ts_m( double m);
void snprintf_buf_ts_s( double s);

} // namespace aghui

#endif

// eof
