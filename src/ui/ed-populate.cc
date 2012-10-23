// ;-*-C++-*-
/*
 *       File name:  ui/ed-populate.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-19
 *
 *         Purpose:  measurements overview view
 *
 *         License:  GPL
 */


#include <cstring>

#include "../expdesign/primaries.hh"
#include "../model/beersma.hh"
#include "misc.hh"
#include "ed.hh"
#include "ed_cb.hh"

using namespace std;

using namespace aghui;


int
aghui::SExpDesignUI::
populate( bool do_load)
{
	printf( "\nSExpDesignUI::populate():\n");
	AghDD = ED->enumerate_sessions();
	_AghDi = AghDD.begin();
	AghGG = ED->enumerate_groups();
	_AghGi = AghGG.begin();
	AghHH = ED->enumerate_all_channels();
	_AghHi = AghHH.begin();
	AghTT = ED->enumerate_eeg_channels();
	_AghTi = AghTT.begin();
	AghEE = ED->enumerate_episodes();
	_AghEi = AghEE.begin();

	printf( "*     Sessions: %s\n"
		"*       Groups: %s\n"
		"* All Channels: %s\n"
		"* EEG Channels: %s\n"
		"*     Episodes: %s\n",
		agh::str::join( AghDD, "; ").c_str(),
		agh::str::join( AghGG, "; ").c_str(),
		agh::str::join( AghHH, "; ").c_str(),
		agh::str::join( AghTT, "; ").c_str(),
		agh::str::join( AghEE, "; ").c_str());

	used_samplerates =
		ED->used_samplerates();
	used_eeg_samplerates =
		ED->used_samplerates( sigfile::SChannel::TType::eeg);
	if ( used_eeg_samplerates.size() == 1 )
		printf( "* single common EEG samplerate: %zu\n", used_eeg_samplerates.front());
	else
		printf( "* multiple EEG samplerates (%zu)\n", used_eeg_samplerates.size());
	printf( "* global AD profiles: %zu\n", global_artifact_detection_profiles.size());

	if ( do_load ) {
		if ( load_settings() )
			fprintf( stderr, "load_settings() had issues\n");
		if ( geometry.w > 0 ) {// implies the rest are, too
			// gtk_window_parse_geometry( wMainWindow, _geometry_placeholder.c_str());
			gtk_window_move( wMainWindow, geometry.x, geometry.y);
			gtk_window_resize( wMainWindow, geometry.w, geometry.h);
		}
	}

	gtk_window_set_title( wMainWindow,
			      (string ("Aghermann: ") + agh::str::homedir2tilda( ED->session_dir())).c_str());
	if ( last_used_version != VERSION ) {
		printf( "Upgrading from version %s, here's ChangeLog for you\n", last_used_version.c_str());
		show_changelog();
		last_used_version = VERSION;
	}

	snprintf_buf( "Smooth: %zu", smooth_profile);
	gtk_button_set_label( (GtkButton*)eMsmtProfileSmooth, __buf__);

	if ( AghTT.empty() )
		aghui::pop_ok_message( wMainWindow,
				       "No EEG channels",
				       "There are no EEG channels found in any recordings in the tree.");
	if ( AghTT.empty() or AghGG.empty() ) {
		show_empty_experiment_blurb();
		gtk_widget_set_visible( (GtkWidget*)lTaskSelector2, FALSE);
		gtk_widget_set_visible( (GtkWidget*)cMsmtMainToolbar, FALSE);
		gtk_widget_set_visible( gtk_notebook_get_nth_page( tTaskSelector, 1), FALSE);
	} else {
		populate_mChannels();
		populate_mSessions();
		populate_mGlobalAnnotations();
		populate_mGlobalADProfiles();
		populate_1();

		gtk_combo_box_set_active( eGlobalADProfiles, 0);

		if ( display_profile_type == sigfile::TMetricType::Psd ) {
			gtk_combo_box_set_active( eMsmtProfileType, 0);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParams2, FALSE);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParams1, TRUE);
			gtk_widget_grab_focus( (GtkWidget*)eMsmtOpFreqFrom);
		} else {
			gtk_combo_box_set_active( eMsmtProfileType, 1);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParams1, FALSE);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParams2, TRUE);
		}

		gtk_widget_set_visible( (GtkWidget*)lTaskSelector2, TRUE);
		gtk_widget_set_visible( (GtkWidget*)cMsmtMainToolbar, TRUE);
		gtk_widget_set_visible( gtk_notebook_get_nth_page( tTaskSelector, 1), TRUE);
	}

	if ( not ED->error_log().empty() ) {
		gtk_text_buffer_set_text( gtk_text_view_get_buffer( tScanLog),
					  ED->error_log().c_str(), -1);
		gtk_widget_show_all( (GtkWidget*)wScanLog);
	}

	return 0;
}


