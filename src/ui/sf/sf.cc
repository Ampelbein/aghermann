/*
 *       File name:  ui/sf/sf.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  scoring facility
 *
 *         License:  GPL
 */

#include <stdexcept>

#include "common/config-validate.hh"
#include "common/fs.hh"
#include "ui/misc.hh"
#include "sf.hh"
#include "d/artifacts.hh"
#include "d/filters.hh"
#include "d/phasediff.hh"
#include "d/patterns.hh"

using namespace std;


size_t	aghui::SScoringFacility::IntersignalSpace = 120,
	aghui::SScoringFacility::HypnogramHeight  =  40,
	aghui::SScoringFacility::EMGProfileHeight =  30;


// class aghui::SScoringFacility

const array<unsigned, 9>
	aghui::SScoringFacility::DisplayPageSizeValues = {
	{4, 5, 10, 15, 20, 30, 60, 60*3, 60*5}
};

size_t
__attribute__ ((pure))
aghui::SScoringFacility::
figure_display_pagesize_item( const size_t seconds)
{
	size_t i = 0;
	while ( i < DisplayPageSizeValues.size()-1 && DisplayPageSizeValues[i] < seconds )
		++i;
	return i;
}


aghui::SScoringFacility::
SScoringFacility (agh::CSubject& J,
		  const string& D, const string& E,
		  aghui::SExpDesignUI& parent)
      : _p (parent),
	_csubject (J),
	_session (D),
	_sepisode (J.measurements.at(D)[E]),
	hypnogram_button_down (false),
	mode (TMode::scoring),
	crosshair_at (10),
	show_cur_pos_time_relative (false),
	draw_crosshair (false),
	alt_hypnogram (false),
	pagesize_item (figure_display_pagesize_item( parent.pagesize())),
	_cur_page (0),
	_cur_vpage (0),
	skirting_run_per1 (.04),
	ica (nullptr),
	interchannel_gap (IntersignalSpace),
	n_hidden (0),
	config_keys_b ({
		confval::SValidator<bool>("show_cur_pos_time_relative",	&show_cur_pos_time_relative),
		confval::SValidator<bool>("draw.crosshair",		&draw_crosshair),
		confval::SValidator<bool>("draw.alt_hypnogram",		&alt_hypnogram),
	}),
	config_keys_d ({
		confval::SValidator<int>("cur_vpage",			(int*)&_cur_vpage,	confval::SValidator<int>::SVFRangeIn (0, INT_MAX)),
		confval::SValidator<int>("pagesize_item",		(int*)&pagesize_item,	confval::SValidator<int>::SVFRangeIn (0, DisplayPageSizeValues.size()-1)),
	}),
	config_keys_g ({
		confval::SValidator<float>("montage.interchannel_gap",	&interchannel_gap,	confval::SValidator<float>::SVFRangeIn (0., 400.)),
		confval::SValidator<float>("montage.height",		&da_ht,			confval::SValidator<float>::SVFRangeIn (10., 4000.)),
	}),
	_patterns_d (nullptr),
	_filters_d (nullptr),
	_phasediff_d (nullptr),
	_artifacts_d (nullptr),
	_artifacts_simple_d (nullptr),
	using_channel (nullptr),
	da_wd (800), // gets properly set in a configure_event cb
	da_ht (NAN) // bad value, to be estimated unless previously saved
{
	aghui::SBusyBlock bb (_p.wMainWindow);

      // complete widget construction
      // histogram -> scores
	get_hypnogram();
	calculate_scored_percent();

      // add channels, EEGs first, then EOG, EMG, then the rest
	{
		size_t	y = interchannel_gap / 2.;
		int	seq = 1;
		for ( auto &H : _sepisode.recordings )
			if ( H.second.signal_type() == sigfile::SChannel::TType::eeg ) {
				snprintf_buf( "Reading and processing EEG channel %s ...", H.first.c_str());
				_p.sb_message( __buf__);
				channels.emplace_back( H.second, *this, y, seq++);
				y += interchannel_gap;
			}
		for ( auto &H : _sepisode.recordings )
			if ( H.second.signal_type() == sigfile::SChannel::TType::eog ) {
				snprintf_buf( "Reading and processing EOG channel %s ...", H.first.c_str());
				_p.sb_message( __buf__);
				channels.emplace_back( H.second, *this, y, seq++);
				y += interchannel_gap;
			}
		for ( auto &H : _sepisode.recordings )
			if ( H.second.signal_type() == sigfile::SChannel::TType::emg ) {
				snprintf_buf( "Reading and processing EMG channel %s ...", H.first.c_str());
				_p.sb_message( __buf__);
				channels.emplace_back( H.second, *this, y, seq++);
				y += interchannel_gap;
			}
		for ( auto &H : _sepisode.recordings ) {
			auto type = H.second.signal_type();
			if ( type != sigfile::SChannel::TType::eeg &&
			     type != sigfile::SChannel::TType::eog &&
			     type != sigfile::SChannel::TType::emg &&
			     type != sigfile::SChannel::TType::embedded_annotation ) {
				snprintf_buf( "Reading and processing channel %s ...", H.first.c_str());
				_p.sb_message( __buf__);
				channels.emplace_back( H.second, *this, y, seq++);
				y += interchannel_gap;
			}
		}
	}
	if ( channels.empty() )
		throw invalid_argument( string ("No channels found for combination (") + J.id + ", " + D + ", " + E + ")");

      // collect common annotations
	for ( auto& H : channels )
		for ( auto& A : H.crecording.F().annotations() )
			if ( not any_of( common_annotations.begin(), common_annotations.end(),
					 [&A]( const pair<const sigfile::CSource*, const sigfile::SAnnotation*>& a1)
					 { return *a1.second == A; }) )
				common_annotations.push_back( {&H.crecording.F(), &A}); // bitch&
	common_annotations.sort(
	      []( const pair<const sigfile::CSource*, const sigfile::SAnnotation*>& a1,
		  const pair<const sigfile::CSource*, const sigfile::SAnnotation*>& a2)
	      { return *a1.second < *a2.second; });
	printf( "collected %zu embedded annotations\n", common_annotations.size());

      // count n_eeg_channels
	n_eeg_channels =
		count_if( channels.begin(), channels.end(),
			  [] (const SChannel& h)
			  { return h.type == sigfile::SChannel::TType::eeg; });

      // load montage, recalibrate display scales as necessary
	load_montage();
	if ( !isfinite(da_ht) )
		estimate_montage_height();

	for ( auto &h : channels ) {
		if ( not isfinite(h.signal_display_scale) )
			h.signal_display_scale =
				agh::alg::calibrate_display_scale(
					h.signal_filtered,
					vpagesize() * h.samplerate() * min (h.crecording.total_pages(), (size_t)10),
					interchannel_gap / 2);
		agh::alg::ensure_within( h.signal_display_scale, 1e-9, 1e9);

		if ( h.type == sigfile::SChannel::TType::eeg ) {
		      // calibrate profile display scales
			if ( not isfinite(h.psd.display_scale) )
				h.psd.display_scale =
					agh::alg::calibrate_display_scale(
						h.psd.course_in_bands[metrics::psd::TBand::delta],
						h.psd.course.size(),
						interchannel_gap / 4);
			agh::alg::ensure_within( h.psd.display_scale, 1e-9, 1e9);

			if ( not isfinite(h.mc.display_scale) )
				h.mc.display_scale =
					agh::alg::calibrate_display_scale(
						h.mc.course,
						h.mc.course.size(),
						interchannel_gap / 4);
			agh::alg::ensure_within( h.mc.display_scale, 1e-9, 1e9);

			if ( not isfinite(h.swu.display_scale) )
				h.swu.display_scale =
					agh::alg::calibrate_display_scale(
						h.swu.course,
						h.swu.course.size(),
						interchannel_gap / 4);
			agh::alg::ensure_within( h.swu.display_scale, 1e-9, 1e9);

		} else if ( h.type == sigfile::SChannel::TType::emg ) {
			if ( not isfinite(h.emg_display_scale) )
				h.emg_display_scale =
					agh::alg::calibrate_display_scale(
						h.raw_profile,
						h.raw_profile.size(),
						interchannel_gap / 4);
			agh::alg::value_within( h.emg_display_scale, 1e-9, 1e9);
		}

		h._put_selection();
	}

      // set up other controls
	// suppress flicker
	suppress_redraw = true;

	// set window title
	gtk_window_set_title(
		(GtkWindow*)wSF,
		(snprintf_buf( "Scoring: %s’s %s in %s", J.name.c_str(), E.c_str(), D.c_str()),
		 __buf__));

	// align empty area next to EMG profile with spectrum panes vertically
	// g_object_set( (GObject*)cSFSleepStageStats,
	// 	      "width-request", settings::WidgetSize_SFSpectrumWidth,
	// 	      NULL);
	g_object_set( (GObject*)daSFHypnogram,
		      "height-request", HypnogramHeight,
		      NULL);
	g_object_set( (GObject*)daSFMontage,
		      "height-request", (int)da_ht,
		      NULL);

	// set tooltip
	set_tooltip( TTipIdx::scoring_mode);

	// grey out phasediff button if there are fewer than 2 EEG channels
	gtk_widget_set_sensitive(
		(GtkWidget*)bSFShowPhaseDiffDialog,
		(n_eeg_channels >= 2));

	// desensitize iSFAcceptAndTakeNext unless there are more episodes
	gtk_widget_set_sensitive(
		(GtkWidget*)iSFAcceptAndTakeNext,
		J.measurements.at(D).episodes.back().name() != E);
	// (de)sensitize various toolbar toggle buttons
	gtk_toggle_button_set_active(
		bSFDrawCrosshair,
		(gboolean)draw_crosshair);

	// add items to iSFPageHidden
	for ( auto &H : channels )
		if ( H.hidden ) {
			++n_hidden;
			auto item = (GtkWidget*)(H.menu_item_when_hidden =
						 (GtkMenuItem*)gtk_menu_item_new_with_label( H.name.c_str()));
			g_object_set(
				(GObject*)item,
				"visible", TRUE,
				NULL);
			g_signal_connect(
				(GObject*)item,
				"activate", (GCallback)iSFPageShowHidden_activate_cb,
				this);
			gtk_container_add(
				(GtkContainer*)iiSFPageHidden,
				item);
		}
	// if there's too many visible, suggest to hide some
	if ( channels.size() > 10 && n_hidden == 0 )
		aghui::pop_ok_message(
			wSF, "<b>Montage seems overcrowded</b>",
			"The recording you are about to view has %zu channels.\n"
			"You can hide some using appropriate channel context menus.", channels.size() - n_hidden);

	{
		int bar_height;
		gtk_widget_get_size_request( (GtkWidget*)cSFControlBar, NULL, &bar_height);
		int optimal_win_height = min(
			(int)(HypnogramHeight + bar_height + da_ht + 100),
			(int)(gdk_screen_get_height( gdk_screen_get_default()) * .95));
		gtk_window_set_default_size( wSF,
					     gdk_screen_get_width( gdk_screen_get_default()) * .90,
					     optimal_win_height);
	}

	// set current page and page size
	set_cur_vpage( _cur_vpage, true);
	set_vpagesize_item( pagesize_item, true); // will do set_cur_vpage one more time, but ok

	suppress_redraw = false;
	gtk_widget_show_all( (GtkWidget*)wSF);

	// display proper control bar (has to be done on a shown widget)
	gtk_widget_set_visible( (GtkWidget*)cSFScoringModeContainer, TRUE);
	gtk_widget_set_visible( (GtkWidget*)cSFICAModeContainer, FALSE);

	queue_redraw_all();

      // advise parent we are open
	_p.open_scoring_facilities.push_front( this);
	gtk_widget_set_visible( (GtkWidget*)_p.iExpRefresh, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.iExpClose, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.tSettings, FALSE);

	// tell main window we are done (so it can start another instance of scoring facility)
	_p.sb_clear();
}


