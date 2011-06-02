// ;-*-C++-*- *  Time-stamp: "2011-06-03 02:06:22 hmmr"
/*
 *       File name:  ui/measurements.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  measurements overview view
 *
 *         License:  GPL
 */


#include <cstring>
#include <ctime>

#include <cairo.h>
#include <cairo-svg.h>

#include "misc.hh"
#include "settings.hh"
#include "measurements.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;



namespace aghui {

// exposed widgets
GtkDialog
	*wFilter,
	*wPattern,
	*wPhaseDiff;
GtkSpinButton
	*eScoringFacCurrentPage;
GtkToggleButton
	*bScoringFacShowFindDialog,
	*bScoringFacShowPhaseDiffDialog;

GtkDialog
	*wEDFFileDetails;
GtkLabel
	*lMsmtHint,
	*lMsmtInfo;
GtkTextView
	*lEDFFileDetailsReport;
GtkComboBox
	*eMsmtChannel,
	*eMsmtSession;
GtkVBox
	*cMeasurements;
GtkSpinButton
	*eMsmtPSDFreqFrom,
	*eMsmtPSDFreqWidth;


namespace msmtview {


// saved variables

float	PPuV2 = 1e-5;

// externally visible ui bits

gulong	eMsmtSession_changed_cb_handler_id,
	eMsmtChannel_changed_cb_handler_id,
	eMsmtPSDFreqFrom_value_changed_cb_handler_id,
	eMsmtPSDFreqWidth_value_changed_cb_handler_id;



// local variables

inline namespace {

      // container
	list<SGroupPresentation>
		GG;

      // supporting machinery
	size_t
		TimelinePPH = 20;

	time_t
		__timeline_start,
		__timeline_end;

	inline size_t
	T2P( time_t t)
	{
		return difftime( t, __timeline_start) / 3600. * TimelinePPH;
	}

	inline time_t
	P2T( size_t p)
	{
		return p * 3600. / TimelinePPH + __timeline_start;
	}


	size_t	__tl_left_margin = 45,
		__tl_right_margin = 20,
		__timeline_pixels,
		__timeline_pages;

      // supporting ui stuff
	GtkTextBuffer
		*textbuf2;

	enum class TTipIdx {
		general = 0,
	};

	const char*
       		__tooltips[] = {
			"<b>Subject timeline:</b>\n"
			"	Ctrl+Wheel:	change scale;\n"
			"	Click1:		view/score episode;\n"
			"	Click3:		show edf file info;\n"
			"	Alt+Click3:	save timeline as svg.",
	};

} // inline namespace



// useful definitions local to module

#define JTLDA_HEIGHT 60

// struct member functions

bool
SSubjectPresentation::get_episode_from_timeline_click( unsigned along)
{
	try {
		auto& ee = csubject.measurements[*_AghDi].episodes;
		along -= __tl_left_margin;
		for ( auto e = ee.begin(); e != ee.end(); ++e )
			if ( along >= T2P(e->start_rel) && along <= T2P(e->end_rel) ) {
				episode_focused = e;
				return true;
			}
		episode_focused = ee.end();
		return false;
	} catch (...) {
		episode_focused = csubject.measurements[*_AghDi].episodes.end();
		return false;
	}
}

void
SSubjectPresentation::draw_timeline( const char *fname) const
{
#ifdef CAIRO_HAS_SVG_SURFACE
	cairo_surface_t *cs =
		cairo_svg_surface_create( fname,
					  __timeline_pixels + __tl_left_margin + __tl_right_margin,
					  JTLDA_HEIGHT);
	cairo_t *cr = cairo_create( cs);
	draw_timeline( cr);
	cairo_destroy( cr);
	cairo_surface_destroy( cs);
#endif
}


void
SSubjectPresentation::draw_timeline( cairo_t *cr) const
{
	// draw subject name
	cairo_move_to( cr, 2, 15);
	cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 11);
	cairo_show_text( cr, csubject.name());

	if ( cscourse == NULL ) {
		cairo_stroke( cr);
		cairo_move_to( cr, 50, JTLDA_HEIGHT/2+9);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 18);
		cairo_set_source_rgba( cr, 0., 0., 0., .13);
		cairo_show_text( cr, "(no episodes)");
		return;
	}

	// draw day and night
	{
		cairo_pattern_t *cp = cairo_pattern_create_linear( __tl_left_margin, 0., __timeline_pixels-__tl_right_margin, 0.);
		struct tm clock_time;
		memcpy( &clock_time, localtime( &__timeline_start), sizeof(clock_time));
		clock_time.tm_hour = 4;
		clock_time.tm_min = clock_time.tm_sec = 0;
		time_t	dawn = mktime( &clock_time),
			t;
		gboolean up = TRUE;
		for ( t = dawn; t < __timeline_end; t += 3600 * 12, up = !up )
			if ( t > __timeline_start ) {
				//printf( "part %lg %d\n", (double)T2P(t) / __timeline_pixels, up);
				cairo_pattern_add_color_stop_rgb( cp, (double)T2P(t) / __timeline_pixels, up?.5:.8, up?.4:.8, 1.);
			}
		cairo_set_source( cr, cp);
		cairo_rectangle( cr, __tl_left_margin, 0., __tl_left_margin+__timeline_pixels, JTLDA_HEIGHT);
		cairo_fill( cr);
		cairo_pattern_destroy( cp);
	}

	struct tm tl_start_fixed_tm;
	memcpy( &tl_start_fixed_tm, localtime( &__timeline_start), sizeof(struct tm));
	// determine the latest full hour before __timeline_start
	tl_start_fixed_tm.tm_min = 0;
	time_t tl_start_fixed = mktime( &tl_start_fixed_tm);

      // SWA
	if ( cscourse == NULL ) {
		cairo_stroke( cr);
		return;
	}

	auto& ee = csubject.measurements[*_AghDi].episodes;

	// boundaries, with scored percentage bars
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size( cr, 11);
	for ( auto e = ee.begin(); e != ee.end(); ++e ) {
		unsigned
			e_pixel_start = T2P( e->start_rel),
			e_pixel_end   = T2P( e->end_rel),
			e_pixels = e_pixel_end - e_pixel_start;

		// episode start timestamp
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2, 12);
		cairo_set_source_rgb( cr, 1., 1., 1.);
		strftime( __buf__, 79, "%F %T",
			  localtime( &e->start_time()));
		g_string_printf( __ss__, "%s | %s",
				 __buf__, e->name());
		cairo_show_text( cr, __ss__->str);
		cairo_stroke( cr);

		// highlight
		if ( is_focused && episode_focused == e ) {
			cairo_set_line_width( cr, .2);
			cairo_set_source_rgba( cr, 1., 1., 1., .5);
			cairo_rectangle( cr,
					 __tl_left_margin + e_pixel_start, 0,
					 e_pixels, JTLDA_HEIGHT);
			cairo_fill( cr);
			cairo_stroke( cr);
		}

		// percentage bar graph
		float pc_scored, pc_nrem, pc_rem, pc_wake;
		pc_scored = e->sources.front().percent_scored( &pc_nrem, &pc_rem, &pc_wake);

		pc_scored *= e_pixels / 100;
		pc_nrem   *= e_pixels / 100;
		pc_rem    *= e_pixels / 100;
		pc_wake   *= e_pixels / 100;

		cairo_set_line_width( cr, 4);

		cairo_set_source_rgb( cr, 0., .1, .9);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2, JTLDA_HEIGHT-5);
		cairo_rel_line_to( cr, pc_nrem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, .9, .0, .5);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2 + pc_nrem, JTLDA_HEIGHT-5);
		cairo_rel_line_to( cr, pc_rem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, 0., .9, .1);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2 + pc_nrem + pc_rem, JTLDA_HEIGHT-5);
		cairo_rel_line_to( cr, pc_wake, 0);
		cairo_stroke( cr);

		cairo_set_line_width( cr, 10);
		cairo_set_source_rgba( cr, 1., 1., 1., .5);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2, JTLDA_HEIGHT-5);
		cairo_rel_line_to( cr, pc_scored, 0);
		cairo_stroke( cr);
	}

      // power
	unsigned
		j_tl_pixel_start = T2P( ee.front().start_rel),
		j_tl_pixel_end   = T2P( ee.back().end_rel),
		j_tl_pixels = j_tl_pixel_end - j_tl_pixel_start;

	CwB[TColour::power_mt].set_source_rgb( cr);
	cairo_set_line_width( cr, .3);
	cairo_move_to( cr, __tl_left_margin + j_tl_pixel_start, JTLDA_HEIGHT-12);
	for ( size_t i = 0; i < cscourse->timeline().size(); ++i )
		// if ( i %10 == 0 )
		// 	printf( "[%zu] %g %g\n", i, (*cscourse)[i].SWA, PPuV2);
		cairo_line_to( cr,
			        __tl_left_margin + j_tl_pixel_start + ((float)i)/cscourse->timeline().size() * j_tl_pixels,
			       -(*cscourse)[i].SWA * PPuV2 + JTLDA_HEIGHT-12);
	cairo_line_to( cr, j_tl_pixel_start + __tl_left_margin + j_tl_pixels, JTLDA_HEIGHT-12);
	cairo_fill( cr);
	cairo_stroke( cr);

      // ticks
	if ( is_focused ) {
		cairo_set_line_width( cr, .5);
		CwB[TColour::ticks_mt].set_source_rgb( cr);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 8);
		unsigned clock_d0 = localtime(&tl_start_fixed)->tm_mday;
		for ( time_t t = tl_start_fixed; t <= __timeline_end; t += 3600 ) {
			size_t x = T2P(t);
			unsigned
				clock_h  = localtime(&t)->tm_hour,
				clock_d  = localtime(&t)->tm_mday;
			if ( clock_h % 6 == 0 ) {
				cairo_move_to( cr, __tl_left_margin + x, ( clock_h % 24 == 0 ) ? 0 : (JTLDA_HEIGHT - 16));
				cairo_line_to( cr, __tl_left_margin + x, JTLDA_HEIGHT - 10);

				snprintf_buf_ts_h( (clock_d - clock_d0) * 24 + clock_h);
				cairo_text_extents_t extents;
				cairo_text_extents( cr, __buf__, &extents);
				cairo_move_to( cr, __tl_left_margin + x - extents.width/2, JTLDA_HEIGHT-1);
				cairo_show_text( cr, __buf__);

			} else {
				cairo_move_to( cr, __tl_left_margin + x, JTLDA_HEIGHT - 14);
				cairo_line_to( cr, __tl_left_margin + x, JTLDA_HEIGHT - 7);
			}
		}
	}
	cairo_stroke( cr);
}





// functions


int
construct_once()
{
	GtkCellRenderer *renderer;

     // ------------- cMeasurements
	if ( !AGH_GBGETOBJ (GtkVBox,	cMeasurements) ||
	     !AGH_GBGETOBJ (GtkLabel,	lMsmtHint) ||
	     !AGH_GBGETOBJ (GtkLabel,	lMsmtInfo) )
		return -1;

	gtk_drag_dest_set( (GtkWidget*)(cMeasurements), GTK_DEST_DEFAULT_ALL,
			   NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets( (GtkWidget*)(cMeasurements));


     // ------------- eMsmtSession
	if ( !AGH_GBGETOBJ (GtkComboBox,	eMsmtSession) )
		return -1;

	gtk_combo_box_set_model( eMsmtSession,
				 (GtkTreeModel*)mSessions);
	gtk_combo_box_set_id_column( eMsmtSession, 0);

	eMsmtSession_changed_cb_handler_id =
		g_signal_connect( eMsmtSession, "changed", G_CALLBACK (eMsmtSession_changed_cb), NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eMsmtSession, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eMsmtSession, renderer,
					"text", 0,
					NULL);

     // ------------- eMsmtChannel
	if ( !AGH_GBGETOBJ ( GtkComboBox, eMsmtChannel) )
		return -1;

	gtk_combo_box_set_model( eMsmtChannel,
				 (GtkTreeModel*)mEEGChannels);
	gtk_combo_box_set_id_column( eMsmtChannel, 0);
	eMsmtChannel_changed_cb_handler_id =
		g_signal_connect( eMsmtChannel, "changed", G_CALLBACK (eMsmtChannel_changed_cb), NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eMsmtChannel, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eMsmtChannel, renderer,
					"text", 0,
					NULL);

     // ------------- eMsmtPSDFreq
	if ( !AGH_GBGETOBJ (GtkSpinButton,	eMsmtPSDFreqFrom) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eMsmtPSDFreqWidth) )
		return -1;
	eMsmtPSDFreqFrom_value_changed_cb_handler_id =
		g_signal_connect_after( eMsmtPSDFreqFrom, "value-changed",
					G_CALLBACK (eMsmtPSDFreqFrom_value_changed_cb),
					NULL);
	eMsmtPSDFreqWidth_value_changed_cb_handler_id =
		g_signal_connect_after( eMsmtPSDFreqWidth, "value-changed",
					G_CALLBACK (eMsmtPSDFreqWidth_value_changed_cb),
					NULL);

     // ------------- wEDFFileDetails
	if ( !AGH_GBGETOBJ (GtkDialog,		wEDFFileDetails) ||
	     !AGH_GBGETOBJ (GtkTextView,	lEDFFileDetailsReport) )
		return -1;

	g_object_set( lEDFFileDetailsReport,
		      "tabs", pango_tab_array_new_with_positions( 2, TRUE,
								  PANGO_TAB_LEFT, 130,
								  PANGO_TAB_LEFT, 190),
		      NULL);
	textbuf2 = gtk_text_view_get_buffer( lEDFFileDetailsReport);


      // --- assorted static objects
	gtk_widget_set_tooltip_markup( (GtkWidget*)(lMsmtHint), __tooltips[(size_t)TTipIdx::general]);

      // ------ colours
	if ( !(CwB[TColour::power_mt].btn	= (GtkColorButton*)gtk_builder_get_object( __builder, "bColourPowerMT")) ||
	     !(CwB[TColour::ticks_mt].btn	= (GtkColorButton*)gtk_builder_get_object( __builder, "bColourTicksMT")) ||
	     !(CwB[TColour::labels_mt].btn	= (GtkColorButton*)gtk_builder_get_object( __builder, "bColourLabelsMT")) )
		return -1;

	return 0;
}


void
destruct()
{
}




