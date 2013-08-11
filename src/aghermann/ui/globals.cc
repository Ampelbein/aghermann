/*
 *       File name:  aghermann/ui/globals.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-08-10
 *
 *         Purpose:  global UI-related variables
 *
 *         License:  GPL
 */

#include <gtk/gtk.h>
#include "globals.hh"

using namespace std;
using namespace agh::ui;

char	global::buf[AGH_BUF_SIZE];

GdkDevice
	*global::client_pointer;

UniqueApp
	*global::unique_app;

GtkWindow
	*global::main_window;

double	global::hdpmm,
	global::vdpmm;


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:

