// ;-*-C-*- *  Time-stamp: "2011-02-21 01:03:44 hmmr"
/*
 *       File name:  ui/misc.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-09-03
 *
 *         Purpose:  misc general-purpose ui bits
 *
 *         License:  GPL
 */


#ifndef _AGH_MISC_H
#define _AGH_MISC_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define __BUF_SIZE 1024
gchar	__buf__[1024];
#define snprintf_buf(...) snprintf( __buf__, __BUF_SIZE-1, __VA_ARGS__)

void snprintf_buf_ts_d( double h);
void snprintf_buf_ts_h( double h);
void snprintf_buf_ts_m( double m);
void snprintf_buf_ts_s( double s);

#define Ai(A,B,C) g_array_index(A,B,C)

extern GString *__ss__;


void pop_ok_message( GtkWindow *parent, const gchar*, ...);
gint pop_question( GtkWindow *parent, const gchar*);
void set_cursor_busy( gboolean busy, GtkWidget *wid);



extern GdkVisual
	*agh_visual;
extern GdkColormap
	*agh_cmap;


G_END_DECLS

#endif

// EOF
