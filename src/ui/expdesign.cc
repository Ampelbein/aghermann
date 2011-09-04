// ;-*-C++-*-
/*
 *       File name:  ui/expdesign.cc
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

#include "../libagh/boost-config-validate.hh"
#include "misc.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

using namespace aghui;



aghui::SExpDesignUI::SSubjectPresentation::SSubjectPresentation( agh::CSubject& _j, SGroupPresentation& parent)
      : csubject (_j),
	is_focused (false),
	_p (parent),
	da (NULL)
{
	try {
		cscourse = new agh::CSCourse (csubject, *_p._p._AghDi, *_p._p._AghTi,
					      _p._p.operating_range_from, _p._p.operating_range_upto,
					      0., 0, false, false);
		tl_start = csubject.measurements[*_p._p._AghDi].episodes.front().start_rel;
	} catch (...) {  // can be invalid_argument (no recording in such session/channel) or some TSimPrepError
		cscourse = NULL;
		fprintf( stderr, "SSubjectPresentation::SSubjectPresentation(): subject %s has no recordings in session %s channel %s\n",
			 csubject.name(), _p._p.AghD(), _p._p.AghT());
	}
	using_episode = sepisodesequence().end();
}


aghui::SExpDesignUI::SSubjectPresentation::~SSubjectPresentation()
{
	if ( cscourse )
		delete cscourse;
}








const char
	*const aghui::SExpDesignUI::FreqBandNames[(size_t)agh::TBand::_total] = {
	"Delta", "Theta", "Alpha", "Beta", "Gamma",
};

const array<unsigned, 4>
	aghui::SExpDesignUI::FFTPageSizeValues = {{15, 20, 30, 60}};

const char
	*const aghui::SExpDesignUI::tooltips[2] = {
	"<b>Subject timeline:</b>\n"
	"	Ctrl+Wheel:	change scale;\n"
	"	Click1:		view/score episode;\n"
	"	Alt+Click3:	save timeline as svg.",
	""
};



aghui::SExpDesignUI::SExpDesignUI( const string& dir)
      : // let ED and cgroups be initialized after the UI gets constructed
	// so we could entertainthe user with progress_indicator
	// ED (NULL),
	// groups (*this),  // incomplete
	using_subject (NULL),
	finalize_ui (false),
	operating_range_from (2.),
	operating_range_upto (3.),
	pagesize_item (2),
	ext_score_codes {
		agh::CHypnogram::TCustomScoreCodes {{" -0", "1", "2", "3", "4", "6Rr8", "Ww5", "mM"}}
	},
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
	runbatch_iterate_ranges (false),
	config_keys_s ({
		SValidator<string>("WindowGeometry.Main",		&_geometry_placeholder),
		SValidator<string>("Common.CurrentSession",		&_aghdd_placeholder),
		SValidator<string>("Common.CurrentChannel",		&_aghtt_placeholder),
		SValidator<string>("Measurements.BrowseCommand",	&browse_command),
	}),
	config_keys_b ({
		SValidator<bool>("BatchRun.IncludeAllChannels",		&runbatch_include_all_channels),
		SValidator<bool>("BatchRun.IncludeAllSessions",		&runbatch_include_all_sessions),
		SValidator<bool>("BatchRun.IterateRanges",		&runbatch_iterate_ranges),
	}),
	config_keys_z ({
		SValidator<size_t>("Measurements.TimelineHeight",	&timeline_height,			SValidator<size_t>::SVFRange (10, 600)),
		SValidator<size_t>("Measurements.TimelinePPH",		&timeline_pph,				SValidator<size_t>::SVFRange (10, 600)),
		SValidator<size_t>("ScoringFacility.IntersignalSpace",	&SScoringFacility::IntersignalSpace,	SValidator<size_t>::SVFRange (10, 800)),
		SValidator<size_t>("ScoringFacility.SpectrumWidth",	&SScoringFacility::SpectrumWidth,	SValidator<size_t>::SVFRange (10, 800)),
		SValidator<size_t>("ScoringFacility.HypnogramHeight",	&SScoringFacility::HypnogramHeight,	SValidator<size_t>::SVFRange (10, 300)),
	}),
	config_keys_g ({
		SValidator<float>("Measurements.TimelinePPuV2",		&ppuv2,					SValidator<float>::SVFRange (1e-10, 1e10)),
		SValidator<float>("Common.OperatingRangeFrom",		&operating_range_from,			SValidator<float>::SVFRange (0., 20.)),
		SValidator<float>("Common.OperatingRangeUpto",		&operating_range_upto,			SValidator<float>::SVFRange (0., 20.)),
		SValidator<float>("ScoringFacility.NeighPagePeek",	&SScoringFacility::NeighPagePeek,	SValidator<float>::SVFRange (0., .4)),
	}),
	browse_command ("rox")
{
	if ( construct_widgets() )
		throw runtime_error ("SExpDesignUI::SExpDesignUI(): failed to construct widgets");
	nodestroy_by_cb = true;

	chooser.hist_filename = string (getenv("HOME")) + "/.config/aghermann/sessionrc";

	ED = new agh::CExpDesign( dir.empty()
				  ? (chooser_read_histfile(), chooser_get_dir())
				  : dir,
				  {bind( &SExpDesignUI::sb_progress_indicator, this, _1, _2, _3)});
	// if ( not ED->error_log().empty() ) {
	// 	gtk_text_buffer_set_text( gtk_text_view_get_buffer( lScanLog),
	// 				  ED->error_log().c_str(), -1);
	// 	gtk_widget_show_all( (GtkWidget*)wScanLog);
	// }

	if ( populate( true) )
		;

	pagesize_item_saved		= pagesize_item;
	FFTWindowType_saved		= ED->fft_params.welch_window_type;
	AfDampingWindowType_saved	= ED->af_dampen_window_type;
	FFTBinSize_saved		= ED->fft_params.bin_size;

	nodestroy_by_cb = false;
//	g_signal_handler_unblock( wMainWindow, wMainWindow_delete_event_cb_handler_id);
//	gtk_widget_show( (GtkWidget*)wMainWindow);
}


aghui::SExpDesignUI::~SExpDesignUI()
{
	delete ED;
	if ( finalize_ui ) {
		save_settings();
		g_object_unref( (GObject*)mEEGChannels);
		g_object_unref( (GObject*)mAllChannels);
		g_object_unref( (GObject*)mSessions);
		pango_font_description_free( monofont);
	}
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
aghui::SExpDesignUI::populate( bool do_load)
{
	printf( "\nSExpDesignUI::populate():\n");
	AghDD = ED->enumerate_sessions();
	_AghDi = AghDD.begin();
	print_xx( "* Sessions:", AghDD);
	AghGG = ED->enumerate_groups();
	_AghGi = AghGG.begin();
	print_xx( "* Groups:", AghGG);
	AghHH = ED->enumerate_all_channels();
	_AghHi = AghHH.begin();
	print_xx( "* All Channels:", AghHH);
	AghTT = ED->enumerate_eeg_channels();
	_AghTi = AghTT.begin();
	print_xx( "* EEG channels:", AghTT);
	AghEE = ED->enumerate_episodes();
	_AghEi = AghEE.begin();
	print_xx( "* Episodes:", AghEE);
	printf( "\n");

	if ( do_load ) {
		if ( load_settings() )
			;
		if ( geometry.w > 0 ) {// implies the rest are, too
			// gtk_window_parse_geometry( wMainWindow, _geometry_placeholder.c_str());
			gtk_window_resize( wMainWindow, geometry.w, geometry.h);
			gtk_window_move( wMainWindow, geometry.x, geometry.y);
		}
	}

	if ( AghGG.empty() ) {
		show_empty_experiment_blurb();
	} else {
		populate_mChannels();
		populate_mSessions();
		populate_1();

		gtk_widget_grab_focus( (GtkWidget*)eMsmtPSDFreqFrom);
//		populate_mSimulations( FALSE);
	}

	if ( not ED->error_log().empty() ) {
		gtk_text_buffer_set_text( gtk_text_view_get_buffer( tScanLog),
					  ED->error_log().c_str(), -1);
		gtk_widget_show_all( (GtkWidget*)wScanLog);
	}

	return 0;
}


void
aghui::SExpDesignUI::depopulate( bool do_save)
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

	__reconnect_sessions_combo();
	g_signal_handler_unblock( eMsmtSession, eMsmtSession_changed_cb_handler_id);
	__reconnect_channels_combo();
	g_signal_handler_unblock( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
}




void
aghui::SExpDesignUI::do_rescan_tree( bool ensure)
{
	set_cursor_busy( true, (GtkWidget*)wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, FALSE);
	while ( gtk_events_pending() )
		gtk_main_iteration();

	depopulate( false);
	if ( ensure )
		ED -> scan_tree( {bind (&SExpDesignUI::sb_progress_indicator, this, _1, _2, _3)});
	else
		ED -> scan_tree();
	populate( false);

	set_cursor_busy( false, (GtkWidget*)wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, TRUE);
	gtk_statusbar_push( sbMainStatusBar, sbContextIdGeneral,
			    "Scanning complete");
}




void
aghui::SExpDesignUI::populate_mSessions()
{
	g_signal_handler_block( eMsmtSession, eMsmtSession_changed_cb_handler_id);
	gtk_list_store_clear( mSessions);
	GtkTreeIter iter;
	for ( auto D = AghDD.begin(); D != AghDD.end(); ++D ) {
		gtk_list_store_append( mSessions, &iter);
		gtk_list_store_set( mSessions, &iter,
				    0, D->c_str(),
				    -1);
	}
	__reconnect_sessions_combo();
	g_signal_handler_unblock( eMsmtSession, eMsmtSession_changed_cb_handler_id);
}






void
aghui::SExpDesignUI::populate_mChannels()
{
	g_signal_handler_block( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
	gtk_list_store_clear( mEEGChannels);
	gtk_list_store_clear( mAllChannels);
	// users of mAllChannels (SF pattern) connect to model dynamically

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
	g_signal_handler_unblock( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
}






void
aghui::SExpDesignUI::__reconnect_channels_combo()
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
aghui::SExpDesignUI::__reconnect_sessions_combo()
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
aghui::SExpDesignUI::populate_1()
{
	if ( ED->n_groups() == 0 )
		return;

      // touch toolbar controls
	g_signal_handler_block( eMsmtPSDFreqFrom, eMsmtPSDFreqFrom_value_changed_cb_handler_id);
	g_signal_handler_block( eMsmtPSDFreqWidth, eMsmtPSDFreqWidth_value_changed_cb_handler_id);
	gtk_spin_button_set_value( eMsmtPSDFreqFrom, operating_range_from);
	gtk_spin_button_set_value( eMsmtPSDFreqWidth, operating_range_upto - operating_range_from);
	g_signal_handler_unblock( eMsmtPSDFreqFrom, eMsmtPSDFreqFrom_value_changed_cb_handler_id);
	g_signal_handler_unblock( eMsmtPSDFreqWidth, eMsmtPSDFreqWidth_value_changed_cb_handler_id);

	gtk_widget_set_visible( (GtkWidget*)lTaskSelector2, TRUE);
	gtk_widget_set_visible( (GtkWidget*)cMsmtFreqRange, TRUE);
	gtk_widget_set_visible( gtk_notebook_get_nth_page( tTaskSelector, 1), TRUE);

      // deal with the main drawing area
	groups.clear();
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);

	time_t	earliest_start = (time_t)-1,
		latest_end = (time_t)-1;

	printf( "SExpDesignUI::populate_1(): session \"%s\", channel \"%s\"\n", AghD(), AghT());
      // first pass: determine common timeline
	for ( auto Gi = ED->groups_begin(); Gi != ED->groups_end(); ++Gi ) {
		groups.emplace_back( Gi, *this); // precisely need the iterator, not object by reference
		SGroupPresentation& Gp = groups.back();
		for_each( Gi->second.begin(), Gi->second.end(),
			  [&] (agh::CSubject& j)
			  {
				  Gp.emplace_back( j, Gp);
				  const SSubjectPresentation& J = Gp.back();
				  if ( J.cscourse && j.have_session(*_AghDi) ) {
					  auto& ee = j.measurements[*_AghDi].episodes;
					  if ( not ee.empty() ) {
						  if ( earliest_start == (time_t)-1 || earliest_start > ee.front().start_rel )
							  earliest_start = ee.front().start_rel;
						  if ( latest_end == (time_t)-1 || latest_end < ee.back().end_rel )
							  latest_end = ee.back().end_rel;
					  } else
						  fprintf( stderr, "SExpDesignUI::populate_1(): session \"%s\", channel \"%s\" for subject \"%s\" is empty\n",
							   AghD(), AghT(), j.name());
				  }
			  });
	};

	timeline_start = earliest_start;
	timeline_end   = latest_end;
	timeline_width = (timeline_end - timeline_start) / 3600 * timeline_pph;
	timeline_pages = (timeline_end - timeline_start) / ED->fft_params.page_size;

	fprintf( stderr, "SExpDesignUI::populate_1(): common timeline:\n");
	fputs( asctime( localtime(&earliest_start)), stderr);
	fputs( asctime( localtime(&latest_end)), stderr);

	tl_left_margin = tl_right_margin = 0;

      // walk again thoroughly, set timeline drawing area length
	for ( auto G = groups.begin(); G != groups.end(); ++G ) {
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
				    (GtkWidget*)G->expander, FALSE, TRUE, 3);
		gtk_container_add( (GtkContainer*)G->expander,
				   (GtkWidget*) (G->vbox = (GtkExpander*)gtk_vbox_new( TRUE, 1)));
		g_object_set( (GObject*)G->vbox,
			      "height-request", -1,
			      NULL);

		for ( auto J = G->begin(); J != G->end(); ++J ) {
			J->da = gtk_drawing_area_new();
			gtk_box_pack_start( (GtkBox*)G->vbox,
					    J->da, TRUE, TRUE, 2);

			// determine tl_left_margin
			{
				cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( J->da));
				cairo_text_extents_t extents;
				cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
				cairo_set_font_size( cr, 11);
				cairo_text_extents( cr, J->csubject.name(), &extents);
				if ( tl_left_margin < extents.width )
					tl_left_margin = extents.width;
				cairo_destroy( cr);
			}

			gtk_widget_add_events( J->da,
					       (GdkEventMask)
					       GDK_EXPOSURE_MASK |
					       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
					       GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
					       GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
					       GDK_POINTER_MOTION_MASK);
			g_signal_connect( J->da, "draw",
					  (GCallback)daSubjectTimeline_draw_cb,
					  &*J);
			g_signal_connect( J->da, "enter-notify-event",
					  (GCallback)daSubjectTimeline_enter_notify_event_cb,
					  &*J);
			g_signal_connect( J->da, "leave-notify-event",
					  (GCallback)daSubjectTimeline_leave_notify_event_cb,
					  &*J);
			g_signal_connect( J->da, "scroll-event",
					  (GCallback)daSubjectTimeline_scroll_event_cb,
					  &*J);
			if ( J->cscourse ) {
				g_signal_connect( J->da, "button-press-event",
						  (GCallback)daSubjectTimeline_button_press_event_cb,
						  &*J);
				g_signal_connect( J->da, "motion-notify-event",
						  (GCallback)daSubjectTimeline_motion_notify_event_cb,
						  &*J);
			}

			g_signal_connect( J->da, "drag-data-received",
					  (GCallback)cMeasurements_drag_data_received_cb,
					  this);
			g_signal_connect( J->da, "drag-drop",
					  (GCallback)cMeasurements_drag_drop_cb,
					  this);
			gtk_drag_dest_set( J->da, GTK_DEST_DEFAULT_ALL,
					   NULL, 0, GDK_ACTION_COPY);
			gtk_drag_dest_add_uri_targets( J->da);
		}
	}

      // walk quickly one last time to set widget attributes (importantly, involving tl_left_margin)
	tl_left_margin += 10;
	for_each( groups.begin(), groups.end(),
		  [&] (SGroupPresentation& G)
		  {
			  for_each( G.begin(), G.end(),
				    [&] (SSubjectPresentation& J)
				    {
					    g_object_set( (GObject*)J.da,
							  "can-focus", TRUE,
							  "app-paintable", TRUE,
							  "height-request", timeline_height,
							  "width-request", timeline_width + tl_left_margin + tl_right_margin,
							  NULL);
				    });
		  });

	snprintf_buf( "<b><small>page: %zu sec  bin: %g Hz  %s</small></b>",
		      ED -> fft_params.page_size,
		      ED -> fft_params.bin_size,
		      agh::SFFTParamSet::welch_window_type_name( ED->fft_params.welch_window_type));
	gtk_label_set_markup( lMsmtInfo, __buf__);
	gtk_widget_show_all( (GtkWidget*)(cMeasurements));
}



void
aghui::SExpDesignUI::show_empty_experiment_blurb()
{
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	const char *blurb =
		"<b><big>Empty experiment\n</big></b>\n"
		"When you have your recordings ready as a set of .edf files,\n"
		"• Create your experiment tree as follows: <i>Experiment/Group/Subject/Session</i>;\n"
		"• Have your EDF sources named <i>Episode</i>.edf, and placed in the corresponding <i>Session</i> directory, or\n"
		"• Drop EDF sources onto here and identify and place them individually.\n\n"
		"Once set up, either:\n"
		"• click <b>⎇</b> and select the top directory of the (newly created) experiment tree, or\n"
		"• click <b>Refresh</b> if this is the tree you have just populated.\n";
		// "\n"
		// "If you have none yet, here is a small subset of EEG data, for a primer, from <a href=\"http://johnhommer.com/academic/aghermann/sample-dataset.tar.bz2\">here</a>.";
	GtkLabel *blurb_label = (GtkLabel*)gtk_label_new( "");
	gtk_label_set_markup( blurb_label, blurb);
	gtk_widget_set_visible( (GtkWidget*)lTaskSelector2, FALSE);
	gtk_widget_set_visible( (GtkWidget*)cMsmtFreqRange, FALSE);
	gtk_widget_set_visible( gtk_notebook_get_nth_page( tTaskSelector, 1), FALSE);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)blurb_label,
			    TRUE, TRUE, 0);
	snprintf_buf( "%s/%s/%s", PACKAGE_DATADIR, PACKAGE, AGH_BG_IMAGE_FNAME);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)gtk_image_new_from_file( __buf__),
			    TRUE, FALSE, 0);
	gtk_widget_show_all( (GtkWidget*)cMeasurements);
}





void
aghui::SExpDesignUI::buf_on_status_bar( bool ensure)
{
	gtk_statusbar_pop( sbMainStatusBar, sbContextIdGeneral);
	gtk_statusbar_push( sbMainStatusBar, sbContextIdGeneral, __buf__);
	if ( ensure )
		while ( gtk_events_pending() )
			gtk_main_iteration();
}

void
aghui::SExpDesignUI::sb_progress_indicator( const char* current, size_t n, size_t i)
{
	snprintf_buf( "(%zu of %zu) %s", i, n, current);
	buf_on_status_bar( true);
}




void
aghui::SExpDesignUI::update_subject_details_interactively( agh::CSubject& J)
{
	gtk_entry_set_text( eSubjectDetailsName, J.full_name.c_str());
	gtk_spin_button_set_value( eSubjectDetailsAge, J.age);
	gtk_toggle_button_set_active( (J.gender == agh::CSubject::TGender::male)
				      ? (GtkToggleButton*)eSubjectDetailsGenderMale
				      : (GtkToggleButton*)eSubjectDetailsGenderFemale,
				      TRUE);
	gtk_entry_set_text( eSubjectDetailsComment, J.comment.c_str());

	if ( gtk_dialog_run( (GtkDialog*)wSubjectDetails) == -5 ) {
		J.full_name.assign( gtk_entry_get_text( eSubjectDetailsName));
		J.age = gtk_spin_button_get_value( eSubjectDetailsAge);
		J.gender =
			gtk_toggle_button_get_active( (GtkToggleButton*)eSubjectDetailsGenderMale)
			? agh::CSubject::TGender::male
			: agh::CSubject::TGender::female;
		J.comment.assign( gtk_entry_get_text( eSubjectDetailsComment));
	}
}


// EOF
