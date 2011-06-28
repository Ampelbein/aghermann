// ;-*-C++-*- *  Time-stamp: "2011-06-29 02:47:24 hmmr"
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
#include "expdesign.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

using namespace aghui;

inline namespace {

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




// struct member functions

bool
aghui::SSubjectPresentation::get_episode_from_timeline_click( unsigned along)
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
aghui::SSubjectPresentation::draw_timeline( const char *fname) const
{
#ifdef CAIRO_HAS_SVG_SURFACE
	cairo_surface_t *cs =
		cairo_svg_surface_create( fname,
					  __timeline_pixels + __tl_left_margin + __tl_right_margin,
					  settings::WidgetSize_MVTimelineHeight);
	cairo_t *cr = cairo_create( cs);
	draw_timeline( cr);
	cairo_destroy( cr);
	cairo_surface_destroy( cs);
#endif
}


void
aghui::SSubjectPresentation::draw_timeline( cairo_t *cr) const
{
	// draw subject name
	cairo_move_to( cr, 2, 15);
	cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 11);
	cairo_show_text( cr, csubject.name());

	if ( cscourse == NULL ) {
		cairo_stroke( cr);
		cairo_move_to( cr, 50, settings::WidgetSize_MVTimelineHeight/2+9);
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
		cairo_rectangle( cr, __tl_left_margin, 0., __tl_left_margin+__timeline_pixels, settings::WidgetSize_MVTimelineHeight);
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
					 e_pixels, settings::WidgetSize_MVTimelineHeight);
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
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2, settings::WidgetSize_MVTimelineHeight-5);
		cairo_rel_line_to( cr, pc_nrem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, .9, .0, .5);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2 + pc_nrem, settings::WidgetSize_MVTimelineHeight-5);
		cairo_rel_line_to( cr, pc_rem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, 0., .9, .1);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2 + pc_nrem + pc_rem, settings::WidgetSize_MVTimelineHeight-5);
		cairo_rel_line_to( cr, pc_wake, 0);
		cairo_stroke( cr);

		cairo_set_line_width( cr, 10);
		cairo_set_source_rgba( cr, 1., 1., 1., .5);
		cairo_move_to( cr, __tl_left_margin + e_pixel_start + 2, settings::WidgetSize_MVTimelineHeight-5);
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
	cairo_move_to( cr, __tl_left_margin + j_tl_pixel_start, settings::WidgetSize_MVTimelineHeight-12);
	for ( size_t i = 0; i < cscourse->timeline().size(); ++i )
		// if ( i %10 == 0 )
		// 	printf( "[%zu] %g %g\n", i, (*cscourse)[i].SWA, PPuV2);
		cairo_line_to( cr,
			        __tl_left_margin + j_tl_pixel_start + ((float)i)/cscourse->timeline().size() * j_tl_pixels,
			       -(*cscourse)[i].SWA * PPuV2 + settings::WidgetSize_MVTimelineHeight-12);
	cairo_line_to( cr, j_tl_pixel_start + __tl_left_margin + j_tl_pixels, settings::WidgetSize_MVTimelineHeight-12);
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
				cairo_move_to( cr, __tl_left_margin + x, ( clock_h % 24 == 0 ) ? 0 : (settings::WidgetSize_MVTimelineHeight - 16));
				cairo_line_to( cr, __tl_left_margin + x, settings::WidgetSize_MVTimelineHeight - 10);

				snprintf_buf_ts_h( (clock_d - clock_d0) * 24 + clock_h);
				cairo_text_extents_t extents;
				cairo_text_extents( cr, __buf__, &extents);
				cairo_move_to( cr, __tl_left_margin + x - extents.width/2, settings::WidgetSize_MVTimelineHeight-1);
				cairo_show_text( cr, __buf__);

			} else {
				cairo_move_to( cr, __tl_left_margin + x, settings::WidgetSize_MVTimelineHeight - 14);
				cairo_line_to( cr, __tl_left_margin + x, settings::WidgetSize_MVTimelineHeight - 7);
			}
		}
		cairo_stroke( cr);
	}
}









