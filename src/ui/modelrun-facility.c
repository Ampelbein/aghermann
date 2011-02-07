// ;-*-C-*- *  Time-stamp: "2011-02-07 01:30:36 hmmr"
/*
 *       File name:  ui/modelrun-facility.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  modelrun facility
 *
 *         License:  GPL
 */


#include <string.h>
#include <math.h>

#include <cairo-svg.h>
#include <glade/glade.h>
#include "../libagh/iface.h"
#include "../libagh/iface-glib.h"
#include "misc.h"
#include "ui.h"


gboolean
	AghSimRunbatchIncludeAllChannels = TRUE,
	AghSimRunbatchIncludeAllSessions = TRUE,
	AghSimRunbatchIterateRanges = FALSE;


GtkWidget
	*wModelRun;

static GtkWidget
	*daModelRunProfile,
	*lModelRunLog,
	*lModelRunIteration,

	*eModelRunVx[_agh_n_tunables_],

	*cModelRunControls;

static GtkTextBuffer
	*__log_text_buffer;



static gdouble __display_factor = .1;

static guint __score_hypn_depth[8] = {
	0, 5, 7, 20, 23, 2, 1, 1
};

static gint __zoomed_episode = -1;






gint
agh_ui_construct_ModelRun( GladeXML *xml)
{
	if ( !(wModelRun		= glade_xml_get_widget( xml, "wModelRun")) ||
	     !(daModelRunProfile	= glade_xml_get_widget( xml, "daModelRunProfile")) ||
	     !(lModelRunLog		= glade_xml_get_widget( xml, "lModelRunLog")) ||
	     !(lModelRunIteration	= glade_xml_get_widget( xml, "lModelRunIteration")) ||
	     !(cModelRunControls	= glade_xml_get_widget( xml, "cModelRunControls")) ||

	     !(eModelRunVx[_rs_]	= glade_xml_get_widget( xml, "eModelRunVrs")) ||
	     !(eModelRunVx[_rc_]	= glade_xml_get_widget( xml, "eModelRunVrc")) ||
	     !(eModelRunVx[_fcR_]	= glade_xml_get_widget( xml, "eModelRunVfcR")) ||
	     !(eModelRunVx[_fcW_]	= glade_xml_get_widget( xml, "eModelRunVfcW")) ||
	     !(eModelRunVx[_S0_]	= glade_xml_get_widget( xml, "eModelRunVS0")) ||
	     !(eModelRunVx[_SU_]	= glade_xml_get_widget( xml, "eModelRunVSU")) ||
	     !(eModelRunVx[_ta_]	= glade_xml_get_widget( xml, "eModelRunVta")) ||
	     !(eModelRunVx[_tp_]	= glade_xml_get_widget( xml, "eModelRunVtp")) ||
	     !(eModelRunVx[_gc1_]	= glade_xml_get_widget( xml, "eModelRunVgc1")) ||
	     !(eModelRunVx[_gc2_]	= glade_xml_get_widget( xml, "eModelRunVgc2")) ||
	     !(eModelRunVx[_gc3_]	= glade_xml_get_widget( xml, "eModelRunVgc3")) ||
	     !(eModelRunVx[_gc4_]	= glade_xml_get_widget( xml, "eModelRunVgc4")) )
		return -1;

	g_object_set( lModelRunLog,
		      "tabs", pango_tab_array_new_with_positions( 6, TRUE,
								  PANGO_TAB_LEFT, 50,
								  PANGO_TAB_LEFT, 150,
								  PANGO_TAB_LEFT, 240,
								  PANGO_TAB_LEFT, 330,
								  PANGO_TAB_LEFT, 420,
								  PANGO_TAB_LEFT, 510),
		      NULL);

	__log_text_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW (lModelRunLog));

	return 0;
}








static GArray
	*__S_course,
	*__SWA_course,
	*__SWA_sim_course,
	*__scores;
static gdouble
	__SWA_avg_value;