aghui::SScoringFacility::
~SScoringFacility ()
{
	// put scores
	put_hypnogram();

	// save montage
	save_montage();

	// cause repopulate
	redraw_ssubject_timeline();

	_p.open_scoring_facilities.remove( this);
	bool enable_expd_destructive_controls =
		_p.open_scoring_facilities.empty();
	gtk_widget_set_visible( (GtkWidget*)_p.iExpRefresh,
				enable_expd_destructive_controls);
	gtk_widget_set_visible( (GtkWidget*)_p.iExpClose,
				enable_expd_destructive_controls);
	gtk_widget_set_visible( (GtkWidget*)_p.tSettings,
				enable_expd_destructive_controls);

	if ( ica )
		delete ica;

	if ( _artifacts_d )
		delete _artifacts_d;
	if ( _patterns_d )
		delete _patterns_d;
	if ( _phasediff_d )
		delete _phasediff_d;
	if ( _filters_d )
		delete _filters_d;
}

void
aghui::SScoringFacility::
redraw_ssubject_timeline() const
{
	auto j = _p.subject_presentation_by_csubject( _csubject);
	if ( j ) {
		j->create_cprofile();
		gtk_widget_queue_draw( (GtkWidget*)j->da);
	}
}



aghui::SScoringFacility::SChannel&
aghui::SScoringFacility::
operator[]( const string& ch)
{
	auto iter = find( channels.begin(), channels.end(), ch);
	if ( unlikely (iter == channels.end()) )
		throw invalid_argument( string ("SScoringFacility::operator[]: bad channel: ") + ch);
	return *iter;
}