aghui::SExpDesignUI::SExpDesignUI( const string& dir)
      : operating_range_from (2.),
	operating_range_upto (3.),
	pagesize_item (3),
	fft_window_type (agh::SFFTParamSet::TWinType::welch),
	af_damping_window_type (agh::SFFTParamSet::TWinType::welch),
	ext_score_codes ({
		{" -0", "1", "2", "3", "4", "6Rr8", "Ww5", "mM"}
	}),
	freq_bands ({
		{  1.5,  4.0 },
		{  4.0,  8.0 },
		{  8.0, 12.0 },
		{ 15.0, 30.0 },
		{ 30.0, 40.0 },
	}),
	ppuv2 (1e-5),
	timeline_height (70),
	timeline_pph (20),
	runbatch_include_all_channels (false),
	runbatch_include_all_sessions (false),
	runbatch_iterate_ranges (false)
{
	if ( construct_widgets() )
		throw runtime_error ("SExpDesignUI::SExpDesignUI(): failed to construct widgets");

}


int
aghui::SExpDesignUI::construct_widgets()
{
      // construct static storage
	if ( !AGH_GBGETOBJ (GtkWindow, wMainWindow) ||
	     !AGH_GBGETOBJ (GtkListStore, mScoringPageSize) ||
	     !AGH_GBGETOBJ (GtkListStore, mFFTParamsPageSize) ||
	     !AGH_GBGETOBJ (GtkListStore, mFFTParamsWindowType) ) {
		return -1;
	}

      // construct list and tree stores
	mSessions =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mEEGChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mAllChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);


	GtkCellRenderer *renderer;

     // ------------- cMeasurements
	if ( !AGH_GBGETOBJ (GtkVBox,	cMeasurements) ||
	     !AGH_GBGETOBJ (GtkLabel,	lMsmtHint) ||
	     !AGH_GBGETOBJ (GtkLabel,	lMsmtInfo) )
		return -1;

	gtk_drag_dest_set( (GtkWidget*)cMeasurements, GTK_DEST_DEFAULT_ALL,
			   NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets( (GtkWidget*)(cMeasurements));


     // ------------- eMsmtSession
	if ( !AGH_GBGETOBJ (GtkComboBox, eMsmtSession) )
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

      // scrub colours
	for_each( CwB.begin(), CwB.end(),
		  [] ( const pair<TColour, SManagedColor>& p)
		  {
			  g_signal_emit_by_name( p.second.btn, "color-set");
		  });



	return 0;
}





inline namespace {
	template <class T>
	void
	print_xx( const char *pre, const list<T>& ss)
	{
		printf( "%s", pre);
		for ( auto S = ss.begin(); S != ss.end(); ++S )
			printf( " %s;", S->c_str());
		printf("\n");
	}
}



int
aghui::SExpDesignUI::populate_1( bool do_load)
{
	printf( "\naghui::populate():\n");
	AghDD = AghCC->enumerate_sessions();
	_AghDi = AghDD.begin();
	print_xx( "* Sessions:", AghDD);
	AghGG = AghCC->enumerate_groups();
	_AghGi = AghGG.begin();
	print_xx( "* Groups:", AghGG);
	AghHH = AghCC->enumerate_all_channels();
	_AghHi = AghHH.begin();
	print_xx( "* All Channels:", AghHH);
	AghTT = AghCC->enumerate_eeg_channels();
	_AghTi = AghTT.begin();
	print_xx( "* EEG channels:", AghTT);
	AghEE = AghCC->enumerate_episodes();
	_AghEi = AghEE.begin();
	print_xx( "* Episodes:", AghEE);
	printf( "\n");

	if ( do_load ) {
		if ( settings::load() )
			;
		else
			if ( GeometryMain.w > 0 ) // implies the rest are, too
				gdk_window_move_resize( gtk_widget_get_window( (GtkWidget*)wMainWindow),
							GeometryMain.x, GeometryMain.y,
							GeometryMain.w, GeometryMain.h);
	}

	if ( AghGG.empty() ) {
		msmt::show_empty_experiment_blurb();
	} else {
		populate_mChannels();
		populate_mSessions();
		msmt::populate();
//		populate_mSimulations( FALSE);
	}

	return 0;
}


