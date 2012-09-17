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
#include <functional>

#include <cairo.h>
#include <cairo-svg.h>
//#include <vte/vte.h>

#include "../common/config-validate.hh"
#include "../libsigfile/page-metrics-base.hh"
#include "../expdesign/primaries.hh"
#include "globals.hh"
#include "expdesign.hh"
#include "expdesign_cb.hh"
#include "scoring-facility.hh"
#include "modelrun-facility.hh"

using namespace std;

using namespace aghui;




aghui::SExpDesignUI::SSubjectPresentation::
SSubjectPresentation( agh::CSubject& _j,
		      SGroupPresentation& parent)
      : csubject (_j),
	using_episode (nullptr),
	is_focused (false),
	_p (parent),
	da (nullptr)
{
	cscourse = nullptr;
	create_cscourse();
}


void
aghui::SExpDesignUI::SSubjectPresentation::
create_cscourse()
{
	if ( cscourse )
		delete cscourse;
	try {
		cscourse =
			new agh::CSCourse (
				csubject, *_p._p._AghDi, *_p._p._AghTi,
				agh::SSCourseParamSet {
					_p._p.display_profile_type,
					_p._p.operating_range_from, _p._p.operating_range_upto,
					0., 0, false});
		tl_start = csubject.measurements[*_p._p._AghDi].episodes.front().start_rel;
	} catch (...) {  // can be invalid_argument (no recording in such session/channel) or some TSimPrepError
		cscourse = nullptr;
		fprintf( stderr, "SSubjectPresentation::SSubjectPresentation(): subject %s has no recordings in session %s channel %s\n",
			 csubject.name(), _p._p.AghD(), _p._p.AghT());
	}
}

aghui::SExpDesignUI::SSubjectPresentation::
~SSubjectPresentation()
{
	if ( cscourse )
		delete cscourse;
}





aghui::SExpDesignUI::SSubjectPresentation*
__attribute__ ((pure))
SExpDesignUI::
subject_presentation_by_csubject( const agh::CSubject& j)
{
	for ( auto& G : groups )
		for ( auto& J : G )
			if ( j == J.csubject )
				return &J;
	return nullptr;
}



const char
	*const aghui::SExpDesignUI::FreqBandNames[(size_t)sigfile::TBand::_total] = {
	"Delta", "Theta", "Alpha", "Beta", "Gamma",
};

const array<unsigned, 4>
	aghui::SExpDesignUI::FFTPageSizeValues = {{4, 20, 30, 60}};
const array<double, 3>
	aghui::SExpDesignUI::FFTBinSizeValues = {{.1, .25, .5}};



