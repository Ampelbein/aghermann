// ;-*-C++-*-
/*
 *       File name:  ui/globals.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-22
 *
 *         Purpose:  general init and global decls
 *
 *         License:  GPL
 */


#ifndef _AGHUI_GLOBALS_H
#define _AGHUI_GLOBALS_H

#include <gtk/gtk.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


namespace aghui {


// convenience assign-once vars
extern GdkDevice
	*__client_pointer__;

// quick tmp storage
#define AGH_BUF_SIZE (1024*5)
extern char
	__buf__[AGH_BUF_SIZE];
extern GString
	*__ss__;

int prepare_for_expdesign();

} // namespace aghui

#endif

// eof
