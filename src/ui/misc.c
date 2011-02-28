// ;-*-C-*- *  Time-stamp: "2011-02-28 09:30:51 hmmr"
/*
 *       File name:  ui/misc.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-09-03
 *
 *         Purpose:  misc non-agh specific ui bits
 *
 *         License:  GPL
 */


#include <math.h>
#include <string.h>
#include "misc.h"
#include "ui.h"


GdkColormap *__cmap;


void
pop_ok_message( GtkWindow *parent, const gchar *str, ...)
{
	va_list ap;
	va_start (ap, str);

	static GString *buf = NULL;
	if ( buf == NULL )
		buf = g_string_new("");

	g_string_vprintf( buf, str, ap);
	GtkWidget *msg = gtk_message_dialog_new( parent,
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_INFO,
						 GTK_BUTTONS_OK,
						 buf->str, NULL);
	va_end (ap);

	gtk_dialog_run( GTK_DIALOG (msg));
	gtk_widget_destroy( msg);
}


gint
pop_question( GtkWindow* parent, const gchar *str)
{
	GtkWidget *msg = gtk_message_dialog_new( parent,
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_INFO,
						 GTK_BUTTONS_YES_NO,
						 str, NULL);
	gint response = gtk_dialog_run( GTK_DIALOG (msg));
	gtk_widget_destroy( msg);
	return response;
}




void
set_cursor_busy( gboolean busy, GtkWidget *wid)
{
	static GdkCursor *cursor_busy   = NULL,
			 *cursor_normal = NULL;
	if ( cursor_normal == NULL) {
		cursor_busy   = gdk_cursor_new_from_name( gdk_display_get_default(), "watch");
		cursor_normal = gdk_cursor_new_from_name( gdk_display_get_default(), "left_ptr");
	}

	gdk_window_set_cursor( wid->window, busy ? cursor_busy : cursor_normal);

	while ( gtk_events_pending () )
		gtk_main_iteration();
}





GdkVisual
	*agh_visual;
GdkColormap
	*agh_cmap;

GString *__ss__;

gint
agh_ui_construct_misc( GladeXML *xml)
{
	__ss__ = g_string_new( "");

      // tell me what they are
	agh_visual = gdk_visual_get_system();
	agh_cmap = gdk_screen_get_default_colormap(
		gdk_screen_get_default());

	return 0;
}




// these are intended for durations, not timestamps
void snprintf_buf_ts_d( double d)
{
	if ( d < 1. )
		snprintf_buf_ts_h( d*24);
	else {
		unsigned i = lroundf(d*10)/10;
		double	f = d - i,
			mf = f*24 - (int)(f*24);
		if ( f < 1./24/60 )
			snprintf_buf( "%ud", i);
		else if ( mf < 1./60 )
			snprintf_buf( "%ud%uh", i, (unsigned)lroundf(f*24));
		else
			snprintf_buf( "%ud%uh%um", i, (unsigned)lroundf(f*24), (unsigned)lroundf(mf*60));
	}
}

void snprintf_buf_ts_h( double h)
{
	if ( h < 1 )
		snprintf_buf_ts_m( h*60);
	else if ( h >= 24 )
		snprintf_buf_ts_d( h/24);
	else {
		unsigned i = lroundf(h*60)/60;
		double f = h - i;
		if ( f < 1./60 )
			snprintf_buf( "%uh", i);
		else
			snprintf_buf( "%uh%um", i, (unsigned)ceilf(f*60));
	}
}

void snprintf_buf_ts_m( double m)
{
	if ( m < 1 )
		snprintf_buf_ts_s( m*60);
	else if ( m >= 60 )
		snprintf_buf_ts_h( m/60);
	else {
		unsigned i = lroundf(m*60)/60;
		double f = m - i;
		if ( f < 1./60 )
			snprintf_buf( "%um", i);
		else
			snprintf_buf( "%um%us", i, (unsigned)ceilf(f*60));
	}
}

void snprintf_buf_ts_s( double s)
{
	if ( s >= 60 )
		snprintf_buf_ts_m( s/60);
	else {
		snprintf_buf( "%.2gs", s);
	}
}





void
decompose_double( double value, float *mantissa, int *exponent)
{
	static char buf[32];
	snprintf( buf, 31, "%e", value);
	*strchr( buf, 'e') = '|';
	sscanf( buf, "%f|%d", mantissa, exponent);

}

// EOF