void
aghui::SExpDesignUI::
depopulate( bool do_save)
{
	if ( do_save )
		save_settings();

	ED->reset_error_log();

	// these are freed on demand immediately before reuse; leave them alone
	AghGG.clear();
	AghDD.clear();
	AghEE.clear();
	AghHH.clear();
	AghTT.clear();

	g_signal_handler_block( eMsmtSession, eMsmtSession_changed_cb_handler_id);
	g_signal_handler_block( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);

	gtk_list_store_clear( mSessions);
	gtk_list_store_clear( mAllChannels);
	gtk_list_store_clear( mEEGChannels);
	gtk_tree_store_clear( mGlobalAnnotations);
	gtk_list_store_clear( mGlobalADProfiles);

	__reconnect_sessions_combo();
	g_signal_handler_unblock( eMsmtSession, eMsmtSession_changed_cb_handler_id);
	__reconnect_channels_combo();
	g_signal_handler_unblock( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
}





void
aghui::SExpDesignUI::
populate_mSessions()
{
	g_signal_handler_block( eMsmtSession, eMsmtSession_changed_cb_handler_id);
	gtk_list_store_clear( mSessions);
	GtkTreeIter iter;
	for ( auto &D : AghDD ) {
		gtk_list_store_append( mSessions, &iter);
		gtk_list_store_set( mSessions, &iter,
				    0, D.c_str(),
				    -1);
	}
	__reconnect_sessions_combo();
	g_signal_handler_unblock( eMsmtSession, eMsmtSession_changed_cb_handler_id);
}






void
aghui::SExpDesignUI::
populate_mChannels()
{
	g_signal_handler_block( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
	gtk_list_store_clear( mEEGChannels);
	gtk_list_store_clear( mAllChannels);
	// users of mAllChannels (SF pattern) connect to model dynamically

	for ( auto &H : AghTT ) {
		GtkTreeIter iter;
		gtk_list_store_append( mEEGChannels, &iter);
		gtk_list_store_set( mEEGChannels, &iter,
				    0, H.c_str(),
				    -1);
	}

	for ( auto &H : AghHH ) {
		GtkTreeIter iter;
		gtk_list_store_append( mAllChannels, &iter);
		gtk_list_store_set( mAllChannels, &iter,
				    0, H.c_str(),
				    -1);
	}

	__reconnect_channels_combo();
	g_signal_handler_unblock( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
}





void
aghui::SExpDesignUI::
__reconnect_channels_combo()
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
aghui::SExpDesignUI::
__reconnect_sessions_combo()
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
aghui::SExpDesignUI::
populate_mGlobalAnnotations()
{
	gtk_tree_store_clear( mGlobalAnnotations);
      // apart from tree store, also refresh global list
	global_annotations.clear();

	GtkTreeIter
		iter_g, iter_j, iter_d, iter_e, iter_a;
	const char
		*last_g = NULL, *last_j = NULL, *last_d = NULL, *last_e = NULL;

	for ( auto &G : ED->groups ) {
		for ( auto &J : G.second ) {
			for ( auto &D : J.measurements ) {
				for ( auto &E : D.second.episodes ) {
					auto annotations = E.get_annotations();
					if ( annotations.size() > 0 ) {
						if ( last_g != G.first.c_str() ) {
							gtk_tree_store_append( mGlobalAnnotations, &iter_g, NULL);
							gtk_tree_store_set( mGlobalAnnotations, &iter_g,
									    0, G.first.c_str(),
									    mannotations_visibility_switch_col, TRUE,
									    -1);
							last_j = last_d = last_e = NULL;
						}
						if ( last_j != J.name() ) {
							gtk_tree_store_append( mGlobalAnnotations, &iter_j, &iter_g);
							gtk_tree_store_set( mGlobalAnnotations, &iter_j,
									    0, last_j = J.name(),
									    mannotations_visibility_switch_col, TRUE,
									    -1);
							last_d = last_e = NULL;
						}
						if ( last_d != D.first.c_str() ) {
							gtk_tree_store_append( mGlobalAnnotations, &iter_d, &iter_j);
							gtk_tree_store_set( mGlobalAnnotations, &iter_d,
									    0, last_d = D.first.c_str(),
									    mannotations_visibility_switch_col, TRUE,
									    -1);
							last_e = NULL;
						}
						if ( last_e != E.name() ) {
							gtk_tree_store_append( mGlobalAnnotations, &iter_e, &iter_d);
							gtk_tree_store_set( mGlobalAnnotations, &iter_e,
									    0, last_e = E.name(),
									    mannotations_visibility_switch_col, TRUE,
									    -1);
						}
						for ( auto &A : annotations ) {

							global_annotations.emplace_front( J, D.first, E, A);

							auto pages = A.page_span( pagesize()) * 1u;
							if ( pages.a == pages.z )
								snprintf_buf( "%u", pages.a + 1);
							else
								snprintf_buf( "%u-%u", pages.a + 1, pages.z + 1);
							gtk_tree_store_append( mGlobalAnnotations, &iter_a, &iter_e);
							gtk_tree_store_set( mGlobalAnnotations, &iter_a,
									    1, __buf__,
									    2, A.channel(),
									    3, A.label.c_str(),
									    mannotations_ref_col, (gpointer)&global_annotations.front(),
									    mannotations_visibility_switch_col, TRUE,
									    -1);
						}
					}
				}
			}
		}
	}
	gtk_tree_view_expand_all( tvGlobalAnnotations);
}



void
aghui::SExpDesignUI::
populate_mGlobalADProfiles()
{
	gtk_list_store_clear( mGlobalADProfiles);
	for ( auto &P : global_artifact_detection_profiles ) {
		GtkTreeIter iter;
		gtk_list_store_append( mGlobalADProfiles, &iter);
		gtk_list_store_set( mGlobalADProfiles, &iter,
				    0, P.first.c_str(),
				    -1);
	}
	gtk_combo_box_set_model( eGlobalADProfiles, (GtkTreeModel*)mGlobalADProfiles);
}



void
aghui::SExpDesignUI::
populate_1()
{
	if ( ED->groups.empty() )
		return;

#ifdef _OPENMP
#pragma omp barrier
#endif

      // touch toolbar controls
	suppress_redraw = true;
	gtk_spin_button_set_value( eMsmtOpFreqFrom, operating_range_from);
	gtk_spin_button_set_value( eMsmtOpFreqWidth, operating_range_upto - operating_range_from);

      // deal with the main drawing area
	groups.clear();
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback)gtk_widget_destroy,
			       NULL);

	printf( "SExpDesignUI::populate_1(): session \"%s\", channel \"%s\"\n", AghD(), AghT());

	time_t	earliest_start = (time_t)-1,
		latest_end = (time_t)-1;
      // first pass: (1) create SSubjectPresentation's
      //             (2) determine common timeline
	for ( auto Gi = ED->groups.begin(); Gi != ED->groups.end(); ++Gi ) {
		groups.emplace_back( Gi, *this); // precisely need the iterator, not object by reference
		SGroupPresentation& Gp = groups.back();
		for ( auto &J : Gi->second ) {
			Gp.emplace_back( J, Gp);
			const SSubjectPresentation& j = Gp.back();
			if ( j.cscourse && J.have_session(*_AghDi) ) {
				auto& ee = J.measurements[*_AghDi].episodes;
				if ( not ee.empty() ) {
					// (2)
					if ( earliest_start == (time_t)-1 || earliest_start > ee.front().start_rel )
						earliest_start = ee.front().start_rel;
					if ( latest_end == (time_t)-1 || latest_end < ee.back().end_rel )
						latest_end = ee.back().end_rel;
				} else
					fprintf( stderr, "SExpDesignUI::populate_1(): session \"%s\", channel \"%s\" for subject \"%s\" is empty\n",
						 AghD(), AghT(), J.name());
			}
		}
	}

	timeline_start = earliest_start;
	timeline_end   = latest_end;
	timeline_width = (timeline_end - timeline_start) / 3600 * timeline_pph;
	timeline_pages = (timeline_end - timeline_start) / ED->fft_params.pagesize;

	if ( profile_scale_psd == 0. || profile_scale_mc == 0. ) // not previously saved
		calculate_profile_scale();

	printf( "SExpDesignUI::populate_1(): common timeline:\n");
	fputs( asctime( localtime(&earliest_start)), stderr);
	fputs( asctime( localtime(&latest_end)), stderr);

	tl_left_margin = tl_right_margin = 0;

      // walk again thoroughly, set timeline drawing area length
	for ( auto &G : groups ) {
	      // convert avg episode times
		g_string_assign( __ss__, "");
		for ( auto &E : AghEE ) {
			pair<float, float>& avge = G.group().avg_episode_times[*_AghDi][E];
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
						E.c_str(),
						h0 % 24, m0, s0,
						h9 % 24, m9, s9);
		}

		gchar *g_escaped = g_markup_escape_text( G.name(), -1);
		snprintf_buf( "<b>%s</b> (%zu) %s", g_escaped, G.size(), __ss__->str);
		g_free( g_escaped);

		G.expander = (GtkExpander*)gtk_expander_new( __buf__);
		gtk_expander_set_use_markup( G.expander, TRUE);
		g_object_set( (GObject*)G.expander,
			      "visible", TRUE,
			      "expanded", TRUE,
			      "height-request", -1,
			      NULL);
		gtk_box_pack_start( (GtkBox*)cMeasurements,
				    (GtkWidget*)G.expander, FALSE, TRUE, 3);
		gtk_container_add( (GtkContainer*)G.expander,
				   (GtkWidget*) (G.vbox = (GtkExpander*)gtk_box_new( GTK_ORIENTATION_VERTICAL, 1)));
		g_object_set( (GObject*)G.vbox,
			      "height-request", -1,
			      NULL);

		for ( auto &J : G ) {
			J.da = gtk_drawing_area_new();
			gtk_box_pack_start( (GtkBox*)G.vbox,
					    J.da, TRUE, TRUE, 2);

			// determine tl_left_margin
			{
				cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( J.da));
				cairo_text_extents_t extents;
				cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
				cairo_set_font_size( cr, 11);
				cairo_text_extents( cr, J.csubject.name(), &extents);
				if ( tl_left_margin < extents.width )
					tl_left_margin = extents.width;
				cairo_destroy( cr);
			}

			gtk_widget_add_events( J.da,
					       (GdkEventMask)
					       GDK_EXPOSURE_MASK |
					       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
					       GDK_SCROLL_MASK |
					       GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
					       GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
					       GDK_POINTER_MOTION_MASK);
			g_signal_connect( J.da, "draw",
					  (GCallback)daSubjectTimeline_draw_cb,
					  &J);
			g_signal_connect( J.da, "enter-notify-event",
					  (GCallback)daSubjectTimeline_enter_notify_event_cb,
					  &J);
			g_signal_connect( J.da, "leave-notify-event",
					  (GCallback)daSubjectTimeline_leave_notify_event_cb,
					  &J);
			g_signal_connect( J.da, "scroll-event",
					  (GCallback)daSubjectTimeline_scroll_event_cb,
					  &J);
			g_signal_connect( J.da, "button-press-event",
					  (GCallback)daSubjectTimeline_button_press_event_cb,
					  &J);
			g_signal_connect( J.da, "motion-notify-event",
					  (GCallback)daSubjectTimeline_motion_notify_event_cb,
					  &J);

			g_signal_connect( J.da, "drag-data-received",
					  (GCallback)common_drag_data_received_cb,
					  this);
			g_signal_connect( J.da, "drag-drop",
					  (GCallback)common_drag_drop_cb,
					  this);
			gtk_drag_dest_set( J.da, GTK_DEST_DEFAULT_ALL,
					   NULL, 0, GDK_ACTION_COPY);
			gtk_drag_dest_add_uri_targets( J.da);
		}
	}

      // walk quickly one last time to set widget attributes (importantly, involving tl_left_margin)
	tl_left_margin += 10;
	for ( auto &G : groups )
		for ( auto &J : G )
			g_object_set( (GObject*)J.da,
				      "can-focus", TRUE,
				      "app-paintable", TRUE,
				      "height-request", timeline_height,
				      "width-request", timeline_width + tl_left_margin + tl_right_margin,
				      NULL);

	snprintf_buf( "<small>%zusec/%gHz/%s</small>",
		      ED->fft_params.pagesize,
		      ED->fft_params.binsize,
		      sigfile::SFFTParamSet::welch_window_type_name( ED->fft_params.welch_window_type));
	gtk_label_set_markup( lMsmtPSDInfo, __buf__);

	snprintf_buf( "<small>%gHz/%g/%g</small>",
		      ED->mc_params.bandwidth,
		      ED->mc_params.iir_backpolate,
		      ED->mc_params.mc_gain);
	gtk_label_set_markup( lMsmtMCInfo, __buf__);

	suppress_redraw = false;
//	set_cursor_busy( false, (GtkWidget*)wMainWindow);
	gtk_widget_show_all( (GtkWidget*)(cMeasurements));
}




// eof