static TModelRef
	__model_ref;

static struct SConsumerTunableSet
	__t_set;

static size_t
	__iteration;


static void
__update_infobar()
{
	agh_modelrun_get_tunables( __model_ref, &__t_set);
	for ( gushort t = 0; t < _agh_n_tunables_; ++t )
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eModelRunVx[t]),
					   __t_set.tunables[t] * agh_tunable_get_description(t)->display_scale_factor);

	if ( __iteration == 0 )
		snprintf_buf( "<small>(initial state)</small>");
	else
		snprintf_buf( "Iteration # <b>%zu</b>", __iteration);
	gtk_label_set_markup( GTK_LABEL (lModelRunIteration), __buf__);
}


gboolean
agh_prepare_modelrun_facility( TModelRef modref)
{
	if ( !__S_course ) {
		__S_course       = g_array_new( FALSE, TRUE, sizeof(double));
		__SWA_course     = g_array_new( FALSE, TRUE, sizeof(double));
		__SWA_sim_course = g_array_new( FALSE, TRUE, sizeof(double));
		__scores         = g_array_new( FALSE, FALSE, sizeof(char));
//		__stridelog	 = g_string_new( "");
	}

	int result = agh_modelrun_run( modref);

	switch ( result ) {
	case AGH_SIMPREP_ENOSCORE:
		pop_ok_message( GTK_WINDOW (wMainWindow),
				"Some measurements are not available, or have not been scored.");
		return FALSE;
	case AGH_SIMPREP_EFARAPART:
		pop_ok_message( GTK_WINDOW (wMainWindow),
				"Some measurements in this subject are more than 4 days apart.");
		return FALSE;
	case AGH_SIMPREP_EAMENDMENTS_INEFFECTIVE:
		pop_ok_message( GTK_WINDOW (wMainWindow),
				"DB2 and AZ amendments are ineffective for single-episode profiles."
				"Please switch these options off first.");
		return FALSE;
	case AGH_SIMPREP_ERS_NONSENSICAL:
		pop_ok_message( GTK_WINDOW (wMainWindow),
				"Tuning the rise rate only makes sense for profiles with multiple episodes."
				"(Set rs step to 0 to prevent it from being tuned).");
		return FALSE;
	case 0:
		break;
	default:
		pop_ok_message( GTK_WINDOW (wMainWindow),
				"Failed to set up this modelrun.");
	}

	agh_modelrun_get_all_courses_as_double_garray( __model_ref,
						       __SWA_course, /* these two will be blank here: */ __S_course, __SWA_sim_course,
						       __scores);
	__iteration = 0;
	__update_infobar();

	// g_string_printf( __stridelog,
	// 		 "Subject %s (group %s), Channel %s, Freq. range %g-%g Hz\n",
	// 		 agh_j_name->str, agh_get_subject_group( agh_j_name->str), agh_get_current_channel_name(),
	// 		 agh_sim_get_operating_range_from(), agh_sim_get_operating_range_upto());
	// gtk_text_buffer_set_text( __log_text_buffer, __stridelog->str, -1);

	__zoomed_episode = -1;

	return TRUE;
}






#define HYPN_DEPTH 30
#define LGD_MARGIN 20

#define X_SPC 4

