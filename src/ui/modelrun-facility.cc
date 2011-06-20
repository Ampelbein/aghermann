// ;-*-C++-*- *  Time-stamp: "2011-06-21 02:08:35 hmmr"
/*
 *       File name:  ui/modelrun-facility.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  modelrun facility
 *
 *         License:  GPL
 */


#include <cassert>
#include <cstring>

#include <cairo-svg.h>

#include "misc.hh"
#include "ui.hh"
#include "settings.hh"
#include "simulations.hh"
#include "modelrun-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif


using namespace std;

namespace aghui {
namespace mf {

inline namespace {
	unsigned short __score_hypn_depth[8] = {
		0, 20, 23, 30, 33, 5, 10, 1
	};
}


SModelrunFacility::SModelrunFacility( agh::CSimulation& csim)
  : csimulation (csim),
// subject is known only by name, so look up his full object now
    csubject (AghCC->subject_by_x( csim.subject())),
    // not sure we need this though
    display_factor (1.),
    zoomed_episode (-1),
    SWA_smoothover (0)
{

	if ( construct_widgets() )
		throw runtime_error( "SModelrunFacility::SModelrunFacility(): Failed to construct own wisgets");

	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_file( builder, PACKAGE_DATADIR "/" PACKAGE "/ui/agh-ui-mf.glade" , NULL) ) {
		g_object_unref( (GObject*)builder);
		throw runtime_error( "SModelrunFacility::SModelrunFacility(): Failed to load GtkBuilder object");
	}
	if ( construct_widgets() )
		throw runtime_error( "SModelrunFacility::SModelrunFacility(): Failed to construct own widgets");

      // do a single cycle to produce SWA_sim and Process S
	cf = csim.snapshot();

	snprintf_buf( "sim start at p. %zu, end at p. %zu, baseline end at p. %zu,\n"
		      "%zu pp with SWA, %zu pp in bed;\n"
		      "SWA_L = %g, SWA_0 = %g, SWA_100 = %g\n",
		      csim.sim_start(), csim.sim_end(), csim.baseline_end(),
		      csim.pages_with_swa(), csim.pages_in_bed(),
		      csim.SWA_L(), csim.SWA_0(), csim.SWA_100());
	gtk_text_buffer_set_text( log_text_buffer, __buf__, -1);

      // determine SWA_max, for scaling purposes;
	SWA_max = 0.;
	for ( size_t p = 0; p < csim.timeline().size(); ++p )
		if ( csim[p].SWA > SWA_max )
			SWA_max = csim[p].SWA;

      // // also smooth the SWA course
      // 	if ( SWA_smoothover ) {
      // 		for ( size_t p = 0; p < __timeline_pages; ++p )
      // 			if ( p < __smooth_SWA_course || p >= __timeline_pages-1 - __smooth_SWA_course )
      // 				tmp[p] = __SWA_course[p];
      // 			else {
      // 				double sum = 0.;
      // 				for ( size_t q = p - __smooth_SWA_course; q <= p + __smooth_SWA_course; ++q )
      // 					sum += __SWA_course[q];
      // 				tmp[p] = sum / (2 * __smooth_SWA_course + 1);
      // 			}
      // 		memcpy( __SWA_course, tmp, __timeline_pages * sizeof(double));
      // 	}

	_suppress_Vx_value_changed = true;
	update_infobar();
	_suppress_Vx_value_changed = false;

	gtk_widget_queue_draw( (GtkWidget*)daMFProfile);