aghui::SExpDesignUI::
SExpDesignUI (aghui::SSessionChooser *parent,
	      const string& dir)
      : // let ED and cgroups be initialized after the UI gets constructed
	// so we could entertain the user with progress_indicator
	// ED (NULL),
	// groups (*this),  // incomplete
	_p (parent),
	using_subject (nullptr),
	draw_nremrem_cycles (true),
	finalize_ui (false),
	suppress_redraw (false),
	display_profile_type (sigfile::TMetricType::Psd),
	operating_range_from (2.),
	operating_range_upto (3.),
	pagesize_item (2),
	binsize_item (1),
	ext_score_codes (
		{{{" -0"}, {"1"}, {"2"}, {"3"}, {"4"}, {"6Rr8"}, {"Ww5"}}}
	),
	freq_bands {
		{  1.5,  4.0 },
		{  4.0,  8.0 },
		{  8.0, 12.0 },
		{ 15.0, 30.0 },
		{ 30.0, 40.0 },
	},
	profile_scale_psd (0.),
	profile_scale_mc (0.),
	autoscale (false),
	smooth_profile (1),
	timeline_height (80),
	timeline_pph (30),
	browse_command ("thunar"),
	config_keys_s ({
		confval::SValidator<string>("WindowGeometry.Main",		&_geometry_placeholder),
		confval::SValidator<string>("Common.CurrentSession",		&_aghdd_placeholder),
		confval::SValidator<string>("Common.CurrentChannel",		&_aghtt_placeholder),
		confval::SValidator<string>("Measurements.BrowseCommand",	&browse_command),
		confval::SValidator<string>("LastUsedVersion",			&last_used_version),
	}),
	config_keys_d ({
		confval::SValidator<int>("Measurements.DisplayProfileMode",	(int*)&display_profile_type,			confval::SValidator<int>::SVFRangeIn ( 0,   1)),
		confval::SValidator<int>("Measurements.SmoothSide",		(int*)&smooth_profile,				confval::SValidator<int>::SVFRangeIn ( 1,  20)),
		confval::SValidator<int>("Measurements.TimelineHeight",		(int*)&timeline_height,				confval::SValidator<int>::SVFRangeIn (10, 600)),
		confval::SValidator<int>("Measurements.TimelinePPH",		(int*)&timeline_pph,				confval::SValidator<int>::SVFRangeIn (10, 600)),
		confval::SValidator<int>("ScoringFacility.IntersignalSpace",	(int*)&SScoringFacility::IntersignalSpace,	confval::SValidator<int>::SVFRangeIn (10, 800)),
		confval::SValidator<int>("ScoringFacility.HypnogramHeight",	(int*)&SScoringFacility::HypnogramHeight,	confval::SValidator<int>::SVFRangeIn (10, 300)),
		confval::SValidator<int>("ModelRun.SWASmoothOver",		(int*)&SModelrunFacility::swa_smoothover,	confval::SValidator<int>::SVFRangeIn ( 1,   5)),
	}),
	config_keys_g ({
		confval::SValidator<float>("Measurements.ProfileScalePSD",	&profile_scale_psd,			confval::SValidator<float>::SVFRangeIn (0., 1e10)), // can be 0, will trigger autoscale
		confval::SValidator<float>("Measurements.ProfileScaleMC",	&profile_scale_mc,			confval::SValidator<float>::SVFRangeIn (0., 1e10)),
		confval::SValidator<float>("Common.OperatingRangeFrom",		&operating_range_from,			confval::SValidator<float>::SVFRangeIn (0., 20.)),
		confval::SValidator<float>("Common.OperatingRangeUpto",		&operating_range_upto,			confval::SValidator<float>::SVFRangeIn (0., 20.)),
	})
{
	if ( construct_widgets() )
		throw runtime_error ("SExpDesignUI::SExpDesignUI(): failed to construct widgets");
	nodestroy_by_cb = true;

	// scrub colors, get CwB color values from glade
	for ( auto &C : CwB )
		g_signal_emit_by_name( C.second.btn, "color-set");

	set_wMainWindow_interactive( false);
	gtk_widget_show_all( (GtkWidget*)wMainWindow);

	try {
		if ( not dir.empty() and not agh::fs::exists_and_is_writable( dir) )
			throw invalid_argument (string("Experiment directory ") + dir + " does not exist or is not writable");

		if ( dir.empty() ) { // again? only happens when user has moved a previously valid expdir between sessions
			string sure_dir = string (getenv("HOME")) + "/EmptyDummy";
			if ( agh::fs::mkdir_with_parents( sure_dir) != 0 )
				throw invalid_argument ("The last used experiment directory does not exist (or is not writable),"
							" and a dummy fallback directory could not be created in your $HOME."
							" Whatever the reason, this is really too bad: I can't fix it for you.");
		}
		ED = new agh::CExpDesign (dir,
					  {bind( &SExpDesignUI::sb_main_progress_indicator, this,
						 placeholders::_1, placeholders::_2, placeholders::_3)});
		nodestroy_by_cb = false;

		fft_params_welch_window_type_saved	= ED->fft_params.welch_window_type;
		af_dampen_window_type_saved		= ED->af_dampen_window_type;
		af_dampen_factor_saved			= ED->af_dampen_factor;
		mc_params_saved				= ED->mc_params;

		pagesize_item_saved = pagesize_item =
			figure_pagesize_item();
		binsize_item_saved = binsize_item =
			figure_binsize_item();

		populate( true);

	} catch (invalid_argument ex) {
		destruct_widgets();
		throw ex; // rethrow
	}

	set_wMainWindow_interactive( true);
}