static void
__draw_timeline( cairo_t *cr, guint wd, guint ht)
{
	guint i /*, ii, ix */;

#define X(x) Ai (__SWA_course, double, x)

      // empirical SWA
	guint	cur_ep, cur_ep_start, cur_ep_end,
		n_eps = agh_modelrun_get_n_episodes( __model_ref),
		tl_start = 0, tl_end, tl_len = 0;

	for ( cur_ep = 0; cur_ep < n_eps; cur_ep++ ) {
		if ( !(__zoomed_episode == -1 || __zoomed_episode == cur_ep) )
			continue;

		cur_ep_start = agh_modelrun_get_nth_episode_start_p( __model_ref, cur_ep);
		cur_ep_end   = agh_modelrun_get_nth_episode_end_p  ( __model_ref, cur_ep);

		if ( __zoomed_episode == -1 )
			tl_start = 0, tl_end = tl_len = __SWA_course->len;
		else
			tl_start = cur_ep_start, tl_end = cur_ep_end, tl_len = tl_end - tl_start;

		cairo_move_to( cr, 0.,
			       ht - LGD_MARGIN-HYPN_DEPTH - X(tl_start+0) * (float)ht / __SWA_avg_value * __display_factor);
		for ( i = 1; i < tl_len; ++i )
			cairo_line_to( cr,
				       (gfloat)i / tl_len * wd,
				       ht - LGD_MARGIN-HYPN_DEPTH - X(tl_start+i) * (gfloat)ht / __SWA_avg_value * __display_factor);

		cairo_move_to( cr, (__zoomed_episode == -1) ?(float)cur_ep_start/tl_len * wd :10, 10);
		cairo_show_text( cr,
				 AghEE[cur_ep]);

#undef X
#define X(x) Ai (__scores, gchar, x)
	      // hypnogram
		cairo_set_source_rgba( cr,
				       (double)__fg1__[cHYPNOGRAM_SCORELINE].red/65536,
				       (double)__fg1__[cHYPNOGRAM_SCORELINE].green/65536,
				       (double)__fg1__[cHYPNOGRAM_SCORELINE].blue/65536,
				       1.);
		cairo_set_line_width( cr, 3.);
		for ( i = 0; i < tl_len; ++i ) {
			gchar c;
			if ( (c = Ai (__scores, gchar, tl_start+i)) != AghScoreCodes[AGH_SCORE_NONE] ) {
				gint y = __score_hypn_depth[ SCOREID(c) ];
				cairo_move_to( cr, lroundf( (gfloat) i   /tl_len * wd), y);
				cairo_line_to( cr, lroundf( (gfloat)(i+1)/tl_len * wd), y);
			}
		}
		cairo_stroke( cr);

	}


	//tl_start = 0, tl_end = tl_len = __SWA_course->len;
	//tl_start = cur_ep_start, tl_end = cur_ep_end, tl_len = tl_end - tl_start;
	// rely on tl_* as set in the above loop

#undef X
#define X(x) Ai (__SWA_sim_course, double, (guint)(x))
      // simulated SWA
	if ( __SWA_sim_course->len && __iteration > 0 ) {
		cairo_move_to( cr, 0., ht - LGD_MARGIN-HYPN_DEPTH - X(tl_start+0) * ht / __SWA_avg_value * __display_factor);
		for ( i = 0; i < tl_len; ++i )
			cairo_line_to( cr,
				       (float)i/tl_len * wd,
				       ht - LGD_MARGIN-HYPN_DEPTH - X(tl_start+i) * ht / __SWA_avg_value * __display_factor);
		cairo_stroke( cr);
	}

#undef X
#define X(x) Ai (__S_course, double, (guint)(x))
      // Process S
	if ( __S_course->len && __iteration > 0 ) {
		cairo_move_to( cr, 0., ht - LGD_MARGIN-HYPN_DEPTH - X(tl_start+0) * ht / __SWA_avg_value * __display_factor);
		for ( i = 0; i < tl_len; ++i )
			cairo_line_to( cr, (float)i/tl_len * wd,
				       ht - LGD_MARGIN-HYPN_DEPTH - X(tl_start+i) * ht / __SWA_avg_value * __display_factor);
	}
#undef X


	guint	pph = 3600/agh_modelrun_get_pagesize(__model_ref),
		tick_spc_rough = (float)tl_len/(wd/80) / pph,
		tick_spc;
	guint	sizes[8] = { 0, 1, 2, 3, 4, 6, 12, 24 };
	i = 7;
	do  tick_spc = sizes[i--];  while ( tick_spc_rough < tick_spc && i );
	tick_spc *= pph;

      // ticks
	for ( i = 0; i < tl_len; i += tick_spc ) {
		snprintf_buf( "%d", (i+tl_start)/pph);
		cairo_move_to( cr,
			       (float)i/tl_len * wd,
			       ht - HYPN_DEPTH-LGD_MARGIN + 8);
		cairo_show_text( cr, __buf__);
		cairo_move_to( cr, (float)i/tl_len * wd, ht - LGD_MARGIN-HYPN_DEPTH);
		cairo_rel_line_to( cr, 0., 5);
	}
	cairo_stroke( cr);

      // zeroline
	cairo_move_to( cr, 0., ht-LGD_MARGIN-HYPN_DEPTH + 5);
	cairo_rel_line_to( cr, wd, 0.);

	cairo_stroke( cr);
}