aghui::SScoringFacility::SChannel&
aghui::SScoringFacility::
channel_by_idx( size_t i)
{
	for ( auto &H : channels )
		if ( i-- == 0 )
			return H;
	throw invalid_argument( string ("SScoringFacility::operator[]: bad channel idx: ") + to_string(i));
}


aghui::SScoringFacility::SChannel*
__attribute__ ((pure))
aghui::SScoringFacility::
channel_near( const int y)
{
	int nearest = INT_MAX, thisy;
	auto nearest_h = &channels.front();
	for ( auto &H : channels ) {
		if ( H.hidden )
			continue;
		thisy = (y > H.zeroy) ? y - H.zeroy : H.zeroy - y;
			// if ( thisy < nearest )
			// 	return &const_cast<SChannel&>(H);
			// else
			// 	return nearest_h;
		if ( thisy < nearest ) {
			nearest = thisy;
			nearest_h = &H;
		}
	}
	return nearest_h;
}





void
aghui::SScoringFacility::
update_all_channels_profile_display_scale()
{
	for ( auto& H : channels )
		if ( sigfile::SChannel::signal_type_is_fftable( H.type) )
			H.update_profile_display_scales();
}



void
aghui::SScoringFacility::
get_hypnogram()
{
	// just get from the first source,
	// trust other sources are no different
	auto &F = _sepisode.sources.front();
	hypnogram.resize( F.sigfile::CHypnogram::pages());
	for ( size_t p = 0; p < F.pages(); ++p )
		hypnogram[p] = F[p].score_code();
}
void
aghui::SScoringFacility::
put_hypnogram()
{
	// but put to all
	for( auto &F : _sepisode.sources )
		for ( size_t p = 0; p < F.sigfile::CHypnogram::pages(); ++p )
			F[p].mark( hypnogram[p]);
}




