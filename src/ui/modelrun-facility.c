// ;-*-C-*- *  Time-stamp: "2011-03-10 01:14:25 hmmr"
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


#include <assert.h>
#include <string.h>
#include <math.h>

#include <cairo-svg.h>
#include <glade/glade.h>
#include "../libagh/iface.h"
#include "../libagh/iface-glib.h"
#include "misc.h"
#include "ui.h"
#include "settings.h"


gboolean
	AghSimRunbatchIncludeAllChannels = TRUE,
	AghSimRunbatchIncludeAllSessions = TRUE,
	AghSimRunbatchIterateRanges = FALSE;


GtkWidget
	*wModelRun;

static GtkWidget
	*daModelRunProfile,
	*lModelRunLog,
	*eModelRunLiveUpdate,
	*eModelRunVx[_agh_n_tunables_],
	*cModelRunControls,
	*lModelRunCF;

GtkWidget
	*bColourSWA,
	*bColourSWASim,
	*bColourProcessS,
	*bColourPaperMR,
	*bColourTicksMR,
	*bColourLabelsMR;

static GtkTextBuffer
	*__log_text_buffer;



static gdouble __display_factor = 1;

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
	     !(eModelRunLiveUpdate	= glade_xml_get_widget( xml, "eModelRunLiveUpdate")) ||
	     !(cModelRunControls	= glade_xml_get_widget( xml, "cModelRunControls")) ||
	     !(lModelRunCF		= glade_xml_get_widget( xml, "lModelRunCF")) ||

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

      // ------ colours
	if ( !(bColourSWA	= glade_xml_get_widget( xml, "bColourSWA")) ||
	     !(bColourSWASim	= glade_xml_get_widget( xml, "bColourSWASim")) ||
	     !(bColourProcessS	= glade_xml_get_widget( xml, "bColourProcessS")) ||
	     !(bColourPaperMR	= glade_xml_get_widget( xml, "bColourPaperMR")) ||
	     !(bColourTicksMR	= glade_xml_get_widget( xml, "bColourTicksMR")) ||
	     !(bColourLabelsMR	= glade_xml_get_widget( xml, "bColourLabelsMR")) )
		return -1;

	return 0;
}








static TModelRef
	__modrun_ref;
static size_t
	__pagesize;
static struct SConsumerTunableSet
	__t_set;
static double
	__cf = NAN;

static size_t
	__n_episodes,
	__timeline_pages;
static double
	*__S_course = NULL,
	*__SWA_course = NULL,
	*__SWA_sim_course = NULL;
static char
	*__scores;

static double
	__SWA_max_value;

static size_t
	__smooth_SWA_course = 2;  // one side

static int
	__suppress_Vx_value_changed;

static void __update_infobar();

gboolean
agh_prepare_modelrun_facility( TModelRef modref)
{
	__modrun_ref = modref;
	__pagesize = agh_modelrun_get_pagesize(__modrun_ref);
	__n_episodes = agh_modelrun_get_n_episodes( __modrun_ref);

	const struct SConsumerSCourseSetupInfo *sc_info =
		agh_modelrun_get_scourse_setup_info( modref, NULL);
	snprintf_buf( "sim start at p. %zu, end at p. %zu, baseline end at p. %zu,\n"
		      "%zu pp with SWA, %zu pp in bed;\n"
		      "SWA_L = %g, SWA_0 = %g, SWA_100 = %g\n",
		      sc_info->_sim_start, sc_info->_sim_end, sc_info->_baseline_end,
		      sc_info->_pages_with_SWA, sc_info->_pages_in_bed,
		      sc_info->_SWA_L, sc_info->_SWA_0,	sc_info->_SWA_100);
	gtk_text_buffer_set_text( __log_text_buffer, __buf__, -1);

      // do a single cycle to recreate mutable courses
	__cf = agh_modelrun_snapshot( __modrun_ref);

      // get all courses
	__timeline_pages =
		agh_modelrun_get_all_courses_as_double( __modrun_ref,
							&__SWA_course,
							&__S_course, &__SWA_sim_course, // these two will be blank here:
							&__scores);
      // determine SWA_max, for scaling purposes;
	__SWA_max_value = 0.;
	for ( size_t p = 0; p < __timeline_pages; ++p )
		if ( __SWA_course[p] > __SWA_max_value )
			__SWA_max_value = __SWA_course[p];

      // also smooth the SWA course
	if ( __smooth_SWA_course ) {
		double *tmp = (double*)malloc( __timeline_pages * sizeof(double));
		assert (tmp);
		for ( size_t p = 0; p < __timeline_pages; ++p )
			if ( p < __smooth_SWA_course || p >= __timeline_pages-1 - __smooth_SWA_course )
				tmp[p] = __SWA_course[p];
			else {
				double sum = 0.;
				for ( size_t q = p - __smooth_SWA_course; q <= p + __smooth_SWA_course; ++q )
					sum += __SWA_course[q];
				tmp[p] = sum / (2 * __smooth_SWA_course + 1);
			}
		memcpy( __SWA_course, tmp, __timeline_pages * sizeof(double));
		free( tmp);
	}

	__suppress_Vx_value_changed = 1;
	__update_infobar();
	__suppress_Vx_value_changed = 0;

	__zoomed_episode = -1;
	gtk_widget_queue_draw( daModelRunProfile);

	return TRUE;
}




