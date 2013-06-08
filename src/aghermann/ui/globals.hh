/*
 *       File name:  aghermann/ui/globals.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-22
 *
 *         Purpose:  ui globals
 *
 *         License:  GPL
 */


#ifndef _AGHUI_GLOBALS_H
#define _AGHUI_GLOBALS_H

#include <gtk/gtk.h>
#include <unique/unique.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


namespace aghui {

extern UniqueApp
	*__unique_app__;

extern GtkWindow
	*__main_window__;

// convenience assign-once vars
extern GdkDevice
	*__client_pointer__;

// quick tmp storage
#define AGH_BUF_SIZE (1024*5)
extern char
	__buf__[AGH_BUF_SIZE];
extern GString
	*__ss__;

extern double
	__hdpmm__,
	__vdpmm__;


int prepare_for_expdesign();

void set_unique_app_window( GtkWindow*);

} // namespace aghui

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
