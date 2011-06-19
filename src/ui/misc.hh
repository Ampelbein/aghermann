// ;-*-C++-*- *  Time-stamp: "2011-06-19 15:00:16 hmmr"
/*
 *       File name:  ui/misc.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-09-03
 *
 *         Purpose:  misc general-purpose bits
 *
 *         License:  GPL
 */


#ifndef _AGH_UI_MISC_H
#define _AGH_UI_MISC_H

#include <gtk/gtk.h>

namespace aghui {

#define __BUF_SIZE 1024
extern char
	__buf__[__BUF_SIZE];
#define snprintf_buf(...) snprintf( __buf__, __BUF_SIZE-1, __VA_ARGS__)

void snprintf_buf_ts_d( double h);
void snprintf_buf_ts_h( double h);
void snprintf_buf_ts_m( double m);
void snprintf_buf_ts_s( double s);

void decompose_double( double value, float *mantissa, int *exponent);


extern GString *__ss__;


void pop_ok_message( GtkWindow *parent, const gchar*, ...);
gint pop_question( GtkWindow *parent, const gchar*);
void set_cursor_busy( bool busy, GtkWidget *wid);



#define AGH_GBGETOBJ(Type, A)				\
	(A = (Type*)(gtk_builder_get_object( __builder, #A)))

#define AGH_GBGETOBJ3(B, Type, A)				\
	(A = (Type*)(gtk_builder_get_object( B, #A)))

}

#endif

// EOF