static void
__update_infobar()
{
	agh_modelrun_get_tunables( __modrun_ref, &__t_set);
	for ( gushort t = 0; t < _agh_n_tunables_; ++t )
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eModelRunVx[t]),
					   __t_set.tunables[t] * agh_tunable_get_description(t)->display_scale_factor);

	snprintf_buf( "CF = <b>%g</b>\n", __cf);
	gtk_label_set_markup( GTK_LABEL (lModelRunCF), __buf__);
}



#define HYPN_DEPTH 30
#define LGD_MARGIN 20

#define TL_PAD  20
#define WD (wd - 2*TL_PAD)

#define X_SPC 4

static void
__draw_episode_empSWA( cairo_t *cr, guint wd, guint ht,
		       size_t ep,
		       size_t tl_start, size_t tl_len,
		       size_t ep_start, size_t ep_end)
{
	size_t i;

	if ( __zoomed_episode != -1 ) {
		cairo_set_source_rgb( cr,
				      (double)__bg2__[cPAPER_MR].red/65535,
				      (double)__bg2__[cPAPER_MR].green/65535,
				      (double)__bg2__[cPAPER_MR].blue/65535);
		cairo_rectangle( cr, 0., 0., wd, ht);
		cairo_fill( cr);
		cairo_stroke( cr);
	}

	cairo_set_line_width( cr, .5);
	cairo_set_source_rgba( cr,
			       (double)__fg2__[cSWA].red/65535,
			       (double)__fg2__[cSWA].green/65535,
			       (double)__fg2__[cSWA].blue/65535,
			       1.);

	cairo_move_to( cr, TL_PAD + (float)(ep_start - tl_start) / tl_len * WD,
		       ht - LGD_MARGIN-HYPN_DEPTH
		       - __SWA_course[ep_start] / __SWA_max_value * (float)ht * __display_factor);
	for ( i = 1; i < ep_end - ep_start; ++i )
		cairo_line_to( cr,
			       TL_PAD + (float)(ep_start - tl_start + i) / tl_len * WD,
			       ht - LGD_MARGIN-HYPN_DEPTH
			       - __SWA_course[ep_start + i] * (float)ht / __SWA_max_value * __display_factor);

	cairo_stroke( cr);

	cairo_set_source_rgba( cr, 0., 0., 0., .6);
	cairo_set_font_size( cr, (__zoomed_episode == -1 ) ? 9 : 14);
	cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_move_to( cr, TL_PAD + (float)(ep_start - tl_start)/tl_len * WD, 16);
	cairo_show_text( cr, AghEE[ep]);
	cairo_stroke( cr);

      // simulated SWA
	cairo_set_line_width( cr, 2);
	cairo_set_source_rgba( cr,
			       (double)__fg2__[cSWA_SIM].red/65535,
			       (double)__fg2__[cSWA_SIM].green/65535,
			       (double)__fg2__[cSWA_SIM].blue/65535,
			       .5);
	cairo_move_to( cr, TL_PAD + (float)(ep_start - tl_start) / tl_len * WD,
		       ht - LGD_MARGIN-HYPN_DEPTH
		       - __SWA_sim_course[ep_start] * ht / __SWA_max_value * __display_factor);
	for ( i = 1; i < ep_end - ep_start; ++i )
		cairo_line_to( cr,
			       TL_PAD + (float)(ep_start - tl_start + i) / tl_len * WD,
			       ht - LGD_MARGIN-HYPN_DEPTH
			       - __SWA_sim_course[ep_start + i] * ht / __SWA_max_value * __display_factor);
	cairo_stroke( cr);

      // Process S
	// draw only for zoomed episode: else it is drawn for all in one go
	if ( __zoomed_episode != -1 ) {
		cairo_set_line_width( cr, 2.);
		cairo_set_source_rgba( cr,
				       (double)__fg2__[cPROCESS_S].red/65535,
				       (double)__fg2__[cPROCESS_S].green/65535,
				       (double)__fg2__[cPROCESS_S].blue/65535,
				       0.6);
		cairo_move_to( cr, TL_PAD + (float)(ep_start - tl_start) / tl_len * WD,
			       ht - LGD_MARGIN-HYPN_DEPTH
			       - __S_course[ep_start] * ht / __SWA_max_value * __display_factor);
		size_t possible_end = ep_end - ep_start +
			((__zoomed_episode == __n_episodes - 1) ? 0 : ((float)__timeline_pages/WD * TL_PAD));
		for ( i = 1; i < possible_end; ++i )
			cairo_line_to( cr,
				       TL_PAD + (float)(ep_start - tl_start + i) / tl_len * WD,
				       ht - LGD_MARGIN-HYPN_DEPTH
				       - __S_course[ep_start + i] * ht / __SWA_max_value * __display_factor);
		cairo_stroke( cr);
	}

      // hypnogram
	cairo_set_source_rgba( cr, 0., 0., 0., .4);
	cairo_set_line_width( cr, 3.);
	for ( i = 0; i < ep_end - ep_start; ++i ) {
		char c;
		if ( (c = __scores[ep_start + i]) != AghScoreCodes[AGH_SCORE_NONE] ) {
			int y = __score_hypn_depth[ SCOREID(c) ];
			cairo_move_to( cr, TL_PAD + (float)(ep_start - tl_start + i  ) / tl_len * WD,
				       ht - HYPN_DEPTH + y);
			cairo_rel_line_to( cr, 1. / tl_len * WD, 0);
			cairo_stroke( cr);
		}
	}
}


