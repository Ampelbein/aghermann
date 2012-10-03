// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  scoring facility
 *
 *         License:  GPL
 */




#include <stdexcept>

#include "../common/config-validate.hh"
#include "../common/fs.hh"
#include "misc.hh"
#include "scoring-facility.hh"
#include "scoring-facility_cb.hh"

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
figure_display_pagesize_item( size_t seconds)
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
	ica (nullptr),
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
	find_dialog (*this),
	filters_dialog (*this),
	phasediff_dialog (*this),
	using_channel (nullptr),
	da_ht (NAN) // bad value, to be estimated unless previously saved
{
      // complete widget construction
	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_resource( builder, "/org/gtk/aghermann/sf.glade", NULL) ) {
		g_object_unref( (GObject*)builder);
		throw runtime_error( "SScoringFacility::SScoringFacility(): Failed to load GtkBuilder object");
	}
	if ( construct_widgets() ||
	     find_dialog.construct_widgets() ||
	     filters_dialog.construct_widgets() ||
	     phasediff_dialog.construct_widgets() )
		throw runtime_error( "SScoringFacility::SScoringFacility(): Failed to construct own widgets");
	gtk_builder_connect_signals( builder, NULL);
	//  we do it all mostly ourself, except for some delete-event binding to gtk_true()

	aghui::SBusyBlock bb (_p.wMainWindow);

      // histogram -> scores
	get_hypnogram();
	calculate_scored_percent();

      // add channels, EEGs first, then EOG, EMG, then the rest
	{
		size_t	y = interchannel_gap / 2.;
		char	seq = 'a';
		for ( auto &H : _sepisode.recordings )
			if ( H.second.signal_type() == sigfile::SChannel::TType::eeg ) {
				snprintf_buf( "Reading and processing EEG channel %s ...", H.first.c_str());
				_p.buf_on_main_status_bar();
				channels.emplace_back( H.second, *this, y, seq++);
				y += interchannel_gap;
			}
		for ( auto &H : _sepisode.recordings )
			if ( H.second.signal_type() == sigfile::SChannel::TType::eog ) {
				snprintf_buf( "Reading and processing EOG channel %s ...", H.first.c_str());
				_p.buf_on_main_status_bar();
				channels.emplace_back( H.second, *this, y, seq++);
				y += interchannel_gap;
			}
		for ( auto &H : _sepisode.recordings )
			if ( H.second.signal_type() == sigfile::SChannel::TType::emg ) {
				snprintf_buf( "Reading and processing EMG channel %s ...", H.first.c_str());
				_p.buf_on_main_status_bar();
				channels.emplace_back( H.second, *this, y, seq++);
				y += interchannel_gap;
			}
		for ( auto &H : _sepisode.recordings ) {
			auto type = H.second.signal_type();
			if ( type != sigfile::SChannel::TType::eeg &&
			     type != sigfile::SChannel::TType::eog &&
			     type != sigfile::SChannel::TType::emg ) {
				snprintf_buf( "Reading and processing channel %s ...", H.first.c_str());
				_p.buf_on_main_status_bar();
				channels.emplace_back( H.second, *this, y, seq++);
				y += interchannel_gap;
			}
		}
	}
	if ( channels.empty() )
		throw invalid_argument( string ("No channels found for combination (") + J.name() + ", " + D + ", " + E + ")");

      // count n_eeg_channels
	n_eeg_channels =
		count_if( channels.begin(), channels.end(),
			  [] (const SChannel& h)
			  {
				  return h.type == sigfile::SChannel::TType::eeg;
			  });

      // load montage, recalibrate display scales as necessary
	load_montage();
	if ( !isfinite(da_ht) )
		estimate_montage_height();

	for ( auto &h : channels ) {
		if ( not isfinite(h.signal_display_scale) || h.signal_display_scale <= DBL_MIN )
			h.signal_display_scale =
				agh::alg::calibrate_display_scale(
					h.signal_filtered,
					vpagesize() * h.samplerate() * min (h.crecording.F().pages(), (size_t)10),
					interchannel_gap / 2);
		if ( h.type == sigfile::SChannel::TType::eeg ) {
		      // calibrate profile display scales
			if ( not isfinite(h.psd.display_scale) || h.psd.display_scale <= DBL_MIN )
				h.psd.display_scale =
					agh::alg::calibrate_display_scale(
						h.psd.course_in_bands[sigfile::TBand::delta],
						h.psd.course.size(),
						interchannel_gap / 4);
			if ( not isfinite(h.mc.display_scale) || h.mc.display_scale <= DBL_MIN )
				h.mc.display_scale =
					agh::alg::calibrate_display_scale(
						h.mc.course,
						h.mc.course.size(),
						interchannel_gap / 4);
		}
		h._put_selection();
	}

      // set up other controls
	// set window title
	snprintf_buf( "Scoring: %s’s %s in %s",
		      J.name(), E.c_str(), D.c_str());
	gtk_window_set_title( (GtkWindow*)wScoringFacility,
			      __buf__);

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


	// grey out phasediff button if there are fewer than 2 EEG channels
	gtk_widget_set_sensitive( (GtkWidget*)bSFShowPhaseDiffDialog, (n_eeg_channels >= 2));

	// desensitize iSFAcceptAndTakeNext unless there are more episodes
	gtk_widget_set_sensitive( (GtkWidget*)iSFAcceptAndTakeNext,
				  J.measurements.at(D).episodes.back().name() != E);
	// (de)sensitize various toolbar toggle buttons
	gtk_toggle_button_set_active( bSFDrawCrosshair,
				      (gboolean)draw_crosshair);

	// add items to iSFPageHidden
	for ( auto &H : channels )
		if ( H.hidden ) {
			++n_hidden;
			auto item = (GtkWidget*)(H.menu_item_when_hidden =
						 (GtkMenuItem*)gtk_menu_item_new_with_label( H.name));
			g_object_set( (GObject*)item,
				      "visible", TRUE,
				      NULL);
			g_signal_connect( (GObject*)item,
					  "activate", (GCallback)iSFPageShowHidden_activate_cb,
					  this);
			gtk_container_add( (GtkContainer*)mSFPageHidden,
					   item);
		}

	// draw all
	suppress_redraw = true;

	{
		int bar_height;
		gtk_widget_get_size_request( (GtkWidget*)cSFControlBar, NULL, &bar_height);
		int optimal_win_height = min(
			(int)(HypnogramHeight + bar_height + da_ht + 100),
			(int)(gdk_screen_get_height( gdk_screen_get_default()) * .95));
		gtk_window_set_default_size( wScoringFacility,
					     gdk_screen_get_width( gdk_screen_get_default()) * .90,
					     optimal_win_height);
	}
	set_cur_vpage( _cur_vpage, true);
	set_vpagesize_item( pagesize_item, true); // will do set_cur_vpage one more time, but ok
	suppress_redraw = false;

	gtk_widget_show_all( (GtkWidget*)wScoringFacility);
	// display proper control bar and set tooltip
	gtk_widget_set_visible( (GtkWidget*)cSFScoringModeContainer, TRUE);
	gtk_widget_set_visible( (GtkWidget*)cSFICAModeContainer, FALSE);
	set_tooltip( TTipIdx::scoring_mode);

	queue_redraw_all();

      // advise parent we are open
	_p.open_scoring_facilities.push_front( this);
	gtk_widget_set_visible( (GtkWidget*)_p.iExpRefresh, false);
	gtk_widget_set_visible( (GtkWidget*)_p.iExpClose, false);
	gtk_widget_set_visible( (GtkWidget*)_p.tSettings, false);

	// tell main window we are done (so it can start another instance of scoring facility)
	gtk_statusbar_pop( _p.sbMainStatusBar, _p.sbMainContextIdGeneral);
}