	snprintf_buf( "Simulation: %s (%s) in %s, %g-%g Hz",
		      csim.subject(),
		      AghD(), AghH(), csim.freq_from(), csim.freq_upto());
	gtk_window_set_title( wModelrunFacility,
			      __buf__);
	gtk_window_set_default_size( wModelrunFacility,
				     gdk_screen_get_width( gdk_screen_get_default()) * .80,
				     gdk_screen_get_height( gdk_screen_get_default()) * .6);
	gtk_widget_show_all( (GtkWidget*)wModelrunFacility);
}


SModelrunFacility::~SModelrunFacility()
{
	gtk_widget_destroy( (GtkWidget*)wModelrunFacility);
	g_object_unref( (GObject*)builder);
}


void
SModelrunFacility::siman_param_printer( void *xp)
{
//	memcpy( __t_set.tunables, xp, __t_set.n_tunables * sizeof(double));
	// access this directly, no?
	gtk_widget_queue_draw( (GtkWidget*)daMFProfile);
	update_infobar();
	while ( gtk_events_pending() )
		gtk_main_iteration();
}


SModelrunFacility*
	__MF;

void
MF_siman_param_printer( void *xp)
{
	__MF -> siman_param_printer( xp);
}





void
SModelrunFacility::draw_timeline( cairo_t *cr)
{
      // empirical SWA
	size_t	cur_ep;

	if ( zoomed_episode != -1 ) {
		size_t	ep_start = csimulation.nth_episode_start_page( zoomed_episode),
			ep_end   = csimulation.nth_episode_end_page  ( zoomed_episode);
		draw_episode( cr,
			      zoomed_episode,
			      ep_start, ep_end - ep_start,
			      ep_start, ep_end);
		draw_ticks( cr, ep_start, ep_end);
	} else {
	      // draw day and night
		{
			time_t	timeline_start = csimulation.mm_list().front()->start(),
				timeline_end   = csimulation.mm_list().back()->end();

			cairo_pattern_t *cp = cairo_pattern_create_linear( 0., 0., da_wd, 0);
			struct tm clock_time;
			memcpy( &clock_time, localtime( &timeline_start), sizeof(clock_time));
			clock_time.tm_hour = 4;
			clock_time.tm_min = clock_time.tm_sec = 0;
			time_t	dawn = mktime( &clock_time),
				t;
			bool up = true;

			for ( t = dawn; t < timeline_end; t += 3600 * 12, up = !up )
				if ( t > timeline_start )
					cairo_pattern_add_color_stop_rgba( cp,
									   (difftime( t, timeline_start)/(timeline_end-timeline_start)),
									   up?.7:.8, up?.6:.8, up?1.:.8, .5);
			cairo_set_source( cr, cp);
			cairo_rectangle( cr, 0., 0., da_wd, da_ht);
			cairo_fill( cr);
			cairo_stroke( cr);
			cairo_pattern_destroy( cp);
		}
	      // draw episodes

		for ( cur_ep = 0; cur_ep < csimulation.mm_list().size(); ++cur_ep )
			draw_episode( cr,
				      cur_ep,
				      0, csimulation.timeline().size(),
				      csimulation.nth_episode_start_page( cur_ep),
				      csimulation.nth_episode_end_page  ( cur_ep));
	      // Process S in one go for the entire timeline
		cairo_set_line_width( cr, 2.);
		CwB[TColour::process_s].set_source_rgba( cr);
		cairo_move_to( cr, tl_pad + 0,
			       da_ht - lgd_margin-hypn_depth
			       - csimulation[0].S * da_ht / SWA_max * display_factor);
		for ( size_t i = 1; i < csimulation.timeline().size(); ++i )
			cairo_line_to( cr,
				       tl_pad + (float)i / csimulation.timeline().size() * da_wd_actual(),
				       da_ht - lgd_margin-hypn_depth
				       - csimulation[i].S * da_ht / SWA_max * display_factor);
		cairo_stroke( cr);

		draw_ticks( cr, 0, csimulation.timeline().size());
	}


      // zeroline
	cairo_set_line_width( cr, .3);
	cairo_set_source_rgb( cr, 0, 0, 0);
	cairo_move_to( cr, 0., da_ht-lgd_margin-hypn_depth + 5);
	cairo_rel_line_to( cr, da_wd, 0.);

	cairo_stroke( cr);
}





void
SModelrunFacility::draw_episode( cairo_t *cr,
				 size_t ep,
				 size_t ep_start, size_t ep_end,
				 size_t tl_start, size_t tl_end)
{
	size_t i;

	if ( zoomed_episode != -1 ) {
		CwB[TColour::paper_mr].set_source_rgb( cr);
		cairo_rectangle( cr, 0., 0., da_wd, da_ht);
		cairo_fill( cr);
		cairo_stroke( cr);
	}

	cairo_set_line_width( cr, .5);
	CwB[TColour::swa].set_source_rgba( cr, 1.);

	size_t tl_len = tl_end - tl_start;
	cairo_move_to( cr, tl_pad + (float)(ep_start - tl_start) / tl_len * da_wd_actual(),
		       da_ht - lgd_margin-hypn_depth
		       - csimulation[ep_start].SWA / SWA_max * (float)da_ht * display_factor);
	for ( i = 1; i < ep_end - ep_start; ++i )
		cairo_line_to( cr,
			       tl_pad + (float)(ep_start - tl_start + i) / tl_len * da_wd_actual(),
			       da_ht - lgd_margin-hypn_depth
			       - csimulation[ep_start + i].SWA * (float)da_ht / SWA_max * display_factor);

	cairo_stroke( cr);

	cairo_set_source_rgba( cr, 0., 0., 0., .6);
	cairo_set_font_size( cr, (zoomed_episode == -1 ) ? 9 : 14);
	cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_move_to( cr, tl_pad + (float)(ep_start - tl_start)/tl_len * da_wd_actual(), 16);
	cairo_show_text( cr, csimulation.mm_list()[ep]->source().episode.c_str());
	cairo_stroke( cr);

      // simulated SWA
	cairo_set_line_width( cr, 2);
	CwB[TColour::swa_sim].set_source_rgba( cr);
	cairo_move_to( cr, tl_pad + (float)(ep_start - tl_start) / tl_len * da_wd_actual(),
		       da_ht - lgd_margin-hypn_depth
		       - csimulation[ep_start].SWA_sim * da_ht / SWA_max * display_factor);
	for ( i = 1; i < ep_end - ep_start; ++i )
		cairo_line_to( cr,
			       tl_pad + (float)(ep_start - tl_start + i) / tl_len * da_wd_actual(),
			       da_ht - lgd_margin-hypn_depth
			       - csimulation[ep_start + i].SWA_sim * da_ht / SWA_max * display_factor);
	cairo_stroke( cr);

      // Process S
	// draw only for zoomed episode: else it is drawn for all in one go
	if ( zoomed_episode != -1 ) {
		cairo_set_line_width( cr, 2.);
		CwB[TColour::process_s].set_source_rgba( cr);
		cairo_move_to( cr, tl_pad + (float)(ep_start - tl_start) / tl_len * da_wd_actual(),
			       da_ht - lgd_margin-hypn_depth
			       - csimulation[ep_start].S * da_ht / SWA_max * display_factor);
		size_t possible_end = ep_end - ep_start +
			((zoomed_episode == (int)csimulation.mm_list().size() - 1) ? 0 : ((float)csimulation.timeline().size()/da_wd_actual() * tl_pad));
		for ( i = 1; i < possible_end; ++i )
			cairo_line_to( cr,
				       tl_pad + (float)(ep_start - tl_start + i) / tl_len * da_wd_actual(),
				       da_ht - lgd_margin-hypn_depth
				       - csimulation[ep_start + i].S * da_ht / SWA_max * display_factor);
		cairo_stroke( cr);
	}

      // hypnogram
	cairo_set_source_rgba( cr, 0., 0., 0., .4);
	cairo_set_line_width( cr, 3.);
	for ( i = 0; i < ep_end - ep_start; ++i ) {
		auto sco = csimulation[i].score();
		if ( sco != TScore::none ) {
			int y = __score_hypn_depth[ (TScore_underlying_type)sco ];
			cairo_move_to( cr, tl_pad + (float)(ep_start - tl_start + i  ) / tl_len * da_wd_actual(),
				       da_ht - hypn_depth + y);
			cairo_rel_line_to( cr, 1. / tl_len * da_wd_actual(), 0);
			cairo_stroke( cr);
		}
	}
}


void
SModelrunFacility::draw_ticks( cairo_t *cr,
			       size_t start, size_t end)
{
      // ticks
	guint	pph = 3600/csimulation.pagesize(),
		pps = pph/2;
	float	tick_spc_rough = (float)(end-start)/(da_wd/120.) / pph,
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
		CwB[TColour::ticks_mr].set_source_rgba( cr, .4);
		cairo_set_line_width( cr, (i % (24*pph) == 0) ? 1 : .3);
		cairo_move_to( cr, (float)(i-start)/(end-start) * da_wd_actual(), 0);
		cairo_rel_line_to( cr, 0., da_ht);
		cairo_stroke( cr);

		CwB[TColour::labels_mr].set_source_rgba( cr);
		cairo_move_to( cr,
			       (float)(i-start)/(end-start) * da_wd_actual() + 2,
			       da_ht - hypn_depth-lgd_margin + 14);
		snprintf_buf_ts_h( (double)i/pph);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}
}