void
populate()
{
	if ( AghCC->n_groups() == 0 )
		return;

      // touch toolbar controls
	g_signal_handler_block( eMsmtPSDFreqFrom, eMsmtPSDFreqFrom_value_changed_cb_handler_id);
	g_signal_handler_block( eMsmtPSDFreqWidth, eMsmtPSDFreqWidth_value_changed_cb_handler_id);
	gtk_spin_button_set_value( eMsmtPSDFreqFrom, settings::OperatingRangeFrom);
	gtk_spin_button_set_value( eMsmtPSDFreqWidth, settings::OperatingRangeUpto - settings::OperatingRangeFrom);
	g_signal_handler_unblock( eMsmtPSDFreqFrom, eMsmtPSDFreqFrom_value_changed_cb_handler_id);
	g_signal_handler_unblock( eMsmtPSDFreqWidth, eMsmtPSDFreqWidth_value_changed_cb_handler_id);

      // deal with the main drawing area
	GG.clear();
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);

	time_t	earliest_start = (time_t)-1,
		latest_end = (time_t)-1;

	printf( "msmtview:populate(): session %s, channel %s\n", AghD(), AghT());
      // first pass: determine common timeline
	for ( auto g = AghCC->groups_begin(); g != AghCC->groups_end(); ++g ) {
		GG.emplace_back( g); // precisely need the iterator, not object by reference
		SGroupPresentation& G = GG.back();
		for_each( g->second.begin(), g->second.end(),
			  [&] (agh::CSubject& j)
			  {
				  G.emplace_back( j);
				  const SSubjectPresentation& J = G.back();
				  if ( J.cscourse ) {
					  auto& ee = J.csubject.measurements[*_AghDi].episodes;
					  if ( earliest_start == (time_t)-1 || earliest_start > ee.front().start_rel )
						  earliest_start = ee.front().start_rel;
					  if ( latest_end == (time_t)-1 || latest_end < ee.back().end_rel )
						  latest_end = ee.back().end_rel;
				  } else
					  fprintf( stderr, "msmtview::populate(): subject %s has no recordings in session %s channel %s\n",
						   j.name(), AghD(), AghT());
			  });
	};

	__timeline_start = earliest_start;
	__timeline_end   = latest_end;
	__timeline_pixels = (__timeline_end - __timeline_start) / 3600 * TimelinePPH;
	__timeline_pages  = (__timeline_end - __timeline_start) / AghCC->fft_params.page_size;

	fprintf( stderr, "msmtview::populate(): common timeline:\n");
	fputs( asctime( localtime(&earliest_start)), stderr);
	fputs( asctime( localtime(&latest_end)), stderr);

	__tl_left_margin = 0;

      // walk again thoroughly, set timeline drawing area length
	for ( auto G = GG.begin(); G != GG.end(); ++G ) {
	      // convert avg episode times
		g_string_assign( __ss__, "");
		for ( auto E = AghEE.begin(); E != AghEE.end(); ++E ) {
			pair<float, float>& avge = G->group().avg_episode_times[*_AghDi][*E];
			unsigned seconds, h0, m0, s0, h9, m9, s9;
			seconds = avge.first * 24 * 60 * 60;
			h0 = seconds / 60 / 60;
			m0  = seconds % 3600 / 60;
			s0  = seconds % 60;
			seconds = avge.second * 24 * 60 * 60;
			h9 = seconds / 60 / 60;
			m9  = seconds % 3600 / 60;
			s9  = seconds % 60;

			g_string_append_printf( __ss__,
						"       <i>%s</i> %02d:%02d:%02d ~ %02d:%02d:%02d",
						E->c_str(),
						h0 % 24, m0, s0,
						h9 % 24, m9, s9);
		}

		gchar *g_escaped = g_markup_escape_text( G->name(), -1);
		snprintf_buf( "<b>%s</b> (%zu) %s", g_escaped, G->size(), __ss__->str);
		g_free( g_escaped);

		G->expander = (GtkExpander*)gtk_expander_new( __buf__);
		gtk_expander_set_use_markup( G->expander, TRUE);
		g_object_set( (GObject*)G->expander,
			      "visible", TRUE,
			      "expanded", TRUE,
			      "height-request", -1,
			      NULL);
		gtk_box_pack_start( (GtkBox*)cMeasurements,
				    (GtkWidget*)G->expander, TRUE, TRUE, 3);
		gtk_container_add( (GtkContainer*)G->expander,
				   (GtkWidget*) (G->vbox = (GtkExpander*)gtk_vbox_new( TRUE, 1)));
		g_object_set( (GObject*)G->vbox,
			      "height-request", -1,
			      NULL);

		for ( auto J = G->begin(); J != G->end(); ++J ) {
			J->da = gtk_drawing_area_new();
			gtk_box_pack_start( (GtkBox*)G->vbox,
					    J->da, TRUE, TRUE, 2);

			// determine __tl_left_margin
			cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( J->da));
			cairo_text_extents_t extents;
			cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_font_size( cr, 11);
			cairo_text_extents( cr, J->csubject.name(), &extents);
			if ( __tl_left_margin < extents.width )
				__tl_left_margin = extents.width;
			cairo_destroy( cr);

			// set it later
//			g_object_set( G_OBJECT (GG[g].subjects[j].da),
//				      "app-paintable", TRUE,
//				      "double-buffered", TRUE,
//				      "height-request", JTLDA_HEIGHT,
//				      "width-request", __timeline_pixels + __tl_left_margin + __tl_right_margin,
//				      NULL);

			gtk_widget_add_events( J->da,
					       (GdkEventMask)
					       GDK_EXPOSURE_MASK |
					       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
					       GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
					       GDK_POINTER_MOTION_MASK);
			g_signal_connect_after( J->da, "draw",
						G_CALLBACK (daSubjectTimeline_draw_cb),
						&*J);
			g_signal_connect_after( J->da, "enter-notify-event",
						G_CALLBACK (daSubjectTimeline_enter_notify_event_cb),
						&*J);
			g_signal_connect_after( J->da, "leave-notify-event",
						G_CALLBACK (daSubjectTimeline_leave_notify_event_cb),
						&*J);
			g_signal_connect_after( J->da, "scroll-event",
						G_CALLBACK (daSubjectTimeline_scroll_event_cb),
						&*J);
			if ( J->cscourse ) {
				g_signal_connect_after( J->da, "button-press-event",
							G_CALLBACK (daSubjectTimeline_button_press_event_cb),
							&*J);
				g_signal_connect_after( J->da, "motion-notify-event",
							G_CALLBACK (daSubjectTimeline_motion_notify_event_cb),
							&*J);
			}
			g_signal_connect_after( J->da, "drag-data-received",
						G_CALLBACK (cMeasurements_drag_data_received_cb),
						&*J);
			g_signal_connect_after( J->da, "drag-drop",
						G_CALLBACK (cMeasurements_drag_drop_cb),
						&*J);
			gtk_drag_dest_set( J->da, GTK_DEST_DEFAULT_ALL,
					   NULL, 0, GDK_ACTION_COPY);
			gtk_drag_dest_add_uri_targets( J->da);
		}
	}

      // walk quickly one last time to set __tl_left_margin
	__tl_left_margin += 10;
	for_each( GG.begin(), GG.end(),
		  [&] (SGroupPresentation& G)
		  {
			  for_each( G.begin(), G.end(),
				    [&] (SSubjectPresentation& J)
				    {
					    g_object_set( (GObject*)J.da,
							  "app-paintable", TRUE,
							  "double-buffered", TRUE,
							  "height-request", JTLDA_HEIGHT,
							  "width-request", __timeline_pixels + __tl_left_margin + __tl_right_margin,
							  NULL);
				    });
		  });

	snprintf_buf( "<b><small>page: %zu  bin: %g Hz  %s</small></b>",
		      AghCC -> fft_params.page_size,
		      AghCC -> fft_params.bin_size,
		      agh::SFFTParamSet::welch_window_type_name( AghCC->fft_params.welch_window_type));
	gtk_label_set_markup( lMsmtInfo, __buf__);
	gtk_widget_show_all( (GtkWidget*)(cMeasurements));
}




} // namespace msmtview






