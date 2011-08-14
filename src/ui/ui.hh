// ;-*-C++-*-
/*
 *       File name:  ui/ui.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  globally accessible widgets, GTK-specific representations, and model storage
 *
 *         License:  GPL
 */


#ifndef _AGH_UI_H
#define _AGH_UI_H

#include <gtk/gtk.h>

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;


namespace aghui {

      // convenience assign-once vars
	extern GdkVisual
		*__visual;
	extern GtkBuilder
		*__builder;

	extern GString
		*__ss__;

	int prepare_for_expdesign();


	void pop_ok_message( GtkWindow *parent, const gchar*, ...);
	gint pop_question( GtkWindow *parent, const gchar*);
	void set_cursor_busy( bool busy, GtkWidget *wid);

	struct SGeometry {
		int x, y, w, h;
	};


#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( __builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)				\
	(A = (Type*)(gtk_builder_get_object( B, #A)))


#define AGH_UI_FILE "ui/agh-ui-main.glade"
#define AGH_BG_IMAGE_FNAME "ui/idle-bg.svg"


} // namespace aghui

#endif

// EOF
