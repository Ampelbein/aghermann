/*
 *       File name:  ui/globals.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  common GTK+ variables; ui init
 *
 *         License:  GPL
 */

#include <cassert>
#include <gtk/gtk.h>

#include "common/alg.hh"
#include "globals.hh"
#include "misc.hh"
#include "ui.hh"
#include "ui++.hh"

using namespace std;

char	aghui::__buf__[AGH_BUF_SIZE];

GString	*aghui::__ss__;

GdkDevice
	*aghui::__client_pointer__;

UniqueApp
	*aghui::__unique_app__;

GtkWindow
	*aghui::__main_window__;

double	aghui::__hdpmm__,
	aghui::__vdpmm__;

#define AGH_UI_GRESOURCE_FILE "aghermann.gresource"

// unique

void
aghui::
set_unique_app_window( GtkWindow* w)
{
	unique_app_watch_window(
		aghui::__unique_app__,
		aghui::__main_window__ = w);
}

// own init

int
aghui::
prepare_for_expdesign()
{
	__ss__ = g_string_new( "");

      // tell me what they are
	__client_pointer__ =
		gdk_device_manager_get_client_pointer(
			gdk_display_get_device_manager( gdk_display_get_default()));

	{
		auto scr = gdk_screen_get_default();
		__hdpmm__ = (double)gdk_screen_get_width ( scr) / gdk_screen_get_width_mm ( scr);
		__vdpmm__ = (double)gdk_screen_get_height( scr) / gdk_screen_get_height_mm( scr);
		printf( "Screen xdpmm is %g v, %g h\n", __hdpmm__, __vdpmm__);
		gdk_screen_set_resolution( scr, (__hdpmm__ + __vdpmm__)/2);
	}

	GResource
		*gresource
		= g_resource_load(
			PACKAGE_DATADIR "/" PACKAGE "/" AGH_UI_GRESOURCE_FILE,
			NULL);
	if ( !gresource ) {
		fprintf( stderr, "Bad or missing " PACKAGE_DATADIR "/" PACKAGE "/" AGH_UI_GRESOURCE_FILE);
		return -1;
	}
	g_resources_register( gresource);

	return 0;
}






// these are intended for durations, not timestamps
void
aghui::
snprintf_buf_ts_d( const double d_)
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
aghui::
snprintf_buf_ts_h( const double h_)
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
aghui::
snprintf_buf_ts_m( const double m_)
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
aghui::
snprintf_buf_ts_s( const double s_)
{
	if ( s_ >= 60. )
		snprintf_buf_ts_m( s_/60);
	else
		snprintf_buf( "%.2gs", s_);
}








// cairo

void
aghui::
cairo_put_banner( cairo_t *cr, const float wd, const float ht,
		  const char *text,
		  const float font_size,
		  const float r, const float g, const float b,
		  const float a)
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


void
aghui::
cairo_draw_signal( cairo_t *cr, const valarray<TFloat>& V,
		   const ssize_t start, const ssize_t end,
		   const size_t hspan, const float hoff, const float voff, const float scale,
		   const unsigned short decimate,
		   const aghui::TDrawSignalDirection direction,
		   const bool continue_path)
{
	switch ( direction ) {

	case TDrawSignalDirection::forward:
		if ( unlikely (start < 0) )
			(continue_path ? cairo_line_to : cairo_move_to)(
				cr, hoff + ((double)(0 - start))/(end-start) * hspan, 0 + voff);
		else
			(continue_path ? cairo_line_to : cairo_move_to)(
				cr, hoff,
				voff - V[start] * scale);
		for ( ssize_t i = max((ssize_t)1, start); i < end && i < (ssize_t)V.size(); i += decimate )
			if ( isfinite(V[i]) )
				cairo_line_to( cr, hoff + ((double)(i - start))/(end-start) * hspan,
					       voff - V[i] * scale);
	    break;

	case TDrawSignalDirection::backward:
		if ( unlikely (end > (ssize_t)V.size()) )
			(continue_path ? cairo_line_to : cairo_move_to)(
				cr, hoff + ((double)(V.size()-1 - start))/(end-start) * hspan, 0 + voff);
		else
			(continue_path ? cairo_line_to : cairo_move_to)(
				cr, hoff,
				voff - V[end-1] * scale);
		for ( ssize_t i = min(end, (ssize_t)V.size()) - 1-1; i >= 0 && i >= start; i -= decimate )
			if ( isfinite(V[i]) )
				cairo_line_to( cr, hoff + ((double)(i - start))/(end-start) * hspan,
					       voff - V[i] * scale);
	    break;
	}
//	cairo_stroke( cr);
}


