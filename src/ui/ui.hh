// ;-*-C++-*-
/*
 *       File name:  ui/ui.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  globally accessible widgets
 *
 *         License:  GPL
 */


#ifndef _AGH_UI_H
#define _AGH_UI_H

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


// project-specific 'misc.hh'
int prepare_for_expdesign();


struct SGeometry {
	int x, y, w, h;
};


#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)				\
	(A = (Type*)(gtk_builder_get_object( B, #A)))


#define AGH_UI_FILE "ui/agh-ui-main.glade"
#define AGH_BG_IMAGE_FNAME "ui/idle-bg.svg"


} // namespace aghui

#endif

// eof