void
aghui::depopulate( bool do_save)
{
	if ( do_save )
		settings::save();

	msmt::destruct();

	// these are freed on demand immediately before reuse; leave them alone
	AghGG.clear();
	AghDD.clear();
	AghEE.clear();
	AghHH.clear();
	AghTT.clear();
}




void
aghui::do_rescan_tree()
{
	set_cursor_busy( true, (GtkWidget*)wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, FALSE);
	while ( gtk_events_pending() )
		gtk_main_iteration();

	depopulate( false);
	AghCC -> scan_tree( sb::progress_indicator);
	populate( false);

	set_cursor_busy( false, (GtkWidget*)wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, TRUE);
	gtk_statusbar_push( sbMainStatusBar, sb::sbContextIdGeneral,
			    "Scanning complete");
}




void
aghui::populate_mSessions()
{
	g_signal_handler_block( eMsmtSession, msmt::eMsmtSession_changed_cb_handler_id);
	gtk_list_store_clear( mSessions);
	GtkTreeIter iter;
	for ( auto D = AghDD.begin(); D != AghDD.end(); ++D ) {
		gtk_list_store_append( mSessions, &iter);
		gtk_list_store_set( mSessions, &iter,
				    0, D->c_str(),
				    -1);
	}
	__reconnect_sessions_combo();
	g_signal_handler_unblock( eMsmtSession, msmt::eMsmtSession_changed_cb_handler_id);
}






void
aghui::populate_mChannels()
{
	g_signal_handler_block( eMsmtChannel, msmt::eMsmtChannel_changed_cb_handler_id);
	gtk_list_store_clear( mEEGChannels);
	gtk_list_store_clear( mAllChannels);
	// users of mAllChannels (SF pattern) connect to model dynamically

	// for ( auto H = AghTT.begin(); H != AghTT.end(); ++H ) {
	// 	gtk_list_store_append( agh_mEEGChannels, &iter);
	// 	gtk_list_store_set( agh_mEEGChannels, &iter,
	// 			    0, H->c_str(),
	// 			    -1);
	// }
	for_each( AghTT.begin(), AghTT.end(),
		  [&] ( const agh::SChannel& H) {
			  GtkTreeIter iter;
			  gtk_list_store_append( mEEGChannels, &iter);
			  gtk_list_store_set( mEEGChannels, &iter,
					      0, H.c_str(),
					      -1);
		  });

	for_each( AghHH.begin(), AghHH.end(),
		  [&] ( const agh::SChannel& H) {
			  GtkTreeIter iter;
			  gtk_list_store_append( mAllChannels, &iter);
			  gtk_list_store_set( mAllChannels, &iter,
					      0, H.c_str(),
					      -1);
		  });

	__reconnect_channels_combo();

	g_signal_handler_unblock( eMsmtChannel, msmt::eMsmtChannel_changed_cb_handler_id);
}






void
aghui::__reconnect_channels_combo()
{
	gtk_combo_box_set_model( eMsmtChannel, (GtkTreeModel*)mEEGChannels);

	if ( !AghTT.empty() ) {
		int Ti = AghTi();
		if ( Ti != -1 )
			gtk_combo_box_set_active( eMsmtChannel, Ti);
		else
			gtk_combo_box_set_active( eMsmtChannel, 0);
	}
}


void
aghui::__reconnect_sessions_combo()
{
	gtk_combo_box_set_model( eMsmtSession, (GtkTreeModel*)mSessions);

	if ( !AghDD.empty() ) {
		int Di = AghDi();
		if ( Di != -1 )
			gtk_combo_box_set_active( eMsmtSession, Di);
		else
			gtk_combo_box_set_active( eMsmtSession, 0);
	}
}





