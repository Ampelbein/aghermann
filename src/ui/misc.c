// ;-*-C-*- *  Time-stamp: "2011-02-19 19:28:18 hmmr"
/*
 *       File name:  ui/misc.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-09-03
 *
 *         Purpose:  misc non-agh specific ui bits
 *
 *         License:  GPL
 */


#include <unistd.h>
#include "misc.h"
#include "ui.h"



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




/*
static GtkWidget *wAndNotify;


void
show_and_notify( GtkWindow *relative, const char *fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);

	static GString *buf = NULL;
	if ( buf == NULL )
		buf = g_string_new("");

	g_string_vprintf( buf, fmt, ap);
	va_end (ap);


	if ( wAndNotify )
		gtk_widget_destroy( GTK_WIDGET (wAndNotify));

	wAndNotify = gtk_window_new( GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated( GTK_WINDOW (wAndNotify), FALSE);
	gtk_window_set_transient_for( GTK_WINDOW (wAndNotify), relative);
	gtk_window_set_destroy_with_parent( GTK_WINDOW (wAndNotify), TRUE);

	GtkLabel *lAndNotify = gtk_label_new( buf->str);
	gtk_container_set_border_width( GTK_CONTAINER (wAndNotify), 8);
	gtk_container_add( GTK_CONTAINER (wAndNotify), GTK_WIDGET (lAndNotify));

	gint px, py, pw, ph, pw2;
	gtk_window_get_position( relative, &px, &py);
	gtk_window_get_size( relative, &pw, &ph);

	gtk_window_set_default_size( GTK_WINDOW (wAndNotify), -1, -1);
	gtk_widget_queue_resize( wAndNotify);

	gtk_window_get_size( GTK_WINDOW (wAndNotify), &pw2, NULL);
//	printf( "to x = %d, y = %d\n", px+pw/2 - pw2/2, (guint)(py+0.8*ph));
	gtk_window_move( GTK_WINDOW (wAndNotify), px+pw/2 - pw2/2, (guint)(py+0.8*ph));
	gtk_widget_show_all( wAndNotify);
}


void
hide_and_notify()
{
	gtk_widget_destroy( wAndNotify);
	wAndNotify = NULL;
}

*/



GString *__ss__;

gint
agh_ui_construct_misc( GladeXML *xml)
{
//	if ( !(wAndNotify    	  = glade_xml_get_widget( xml, "wAndNotify")) ||
//	     !(lAndNotify    	  = glade_xml_get_widget( xml, "lAndNotify")) )
//		return -1;

	__ss__ = g_string_new( "");

	return 0;
}




// these are intended for durations, not timestamps
void snprintf_buf_ts_d( float d)
{
	if ( d < 1. )
		snprintf_buf_ts_h( d*24);
	else {
		int i = (int)d;
		float	f = d - i,
			mf = f*24 - (int)(f*24);
		if ( f < 1./24/60 )
			snprintf_buf( "%dd", i);
		else if ( mf < 1./60 )
			snprintf_buf( "%dd%2dh", i, (int)(f*24));
		else
			snprintf_buf( "%dd%2dh%2dm", i, (int)(f*24), (int)(mf*60));
	}
}

void snprintf_buf_ts_h( float h)
{
	if ( h < 1 )
		snprintf_buf_ts_m( h*60);
	else if ( h >= 24 )
		snprintf_buf_ts_d( h/24);
	else {
		int i = (int)h;
		float f = h - i;
		if ( f < 1./60 )
			snprintf_buf( "%2dh", i);
		else
			snprintf_buf( "%dh%2d", i, (int)(f*60));
	}
}

void snprintf_buf_ts_m( float m)
{
	if ( m < 1 )
		snprintf_buf_ts_s( m*60);
	else if ( m >= 60 )
		snprintf_buf_ts_h( m/60);
	else {
		int i = (int)m;
		float f = m - i;
		snprintf_buf( "%dm%2ds", i, (int)(f*60));
	}
}

void snprintf_buf_ts_s( float s)
{
	if ( s >= 60 )
		snprintf_buf_ts_m( s/60);
	else {
		snprintf_buf( "%4gs", s);
	}
}




// EOF