size_t
__attribute__ ((pure))
aghui::SExpDesignUI::
figure_pagesize_item()
{
	size_t i = 0;
	while ( FFTPageSizeValues[i] < ED->fft_params.pagesize )
		++i;
	return i;
}
size_t
__attribute__ ((pure))
aghui::SExpDesignUI::
figure_binsize_item()
{
	size_t i = 0;
	while ( FFTPageSizeValues[i] < ED->fft_params.pagesize )
		++i;
	return i;
}



aghui::SExpDesignUI::
~SExpDesignUI()
{
	printf( "~SExpDesignUI(\"%s\")\n", ED->session_dir());
	delete ED;

	save_settings();
	destruct_widgets();
}




void
aghui::SExpDesignUI::
set_wMainWindow_interactive( bool indeed, bool flush)
{
	set_cursor_busy( not indeed, (GtkWidget*)wMainWindow);
	//gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, indeed);

	gtk_widget_set_sensitive( (GtkWidget*)cMsmtProfileParamsContainer, indeed);
	gtk_widget_set_sensitive( (GtkWidget*)cMeasurements, indeed);

	gtk_widget_set_visible( (GtkWidget*)lTaskSelector2, indeed);
	gtk_widget_set_visible( gtk_notebook_get_nth_page( tTaskSelector, 1), indeed);
	gtk_widget_set_visible( (GtkWidget*)lSettings, indeed);
	gtk_widget_set_sensitive( gtk_notebook_get_nth_page( tDesign, 1), indeed);

	gtk_widget_set_sensitive( (GtkWidget*)iiMainMenu, indeed);
	gtk_widget_set_sensitive( (GtkWidget*)eMsmtSession, indeed);
	gtk_widget_set_sensitive( (GtkWidget*)eMsmtChannel, indeed);

	if ( flush )
		gtk_flush();
}


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
		aghui::pop_ok_message( wMainWindow, "No usable EEG channels found in any recordings in the tree.");
	if ( AghTT.empty() or AghGG.empty() ) {
		show_empty_experiment_blurb();
		gtk_widget_set_visible( (GtkWidget*)lTaskSelector2, FALSE);
		gtk_widget_set_visible( (GtkWidget*)cMsmtMainToolbar, FALSE);
		gtk_widget_set_visible( gtk_notebook_get_nth_page( tTaskSelector, 1), FALSE);
	} else {
		populate_mChannels();
		populate_mSessions();
		populate_mGlobalAnnotations();
		populate_1();

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

	__reconnect_sessions_combo();
	g_signal_handler_unblock( eMsmtSession, eMsmtSession_changed_cb_handler_id);
	__reconnect_channels_combo();
	g_signal_handler_unblock( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
}




void
aghui::SExpDesignUI::
do_rescan_tree( bool ensure)
{
	set_wMainWindow_interactive( false);

	depopulate( false);
	ED -> sync();
	if ( ensure )
		ED -> scan_tree( {bind (&SExpDesignUI::sb_main_progress_indicator, this,
					placeholders::_1, placeholders::_2, placeholders::_3)});
	else
		ED -> scan_tree();
	populate( false);

	gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
	gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral,
			    "Scanning complete");

	set_wMainWindow_interactive( true);
}