static void
__draw_ticks( cairo_t *cr, guint wd, guint ht,
	      size_t start, size_t end)
{
      // ticks
	guint	pph = 3600/__pagesize,
		pps = pph/2;
	float	tick_spc_rough = (float)(end-start)/(wd/120.) / pph,
		tick_spc;
	float	sizes[] = { NAN, .25, .5, 1, 2, 3, 4, 6, 12 };
	size_t i = 8;
	while ( i > 0 && (tick_spc = sizes[i]) > tick_spc_rough )
		--i;
	tick_spc *= pph;

	cairo_set_font_size( cr, 9);
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	start = start/pps * pps;  // align to 30 min
	for ( i = start; i < end; i += (unsigned)tick_spc ) {
		cairo_set_source_rgba( cr,
				       (double)__fg2__[cTICKS_MR].red/65535,
				       (double)__fg2__[cTICKS_MR].green/65535,
				       (double)__fg2__[cTICKS_MR].blue/65535,
				       .4);
		cairo_set_line_width( cr, (i % (24*pph) == 0) ? 1 : .3);
		cairo_move_to( cr, (float)(i-start)/(end-start) * WD, 0);
		cairo_rel_line_to( cr, 0., ht);
		cairo_stroke( cr);

		cairo_set_source_rgba( cr,
				       (double)__fg2__[cLABELS_MR].red/65535,
				       (double)__fg2__[cLABELS_MR].green/65535,
				       (double)__fg2__[cLABELS_MR].blue/65535,
				       .4);
		cairo_move_to( cr,
			       (float)(i-start)/(end-start) * WD + 2,
			       ht - HYPN_DEPTH-LGD_MARGIN + 14);
		snprintf_buf_ts_h( (double)i/pph);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}
}


