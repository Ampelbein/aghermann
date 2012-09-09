// ;-*-C++-*-
/*
 *       File name:  ui/ui.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose: collected global variables for use after
 *                  gtk_main(), and __buf__ and __ss__ strings
 *
 *         License:  GPL
 */


#ifndef _AGHUI_H
#define _AGHUI_H

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
	SGeometry()
	      : x (-1), y (-1), w (-1), h (-1)
		{}
	SGeometry( int x_, int y_, int w_, int h_)
	      : x (x_), y (y_), w (w_), h (h_)
		{}
	bool is_valid() const
		{
			return	(x > 0) && (x < 3000) &&
				(y > 0) && (y < 3000) &&
				(w > 1) && (w < 50000) &&
				(h > 1) && (h < 50000);
		}
};


#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)				\
	(A = (Type*)(gtk_builder_get_object( B, #A)))


// our files in share/aghermann/data
#define AGH_UI_MAIN_GLADE "ui/main.glade"
#define AGH_UI_SESSION_CHOOSER_GLADE "ui/session-chooser.glade"

#define AGH_BG_IMAGE_FNAME "ui/idle-bg.svg"


} // namespace aghui

#endif

// eof