void
aghui::msmt::populate()
{
	if ( AghCC->n_groups() == 0 )
		return;

      // touch toolbar controls
	g_signal_handler_block( eMsmtPSDFreqFrom, eMsmtPSDFreqFrom_value_changed_cb_handler_id);
	g_signal_handler_block( eMsmtPSDFreqWidth, eMsmtPSDFreqWidth_value_changed_cb_handler_id);
	gtk_spin_button_set_value( eMsmtPSDFreqFrom, OperatingRangeFrom);
	gtk_spin_button_set_value( eMsmtPSDFreqWidth, OperatingRangeUpto - OperatingRangeFrom);
	g_signal_handler_unblock( eMsmtPSDFreqFrom, eMsmtPSDFreqFrom_value_changed_cb_handler_id);
	g_signal_handler_unblock( eMsmtPSDFreqWidth, eMsmtPSDFreqWidth_value_changed_cb_handler_id);

      // deal with the main drawing area
	GG.clear();
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);

	time_t	earliest_start = (time_t)-1,
		latest_end = (time_t)-1;

	printf( "msmt:populate(): session %s, channel %s\n", AghD(), AghT());
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
					  fprintf( stderr, "msmt::populate(): subject %s has no recordings in session %s channel %s\n",
						   j.name(), AghD(), AghT());
			  });
	};

	__timeline_start = earliest_start;
	__timeline_end   = latest_end;
	__timeline_pixels = (__timeline_end - __timeline_start) / 3600 * TimelinePPH;
	__timeline_pages  = (__timeline_end - __timeline_start) / AghCC->fft_params.page_size;

	fprintf( stderr, "msmt::populate(): common timeline:\n");
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
			{
				cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( J->da));
				cairo_text_extents_t extents;
				cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
				cairo_set_font_size( cr, 11);
				cairo_text_extents( cr, J->csubject.name(), &extents);
				if ( __tl_left_margin < extents.width )
					__tl_left_margin = extents.width;
				cairo_destroy( cr);
			}

			// set it later
//			g_object_set( G_OBJECT (GG[g].subjects[j].da),
//				      "app-paintable", TRUE,
//				      "double-buffered", TRUE,
//				      "height-request", settings::WidgetSize_MVTimelineHeight,
//				      "width-request", __timeline_pixels + __tl_left_margin + __tl_right_margin,
//				      NULL);

			gtk_widget_add_events( J->da,
					       (GdkEventMask)
					       GDK_EXPOSURE_MASK |
					       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
					       GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
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

      // walk quickly one last time to set widget attributes (importantly, involving __tl_left_margin)
	__tl_left_margin += 10;
	for_each( GG.begin(), GG.end(),
		  [&] (SGroupPresentation& G)
		  {
			  for_each( G.begin(), G.end(),
				    [&] (SSubjectPresentation& J)
				    {
					    g_object_set( (GObject*)J.da,
							  "can-focus", FALSE,
							  "app-paintable", TRUE,
							  "double-buffered", TRUE,
							  "height-request", settings::WidgetSize_MVTimelineHeight,
							  "width-request", __timeline_pixels + __tl_left_margin + __tl_right_margin,
							  NULL);
				    });
		  });

	snprintf_buf( "<b><small>page: %zu sec  bin: %g Hz  %s</small></b>",
		      AghCC -> fft_params.page_size,
		      AghCC -> fft_params.bin_size,
		      agh::SFFTParamSet::welch_window_type_name( AghCC->fft_params.welch_window_type));
	gtk_label_set_markup( lMsmtInfo, __buf__);
	gtk_widget_show_all( (GtkWidget*)(cMeasurements));
}



void
aghui::msmt::show_empty_experiment_blurb()
{
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	const char *briefly =
		"<b><big>Empty experiment\n</big></b>\n"
		"When you have your recordings ready as a set of .edf files,\n"
		"• Create your experiment tree as follows: <i>Experiment/Group/Subject/Session</i>;\n"
		"• Have your EDF sources named <i>Episode</i>.edf, and placed in the corresponding <i>Session</i> directory, or\n"
		"• Drop EDF sources onto here and identify and place them individually.\n\n"
		"Once set up, either:\n"
		"• click <b>⎇</b> and select the top directory of the (newly created) experiment tree, or\n"
		"• click <b>Rescan</b> if this is the tree you have just populated.";
	GtkLabel *text = (GtkLabel*)gtk_label_new( "");
	gtk_label_set_markup( text, briefly);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)text,
			    TRUE, TRUE, 0);

	snprintf_buf( "%s/%s/%s", PACKAGE_DATADIR, PACKAGE, AGH_BG_IMAGE_FNAME);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)gtk_image_new_from_file( __buf__),
			    TRUE, FALSE, 0);
	gtk_widget_show_all( (GtkWidget*)cMeasurements);
}









// callbacks


// EOF