static void
__draw_timeline( cairo_t *cr, guint wd, guint ht)
{
      // empirical SWA
	size_t	cur_ep;

	if ( __zoomed_episode != -1 ) {
		size_t	ep_start = agh_modelrun_get_nth_episode_start_p( __modrun_ref, __zoomed_episode),
			ep_end   = agh_modelrun_get_nth_episode_end_p  ( __modrun_ref, __zoomed_episode);
		__draw_episode_empSWA( cr, wd, ht,
				       __zoomed_episode,
				       ep_start, ep_end - ep_start,
				       ep_start, ep_end);
		__draw_ticks( cr, wd, ht, ep_start, ep_end);
	} else {
	      // draw day and night

		{
			const struct SSubject *hold_j =
				agh_subject_find_by_name(
					agh_modelrun_get_subject( __modrun_ref), NULL);
			time_t	timeline_start = hold_j->sessions[AghDi].episodes[0].start_rel,
				timeline_end = hold_j->sessions[AghDi].episodes[__n_episodes-1].end_rel;

			cairo_pattern_t *cp = cairo_pattern_create_linear( 0., 0., wd, 0);
			struct tm clock_time;
			memcpy( &clock_time, localtime( &timeline_start), sizeof(clock_time));
			clock_time.tm_hour = 4;
			clock_time.tm_min = clock_time.tm_sec = 0;
			time_t	dawn = mktime( &clock_time),
				t;
			gboolean up = TRUE;

			for ( t = dawn; t < timeline_end; t += 3600 * 12, up = !up )
				if ( t > timeline_start )
					cairo_pattern_add_color_stop_rgba( cp,
									   (difftime( t, timeline_start)/(timeline_end-timeline_start)),
									   up?.7:.8, up?.6:.8, up?1.:.8, .5);
			cairo_set_source( cr, cp);
			cairo_rectangle( cr, 0., 0., wd, ht);
			cairo_fill( cr);
			cairo_stroke( cr);
			cairo_pattern_destroy( cp);
		}
	      // draw episodes

		for ( cur_ep = 0; cur_ep < __n_episodes; ++cur_ep )
			__draw_episode_empSWA( cr, wd, ht,
					       cur_ep,
					       0, __timeline_pages,
					       agh_modelrun_get_nth_episode_start_p( __modrun_ref, cur_ep),
					       agh_modelrun_get_nth_episode_end_p  ( __modrun_ref, cur_ep));
	      // Process S in one go for the entire timeline
		cairo_set_line_width( cr, 2.);
		cairo_set_source_rgba( cr,
				       (double)__fg2__[cPROCESS_S].red/65535,
				       (double)__fg2__[cPROCESS_S].green/65535,
				       (double)__fg2__[cPROCESS_S].blue/65535,
				       0.6);
		cairo_move_to( cr, TL_PAD + 0,
			       ht - LGD_MARGIN-HYPN_DEPTH
			       - __S_course[0] * ht / __SWA_max_value * __display_factor);
		for ( size_t i = 1; i < __timeline_pages; ++i )
			cairo_line_to( cr,
				       TL_PAD + (float)i / __timeline_pages * WD,
				       ht - LGD_MARGIN-HYPN_DEPTH
				       - __S_course[i] * ht / __SWA_max_value * __display_factor);
		cairo_stroke( cr);

		__draw_ticks( cr, wd, ht, 0, __timeline_pages);
	}


      // zeroline
	cairo_set_line_width( cr, .3);
	cairo_set_source_rgb( cr, 0, 0, 0);
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

	__update_infobar();

	return TRUE;
}