void
aghui::SScoringFacility::
calculate_scored_percent()
{
	using namespace sigfile;
	scored_percent_nrem =
		(float)count_if( hypnogram.begin(), hypnogram.end(),
				 [] ( const char& c)
				 {
					 return
					    c == SPage::score_code(SPage::TScore::nrem1)
					 || c == SPage::score_code(SPage::TScore::nrem2)
					 || c == SPage::score_code(SPage::TScore::nrem3)
					 || c == SPage::score_code(SPage::TScore::nrem4);
				 }) / hypnogram.size() * 100;
	scored_percent_rem =
		(float)count( hypnogram.begin(), hypnogram.end(),
			      SPage::score_code(SPage::TScore::rem)) / hypnogram.size() * 100;
	scored_percent_wake =
		(float)count( hypnogram.begin(), hypnogram.end(),
			      SPage::score_code(SPage::TScore::wake)) / hypnogram.size() * 100;

	scored_percent =
		100. - (float)count( hypnogram.begin(), hypnogram.end(),
				     SPage::score_code(SPage::TScore::none)) / hypnogram.size() * 100;
}



void
aghui::SScoringFacility::
set_cur_vpage( size_t p, const bool touch_self)
{
	if ( _cur_vpage == p && !touch_self )
		return;

	agh::alg::ensure_within( p, (size_t)0, total_vpages()-1);
	_cur_vpage = p;

	if ( ap2p(p) != _cur_page ) { // vpage changed but page is same
		_cur_page = ap2p(p);
		for ( auto& H : channels )
			if ( H.type == sigfile::SChannel::TType::eeg && H.draw_spectrum )
				H.get_spectrum( _cur_page);

		gtk_widget_set_sensitive( (GtkWidget*)bSFForward, _cur_vpage < total_vpages()-1);
		gtk_widget_set_sensitive( (GtkWidget*)bSFBack, _cur_vpage > 0);
	}

	if ( touch_self )
		gtk_spin_button_set_value( eSFCurrentPage, _cur_vpage+1);

	draw_current_pos( 0.);
}

