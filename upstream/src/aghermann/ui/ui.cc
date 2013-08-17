/*
 *       File name:  aghermann/ui/ui.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  general-purpose GTK+ functions; ui init
 *
 *         License:  GPL
 */

#include <gtk/gtk.h>

#include "common/alg.hh"
#include "globals.hh"
#include "ui.hh"

using namespace std;
using namespace agh::ui;

#define AGH_UI_GRESOURCE_FILE "aghermann.gresource"

// unique

void
agh::ui::
set_unique_app_window( GtkWindow* w)
{
	unique_app_watch_window(
		global::unique_app,
		global::main_window = w);
}

// own init

int
agh::ui::
prepare_for_expdesign()
{
      // tell me what they are
	global::client_pointer =
		gdk_device_manager_get_client_pointer(
			gdk_display_get_device_manager( gdk_display_get_default()));

	{
		auto scr = gdk_screen_get_default();

		using global::hdpmm;
		using global::vdpmm;

		hdpmm = (double)gdk_screen_get_width ( scr) / gdk_screen_get_width_mm ( scr);
		vdpmm = (double)gdk_screen_get_height( scr) / gdk_screen_get_height_mm( scr);
		printf( "Screen xdpmm is %g v, %g h\n", hdpmm, vdpmm);
		gdk_screen_set_resolution( scr, (hdpmm + vdpmm)/2);
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








// cairo

void
agh::ui::
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
agh::ui::
cairo_draw_signal( cairo_t *cr, const valarray<TFloat>& V,
		   const ssize_t start, const ssize_t end,
		   const size_t hspan, const float hoff, const float voff, const float scale,
		   const unsigned short decimate,
		   const TDrawSignalDirection direction,
		   const TDrawSignalPathOption continue_path_option)
{
	bool continue_path = continue_path_option == TDrawSignalPathOption::yes;
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
agh::ui::
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
agh::ui::
gtk_combo_box_set_model_properly( GtkComboBox *cb, GtkListStore *m)
{
	gtk_combo_box_set_model( cb, (GtkTreeModel*)m);
	gtk_combo_box_set_id_column( cb, 0);
	gtk_cell_layout_set_renderer( cb);
}


void
agh::ui::
gtk_cell_layout_set_renderer( GtkComboBox *cb)
{
	GtkCellRenderer *r = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(
		(GtkCellLayout*)cb,
		r,
		FALSE);
	gtk_cell_layout_set_attributes(
		(GtkCellLayout*)cb, r,
		"text", 0,
		NULL);
}

void
agh::ui::
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


gint
agh::ui::
pop_question( GtkWindow* parent,
	      const char* primary_text,
	      const char* fmt, ...)
{
	auto W = (GtkMessageDialog*)
		gtk_message_dialog_new_with_markup(
			parent,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_YES_NO,
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

	gint response = gtk_dialog_run( (GtkDialog*)W);
	gtk_widget_destroy( (GtkWidget*)W);

	return response;
}




void
agh::ui::
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

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