int
SModelrunFacility::construct_widgets()
{
	if ( !(AGH_GBGETOBJ3 (builder, GtkWindow,	wModelrunFacility)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDrawingArea,	daMFProfile)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkTextView,	lMFLog)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFLiveUpdate)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkHBox,		cMFControls)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,	lMFCostFunction)) )
		return -1;

	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVrs" )] = TTunable::rs ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVrc" )] = TTunable::rc ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVfcR")] = TTunable::fcR;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVfcW")] = TTunable::fcW;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVS0" )] = TTunable::S0 ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVSU" )] = TTunable::SU ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVta" )] = TTunable::ta ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVtp" )] = TTunable::tp ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc1")] = TTunable::gc1;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc2")] = TTunable::gc2;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc3")] = TTunable::gc3;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc4")] = TTunable::gc4;
	for ( auto e = eMFVx.begin(); e != eMFVx.end(); ++e )
		if ( e->first == NULL )
			return -1;

	g_object_set( (GObject*)lMFLog,
		      "tabs", pango_tab_array_new_with_positions( 6, TRUE,
								  PANGO_TAB_LEFT, 50,
								  PANGO_TAB_LEFT, 150,
								  PANGO_TAB_LEFT, 240,
								  PANGO_TAB_LEFT, 330,
								  PANGO_TAB_LEFT, 420,
								  PANGO_TAB_LEFT, 510),
		      NULL);

	log_text_buffer = gtk_text_view_get_buffer( lMFLog);

	g_signal_connect_after( daMFProfile, "configure-event",
				G_CALLBACK (daMFProfile_configure_event_cb),
				this);
	g_signal_connect_after( daMFProfile, "draw",
				G_CALLBACK (daMFProfile_draw_cb),
				this);
	g_signal_connect_after( daMFProfile, "button-press-event",
				G_CALLBACK (daMFProfile_button_press_event_cb),
				this);
	g_signal_connect_after( daMFProfile, "scroll-event",
				G_CALLBACK (daMFProfile_scroll_event_cb),
				this);

	g_signal_connect_after( bMFRun, "clicked",
				G_CALLBACK (bMFRun_clicked_cb),
				this);
	g_signal_connect_after( bMFReset, "clicked",
				G_CALLBACK (bMFReset_clicked_cb),
				this);
	g_signal_connect_after( bMFAccept, "clicked",
				G_CALLBACK (bMFAccept_clicked_cb),
				this);

	for_each( eMFVx.begin(), eMFVx.end(),
		  [&] ( pair<GtkSpinButton*const , TTunable>& tuple)
		  {
			  g_signal_connect_after( tuple.first, "value-changed",
						  G_CALLBACK (eMFVx_value_changed_cb),
						  this);
		  });

	return 0;
}