void
aghui::SScoringFacility::
set_vpagesize_item( int item, const bool touch_self)
{
	if ( pagesize_item == item && !touch_self )
		return;

	agh::alg::ensure_within( item, 0, (int)DisplayPageSizeValues.size()-1 );
	pagesize_item = item;
	gtk_adjustment_set_upper( jSFPageNo, total_vpages());

	set_cur_vpage( p2ap(_cur_page), true);

	gboolean sensitive_indeed = pagesize_is_right();
	for ( auto& B : {bSFScoreClear, bSFScoreNREM1, bSFScoreNREM2, bSFScoreNREM3, bSFScoreNREM4, bSFScoreREM, bSFScoreWake,
				bSFGotoPrevUnscored, bSFGotoNextUnscored} )
		gtk_widget_set_sensitive( (GtkWidget*)(B), sensitive_indeed);

	snprintf_buf( "of %zu", total_vpages());
	gtk_label_set_markup( lSFTotalPages, __buf__);

	if ( touch_self )
		gtk_combo_box_set_active( eSFPageSize, pagesize_item);

	draw_current_pos( 0.);
}



void
aghui::SScoringFacility::
do_score_forward( const char score_ch)
{
	hypnogram[_cur_page] = score_ch;
	calculate_scored_percent();
	draw_score_stats();
	set_cur_vpage( _cur_page+1);
}

void
aghui::SScoringFacility::
do_score_back( const char score_ch)
{
	hypnogram[_cur_page] = score_ch;
	calculate_scored_percent();
	draw_score_stats();
	set_cur_vpage( _cur_page-1);
}

size_t
aghui::SScoringFacility::SChannel::
marquee_to_selection()
{
	if ( marquee_mstart < marquee_mend) {
		marquee_start = marquee_mstart;
		marquee_end = marquee_mend;
	} else {
		marquee_start = marquee_mend;
		marquee_end = marquee_mstart;
	}

	selection_start = sample_at_click( marquee_start);
	selection_end   = sample_at_click( marquee_end);

	if ( selection_start > n_samples() )
		selection_start = selection_end = 0;
	else if ( selection_end > n_samples() )
		selection_end = n_samples();

	selection_start_time = (double)selection_start / samplerate();
	selection_end_time   = (double)selection_end / samplerate();

	return (selection_end - selection_start);
}



bool
aghui::SScoringFacility::
page_has_artifacts( const size_t p, const bool search_all) const
{
	for ( auto &H : channels )
		if ( ! search_all && H.hidden )
			continue;
		else {
			if ( ((agh::alg::SSpan<double> ((double)p, (double)p+1)) * (double)vpagesize())
			     . dirty( H.artifacts()) > 0. )
				return true;
		}
	return false;
}


bool
aghui::SScoringFacility::
page_has_annotations( const size_t p, const SChannel& H) const
{
	int	half_pad_samples = skirting_run_per1 * vpagesize() * H.samplerate();
	int	cvpa =  p    * pagesize() * H.samplerate() - half_pad_samples,
		cvpe = (p+1) * pagesize() * H.samplerate() + half_pad_samples;
	for ( auto &A : H.annotations )
		if ( agh::alg::overlap( (int)A.span.a, (int)A.span.z, cvpa, cvpe) )
			return true;
		else if ( (int)A.span.a > cvpe )  // no more up to and on current page
			return false;
	return false;
}



void
aghui::SScoringFacility::
draw_score_stats() const
{
	snprintf_buf( "<b>%3.1f</b> %% scored", scored_percent);
	gtk_label_set_markup( lSFPercentScored, __buf__);

	snprintf_buf( "<small>%3.1f</small> %%", scored_percent_nrem);
	gtk_label_set_markup( lScoreStatsNREMPercent, __buf__);

	snprintf_buf( "<small>%3.1f</small> %%", scored_percent_rem);
	gtk_label_set_markup( lScoreStatsREMPercent, __buf__);

	snprintf_buf( "<small>%3.1f</small> %%", scored_percent_wake);
	gtk_label_set_markup( lScoreStatsWakePercent, __buf__);
}


void
aghui::SScoringFacility::
draw_current_pos( const double x) const
{
	static const time_t epoch_clockhour = 3 * 60 * 60;
	if ( isfinite(x) ) {
		double	clickt = time_at_click( x);
		if ( likely (clickt > 0.) ) {
			time_t time_at_cur_pos =
				(time_t)(clickt + (show_cur_pos_time_relative ? -epoch_clockhour : start_time()));
			struct tm *ltime = localtime( &time_at_cur_pos);
			char tmp[10];
			strftime( tmp, 9, "%H:%M:%S", ltime);
			snprintf_buf( "%s.%02d",
				      tmp, (int)((clickt - floor(clickt)) * 100));
		} else
			snprintf_buf( "--:--:--");
	} else {
		time_t time_at_cur_pos = cur_vpage_start()
			+ (time_t)(show_cur_pos_time_relative ? -epoch_clockhour : start_time());
		struct tm *ltime = localtime( &time_at_cur_pos);
		char tmp[10];
		strftime( tmp, 9, "%H:%M:%S", ltime);
		snprintf_buf( "%s",
			      tmp);
	}

	gtk_button_set_label( eSFCurrentPos, __buf__);
}


