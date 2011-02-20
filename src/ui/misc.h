// ;-*-C-*- *  Time-stamp: "2011-02-19 17:31:13 hmmr"
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

extern GString *__ss__;

// safe snprintf into a __buf__
gchar	__buf__[256];
#define snprintf_buf(...) snprintf( __buf__, 255, __VA_ARGS__)
void snprintf_buf_ts_d( float h);
void snprintf_buf_ts_h( float h);
void snprintf_buf_ts_m( float m);
void snprintf_buf_ts_s( float s);

#define Ai(A,B,C) g_array_index(A,B,C)




G_END_DECLS

#endif

// EOF
