// ;-*-C++-*- *  Time-stamp: "2011-07-04 12:49:30 hmmr"
/*
 *       File name:  ui/modelrun-facility_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-03
 *
 *         Purpose:  modelrun facility callbacks
 *
 *         License:  GPL
 */


#include <cairo-svg.h>

#include "misc.hh"
#include "modelrun-facility.hh"


using namespace aghui;

extern "C" {

	gboolean
	daMFProfile_configure_event_cb( GtkWidget *widget, GdkEventConfigure *event, gpointer userdata)
	{
		if ( event->type == GDK_CONFIGURE ) {
			auto& MF = *(SModelrunFacility*)userdata;
			MF.da_ht = event->height;
			MF.da_wd = event->width;
		}
		return FALSE;
	}


	gboolean
	daMFProfile_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
	{
		auto& MF = *(SModelrunFacility*)userdata;
		MF.draw_timeline( cr);

		MF.update_infobar();

		return TRUE;
	}




	gboolean
	daMFProfile_button_press_event_cb( GtkWidget *wid,
					   GdkEventButton *event,
					   gpointer userdata)
	{
		auto& MF = *(SModelrunFacility*)userdata;

		switch ( event->button ) {
		case 1:
			if ( event->state & GDK_MOD1_MASK ) {
				GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Export Profile Snapshot",
										    NULL,
										    GTK_FILE_CHOOSER_ACTION_SAVE,
										    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
										    NULL);
				g_object_ref_sink( f_chooser);
				GtkFileFilter *file_filter = gtk_file_filter_new();
				g_object_ref_sink( file_filter);

				gtk_file_filter_set_name( file_filter, "SVG images");
				gtk_file_filter_add_pattern( file_filter, "*.svg");
				gtk_file_chooser_add_filter( (GtkFileChooser*)f_chooser, file_filter);
				if ( gtk_dialog_run( (GtkDialog*)f_chooser) == GTK_RESPONSE_ACCEPT ) {
					char *fname_ = gtk_file_chooser_get_filename( (GtkFileChooser*)f_chooser);
					snprintf_buf( "%s%s", fname_,
						      g_str_has_suffix( fname_, ".svg") ? "" : ".svg");
					g_free( fname_);
#ifdef CAIRO_HAS_SVG_SURFACE
					cairo_surface_t *cs = cairo_svg_surface_create( __buf__, MF.da_wd, MF.da_ht);
					cairo_t *cr = cairo_create( cs);
					MF.draw_timeline( cr);
					cairo_destroy( cr);
					cairo_surface_destroy( cs);
#endif
				}
				g_object_unref( file_filter);
				g_object_unref( f_chooser);
				gtk_widget_destroy( f_chooser);
			} else {
				if ( MF.zoomed_episode == -1 ) {
					for ( int ep = MF.csimulation.mm_list().size()-1; ep > -1; --ep )
						if ( event->x/MF.da_wd * MF.csimulation.timeline().size() >
						     MF.csimulation.nth_episode_start_page( ep) ) {
							MF.zoomed_episode = ep;
							break;
						}
				} else
					MF.zoomed_episode = -1;
				gtk_widget_queue_draw( wid);
			}
			break;
		}

		return TRUE;
	}





	gboolean
	daMFProfile_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	{
		auto& MF = *(SModelrunFacility*)userdata;

		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			MF.display_factor /= 1.1;
		    break;
		case GDK_SCROLL_UP:
			MF.display_factor *= 1.1;
		    break;
		case GDK_SCROLL_LEFT:
		    break;
		case GDK_SCROLL_RIGHT:
		    break;
		}

		if ( event->state & GDK_CONTROL_MASK ) {
			;
		} else
			;

		gtk_widget_queue_draw( wid);

		return TRUE;
	}








	void
	bMFRun_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& MF = *(SModelrunFacility*)userdata;

		if ( __MF != NULL ) {
			pop_ok_message( MF.wModelrunFacility,
					"Another instance of Modelrun Facility is currently busy running simulations;"
					" please wait until it completes.");
			return;
		}
		__MF = &MF;

		gtk_widget_set_sensitive( (GtkWidget*)MF.cMFControls, FALSE);
		set_cursor_busy( true, (GtkWidget*)MF.wModelrunFacility);

		// tunables have been set live

		MF.csimulation.watch_simplex_move(
			gtk_toggle_button_get_active( (GtkToggleButton*)MF.eMFLiveUpdate)
			? MF.MF_siman_param_printer : NULL);

		// GtkTextMark *mark = gtk_text_buffer_get_insert( __log_text_buffer);
		// GtkTextIter iter;
		// gtk_text_buffer_get_iter_at_mark( __log_text_buffer, &iter, mark);

		// GtkTextIter iter_end;
		// gtk_text_buffer_get_end_iter( __log_text_buffer, &iter_end);
		// gtk_text_buffer_delete( __log_text_buffer, &iter, &iter_end);

		// gchar mark_name[6];
		// snprintf( mark_name, 5, "s%d", __stride-1);
		// mark = gtk_text_buffer_create_mark( __log_text_buffer, mark_name, &iter, TRUE);
		// gtk_text_buffer_insert_at_cursor( __log_text_buffer, __stridelog->str, -1);

		// gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (lMFLog), mark,
		// 			      .2, TRUE, 0., 0.5);

		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
		MF.update_infobar();

		gtk_widget_set_sensitive( (GtkWidget*)MF.cMFControls, TRUE);
		set_cursor_busy( FALSE, (GtkWidget*)MF.wModelrunFacility);

		__MF = NULL;
	}







	void
	bMFReset_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& MF = *(SModelrunFacility*)userdata;

		printf( "Don't know what to do here\n");
		MF.update_infobar();

		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
		gtk_text_buffer_set_text( MF.log_text_buffer, "", -1);
	}






	void
	bMFAccept_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& MF = *(SModelrunFacility*)userdata;

		if ( MF.csimulation.status & agh::CModelRun::modrun_tried ) {
//		agh_modelrun_save( __modrun_ref);
			MF._p.populate_2();
		}

		delete &MF;
	}




	void
	eMFVx_value_changed_cb( GtkSpinButton* e, gpointer u)
	{
		auto& MF = *(SModelrunFacility*)u;
		if ( !MF._suppress_Vx_value_changed ) {
			agh::TTunable t = MF.eMFVx[e];
			MF.csimulation.cur_tset[t] =
				gtk_spin_button_get_value(e)
				/ agh::STunableSet::stock[(agh::TTunable_underlying_type)t].display_scale_factor;
			MF.snapshot();
			gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
		}
	}



	void
	wModelrunFacility_delete_event_cb( GtkWidget *widget, GdkEvent *event, gpointer userdata)
	{
		auto& MF = *(SModelrunFacility*)userdata;
		delete &MF;
	}

}  // extern "C"

// eof