gboolean
daModelRunProfile_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	gint ht, wd;
	gdk_drawable_get_size( wid->window, &wd, &ht);
	cairo_t *cr = gdk_cairo_create( wid->window);
	__draw_timeline( cr, wd, ht);
	cairo_destroy( cr);

	return TRUE;
}





gboolean
daModelRunProfile_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		__display_factor /= 1.5;
	    break;
	case GDK_SCROLL_UP:
		__display_factor *= 1.5;
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




gboolean
daModelRunProfile_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	gint ht, wd;
	gdk_drawable_get_size( wid->window, &wd, &ht);

	switch ( event->button ) {
	case 1:
		if ( event->state & GDK_MOD1_MASK ) {
			GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Export Profile Snapshot",
									    NULL,
									    GTK_FILE_CHOOSER_ACTION_SAVE,
									    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
									    NULL);
			GtkFileFilter *file_filter = gtk_file_filter_new();
			gtk_file_filter_set_name( file_filter, "SVG images");
			gtk_file_filter_add_pattern( file_filter, "*.svg");
			gtk_file_chooser_add_filter( GTK_FILE_CHOOSER (f_chooser), file_filter);
			if ( gtk_dialog_run( GTK_DIALOG (f_chooser)) == GTK_RESPONSE_ACCEPT ) {
				char *fname_ = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (f_chooser));
				snprintf_buf( "%s%s", fname_,
					      g_str_has_suffix( fname_, ".svg") ? "" : ".svg");
				g_free( fname_);
#ifdef CAIRO_HAS_SVG_SURFACE
				cairo_surface_t *cs = cairo_svg_surface_create( __buf__, wd, ht);
				cairo_t *cr = cairo_create( cs);
				__draw_timeline( cr, wd, ht);
				cairo_destroy( cr);
				cairo_surface_destroy( cs);
#endif
			}
			gtk_widget_destroy( f_chooser);
		} else {
			if ( __zoomed_episode == -1 ) {
				gint ep;
				for ( ep = agh_modelrun_get_n_episodes( __model_ref)-1; ep > -1; ep-- )
					if ( event->x/wd * __SWA_course->len > agh_modelrun_get_nth_episode_start_p( __model_ref, ep) ) {
						__zoomed_episode = ep;
						break;
					}
			} else
				__zoomed_episode = -1;
			gtk_widget_queue_draw( wid);
		}
	    break;
	}

	return TRUE;
}










void
bModelRunRun_clicked_cb()
{
	gtk_widget_set_sensitive( cModelRunControls, FALSE);
	set_cursor_busy( TRUE, wModelRun);

      // set tunables per user modification
	for ( guint t = 0; t < _agh_n_tunables_; ++t )
		__t_set.tunables[t] = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eModelRunVx[t]))
			/ agh_tunable_get_description(t)->display_scale_factor;

      // send tunables to ModelRun
	agh_modelrun_put_tunables( __model_ref, &__t_set);

	agh_modelrun_run( __model_ref);

		// n_unstable = agh_sim_modelrun_get_snapshot( __R_ptr, __stride,
		// 					    __SWA_sim_course, __S_course,
		// 					    &__t_set, NULL);

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

		// gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (lModelRunLog), mark,
		// 			      .2, TRUE, 0., 0.5);

	agh_modelrun_get_all_courses_as_double_garray( __model_ref,
						       __SWA_course, __S_course, __SWA_sim_course,
						       __scores);

	gtk_widget_queue_draw( daModelRunProfile);
	__update_infobar();

	// while ( gtk_events_pending () )
	// 	gtk_main_iteration();

	gtk_widget_set_sensitive( cModelRunControls, TRUE);
	set_cursor_busy( FALSE, wModelRun);
}