void
aghui::SScoringFacility::
queue_redraw_all() const
{
	if ( suppress_redraw )
		return;
	draw_score_stats();
	gtk_widget_queue_draw( (GtkWidget*)daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)daSFHypnogram);
}










void
aghui::SScoringFacility::
load_montage()
{
	libconfig::Config conf;
	string montage_file = (agh::fs::make_fname_base( channels.front().crecording.F().filename(), ".edf", true) + ".montage");
	try {
		conf.readFile (montage_file.c_str());
	} catch (libconfig::ParseException ex) {
		fprintf( stderr, "Failed parsing montage file %s:%d: %s\n",
			 montage_file.c_str(),
			 ex.getLine(), ex.getError());
		return;
	} catch (libconfig::FileIOException ex) {
		return;
	}
	confval::get( config_keys_b, conf);
	confval::get( config_keys_d, conf);

	for ( auto &h : channels ) {
		confval::get( h.config_keys_b, conf);
		confval::get( h.config_keys_d, conf);
		confval::get( h.config_keys_g, conf);

	      // postprocess a little
		h.selection_start = h.selection_start_time * h.samplerate();
		h.selection_end = h.selection_end_time * h.samplerate();

	      // make sure these won't cause any confusion later
		if ( h.type == sigfile::SChannel::TType::eeg )
			h.draw_emg = false;
		if ( h.type == sigfile::SChannel::TType::emg )
			h.draw_psd = h.draw_swu = h.draw_mc = false;
	}

      // any additional checks
	;
}



void
aghui::SScoringFacility::
save_montage()
{
	libconfig::Config conf;
	confval::put( config_keys_b, conf);
	confval::put( config_keys_d, conf);

	for ( auto &h : channels ) {
		confval::put( h.config_keys_b, conf);
		confval::put( h.config_keys_d, conf);
		confval::put( h.config_keys_g, conf);
	}
	try {
		conf.writeFile ((agh::fs::make_fname_base( channels.front().crecording.F().filename(), ".edf", true) + ".montage").c_str());
	} catch (...) {
		;
	}
}



void
aghui::SScoringFacility::
sb_message( const string& msg) const
{
	gtk_statusbar_pop(  sbSF, sbSFContextIdGeneral);
	gtk_statusbar_push( sbSF, sbSFContextIdGeneral, msg.c_str());
}

void
aghui::SScoringFacility::
sb_clear() const
{
	gtk_statusbar_pop(  sbSF, sbSFContextIdGeneral);
}



const char* const
	aghui::SScoringFacility::tooltips[2] = {
	"<b>Page views:</b>\n"
	"  Wheel:	adjust display scale;\n"
	"  Ctrl+Wheel:	change scale for\n"
	"		all channels;\n"
	"  Click2:	reset display scale;\n"
	"  Move1:	mark selection;\n"
	"  Alt+Move1:	move channel around\n"
	"		in montage;\n"
	" Alt+Wheel:	change montage height;\n"
	" <i>on profile:</i>\n"
	"  Click1:	position cursor;\n"
	"  Click2:	bands/discrete 1Hz bins.\n"
	"  Shift+Wheel:	cycle focused PSD band\n"
	"		/ in-/decrement bin;\n"
	"  Wheel:	in-/decrement scale;\n"
	"  Ctrl+Wheel:	in-/decrement scale for all.\n"
	"  Alt+1..9:	context menu for channels 1..9\n"
	"\n"
	"<b>Hypnogram:</b>\n"
	"  Click1:	position cursor;\n"
	"  Click2:	alt view;\n"
	"  Click3:	context menu.",

	"<b>ICA:</b>\n"
	"  Wheel:	adjust display scale;\n"
	"  Click1:	\"apply\" toggle;\n"
	"  Click3:	IC map context menu.\n",
};


void
aghui::SScoringFacility::
set_tooltip( TTipIdx i) const
{
	gtk_widget_set_tooltip_markup( (GtkWidget*)lSFHint, tooltips[i]);
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

