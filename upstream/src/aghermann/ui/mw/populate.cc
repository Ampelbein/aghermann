/*
 *       File name:  aghermann/ui/mw/populate.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-19
 *
 *         Purpose:  measurements overview view
 *
 *         License:  GPL
 */


#include <cstring>
#include <sstream>

#include "aghermann/expdesign/expdesign.hh"
#include "aghermann/model/beersma.hh"
#include "aghermann/ui/misc.hh"
#include "mw.hh"
#include "mw_cb.hh"

using namespace std;
using namespace agh::ui;


int
SExpDesignUI::
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
		sigfile::join_channel_names( AghHH, "; ").c_str(),
		sigfile::join_channel_names( AghTT, "; ").c_str(),
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

	gtk_window_set_title(
		wMainWindow,
		(string ("Aghermann: ") + agh::str::homedir2tilda( ED->session_dir())).c_str());

	if ( ED->last_used_version != VERSION ) {
		printf( "Upgrading from version %s, here's ChangeLog for you\n", ED->last_used_version.c_str());
		show_changelog();
	}
	ED->last_used_version = VERSION;

	gtk_button_set_label(
		(GtkButton*)eMsmtProfileSmooth,
		snprintf_buf( "Smooth: %zu", smooth_profile));

	if ( AghTT.empty() )
		pop_ok_message(
			wMainWindow,
			"No EEG channels",
			"There are no EEG channels found in any recordings in the tree.");
	if ( AghTT.empty() or AghGG.empty() ) {
		show_empty_experiment_blurb();
		set_controls_for_empty_experiment( true);
	} else {
		populate_mChannels();
		populate_mSessions();
		populate_mGlobalAnnotations();
		populate_mGlobalADProfiles();
		populate_1();

		gtk_combo_box_set_active( eGlobalADProfiles, 0);

		switch ( display_profile_type ) {
		case metrics::TType::psd:
			gtk_combo_box_set_active( eMsmtProfileType, 0);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParamsPSD, TRUE);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParamsSWU, FALSE);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParamsMC, FALSE);
			gtk_widget_grab_focus( (GtkWidget*)eMsmtProfileParamsPSDFreqFrom);
		    break;
		case metrics::TType::swu:
			gtk_combo_box_set_active( eMsmtProfileType, 1);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParamsPSD, FALSE);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParamsSWU, TRUE);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParamsMC, FALSE);
			gtk_widget_grab_focus( (GtkWidget*)eMsmtProfileParamsPSDFreqFrom);
		    break;
		case metrics::TType::mc:
			gtk_combo_box_set_active( eMsmtProfileType, 2);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParamsPSD, FALSE);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParamsSWU, FALSE);
			gtk_widget_set_visible( (GtkWidget*)cMsmtProfileParamsMC, TRUE);
		    break;
		default:
			// throw something
		    break;
		}

		set_controls_for_empty_experiment( false);
	}

	if ( ED->error_log_n_messages() > 0 ) {
		if ( not suppress_scan_report ) {
			auto buffer = gtk_text_view_get_buffer( tScanLog);
			for ( const auto& L : ED->error_log() ) {

				GtkTextIter X0, X9;
				gtk_text_buffer_get_end_iter(
					buffer, &X0);
				gint x0 = gtk_text_iter_get_offset( &X0);

				gtk_text_buffer_insert_at_cursor(
					buffer, (L.first + '\n').c_str(), -1);
				gtk_text_buffer_get_iter_at_offset(
					buffer, &X0, x0);
				gtk_text_buffer_get_end_iter(
					buffer, &X9);
				// gtk_text_iter_backward_cursor_position(
				// 	&X9);

				switch ( L.second ) {
				case agh::CExpDesign::TLogEntryStyle::bold:
					gtk_text_buffer_apply_tag_by_name(
						buffer, "bold",
						&X0, &X9);
					break;
				case agh::CExpDesign::TLogEntryStyle::italic:
					gtk_text_buffer_apply_tag_by_name(
						buffer, "italic",
						&X0, &X9);
					break;
				case agh::CExpDesign::TLogEntryStyle::plain:
				default:
					break;
				}
			}
			gtk_widget_show_all( (GtkWidget*)wScanLog);
		} else
			gdk_window_beep( gtk_widget_get_window( (GtkWidget*)wMainWindow));
	}

	return 0;
}