void
aghui::SExpDesignUI::
do_purge_computed()
{
	set_wMainWindow_interactive( false);
	set_cursor_busy( true, (GtkWidget*)wMainWindow);

	snprintf_buf( "find '%s' \\( -name '.*.psd' -or -name '.*.mc' \\) -delete",
		      ED->session_dir());
	set_wMainWindow_interactive( FALSE);
	if ( system( __buf__) ) {
		fprintf( stderr, "Command '%s' returned a non-zero status. This is suspicious.\n", __buf__);
		gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
		gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral,
				    "Failed to purge cache files. This is odd.");
	}

	set_cursor_busy( false, (GtkWidget*)wMainWindow);
	gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
	gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral,
			    "Purged computed files cache");
	set_wMainWindow_interactive( true);
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
__adjust_op_freq_spinbuttons()
{
	suppress_redraw = true;

	switch ( display_profile_type ) {
	case sigfile::Psd:
		gtk_adjustment_set_step_increment( jMsmtOpFreqFrom,  ED->fft_params.binsize);
		gtk_adjustment_set_step_increment( jMsmtOpFreqWidth, ED->fft_params.binsize);
		if ( not used_eeg_samplerates.empty() )
			gtk_adjustment_set_upper(
				jMsmtOpFreqFrom,
				ED->fft_params.binsize * (ED->fft_params.compute_n_bins( used_eeg_samplerates.back()) - 1));

		gtk_widget_set_sensitive( (GtkWidget*)eMsmtOpFreqWidth, TRUE);
	    break;
	case sigfile::Mc:
		gtk_adjustment_set_step_increment( jMsmtOpFreqFrom, ED->mc_params.bandwidth);
		gtk_spin_button_set_value( eMsmtOpFreqWidth, ED->mc_params.bandwidth);
		if ( not used_eeg_samplerates.empty() )
			gtk_adjustment_set_upper(
				jMsmtOpFreqFrom,
				ED->mc_params.freq_from
				+ ED->mc_params.bandwidth * (ED->mc_params.compute_n_bins( used_eeg_samplerates.back()) - 1));

		gtk_widget_set_sensitive( (GtkWidget*)eMsmtOpFreqWidth, FALSE);
	    break;
	}

	suppress_redraw = false;
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
calculate_profile_scale()
{
	double	avg_profile_height = 0.;
	size_t	valid_episodes = 0;
	for ( auto& G : groups )
		for ( auto &J : G )
			if ( J.cscourse && !J.cscourse->mm_list().empty() ) {
				avg_profile_height += J.cscourse->metric_avg();
				++valid_episodes;
			}
	avg_profile_height /= valid_episodes;

	switch ( display_profile_type ) {
	case sigfile::TMetricType::Psd:
		profile_scale_psd = timeline_height / avg_profile_height * .3;
	    break;
	case sigfile::TMetricType::Mc:
		profile_scale_mc = timeline_height / avg_profile_height * .3;
	    break;
	}
}


void
aghui::SExpDesignUI::
populate_1()
{
	if ( ED->groups.empty() )
		return;

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




void
aghui::SExpDesignUI::
show_changelog()
{
	gtk_widget_show_all( (GtkWidget*)wAbout);
	gtk_notebook_set_current_page( cAboutTabs, 2);
}



void
aghui::SExpDesignUI::
show_empty_experiment_blurb()
{
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	const char *blurb =
		"<b><big>Empty experiment\n</big></b>\n"
		"When you have your recordings ready as a set of .edf files,\n"
		"• Create your experiment tree as follows: <i>Experiment/Group/Subject/Session</i>;\n"
		"• Have your EDF sources named <i>Episode</i>.edf, and placed in the corresponding <i>Session</i> directory, or\n"
		"• Drag-and-Drop any EDF sources onto this window and identify and place them individually.\n\n"
		"Once set up, either:\n"
		"• select <b>Experiment→Change</b> and select the top directory of the (newly created) experiment tree, or\n"
		"• select <b>Experiment→Rescan Tree</b> if this is the tree you have just populated.\n"
		"\n"
		"Or, If you have none yet, here is a <a href=\"http://johnhommer.com/academic/code/aghermann/Experiment.tar.bz2\">set of EEG data</a>, for a primer;"
		" push the button below to download it into the current directory:";
	GtkLabel *blurb_label = (GtkLabel*)gtk_label_new( "");
	gtk_label_set_markup( blurb_label, blurb);

	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)blurb_label,
			    TRUE, TRUE, 0);
	GtkWidget *bDownload = gtk_button_new_with_label("  Download  ");
	g_object_set( (GObject*)bDownload,
		      "expand", FALSE,
		      "halign", GTK_ALIGN_CENTER,
		      NULL);
	g_signal_connect( bDownload, "clicked",
			  (GCallback)bDownload_clicked_cb,
			  this);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    bDownload,
			    FALSE, FALSE, 0);

	snprintf_buf( "%s/%s/%s", PACKAGE_DATADIR, PACKAGE, AGH_BG_IMAGE_FNAME);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)gtk_image_new_from_file( __buf__),
			    TRUE, FALSE, 0);

	gtk_widget_show_all( (GtkWidget*)cMeasurements);
}


