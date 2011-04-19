// ;-*-C++-*- *  Time-stamp: "2011-04-19 01:10:06 hmmr"
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

#define __BUF_SIZE 1024
gchar	__buf__[1024];
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



#define GLADEXML2(WidgetType, A) \
	(A = (WidgetType*)glade_xml_get_widget( xml, #A))

#endif

// EOF
