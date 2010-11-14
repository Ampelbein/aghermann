// ;-*-C-*- *  Time-stamp: "2010-11-14 22:05:42 hmmr"
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


// safe snprintf into a __buf__
gchar	__buf__[256];
#define snprintf_buf(...) snprintf( __buf__, 255, __VA_ARGS__)

#define Ai(A,B,C) g_array_index(A,B,C)


extern GString
	*__ss__;
extern PangoLayout
	*__pp__;
extern GArray
	*__ll__;
#define LL(x) Ai(__ll__, GdkPoint, x)

inline void
__ensure_enough_lines( guint n)
{
	if ( __ll__->len < n )
		g_array_set_size( __ll__, n);
}



G_END_DECLS

#endif

// EOF