gboolean
daModelRunProfile_button_press_event_cb( GtkWidget *wid,
					 GdkEventButton *event,
					 gpointer userdata)
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
			g_object_ref_sink( f_chooser);
			GtkFileFilter *file_filter = gtk_file_filter_new();
			g_object_ref_sink( file_filter);
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
			g_object_unref( file_filter);
			g_object_unref( f_chooser);
			gtk_widget_destroy( f_chooser);
		} else {
			if ( __zoomed_episode == -1 ) {
				int ep;
				for ( ep = __n_episodes-1; ep > -1; --ep )
					if ( event->x/wd * __timeline_pages >
					     agh_modelrun_get_nth_episode_start_p( __modrun_ref, ep) ) {
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





gboolean
daModelRunProfile_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		__display_factor /= 1.1;
	    break;
	case GDK_SCROLL_UP:
		__display_factor *= 1.1;
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
siman_param_printer( void *xp)
{
	memcpy( __t_set.tunables, xp, __t_set.n_tunables * sizeof(double));
	agh_modelrun_get_mutable_courses_as_double_direct( __modrun_ref,
							   __S_course, __SWA_sim_course);
	gtk_widget_queue_draw( daModelRunProfile);
	__update_infobar();
	while ( gtk_events_pending() )
		gtk_main_iteration();
}



void
bModelRunRun_clicked_cb()
{
	gtk_widget_set_sensitive( cModelRunControls, FALSE);
	set_cursor_busy( TRUE, wModelRun);

	// tunables have been set live

	// specially treat ctlparams: as modruns are set up once, any
        // user modifications on ctlparams tab are ignored until modruns are cleaned up and re-set up;
        // so inject expdesign mother ctlparams into individual modruns here
	{
		struct SConsumerCtlParams c;
		agh_ctlparams0_get( &c);
		agh_modelrun_put_ctlparams( __modrun_ref, &c);
	}
	agh_modelrun_run( __modrun_ref,
			  gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (eModelRunLiveUpdate))
			  ? siman_param_printer : NULL);

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

	agh_modelrun_get_mutable_courses_as_double_direct( __modrun_ref,
							   __S_course, __SWA_sim_course);
	gtk_widget_queue_draw( daModelRunProfile);
	__update_infobar();

	gtk_widget_set_sensitive( cModelRunControls, TRUE);
	set_cursor_busy( FALSE, wModelRun);
}







void
bModelRunReset_clicked_cb()
{
	agh_modelrun_reset( __modrun_ref);
	agh_modelrun_get_tunables( __modrun_ref, &__t_set);
	__update_infobar();

	agh_modelrun_get_mutable_courses_as_double_direct( __modrun_ref,
							   __S_course, __SWA_sim_course);
	gtk_widget_queue_draw( daModelRunProfile);
	gtk_text_buffer_set_text( __log_text_buffer, "", -1);
}






void
bModelRunAccept_clicked_cb()
{
	if ( agh_modelrun_get_status( __modrun_ref) & AGH_MODRUN_TRIED ) {
		agh_modelrun_save( __modrun_ref);
		agh_populate_mSimulations( TRUE);
	}

	gtk_widget_hide( wModelRun);
}




#define __Vx_value_changed(x) \
	if ( !__suppress_Vx_value_changed ) { \
		__t_set.tunables[x] = gtk_spin_button_get_value(e) / agh_tunable_get_description(x)->display_scale_factor; \
		agh_modelrun_put_tunables( __modrun_ref, &__t_set);			\
		__cf = agh_modelrun_snapshot( __modrun_ref); \
	        agh_modelrun_get_mutable_courses_as_double_direct( __modrun_ref,		\
							   __S_course, __SWA_sim_course); \
		gtk_widget_queue_draw( daModelRunProfile); \
	}

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






void
wModelRun_delete_event_cb()
{
	free( __S_course),	 __S_course	  = NULL;
	free( __SWA_course),	 __SWA_course	  = NULL;
	free( __SWA_sim_course), __SWA_sim_course = NULL;
	free( __scores),	 __scores	  = NULL;
	gtk_widget_hide( wModelRun);
}





// ---------- colours

GdkColor
	__fg2__[cTOTAL_MR],
	__bg2__[cTOTAL_MR];


static void
change_fg_colour( guint c, GtkColorButton *cb)
{
	gdk_colormap_free_colors( agh_cmap, &__fg2__[c], 1);
	gtk_color_button_get_color( cb, &__fg2__[c]);
	gdk_colormap_alloc_color( agh_cmap, &__fg2__[c], FALSE, TRUE);
}
static void
change_bg_colour( guint c, GtkColorButton *cb)
{
	gdk_colormap_free_colors( agh_cmap, &__bg2__[c], 1);
	gtk_color_button_get_color( cb, &__bg2__[c]);
	gdk_colormap_alloc_color( agh_cmap, &__bg2__[c], FALSE, TRUE);

	__fg2__[c] = *contrasting_to( &__bg2__[c]);
//	printf( "%4d:  %5d %5d %5d :: %5d %5d %5d\n", c, __bg1__[c].red, __bg1__[c].green, __bg1__[c].blue, __fg1__[c].red, __fg1__[c].green, __fg1__[c].blue);
}


void
bColourSWA_color_set_cb( GtkColorButton *widget,
			 gpointer        user_data)
{
	change_fg_colour( cSWA, widget);
}


void
bColourSWASim_color_set_cb( GtkColorButton *widget,
			    gpointer        user_data)
{
	change_fg_colour( cSWA_SIM, widget);
}

void
bColourProcessS_color_set_cb( GtkColorButton *widget,
			      gpointer        user_data)
{
	change_fg_colour( cPROCESS_S, widget);
}

void
bColourPaperMR_color_set_cb( GtkColorButton *widget,
			     gpointer        user_data)
{
	change_bg_colour( cPAPER_MR, widget);
}

void
bColourTicksMR_color_set_cb( GtkColorButton *widget,
			     gpointer        user_data)
{
	change_fg_colour( cTICKS_MR, widget);
}


void
bColourLabelsMR_color_set_cb( GtkColorButton *widget,
			      gpointer        user_data)
{
	change_fg_colour( cLABELS_MR, widget);
}







// EOF