void
aghui::
cairo_draw_envelope( cairo_t *cr, const valarray<TFloat>& V,
		     ssize_t start, ssize_t end,
		     const size_t hspan,
		     const float hoff, const float voff,
		     const float scale)
{
	agh::alg::ensure_within( start, (ssize_t)0, (ssize_t)V.size());
	agh::alg::ensure_within( end,   (ssize_t)0, (ssize_t)V.size());

	double dps = (double)(end - start) / hspan;
	cairo_move_to( cr, hoff, voff);
	size_t i = start;
	for ( ; i < end; ++i )
		cairo_line_to( cr, i / dps,
			       voff - V[i] * scale/2);
	for ( --i; i > start; --i )
		cairo_line_to( cr, i / dps,
			       voff + V[i] * scale/2);
	cairo_fill( cr);
}







// gtk


void
aghui::
gtk_combo_box_set_model_properly( GtkComboBox *cb, GtkListStore *m)
{
	gtk_combo_box_set_model( cb, (GtkTreeModel*)m);
	gtk_combo_box_set_id_column( cb, 0);
	gtk_cell_layout_set_renderer( cb);
}


void
aghui::
gtk_cell_layout_set_renderer( GtkComboBox *cb)
{
	GtkCellRenderer *r = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)cb, r, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)cb, r,
					"text", 0,
					NULL);
}

void
aghui::
pop_ok_message( GtkWindow *parent,
		const char* primary_text,
		const char* fmt, ...)
{
	auto W = (GtkMessageDialog*)
		gtk_message_dialog_new_with_markup(
			parent,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			primary_text, NULL);

	if ( fmt ) {
		va_list ap;
		va_start (ap, fmt);

		char *_;
		assert (vasprintf( &_, fmt, ap) > 0);
		va_end (ap);
		gtk_message_dialog_format_secondary_markup( W, "%s", _);
		free( (void*)_);
	}

	gtk_dialog_run( (GtkDialog*)W);

	gtk_widget_destroy( (GtkWidget*)W);
}


int
aghui::
pop_question( GtkWindow* parent, const gchar *str, ...)
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
aghui::
set_cursor_busy( bool busy, GtkWidget *wid)
{
	static GdkCursor *cursor_busy   = NULL,
			 *cursor_normal = NULL;
	if ( cursor_normal == NULL) {
		cursor_busy   = gdk_cursor_new_from_name( gdk_display_get_default(), "watch");
		cursor_normal = gdk_cursor_new_from_name( gdk_display_get_default(), "left_ptr");
	}

	gdk_window_set_cursor( gtk_widget_get_window( wid), busy ? cursor_busy : cursor_normal);
}


namespace aghui {

template <> void
SUIVar_<GtkListStore, list<string>>::up() const
{
	gtk_list_store_clear( w);
	GtkTreeIter iter;
	for ( auto& s : *v ) {
		gtk_list_store_append( w, &iter);
		gtk_list_store_set( w, &iter,
				    1, s.c_str(),
				    -1);
	}
}
template <> void
SUIVar_<GtkListStore, list<string>>::down() const
{
	v->clear();
	GtkTreeIter
		iter;
	gchar	*entry;
	while ( gtk_tree_model_get_iter_first( (GtkTreeModel*)w, &iter) ) {
		gtk_tree_model_get( (GtkTreeModel*)w, &iter,
				    1, &entry,
				    -1);
		v->emplace_back( entry);
		g_free( entry);
	}
}


} // namespace aghui


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