aghui::SScoringFacility::
~SScoringFacility()
{
	if ( ica )
		delete ica;

	// put scores
	put_hypnogram();

	// save montage
	save_montage();

	// destroy widgets
	gtk_widget_destroy( (GtkWidget*)wScoringFacility);
	g_object_unref( (GObject*)builder);

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
}

void
aghui::SScoringFacility::
redraw_ssubject_timeline() const
{
	auto j = _p.subject_presentation_by_csubject( _csubject);
	if ( j ) {
		j->create_cscourse();
		gtk_widget_queue_draw( (GtkWidget*)j->da);
	}
}



aghui::SScoringFacility::SChannel&
aghui::SScoringFacility::
operator[]( const char *ch)
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
set_cur_vpage( size_t p, bool touch_self)
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
set_vpagesize_item( size_t item, bool touch_self)
{
	if ( pagesize_item == item && !touch_self )
		return;

	agh::alg::ensure_within( item, (size_t)0, DisplayPageSizeValues.size()-1 );
	pagesize_item = item;
	gtk_adjustment_set_upper( jPageNo, total_vpages());

	set_cur_vpage( p2ap(_cur_page), true);

	gboolean sensitive_indeed = pagesize_is_right();
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreClear), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreNREM1), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreNREM2), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreNREM3), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreNREM4), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreREM),   sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreWake),  sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreGotoPrevUnscored), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreGotoNextUnscored), sensitive_indeed);

	snprintf_buf( "of %zu", total_vpages());
	gtk_label_set_markup( lSFTotalPages, __buf__);

	if ( touch_self )
		gtk_combo_box_set_active( eSFPageSize, pagesize_item);

	draw_current_pos( 0.);
}



void
aghui::SScoringFacility::
do_score_forward( char score_ch)
{
	hypnogram[_cur_page] = score_ch;
	calculate_scored_percent();
	draw_score_stats();
	set_cur_vpage( _cur_page+1);
}

void
aghui::SScoringFacility::
do_score_back( char score_ch)
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
page_has_artifacts( size_t p) const
{
	for ( auto &H : channels ) {
		size_t spp = vpagesize() * H.samplerate();
		if ( ((agh::alg::SSpan<size_t> (p, p+1)) * spp) . dirty( H.artifacts()) > 0. )
			return true;
	}
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
draw_current_pos( double x) const
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




aghui::SScoringFacility::SChannel*
__attribute__ ((pure))
aghui::SScoringFacility::
channel_near( int y)
{
	int nearest = INT_MAX, thisy;
	SChannel* nearest_h = &channels.front();
	for ( auto &H : channels ) {
		if ( H.hidden )
			continue;
		thisy = y - H.zeroy;
		if ( thisy < 0 ) {
			if ( -thisy < nearest )
				return &const_cast<SChannel&>(H);
			else
				return nearest_h;
		}
		if ( thisy < nearest ) {
			nearest = thisy;
			nearest_h = &H;
		}
	}
	return nearest_h;
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
		fprintf( stderr, "Failed reading montage file %s: %s\n",
			 montage_file.c_str(),
			 "ubuntu people please upgrade your libconfig to see this message"); // ex.what());
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
			h.draw_psd = h.draw_mc = false;
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
reset_montage()
{
	FAFA;
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
	" <i>on PSD/uC profile:</i>\n"
	"  Click1:	position cursor;\n"
	"  Click2:	bands/discrete 1Hz bins.\n"
	"  Shift+Wheel:	cycle focused PSD band\n"
	"		/ in-/decrement bin;\n"
	"  Shift+Alt+Wheel:\n"
	"		in-/decrement uC bin;\n"
	"  Wheel:	in-/decrement scale.\n"
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



// eof

