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


#ifndef AGH_AGHERMANN_UI_GLOBALS_H_
#define AGH_AGHERMANN_UI_GLOBALS_H_

#include <gtk/gtk.h>
#include <unique/unique.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


namespace agh {
namespace ui {
namespace global {

// convenience assign-once vars
extern UniqueApp
	*unique_app;

extern GtkWindow
	*main_window;

extern GdkDevice
	*client_pointer;

extern double
	hdpmm,
	vdpmm;

// quick tmp storage
#define AGH_BUF_SIZE (1024*5)
extern char
	buf[AGH_BUF_SIZE];

}
}
} // namespace agh::ui::global

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