void
SExpDesignUI::
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
SExpDesignUI::
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
SExpDesignUI::
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
				    0, H.name(),
				    -1);
	}

	for ( auto &H : AghHH ) {
		GtkTreeIter iter;
		gtk_list_store_append( mAllChannels, &iter);
		gtk_list_store_set( mAllChannels, &iter,
				    0, H.name(),
				    -1);
	}

	__reconnect_channels_combo();
	g_signal_handler_unblock( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
}





void
SExpDesignUI::
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
SExpDesignUI::
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


namespace {
const char*
annotation_type_s( const sigfile::SAnnotation::TType t)
{
	static const char* types[] = {"", "S", "K", "E"};
	return types[t];
}
}

void
SExpDesignUI::
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
						if ( last_j != J.id.c_str() ) {  // comparing pointers here
							gtk_tree_store_append( mGlobalAnnotations, &iter_j, &iter_g);
							gtk_tree_store_set( mGlobalAnnotations, &iter_j,
									    0, last_j = J.id.c_str(),
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

						for ( auto &A : annotations )
							if ( (only_plain_global_annotations and
							      A.type == sigfile::SAnnotation::plain) or
							     not only_plain_global_annotations ) {
								global_annotations.emplace_front( J, D.first, E, A);

								auto pages = A.page_span( pagesize()) * 1u;
								if ( pages.a == pages.z )
									snprintf_buf( "%u", pages.a + 1);
								else
									snprintf_buf( "%u-%u", pages.a + 1, pages.z + 1);
								gtk_tree_store_append( mGlobalAnnotations, &iter_a, &iter_e);
								gtk_tree_store_set( mGlobalAnnotations, &iter_a,
										    1, global::buf,
										    2, A.channel(),
										    3, annotation_type_s(A.type),
										    4, A.label.c_str(),
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
SExpDesignUI::
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
SExpDesignUI::
populate_1()
{
	if ( ED->groups.empty() )
		return;

      // touch toolbar controls
	suppress_redraw = true;
	gtk_spin_button_set_value( eMsmtProfileParamsPSDFreqFrom, active_profile_psd_freq_from);
	gtk_spin_button_set_value( eMsmtProfileParamsPSDFreqWidth, active_profile_psd_freq_upto - active_profile_psd_freq_from);
	gtk_spin_button_set_value( eMsmtProfileParamsSWUF0, active_profile_swu_f0);
	gtk_spin_button_set_value( eMsmtProfileParamsMCF0, active_profile_mc_f0);

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
			SSubjectPresentation& j = Gp.back(); // not const because admission_date is set right here:
			j.admission_date = (time_t)0;
			if ( j.cprofile && J.have_session(*_AghDi) ) {
				auto& ee = J.measurements[*_AghDi].episodes;
				if ( not ee.empty() ) {
					// (2)
					if ( earliest_start == (time_t)-1 || earliest_start > ee.front().start_rel )
						earliest_start = ee.front().start_rel;
					if ( latest_end == (time_t)-1 || latest_end < ee.back().end_rel )
						latest_end = ee.back().end_rel;

					j.admission_date = ee.front().start_time();
				} else
					fprintf( stderr, "SExpDesignUI::populate_1(): session \"%s\", channel \"%s\" for subject \"%s\" is empty\n",
						 AghD(), AghT(), J.id.c_str());
			}
		}
	}

	sort_subjects();

	timeline_start = earliest_start;
	timeline_end   = latest_end;
	tl_width = (timeline_end - timeline_start) / 3600 * tl_pph;
	tl_pages = (timeline_end - timeline_start) / ED->fft_params.pagesize;

	printf( "SExpDesignUI::populate_1(): common timeline:\n");
	fputs( asctime( localtime(&earliest_start)), stdout);
	fputs( asctime( localtime(&latest_end)), stdout);

	tl_left_margin = tl_right_margin = 0;

      // walk again thoroughly, set timeline drawing area length
	for ( auto &G : groups ) {
	      // convert avg episode times
		ostringstream ss;
		for ( auto &E : AghEE ) {
			pair<float, float>& avge = G.cjgroup().avg_episode_times[*_AghDi][E];
			unsigned seconds, h0, m0, s0, h9, m9, s9;
			seconds = avge.first * 24 * 60 * 60;
			h0 = seconds / 60 / 60;
			m0  = seconds % 3600 / 60;
			s0  = seconds % 60;
			seconds = avge.second * 24 * 60 * 60;
			h9 = seconds / 60 / 60;
			m9  = seconds % 3600 / 60;
			s9  = seconds % 60;

			ss << agh::str::sasprintf(
				"       <i>%s</i> %02d:%02d:%02d ~ %02d:%02d:%02d",
				E.c_str(),
				h0 % 24, m0, s0,
				h9 % 24, m9, s9);
		}

		{
			gchar *g_escaped = g_markup_escape_text( G.name(), -1);
			snprintf_buf( "<b>%s</b> (%zu) %s", g_escaped, G.size(), ss.str().c_str());
			g_free( g_escaped);
		}

		GtkExpander *expander = (GtkExpander*)gtk_expander_new( global::buf);
		gtk_expander_set_use_markup( expander, TRUE);
		g_object_set( (GObject*)expander,
			      "visible", TRUE,
			      "expanded", not group_unvisibility[G.name()],
			      "height-request", -1,
			      NULL);
		g_signal_connect( expander, "activate",
				  (GCallback)cGroupExpander_activate_cb,
				  &G);
		gtk_box_pack_start( (GtkBox*)cMeasurements,
				    (GtkWidget*)expander, FALSE, TRUE, 3);
		GtkWidget *vbox;
		gtk_container_add( (GtkContainer*)expander,
				   vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 1));
		g_object_set( (GObject*)vbox,
			      "height-request", -1,
			      NULL);

		for ( auto &J : G ) {
			J.da = gtk_drawing_area_new();
			gtk_box_pack_start( (GtkBox*)vbox,
					    J.da, TRUE, TRUE, 2);

			// determine tl_left_margin
			{
				cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( J.da));
				cairo_text_extents_t extents;
				cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
				cairo_set_font_size( cr, 11);
				cairo_text_extents( cr, J.csubject.id.c_str(), &extents);
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
					  (GCallback)cMeasurements_drag_data_received_cb,
					  this);
			g_signal_connect( J.da, "drag-drop",
					  (GCallback)cMeasurements_drag_drop_cb,
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
				      "height-request", tl_height,
				      "width-request", tl_width + tl_left_margin + tl_right_margin,
				      NULL);

	gtk_label_set_markup(
		lMsmtProfilePSDExtra,
		snprintf_buf( "<small>%gsec/%gHz/%s</small>",
			      ED->fft_params.pagesize,
			      ED->fft_params.binsize,
			      sigproc::welch_window_type_names[ED->fft_params.welch_window_type]));

	gtk_label_set_markup(
		lMsmtProfileMCExtra,
		snprintf_buf( "<small>%gHz/%g/%g</small>",
			      ED->mc_params.bandwidth,
			      ED->mc_params.iir_backpolate,
			      ED->mc_params.mc_gain));

	suppress_redraw = false;
//	set_cursor_busy( false, (GtkWidget*)wMainWindow);
	gtk_widget_show_all( (GtkWidget*)(cMeasurements));
}


void
SExpDesignUI::
sort_subjects()
{
	for ( auto Gi = groups.begin(); Gi != groups.end(); ++Gi )
		Gi->sort();
}


bool
SExpDesignUI::SSubjectPresentation::
operator<( const SSubjectPresentation& rv) const
{
	if ( _p._p.sort_segregate and csubject.gender != rv.csubject.gender )
		return csubject.gender < rv.csubject.gender;

	bool	result = false,
		unsure = true; // avoid swapping if result == false
	switch ( _p._p.sort_by ) {
	case TSubjectSortBy::name:
		result = csubject.id <  rv.csubject.id;
		unsure = csubject.id == rv.csubject.id;
		break;
	case TSubjectSortBy::age:
		result = csubject.age(*_p._p._AghDi) <  rv.csubject.age(*_p._p._AghDi);
		unsure = csubject.age(*_p._p._AghDi) == rv.csubject.age(*_p._p._AghDi);
		break;
	case TSubjectSortBy::admission_date:
		result = tl_start <  rv.tl_start;
		unsure = tl_start == rv.tl_start;
		break;
	case TSubjectSortBy::avg_profile_power:
		if ( cprofile and rv.cprofile ) {
			result = cprofile->metric_avg() < rv.cprofile->metric_avg();
			unsure = false;
		} else {
			result = false;
			unsure = false;
		}
		break;
	}

	if ( unsure )
		return false;
	if ( _p._p.sort_ascending )
		result = !result;

	return result;
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