// callbacks


using namespace msmtview;

extern "C" {

	void
	eMsmtSession_changed_cb( GtkComboBox *widget, gpointer user_data)
	{
		auto oldval = _AghDi;
		_AghDi = find( AghDD.begin(), AghDD.end(),
			       gtk_combo_box_get_active_id( eMsmtSession));

		if ( oldval != _AghDi )
			msmtview::populate();
	}

	void
	eMsmtChannel_changed_cb( GtkComboBox *widget, gpointer user_data)
	{
		auto oldval = _AghTi;
		_AghTi = find( AghTT.begin(), AghTT.end(),
			       gtk_combo_box_get_active_id( eMsmtChannel));
		if ( /* _AghTi != AghTT.end() && */ oldval != _AghTi )
			msmtview::populate();
	}



	void
	eMsmtPSDFreqFrom_value_changed_cb( GtkSpinButton *spinbutton, gpointer user_data)
	{
		using namespace settings;
		OperatingRangeFrom = gtk_spin_button_get_value( eMsmtPSDFreqFrom);
		OperatingRangeUpto = OperatingRangeFrom + gtk_spin_button_get_value( eMsmtPSDFreqWidth);
		msmtview::populate();
	}

	void
	eMsmtPSDFreqWidth_value_changed_cb( GtkSpinButton *spinbutton, gpointer user_data)
	{
		using namespace settings;
		OperatingRangeUpto = OperatingRangeFrom + gtk_spin_button_get_value( eMsmtPSDFreqWidth);
		msmtview::populate();
	}

	gboolean
	daSubjectTimeline_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
	{
		((const msmtview::SSubjectPresentation*)userdata) -> draw_timeline( cr);
		return TRUE;
	}


	gboolean
	daSubjectTimeline_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
	{
		SSubjectPresentation& J = *(SSubjectPresentation*)userdata;
		if ( J.get_episode_from_timeline_click( event->x) )
			gtk_widget_queue_draw( wid);
		return TRUE;
	}
	gboolean
	daSubjectTimeline_leave_notify_event_cb( GtkWidget *wid, GdkEventCrossing *event, gpointer userdata)
	{
		SSubjectPresentation& J = *(SSubjectPresentation*)userdata;
		J.is_focused = false;
		J.episode_focused = J.csubject.measurements[*_AghDi].episodes.end();
		gtk_widget_queue_draw( wid);
		return TRUE;
	}
	gboolean
	daSubjectTimeline_enter_notify_event_cb( GtkWidget *wid, GdkEventCrossing *event, gpointer userdata)
	{
		SSubjectPresentation& J = *(SSubjectPresentation*)userdata;
		J.is_focused = true;
		if ( J.get_episode_from_timeline_click( event->x) )
			;
		gtk_widget_queue_draw( wid);
		return TRUE;
	}



	gboolean
	daSubjectTimeline_button_press_event_cb( GtkWidget *widget, GdkEventButton *event, gpointer userdata)
	{
		using namespace msmtview;
		SSubjectPresentation& J = *(SSubjectPresentation*)userdata;

		if ( J.get_episode_from_timeline_click( event->x) ) {
			// should some episodes be missing, we make sure the correct one gets identified by number
			_AghEi = find( AghEE.begin(), AghEE.end(), J.episode_focused->name());
		} else
			_AghEi = AghEE.end();
//		AghJ = _j;

		switch ( event->button ) {
		case 1:
			if ( _AghEi != AghEE.end() ) {
				new sf::SScoringFacility( J.csubject, *_AghDi, *_AghEi);
				// will be destroyed by its ui callbacks it has registered
			}
		    break;
		case 2:
		case 3:
			if ( event->state & GDK_MOD1_MASK ) {
				snprintf_buf( "%s/%s/%s/%s/%s.svg",
					      AghCC->session_dir(), AghCC->group_of( J.csubject), J.csubject.name(),
					      AghD(), AghT());
				string tmp (__buf__);
				J.draw_timeline( __buf__);
				snprintf_buf( "Wrote \"%s\"", tmp.c_str());
				gtk_statusbar_pop( sbMainStatusBar, sb::sbContextIdGeneral);
				gtk_statusbar_push( sbMainStatusBar, sb::sbContextIdGeneral,
						    __buf__);
			} else if ( AghE() ) {
				const agh::CEDFFile& F = J.cscourse->mm_list().front()->source();
				gtk_text_buffer_set_text( textbuf2, F.details().c_str(), -1);
				snprintf_buf( "%s header", F.filename());
				gtk_window_set_title( (GtkWindow*)wEDFFileDetails,
						      __buf__);
				gtk_widget_show_all( (GtkWidget*)(wEDFFileDetails));
			}
		    break;
		}

		return TRUE;
	}


	gboolean
	daSubjectTimeline_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer ignored)
	{
		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			if ( event->state & GDK_CONTROL_MASK ) {
				PPuV2 /= 1.3;
				gtk_widget_queue_draw( (GtkWidget*)(cMeasurements));
				return TRUE;
			}
			break;
		case GDK_SCROLL_UP:
			if ( event->state & GDK_CONTROL_MASK ) {
				PPuV2 *= 1.3;
				gtk_widget_queue_draw( (GtkWidget*)(cMeasurements));
				return TRUE;
			}
			break;
		default:
			break;
		}

		return FALSE;
	}



      // -------- colours
	void
	bColourPowerMT_color_set_cb( GtkColorButton *widget,
				     gpointer        user_data)
	{
		CwB[TColour::power_mt].acquire();
	}

	void
	bColourTicksMT_color_set_cb( GtkColorButton *widget,
				     gpointer        user_data)
	{
		CwB[TColour::ticks_mt].acquire();
	}

	void
	bColourLabelsMT_color_set_cb( GtkColorButton *widget,
				      gpointer        user_data)
	{
		CwB[TColour::labels_mt].acquire();
	}
}







} // namespace aghui


// EOF