// void
// bModelRunStrideForward_clicked_cb()
// {
// 	__stride++;
// 	agh_sim_modelrun_get_snapshot( __R_ptr, __stride, __SWA_sim_course, __S_course,
// 				       &__t_set, &__stride);  // if there is no further strides, get_snapshot will
// 							      // revert the increment
// 	gtk_widget_queue_draw( daModelRunProfile);
// 	__update_infobar();

// 	gchar mark_name[6];
// 	snprintf( mark_name, 5, "s%d", __stride);
// 	GtkTextMark *mark = gtk_text_buffer_get_mark( __log_text_buffer, mark_name);
// 	GtkTextIter iter;
// 	if ( mark )
// 		gtk_text_buffer_get_iter_at_mark( __log_text_buffer, &iter, mark);
// //		gtk_text_buffer_insert_at_cursor( __log_text_buffer, ">", -1);
// 	else
// 		gtk_text_buffer_get_end_iter( __log_text_buffer, &iter);
// 	gtk_text_buffer_place_cursor( __log_text_buffer, &iter);

// 	gtk_text_view_place_cursor_onscreen( GTK_TEXT_VIEW (lModelRunLog));
// }




void
bModelRunReset_clicked_cb()
{
	agh_modelrun_reset( __model_ref);
	agh_modelrun_get_tunables( __model_ref, &__t_set);
	__update_infobar();

	agh_modelrun_get_all_courses_as_double_garray( __model_ref,
						       __SWA_course, __S_course, __SWA_sim_course,
						       __scores);
	__iteration = 0;
	gtk_widget_queue_draw( daModelRunProfile);
	gtk_text_buffer_set_text( __log_text_buffer, "", -1);
}






void
bModelRunAccept_clicked_cb()
{
	agh_modelrun_save( __model_ref);

	agh_populate_mSimulations( TRUE);

	gtk_widget_hide( wModelRun);
}




#define __Vx_value_changed(x) \
	__t_set.tunables[x] = gtk_spin_button_get_value(e) / agh_tunable_get_description(x)->display_scale_factor; \
	agh_modelrun_put_tunables( __model_ref, &__t_set);			\
	agh_modelrun_snapshot( __model_ref);					\
        agh_modelrun_get_mutable_courses_as_double_garray( __model_ref,		\
							   __S_course, __SWA_sim_course); \
	gtk_widget_queue_draw( daModelRunProfile);

void eModelRunVrs_value_changed_cb  ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_rs_ ); }
void eModelRunVrc_value_changed_cb  ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_rc_ ); }
void eModelRunVfcR_value_changed_cb ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_fcR_); }
void eModelRunVfcW_value_changed_cb ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_fcW_); }
void eModelRunVS0_value_changed_cb  ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_S0_ ); }
void eModelRunVSU_value_changed_cb  ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_SU_ ); }
void eModelRunVta_value_changed_cb  ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_ta_ ); }
void eModelRunVtp_value_changed_cb  ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_tp_ ); }
void eModelRunVgc1_value_changed_cb ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_gc1_); }
void eModelRunVgc2_value_changed_cb ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_gc2_); }
void eModelRunVgc3_value_changed_cb ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_gc3_); }
void eModelRunVgc4_value_changed_cb ( GtkSpinButton *e, gpointer u)	{ __Vx_value_changed (_gc4_); }


// ---------- colours




// EOF
