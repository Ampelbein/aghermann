// ;-*-C++-*-
/*
 *       File name:  ui/ui.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  common GTK+ variables; ui init
 *
 *         License:  GPL
 */



#include "misc.hh"
#include "ui.hh"

using namespace std;

char	aghui::__buf__[AGH_BUF_SIZE];

GString	*aghui::__ss__;

GdkDevice
	*aghui::__client_pointer__;

using namespace aghui;

inline namespace {
}

int
aghui::
prepare_for_expdesign()
{
	__ss__ = g_string_new( "");

      // tell me what they are
	__client_pointer__ = gdk_device_manager_get_client_pointer(
		gdk_display_get_device_manager( gdk_display_get_default()));

	return 0;
}


// eof