void
SModelrunFacility::update_infobar()
{
	for_each( eMFVx.begin(), eMFVx.end(),
		  [&] ( pair<GtkSpinButton* const, TTunable>& tuple)
		  {
			  gtk_spin_button_set_value( tuple.first,
						     csimulation.cur_tset[tuple.second] * agh::STunableSet::stock[(TTunable_underlying_type)tuple.second].display_scale_factor);
		  });
	snprintf_buf( "CF = <b>%g</b>\n", cf);
	gtk_label_set_markup( lMFCostFunction, __buf__);
}



int
construct_once()
{
      // ------ colours
	if ( !(CwB[TColour::swa      ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourSWA")) ||
	     !(CwB[TColour::swa_sim  ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourSWASim")) ||
	     !(CwB[TColour::process_s].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourProcessS")) ||
	     !(CwB[TColour::paper_mr ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourPaperMR")) ||
	     !(CwB[TColour::ticks_mr ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourTicksMR")) ||
	     !(CwB[TColour::labels_mr].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourLabelsMR")) )
		return -1;
	return 0;
}


} // namespace mf





using namespace mf;

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
			? MF_siman_param_printer : NULL);

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

		if ( MF.csimulation.status & AGH_MODRUN_TRIED ) {
//		agh_modelrun_save( __modrun_ref);
			simview::populate();
		}

		delete &MF;
	}




	void
	eMFVx_value_changed_cb( GtkSpinButton* e, gpointer u)
	{
		auto& MF = *(SModelrunFacility*)u;
		if ( !MF._suppress_Vx_value_changed ) {
			TTunable t = MF.eMFVx[e];
			MF.csimulation.cur_tset[t] =
				gtk_spin_button_get_value(e)
				/ agh::STunableSet::stock[(TTunable_underlying_type)t].display_scale_factor;
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





// ---------- colours


	void
	bColourSWA_color_set_cb( GtkColorButton *widget, gpointer user_data)
	{
		CwB[TColour::swa].acquire();
	}


	void
	bColourSWASim_color_set_cb( GtkColorButton *widget, gpointer user_data)
	{
		CwB[TColour::swa_sim].acquire();
	}

	void
	bColourProcessS_color_set_cb( GtkColorButton *widget, gpointer user_data)
	{
		CwB[TColour::process_s].acquire();
	}

	void
	bColourPaperMR_color_set_cb( GtkColorButton *widget, gpointer user_data)
	{
		CwB[TColour::paper_mr].acquire();
	}

	void
	bColourTicksMR_color_set_cb( GtkColorButton *widget, gpointer user_data)
	{
		CwB[TColour::ticks_mr].acquire();
	}


	void
	bColourLabelsMR_color_set_cb( GtkColorButton *widget, gpointer user_data)
	{
		CwB[TColour::labels_mr].acquire();
	}


}  // extern "C"

} // namespace aghui

// eof