extern "C" void
bDownload_clicked_cb( GtkButton* button, gpointer userdata)
{
	auto EDp = (SExpDesignUI*)userdata;
	EDp->try_download();
}

int
aghui::SExpDesignUI::
try_download()
{
	const char
		*url = "http://johnhommer.com/academic/code/aghermann/Experiment.tar.bz2",
		*archive_file = "Experiment.tar.bz2";
	snprintf_buf( "xterm -e sh -c "
		      "'cd \"%s\" && "
		      " wget -c \"%s\" && "
		      " tar xjf \"%s\" && "
		      " rm -f \"%s\" && "
		      " echo \"Sample data set downloaded and unpacked\" && "
		      " read -p \"Press <Enter> to close this window...\"'",
		      ED->session_dir(), url, archive_file, archive_file);
	set_cursor_busy( true, (GtkWidget*)wMainWindow);
	set_wMainWindow_interactive( FALSE);
	if ( system( __buf__) ) {
		gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
		gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral,
				    "Download failed for some reason");
	}
	do_rescan_tree( true);
	populate( true);
	set_wMainWindow_interactive( TRUE);
	// gtk_container_foreach( (GtkContainer*)cMeasurements,
	// 		       (GtkCallback) gtk_widget_destroy,
	// 		       NULL);
	// GtkWidget *tTerm = vte_terminal_new();
	// gtk_box_pack_start( (GtkBox*)cMeasurements,
	// 		    tTerm,
	// 		    TRUE, FALSE, 0);
	// GPid download_process_pid;
	// char *argv[] = {
	// 	"ls",
	// 	".",
	// };
	// vte_terminal_fork_command_full(
	// 	(VteTerminal*)tTerm,
	// 	VTE_PTY_DEFAULT,
	// 	ED->session_dir(),
	// 	argv,
	// 	NULL, // char **envv,
	// 	(GSpawnFlags)0, // GSpawnFlags spawn_flags,
	// 	NULL, // GSpawnChildSetupFunc child_setup,
	// 	NULL, // gpointer child_setup_data,
	// 	&download_process_pid,
	// 	NULL); // GError **error);
	return 0;
}


void
aghui::SExpDesignUI::
buf_on_main_status_bar( bool ensure)
{
	gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
	gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral, __buf__);
	if ( ensure )
		aghui::gtk_flush();
}

void
aghui::SExpDesignUI::
sb_main_progress_indicator( const char* current, size_t n, size_t i)
{
	snprintf_buf( "(%zu of %zu) %s", i, n, current);
	buf_on_main_status_bar( true);
}




void
aghui::SExpDesignUI::
update_subject_details_interactively( agh::CSubject& J)
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


// eof
