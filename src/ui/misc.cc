// ;-*-C++-*-
/*
 *       File name:  ui/misc.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-09-03
 *
 *         Purpose:  misc non-agh specific ui bits
 *
 *         License:  GPL
 */


#include <cmath>
#include <cstring>

#include "misc.hh"
#include "ui.hh"


using namespace std;

char	aghui::__buf__[AGH_BUF_SIZE];




// these are intended for durations, not timestamps
void
aghui::snprintf_buf_ts_d( double d_)
{
	if ( d_ < 1. )
		snprintf_buf_ts_h( d_ * 24);
	else {
		unsigned m_ = lroundf(d_*24*60*60) / 60,
			m = (m_ % 60),
			h = (m_ / 60) % 24,
			d = (m_ / 60 / 24);
		if ( h % 24 == 0 && m % 60 == 0 )
			snprintf_buf( "%ud", d);
		else if ( m % 60 == 0 )
			snprintf_buf( "%ud%uh", d, h);
		else
			snprintf_buf( "%ud%uh%um", d, h, m);
	}
}

void
aghui::snprintf_buf_ts_h( double h_)
{
	if ( h_ < 1. )
		snprintf_buf_ts_m( h_ * 60);
	else if ( h_ >= 24. )
		snprintf_buf_ts_d( h_ / 24);
	else {
		unsigned m_ = lroundf( h_*60*60) / 60,
			m = (m_ % 60),
			h = (m_ / 60);
		if ( m % 60 == 0 )
			snprintf_buf( "%uh", h);
		else
			snprintf_buf( "%uh%um", h, m);
	}
}

void
aghui::snprintf_buf_ts_m( double m_)
{
	if ( m_ < 1. )
		snprintf_buf_ts_s( m_ * 60);
	else if ( m_ >= 60. )
		snprintf_buf_ts_h( m_ / 60);
	else {
		unsigned s_ = lroundf( m_*60) / 60,
			s = (s_ % 60),
			m = (s_ / 60);
		if ( s % 60 == 0 )
			snprintf_buf( "%um", m);
		else
			snprintf_buf( "%um%us", m, s);
	}
}

void
aghui::snprintf_buf_ts_s( double s_)
{
	if ( s_ >= 60. )
		snprintf_buf_ts_m( s_/60);
	else {
		snprintf_buf( "%.2gs", s_);
	}
}





void
aghui::decompose_double( double value, float *mantissa, int *exponent)
{
	static char buf[32];
	snprintf( buf, 31, "%e", value);
	*strchr( buf, 'e') = '|';
	sscanf( buf, "%f|%d", mantissa, exponent);

}




// cairo

void
aghui::cairo_put_banner( cairo_t *cr, float wd, float ht,
			 const char *text,
			 float font_size,
			 float r, float g, float b, float a)
{
	cairo_set_font_size( cr, font_size);
	cairo_set_source_rgba( cr, r, g, b, a);
	cairo_text_extents_t extents;
	cairo_text_extents( cr, text, &extents);
	double	idox = wd/2 - extents.width/2,
		idoy = ht/2 + extents.height/2;
	cairo_move_to( cr, idox, idoy);
	cairo_show_text( cr, text);
	cairo_stroke( cr);
}



// gtk

void
aghui::pop_ok_message( GtkWindow *parent, const char *str, ...)
{
	va_list ap;
	va_start (ap, str);

	static GString *buf = NULL;
	if ( buf == NULL )
		buf = g_string_new("");

	g_string_vprintf( buf, str, ap);
	va_end (ap);

	GtkWidget *msg =
		gtk_message_dialog_new_with_markup(
			parent,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			buf->str, NULL);
	gtk_dialog_run( (GtkDialog*)msg);
	gtk_widget_destroy( msg);
}


int
aghui::pop_question( GtkWindow* parent, const gchar *str, ...)
{
	va_list ap;
	va_start (ap, str);

	static GString *buf = NULL;
	if ( buf == NULL )
		buf = g_string_new("");

	g_string_vprintf( buf, str, ap);
	va_end (ap);

	GtkWidget *msg =
		gtk_message_dialog_new_with_markup(
			parent,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_YES_NO,
			buf->str, NULL);
	gint response = gtk_dialog_run( (GtkDialog*)msg);
	gtk_widget_destroy( msg);
	return response;
}




void
aghui::set_cursor_busy( bool busy, GtkWidget *wid)
{
	static GdkCursor *cursor_busy   = NULL,
			 *cursor_normal = NULL;
	if ( cursor_normal == NULL) {
		cursor_busy   = gdk_cursor_new_from_name( gdk_display_get_default(), "watch");
		cursor_normal = gdk_cursor_new_from_name( gdk_display_get_default(), "left_ptr");
	}

	gdk_window_set_cursor( gtk_widget_get_window( wid), busy ? cursor_busy : cursor_normal);
}



// eof
