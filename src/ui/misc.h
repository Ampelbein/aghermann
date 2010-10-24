// ;-*-C-*- *  Time-stamp: "2010-10-07 09:27:41 hmmr"
/*
 *       File name:  misc.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-09-03
 *
 *         Purpose:  misc general-purpose ui bits
 *
 *         License:  GPL
 */


#ifndef _MISC_H
#define _MISC_H

#include <gtk/gtk.h>
#include <glade/glade.h>

G_BEGIN_DECLS

void pop_ok_message( GtkWindow *parent, const gchar*, ...);
gint pop_question( GtkWindow *parent, const gchar*);
void set_cursor_busy( gboolean busy, GtkWidget *wid);

//void show_and_notify( GtkWindow*, const char *fmt, ...);
//void hide_and_notify();


#define Ai(A,B,C) g_array_index(A,B,C)

G_END_DECLS

#endif

// EOF
