// ;-*-C++-*- *  Time-stamp: "2011-06-20 02:10:58 hmmr"
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




#include <initializer_list>
#include <stdexcept>
#include <fstream>

#include "libexstrom/exstrom.hh"
#include "libagh/misc.hh"
#include "misc.hh"
#include "ui.hh"
#include "settings.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {

// 	GtkMenu
// 		*mSFPage,
// 		*mSFPageSelection,
// //		*mSFPageSelectionInspectChannels,
// 		*mSFPower,
// 		*mSFScore,
// 		*mSFSpectrum;




namespace sf {

// saved variables

SGeometry
	GeometryScoringFac;



// module variables



inline namespace {

} // inline namespace



// struct member functions


// class SScoringFacility::SChannel

SScoringFacility::SChannel::SChannel( agh::CRecording& r,
				      SScoringFacility& parent,
				      size_t y0)
      : name (r.channel()),
	type (r.signal_type()),
	recording (r),
	sf (parent),
	low_pass ({INFINITY, (unsigned)-1}),
	high_pass ({INFINITY, (unsigned)-1}),
	zeroy (y0),
	draw_original_signal (false),
	draw_filtered_signal (true),
	draw_power (true),
	draw_bands (true),
	draw_spectrum_absolute (true),
	use_resample (true),
	selection_start (0),
	selection_end (0),
	_h (recording.F().which_channel(name)),
	_ssignal (recording.F()[_h])
{
      // load any previously saved filters
	{
		ifstream ifs (make_fname__common( recording.F().filename(), true) + '-' + name + ".filters");
		if ( not ifs.good() ||
		     (ifs >> low_pass.order >> low_pass.cutoff >> high_pass.order >> high_pass.cutoff,
		      not (ifs.gcount() == 4 && low_pass.is_sane() && high_pass.is_sane()) ) ) {
				low_pass.order = high_pass.order = 1;
				low_pass.cutoff = high_pass.cutoff = 0;
		     }
	}
	get_signal_original();
	get_signal_filtered();

	signal_display_scale =
		calibrate_display_scale( signal_filtered,
					 sf.vpagesize() * samplerate() * min (recording.F().length(), (size_t)10),
					 settings::WidgetSize_SFPageHeight / 2);

      // power and spectrum
	if ( agh::SChannel::signal_type_is_fftable( type) ) {

		// snprintf_buf( "(%zu/%zu) %s: power...", h+1, __n_all_channels, HH[h].name);
		// BUF_ON_STATUS_BAR;

		// power in a single bin
		from = settings::OperatingRangeFrom;
		upto = settings::OperatingRangeUpto;
		get_power();
	      // power spectrum (for the first page)
		n_bins = last_spectrum_bin = recording.n_bins();
		get_spectrum( 0);
		// will be reassigned in REDRAW_ALL
		spectrum_upper_freq = n_bins * recording.binsize();

	      // power in bands
		TBand n_bands = TBand::delta;
		while ( n_bands != TBand::_total )
			if ( settings::FreqBands[(size_t)n_bands][0] >= spectrum_upper_freq )
				break;
			else
				next(n_bands);
		uppermost_band = prev(n_bands);
		get_power_in_bands();

	      // delta comes first, calibrate display scale against it
		power_display_scale =
			calibrate_display_scale( power_in_bands[(size_t)TBand::delta],
						 power_in_bands[(size_t)TBand::delta].size(),
						 settings::WidgetSize_SFPageHeight/2.);
	      // switches
		draw_spectrum_absolute = true;
		draw_bands = true;
		focused_band = TBand::delta; // delta
	}

	if ( strcmp( type, "EMG") == 0 ) {
		emg_fabs_per_page.resize( recording.F().agh::CHypnogram::length());
		float largest = 0.;
		size_t i;
		// snprintf_buf( "(%zu/%zu) %s: EMG...", h+1, __n_all_channels, HH[h].name);
		// BUF_ON_STATUS_BAR;
		for ( i = 0; i < emg_fabs_per_page.size(); ++i ) {
			float	current = emg_fabs_per_page[i]
				= abs( valarray<float>
				       (signal_original[ slice (i * sf.pagesize() * samplerate(),
								(i+1) * sf.pagesize() * samplerate(), 1) ])).max();
			 if ( largest < current )
				 largest = current;
		 }

		 emg_scale = settings::WidgetSize_SFEMGProfileHeight/2 / largest;
	}

	percent_dirty = calculate_dirty_percent();


      // widgetz!

      // //expander and vbox
	gchar *h_escaped = g_markup_escape_text( name, -1);
	snprintf_buf( "%s <b>%s</b>", type, h_escaped);
	g_free( h_escaped);

      // page view
	;
}

SScoringFacility::SChannel::~SChannel()
{
	{
		ofstream ofs (make_fname__common( recording.F().filename(), true) + '-' + name + ".filters");
		if ( ofs.good() )
			ofs << low_pass.order << low_pass.cutoff << '\n'
			    << high_pass.order << high_pass.cutoff;
	}

}

void
SScoringFacility::SChannel::get_signal_original()
{
	// also filter in situ, for display
	if ( !have_low_pass() && !have_high_pass() )
		signal_original = recording.F().get_signal_original<const char*, float>( name);
	else if ( have_low_pass() && have_high_pass() )
		signal_original = exstrom::band_pass(
			recording.F().get_signal_original<const char*, float>( name),
			samplerate(), low_pass.cutoff, high_pass.cutoff, low_pass.order, true);
	else if ( have_low_pass() )
		signal_original = exstrom::low_pass(
			recording.F().get_signal_original<const char*, float>( name),
			samplerate(), low_pass.cutoff, low_pass.order, true);
	else
		signal_original = exstrom::high_pass(
			recording.F().get_signal_original<const char*, float>( name),
			samplerate(), high_pass.cutoff, high_pass.order, true);
}

void
SScoringFacility::SChannel::get_signal_filtered()
{
	if ( !have_low_pass() && !have_high_pass() )
		signal_filtered = recording.F().get_signal_filtered<const char*, float>( name);
	else if ( have_low_pass() && have_high_pass() )
		signal_filtered = exstrom::band_pass(
			recording.F().get_signal_filtered<const char*, float>( name),
			samplerate(), low_pass.cutoff, high_pass.cutoff, low_pass.order, true);
	else if ( have_low_pass() )
		signal_filtered = exstrom::low_pass(
			recording.F().get_signal_filtered<const char*, float>( name),
			samplerate(), low_pass.cutoff, low_pass.order, true);
	else
		signal_filtered = exstrom::high_pass(
			recording.F().get_signal_filtered<const char*, float>( name),
			samplerate(), high_pass.cutoff, high_pass.order, true);
}

float
SScoringFacility::SChannel::calibrate_display_scale( const valarray<float>& signal,
						     size_t over, float fit)
{
	float max_over = 0.;
	for ( size_t i = 0; i < over; ++i )
		if ( max_over < signal[i] )
			max_over = signal[i];
	return fit / max_over;
}


float
SScoringFacility::SChannel::calculate_dirty_percent()
{
	size_t total = 0; // in samples
	auto& af = recording.F()[name].artifacts;
	for_each( af.begin(), af.end(),
		  [&total] ( const agh::CEDFFile::SSignal::TRegion& r)
		  {
			  total += r.second - r.first;
		  });
	return percent_dirty = (float)total / n_samples();
}




void
SScoringFacility::SChannel::mark_region_as_artifact( bool do_mark)
{
	if ( do_mark )
		recording.F()[name].mark_artifact( selection_start, selection_end);
	else
		recording.F()[name].clear_artifact( selection_start, selection_end);

	calculate_dirty_percent();

	get_signal_filtered();

	if ( have_power() ) {
		get_power();
		get_power_in_bands();
		get_spectrum( sf.cur_page());
	}
	gtk_widget_queue_draw( (GtkWidget*)sf.daScoringFacMontage);
}


void
SScoringFacility::SChannel::mark_region_as_pattern()
{
	sf.find_dialog.load_pattern( *this);
}





// class SScoringFacility


SScoringFacility::SScoringFacility( agh::CSubject& J,
				    const string& D, const string& E)
      : _csubject (J),
	_sepisode (J.measurements.at(D)[E]),
	draw_crosshair (false),
	draw_power (true),
	marking_now (false),
	draw_spp (true),
	skirting_run_per1 (settings::SFNeighPagePeek),
	crosshair_at (10),
	using_channel (NULL),
	unfazer_mode (TUnfazerMode::none),
	unfazer_offending_channel (NULL),
	unfazer_factor (0.1),
	find_dialog (*this),
	filters_dialog (*this),
	phasediff_dialog (*this),
	_cur_page (0),
	_cur_vpage (0),
	pagesize_item (figure_display_pagesize_item( FFTPageSizeValues[settings::FFTPageSizeItem]))
{
	set_cursor_busy( true, (GtkWidget*)wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, FALSE);

      // complete widget construction
	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_file( builder, PACKAGE_DATADIR "/" PACKAGE "/ui/agh-ui-sf.glade" , NULL) ) {
		g_object_unref( (GObject*)builder);
		throw runtime_error( "SScoringFacility::SScoringFacility(): Failed to load GtkBuilder object");
	}
	if ( construct_widgets() ||
	     find_dialog.construct_widgets() ||
	     filters_dialog.construct_widgets() ||
	     phasediff_dialog.construct_widgets() )
		throw runtime_error( "SScoringFacility::SScoringFacility(): Failed to construct own widgets");
	gtk_builder_connect_signals( builder, NULL);
	g_object_unref( (GObject*)builder);

      // iterate all of AghHH, mark our channels
	size_t y = settings::WidgetSize_SFPageHeight / 2.;
	for ( auto H = AghHH.begin(); H != AghHH.end(); ++H ) {
		snprintf_buf( "Reading and processing channel %s...", H->c_str());
		sb::buf_on_status_bar();
		try {
			channels.emplace_back( _sepisode.recordings.at(*H),
					       *this,
					       y);
			y += settings::WidgetSize_SFPageHeight;
		} catch (...) {
		}
	}
	da_ht = montage_est_height();

      // get display scales
	{
		ifstream ifs (make_fname__common( channels.front().recording.F().filename(), true) + ".displayscale");
		if ( not ifs.good() ||
		     (ifs >> sane_signal_display_scale >> sane_power_display_scale, ifs.gcount() == 0) )
			sane_signal_display_scale = sane_power_display_scale = NAN;
	}
	// sane values, now set, will be used in SChannel ctors

	if ( channels.size() == 0 )
		throw invalid_argument( string ("No channels found for combination (") + J.name() + ", " + D + ", " + E + ")");

      // histogram -> scores
	get_hypnogram();
	calculate_scored_percent();

      // count n_eeg_channels
	n_eeg_channels =
		count_if( channels.begin(), channels.end(),
			  [] (const SChannel& h)
			  {
				  return strcmp( h.type, "EEG") == 0;
			  });

      // recalculate (average) signal and power display scales
	if ( isfinite( sane_signal_display_scale) ) {
		;  // we've got it saved previously
	} else {
		sane_signal_display_scale = sane_power_display_scale = 0.;
		size_t n_with_power = 0;
		for ( auto h = channels.begin(); h != channels.end(); ++h ) {
			sane_signal_display_scale += h->signal_display_scale;
			if ( h->have_power() ) {
				++n_with_power;
				sane_power_display_scale += h->power_display_scale;
			}
		}
		sane_signal_display_scale /= channels.size();
		sane_power_display_scale /= n_with_power;
		for ( auto h = channels.begin(); h != channels.end(); ++h ) {
			h->signal_display_scale = sane_signal_display_scale;
			if ( h->have_power() )
				h->power_display_scale = sane_power_display_scale;
		}
	}

      // set up other controls
	// set window title
	snprintf_buf( "Scoring: %s\342\200\231s %s in %s",
		      J.name(), E.c_str(), D.c_str());
	gtk_window_set_title( (GtkWindow*)wScoringFacility,
			      __buf__);

	// assign tooltip
	set_tooltip( TTipIdx::general);

	// align empty area next to EMG profile with spectrum panes vertically
	// g_object_set( (GObject*)cScoringFacSleepStageStats,
	// 	      "width-request", settings::WidgetSize_SFSpectrumWidth,
	// 	      NULL);
	g_object_set( (GObject*)daScoringFacHypnogram,
		      "height-request", settings::WidgetSize_SFHypnogramHeight,
		      NULL);
	g_object_set( (GObject*)daScoringFacMontage,
		      "height-request", da_ht,
		      NULL);


	// grey out phasediff button if there are fewer than 2 EEG channels
	gtk_widget_set_sensitive( (GtkWidget*)bScoringFacShowPhaseDiffDialog, (n_eeg_channels >= 2));

	// desensitize iSFAcceptAndTakeNext unless there are more episodes
	gtk_widget_set_sensitive( (GtkWidget*)(iSFAcceptAndTakeNext),
				  J.measurements.at(D).episodes.back().name() != E);

	// draw all
	suppress_redraw = true;

	repaint_score_stats();

	gtk_combo_box_set_active( (GtkComboBox*)(eScoringFacPageSize),
				  pagesize_item);

	gtk_spin_button_set_value( eScoringFacCurrentPage,
				   1);
	suppress_redraw = false;
	g_signal_emit_by_name( eScoringFacPageSize, "changed");

	// tell main window we are done (so it can start another instance of scoring facility)
	gtk_statusbar_pop( sbMainStatusBar, sb::sbContextIdGeneral);
	set_cursor_busy( false, (GtkWidget*)(wMainWindow));

	{
		int bar_height;
		gtk_widget_get_size_request( (GtkWidget*)cScoringFacControlBar, NULL, &bar_height);
		int optimal_win_height = min(
			(int)settings::WidgetSize_SFHypnogramHeight + bar_height + da_ht + 70,
			(int)(gdk_screen_get_height( gdk_screen_get_default()) * .92));
		gtk_window_set_default_size( wScoringFacility,
					     gdk_screen_get_width( gdk_screen_get_default()) * .90,
					     optimal_win_height);
	}
	gtk_widget_show_all( (GtkWidget*)(wScoringFacility));

	set_cursor_busy( false, (GtkWidget*)wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)wMainWindow, TRUE);
}


SScoringFacility::~SScoringFacility()
{
	// put scores
	put_hypnogram();

	// save display scales
	{
		ofstream ofs (make_fname__common( channels.front().recording.F().filename(), true) + ".displayscale");
		if ( ofs.good() )
			ofs << sane_signal_display_scale << sane_power_display_scale;
	}

	// destroy widgets
//	g_object_unref( (GObject*)wScoringFacility);
	gtk_widget_destroy( (GtkWidget*)wScoringFacility);
}


size_t
SScoringFacility::set_cur_page( size_t p)
{
	if ( p < total_pages() ) {
		_cur_page = p;
		_cur_vpage = p2ap(p);
	}
	queue_redraw_all();
	return _cur_page;
}
size_t
SScoringFacility::set_cur_vpage( size_t p)
{
	if ( ap2p(p) < total_pages() ) {
		_cur_vpage = p;

		if ( ap2p(p) != _cur_page ) { // vpage changed but page is same
			_cur_page = ap2p(p);
			for ( auto H = channels.begin(); H != channels.end(); ++H )
				if ( H->draw_power && H->have_power() )
					H->spectrum = H->recording.power_spectrum<float>( _cur_page);
		}

		// auto	cur_stage = cur_page_score();
		// snprintf_buf( "<b><big>%s</big></b>", agh::SPage::score_name(cur_stage));
		// gtk_label_set_markup( lScoringFacCurrentStage, __buf__);

		auto	cur_pos = cur_vpage_start(); // in sec
		size_t	cur_pos_hr  =  cur_pos / 3600,
			cur_pos_min = (cur_pos - cur_pos_hr * 3600) / 60,
			cur_pos_sec =  cur_pos % 60;

		snprintf_buf( "<b>%2zu:%02zu:%02zu</b>", cur_pos_hr, cur_pos_min, cur_pos_sec);
		gtk_label_set_markup( lScoringFacCurrentPos, __buf__);

		time_t time_at_cur_pos = start_time() + cur_pos;
		char tmp[10];
		strftime( tmp, 9, "%H:%M:%S", localtime( &time_at_cur_pos));
		snprintf_buf( "<b>%s</b>", tmp);
		gtk_label_set_markup( lScoringFacClockTime, __buf__);

		gtk_spin_button_set_value( eScoringFacCurrentPage, _cur_vpage+1);
		queue_redraw_all();
	}
	return _cur_vpage;
}

void
SScoringFacility::set_pagesize( int item)
{
	snprintf_buf( "<small>of</small> %zu", total_vpages());
	gtk_label_set_markup( lScoringFacTotalPages, __buf__);

	if ( item == pagesize_item || item > (int)DisplayPageSizeValues.size() )
		return;
	pagesize_item = item;
	_cur_vpage = p2ap(_cur_page);

	gtk_spin_button_set_range( eScoringFacCurrentPage, 1, total_vpages());
	gtk_spin_button_set_value( eScoringFacCurrentPage, _cur_vpage+1);

	gboolean sensitive_indeed = pagesize_is_right();
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreClear), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreNREM1), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreNREM2), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreNREM3), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreNREM4), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreREM),   sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreWake),  sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreMVT),   sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreGotoPrevUnscored), sensitive_indeed);
	gtk_widget_set_sensitive( (GtkWidget*)(bScoreGotoNextUnscored), sensitive_indeed);

	queue_redraw_all();
}



void
SScoringFacility::do_score_forward( char score_ch)
{
	if ( cur_page() < total_pages() ) {
		hypnogram[_cur_page] = score_ch;
		++_cur_page;  // it's OK as this method is not called (via callback) when !pagesize_is_right()
		++_cur_vpage;
		gtk_spin_button_set_value( eScoringFacCurrentPage, _cur_vpage+1); // implicit queue_redraw_all
		calculate_scored_percent();
		repaint_score_stats();
	}
}

size_t
SScoringFacility::SChannel::marquee_to_selection()
{
	if ( marquee_mstart < marquee_mend)
		marquee_start = marquee_mstart, marquee_end = marquee_mend;
	else
		marquee_start = marquee_mend, marquee_end = marquee_mstart;

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
SScoringFacility::page_has_artifacts( size_t p)
{
	for ( auto H = channels.begin(); H != channels.end(); ++H ) {
		auto& Aa = H->recording.F()[H->name].artifacts;
		auto spp = vpagesize() * H->samplerate();
		if ( any_of( Aa.begin(), Aa.end(),
			     [&] (const agh::CEDFFile::SSignal::TRegion& span)
			     {
				     return ( (p * spp < span.first &&
					       span.first < (p+1) * spp) ||
					      (p * spp < span.second &&
					       span.second < (p+1) * spp)
					      ||
					      (span.first < p * spp &&
					       (p+1) * spp < span.second) );
			     }) )
			return true;
	}
	return false;
}


void
SScoringFacility::repaint_score_stats() const
{
	snprintf_buf( "<b>%3.1f</b> %% scored", scored_percent);
	gtk_label_set_markup( lScoringFacPercentScored, __buf__);

	snprintf_buf( "<small>%3.1f</small> %%", scored_percent_nrem);
	gtk_label_set_markup( lScoreStatsNREMPercent, __buf__);

	snprintf_buf( "<small>%3.1f</small> %%", scored_percent_rem);
	gtk_label_set_markup( lScoreStatsREMPercent, __buf__);

	snprintf_buf( "<small>%3.1f</small> %%", scored_percent_wake);
	gtk_label_set_markup( lScoreStatsWakePercent, __buf__);
}

void
SScoringFacility::queue_redraw_all() const
{
	if ( suppress_redraw )
		return;
	gtk_widget_queue_draw( (GtkWidget*)daScoringFacMontage);
	gtk_widget_queue_draw( (GtkWidget*)daScoringFacHypnogram);
	repaint_score_stats();
}




SScoringFacility::SChannel*
SScoringFacility::channel_near( int y)
{
	int nearest = INT_MAX, thisy;
	SChannel* nearest_h = &channels.front();
	for ( auto H = channels.begin(); H != channels.end(); ++H ) {
		thisy = y - H->zeroy;
		if ( thisy < 0 ) {
			if ( -thisy < nearest )
				return &const_cast<SChannel&>(*H);
			else
				return nearest_h;
		}
		if ( thisy < nearest ) {
			nearest = thisy;
			nearest_h = &*H;
		}
	}
	return nearest_h;
}

int
SScoringFacility::construct_widgets()
{
	GtkCellRenderer *renderer;

	if ( !(AGH_GBGETOBJ3 (builder, GtkWindow,		wScoringFacility)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eScoringFacPageSize)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDrawingArea,		daScoringFacMontage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDrawingArea,		daScoringFacHypnogram)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoringFacTotalPages)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eScoringFacCurrentPage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoringFacClockTime)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoringFacCurrentPos)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoringFacPercentScored)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoreStatsNREMPercent)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoreStatsREMPercent)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoreStatsWakePercent)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkExpander,		cScoringFacHypnogram)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkHBox,			cScoringFacControlBar)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoringFacBack)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoringFacForward)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreClear)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreNREM1)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreNREM2)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreNREM3)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreNREM4)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreREM))   ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreWake))  ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreMVT)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreGotoPrevUnscored)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreGotoNextUnscored)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreGotoPrevArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bScoreGotoNextArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,		bSFAccept)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bScoringFacShowFindDialog)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bScoringFacShowPhaseDiffDialog)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bScoringFacDrawPower)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bScoringFacDrawCrosshair)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkTable,		cScoringFacSleepStageStats)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoringFacHint)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkStatusbar,		sbSF)) )
		return -1;

	gtk_combo_box_set_model( eScoringFacPageSize,
				 (GtkTreeModel*)(settings::mScoringPageSize));
	gtk_combo_box_set_id_column( eScoringFacPageSize, 0);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eScoringFacPageSize, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eScoringFacPageSize, renderer,
					"text", 0,
					NULL);

	// ------- menus
	if ( !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPageSelection)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPower)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFScore)) ||
//	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFSpectrum)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageShowOriginal)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageShowProcessed)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageUseResample)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageUnfazer)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageFilter)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionMarkArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionClearArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionFindPattern)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSaveAs)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageExportSignal)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageUseThisScale)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerExportRange)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerExportAll)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerUseThisScale)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreAssist)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreImport)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreExport)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreClear)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFAcceptAndTakeNext)) )
		return -1;

	// orient control widget callbacks
	g_signal_connect_after( eScoringFacPageSize, "changed",
				G_CALLBACK (eScoringFacPageSize_changed_cb),
				this);
	g_signal_connect_after( eScoringFacCurrentPage, "value-changed",
				G_CALLBACK (eScoringFacCurrentPage_value_changed_cb),
				this);

	g_signal_connect_after( bScoreClear, "clicked",
				G_CALLBACK (bScoreClear_clicked_cb),
				this);
	g_signal_connect_after( bScoreNREM1, "clicked",
				G_CALLBACK (bScoreNREM1_clicked_cb),
				this);
	g_signal_connect_after( bScoreNREM2, "clicked",
				G_CALLBACK (bScoreNREM2_clicked_cb),
				this);
	g_signal_connect_after( bScoreNREM3, "clicked",
				G_CALLBACK (bScoreNREM3_clicked_cb),
				this);
	g_signal_connect_after( bScoreNREM4, "clicked",
				G_CALLBACK (bScoreNREM4_clicked_cb),
				this);
	g_signal_connect_after( bScoreREM, "clicked",
				G_CALLBACK (bScoreREM_clicked_cb),
				this);
	g_signal_connect_after( bScoreWake, "clicked",
				G_CALLBACK (bScoreWake_clicked_cb),
				this);
	g_signal_connect_after( bScoreMVT, "clicked",
				G_CALLBACK (bScoreMVT_clicked_cb),
				this);

	g_signal_connect_after( bScoringFacForward, "clicked",
				G_CALLBACK (bScoringFacForward_clicked_cb),
				this);
	g_signal_connect_after( bScoringFacBack, "clicked",
				G_CALLBACK (bScoringFacBack_clicked_cb),
				this);

	g_signal_connect_after( bScoreGotoNextUnscored, "clicked",
				G_CALLBACK (bScoreGotoNextUnscored_clicked_cb),
				this);
	g_signal_connect_after( bScoreGotoPrevUnscored, "clicked",
				G_CALLBACK (bScoreGotoPrevUnscored_clicked_cb),
				this);

	g_signal_connect_after( bScoreGotoNextArtifact, "clicked",
				G_CALLBACK (bScoreGotoNextArtifact_clicked_cb),
				this);
	g_signal_connect_after( bScoreGotoPrevArtifact, "clicked",
				G_CALLBACK (bScoreGotoPrevArtifact_clicked_cb),
				this);

	g_signal_connect_after( bScoringFacDrawPower, "toggled",
				G_CALLBACK (bScoringFacDrawPower_toggled_cb),
				this);
	g_signal_connect_after( bScoringFacDrawCrosshair, "toggled",
				G_CALLBACK (bScoringFacDrawCrosshair_toggled_cb),
				this);

	g_signal_connect_after( bScoringFacShowFindDialog, "toggled",
				G_CALLBACK (bScoringFacShowFindDialog_toggled_cb),
				this);
	g_signal_connect_after( bScoringFacShowPhaseDiffDialog, "toggled",
				G_CALLBACK (bScoringFacShowPhaseDiffDialog_toggled_cb),
				this);

	g_signal_connect_after( bSFAccept, "clicked",
				G_CALLBACK (bSFAccept_clicked_cb),
				this);

	g_signal_connect_after( wScoringFacility, "delete-event",
				G_CALLBACK (wScoringFacility_delete_event_cb),
				this);
	// menus
	g_signal_connect_after( mSFPage, "show",
				G_CALLBACK (mSFPage_show_cb),
				this);

	g_signal_connect_after( iSFPageShowOriginal, "toggled",
				G_CALLBACK (iSFPageShowOriginal_toggled_cb),
				this);
	g_signal_connect_after( iSFPageShowProcessed, "toggled",
				G_CALLBACK (iSFPageShowProcessed_toggled_cb),
				this);
	g_signal_connect_after( iSFPageUseResample, "toggled",
				G_CALLBACK (iSFPageUseResample_toggled_cb),
				this);

	g_signal_connect_after( iSFPageSelectionMarkArtifact, "activate",
				G_CALLBACK (iSFPageSelectionMarkArtifact_activate_cb),
				this);
	g_signal_connect_after( iSFPageSelectionClearArtifact, "activate",
				G_CALLBACK (iSFPageSelectionClearArtifact_activate_cb),
				this);
	g_signal_connect_after( iSFPageSelectionFindPattern, "activate",
				G_CALLBACK (iSFPageSelectionFindPattern_activate_cb),
				this);

	g_signal_connect_after( iSFPageUnfazer, "activate",
				G_CALLBACK (iSFPageUnfazer_activate_cb),
				this);
	g_signal_connect_after( iSFPageFilter, "activate",
				G_CALLBACK (iSFPageFilter_activate_cb),
				this);
	g_signal_connect_after( iSFPageSaveAs, "activate",
				G_CALLBACK (iSFPageSaveAs_activate_cb),
				this);
	g_signal_connect_after( iSFPageExportSignal, "activate",
				G_CALLBACK (iSFPageExportSignal_activate_cb),
				this);
	g_signal_connect_after( iSFPageUseThisScale, "activate",
				G_CALLBACK (iSFPageUseThisScale_activate_cb),
				this);

	g_signal_connect_after( iSFPowerExportRange, "activate",
				G_CALLBACK (iSFPowerExportRange_activate_cb),
				this);
	g_signal_connect_after( iSFPowerExportAll, "activate",
				G_CALLBACK (iSFPowerExportAll_activate_cb),
				this);
	g_signal_connect_after( iSFPowerUseThisScale, "activate",
				G_CALLBACK (iSFPowerUseThisScale_activate_cb),
				this);

	g_signal_connect_after( iSFScoreAssist, "activate",
				G_CALLBACK (iSFScoreAssist_activate_cb),
				this);
	g_signal_connect_after( iSFScoreExport, "activate",
				G_CALLBACK (iSFScoreExport_activate_cb),
				this);
	g_signal_connect_after( iSFScoreImport, "activate",
				G_CALLBACK (iSFScoreImport_activate_cb),
				this);
	g_signal_connect_after( iSFScoreClear, "activate",
				G_CALLBACK (iSFScoreClear_activate_cb),
				this);

	g_signal_connect_after( daScoringFacMontage, "draw",
				G_CALLBACK (daScoringFacMontage_draw_cb),
				this);
	g_signal_connect_after( daScoringFacMontage, "configure-event",
				G_CALLBACK (daScoringFacMontage_configure_event_cb),
				this);
	g_signal_connect_after( daScoringFacMontage, "button-press-event",
				G_CALLBACK (daScoringFacMontage_button_press_event_cb),
				this);
	g_signal_connect_after( daScoringFacMontage, "button-release-event",
				G_CALLBACK (daScoringFacMontage_button_release_event_cb),
				this);
	g_signal_connect_after( daScoringFacMontage, "scroll-event",
				G_CALLBACK (daScoringFacMontage_scroll_event_cb),
				this);
	g_signal_connect_after( daScoringFacMontage, "motion-notify-event",
				G_CALLBACK (daScoringFacMontage_motion_notify_event_cb),
				this);

	g_signal_connect_after( daScoringFacHypnogram, "draw",
				G_CALLBACK (daScoringFacHypnogram_draw_cb),
				this);
	// g_signal_connect_after( daScoringFacHypnogram, "configure-event",
	// 			G_CALLBACK (daScoringFacHypnogram_configure_event_cb),
	// 			this);
	g_signal_connect_after( daScoringFacHypnogram, "button-press-event",
				G_CALLBACK (daScoringFacHypnogram_button_press_event_cb),
				this);
	return 0;
}


const char* const
SScoringFacility::tooltips[2] = {
	"<b>Page views:</b>\n"
	"	Wheel:		change signal display scale;\n"
	"	Ctrl+Wheel:	change scale for all channels;\n"
	"	Click2:		reset display scale;\n"
	"  <i>in upper half:</i>\n"
	"	Click1, move, release:	mark artifact;\n"
	"	Click3, move, release:	unmark artifact;\n"
	"  <i>in lower half:</i>\n"
	"	Click3:		context menu.\n"
	"\n"
	"<b>Power profile views:</b>\n"
	"	Click1:	position cursor;\n"
	"	Click2:	draw bands / discrete freq. bins;\n"
	"	Click3:	context menu;\n"
	"	Wheel:	cycle focused band / in-/decrement freq. range;\n"
	"	Shift+Wheel:	in-/decrement scale.\n"
	"\n"
	"<b>Freq. spectrum view:</b>\n"
	"	Click2:	Toggle absolute/relative y-scale;\n"
	"	Wheel:	Scale power (when in abs. mode);\n"
	"	Shift+Wheel:	In-/decrease freq. range.\n"
	"\n"
	"<b>Hypnogram:</b>\n"
	"	Click1:	position cursor;\n"
	"	Click3:	context menu.",

	"<b>Unfazer:</b>\n"
	"	Wheel:		adjust factor;\n"
	"	Click1:		accept;\n"
	"	Click2:		reset factor to 1.;\n"
	"	Ctrl+Click2:	remove unfazer;\n"
	"	Click3:		cancel.\n",
};


// functions


// common widgets for all instances of SScoringFacility
int
construct_once()
{
      // ------ colours
	if ( !(CwB[TColour::score_none ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNONE")) ||
	     !(CwB[TColour::score_nrem1].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNREM1")) ||
	     !(CwB[TColour::score_nrem2].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNREM2")) ||
	     !(CwB[TColour::score_nrem3].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNREM3")) ||
	     !(CwB[TColour::score_nrem4].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourNREM4")) ||
	     !(CwB[TColour::score_rem  ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourREM")) ||
	     !(CwB[TColour::score_wake ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourWake")) ||
	     !(CwB[TColour::score_mvt  ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourWake")) ||
	     !(CwB[TColour::power_sf   ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourPowerSF")) ||
	     !(CwB[TColour::emg        ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourEMG")) ||
	     !(CwB[TColour::hypnogram  ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourHypnogram")) ||
	     !(CwB[TColour::artifact   ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourArtifacts")) ||
	     !(CwB[TColour::ticks_sf   ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourTicksSF")) ||
	     !(CwB[TColour::labels_sf  ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourLabelsSF")) ||
	     !(CwB[TColour::cursor     ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourCursor")) ||
	     !(CwB[TColour::band_delta ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandDelta")) ||
	     !(CwB[TColour::band_theta ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandTheta")) ||
	     !(CwB[TColour::band_alpha ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandAlpha")) ||
	     !(CwB[TColour::band_beta  ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandBeta")) ||
	     !(CwB[TColour::band_gamma ].btn = (GtkColorButton*)gtk_builder_get_object( __builder, "bColourBandGamma")) )
		return -1;

	return 0;
}

void
destruct()
{
}





} // namespace sf



using namespace sf;

inline namespace {

}


// callbaaaackz!

extern "C" {


// ---------- page value_changed


	void
	eScoringFacPageSize_changed_cb( GtkComboBox *widget, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		gint item = gtk_combo_box_get_active( (GtkComboBox*)widget);
		SF->set_pagesize( item); // -1 is fine here
	}

	void
	eScoringFacCurrentPage_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		SF->set_cur_vpage( gtk_spin_button_get_value( SF->eScoringFacCurrentPage) - 1);
	}



// -------------- various buttons


	void bScoreClear_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(TScore::none)); }
	void bScoreNREM1_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(TScore::nrem1)); }
	void bScoreNREM2_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(TScore::nrem2)); }
	void bScoreNREM3_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(TScore::nrem3)); }
	void bScoreNREM4_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(TScore::nrem4)); }
	void bScoreREM_clicked_cb  ( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(TScore::rem)); }
	void bScoreWake_clicked_cb ( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(TScore::wake)); }
	void bScoreMVT_clicked_cb  ( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(TScore::mvt)); }





	void
	bScoringFacForward_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.set_cur_vpage( SF.cur_vpage() + 1);
	}

	void
	bScoringFacBack_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.set_cur_vpage( SF.cur_vpage() - 1);
	}




	void
	bScoreGotoPrevUnscored_clicked_cb( GtkButton *button, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		if ( SF->cur_page() == 0 )
			return;
		size_t p = SF->cur_page() - 1;
		while ( SF->hypnogram[p] != agh::SPage::score_code(TScore::none) )
			if ( p != (size_t)-1 )
				--p;
			else
				break;
		// overflown values will be reset here:
		SF->set_cur_vpage( SF->p2ap(p));
	}

	void
	bScoreGotoNextUnscored_clicked_cb( GtkButton *button, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		if ( SF->cur_page() == SF->total_pages()-1 )
			return;
		size_t p = SF->cur_page() + 1;
		while ( SF->hypnogram[p] != agh::SPage::score_code(TScore::none) )
			if ( p < SF->total_pages() )
				++p;
			else
				break;
		// out-of-range values will be reset here:
		SF->set_cur_vpage( SF->p2ap(p));
	}




	void
	bScoreGotoPrevArtifact_clicked_cb( GtkButton *button, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		if ( SF->cur_page() > 0 )
			return;
		size_t p = SF->cur_page() - 1;
		bool p_has_af;
		while ( !(p_has_af = SF->page_has_artifacts( p)) )
			if ( p != (size_t)-1 )
				--p;
			else
				break;
		SF->set_cur_vpage( SF->p2ap(p));
	}

	void
	bScoreGotoNextArtifact_clicked_cb( GtkButton *button, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		if ( SF->cur_page() == SF->total_pages()-1 )
			return;
		size_t p = SF->cur_page() + 1;
		bool p_has_af;
		while ( !(p_has_af = SF->page_has_artifacts( p)) )
			if ( p < SF->total_pages() )
				++p;
			else
				break;
		SF->set_cur_vpage( SF->p2ap(p));
	}




	void
	bScoringFacDrawPower_toggled_cb( GtkToggleButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.draw_power = !SF.draw_power;
		for ( auto H = SF.channels.begin(); H != SF.channels.end(); ++H )
			// if ( H->have_power() )
				H->draw_power = SF.draw_power;
		SF.queue_redraw_all();
	}

	void
	bScoringFacDrawCrosshair_toggled_cb( GtkToggleButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.draw_crosshair = !SF.draw_crosshair;
		SF.queue_redraw_all();
	}





	void
	bScoringFacShowFindDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( gtk_toggle_button_get_active( togglebutton) ) {
			gtk_widget_show_all( (GtkWidget*)SF.find_dialog.wPattern);
		} else
			gtk_widget_hide( (GtkWidget*)SF.find_dialog.wPattern);
	}



	void
	bScoringFacShowPhaseDiffDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( gtk_toggle_button_get_active( togglebutton) ) {
			gtk_widget_show_all( (GtkWidget*)SF.phasediff_dialog.wPhaseDiff);
		} else
			gtk_widget_hide( (GtkWidget*)SF.phasediff_dialog.wPhaseDiff);
	}







// -- PageSelection


	void
	bSFAccept_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto SF = (SScoringFacility*)userdata;

		gtk_widget_queue_draw( (GtkWidget*)cMeasurements);

		delete SF;
	}


	void
	iSFAcceptAndTakeNext_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto SF = (SScoringFacility*)userdata;
		set_cursor_busy( true, (GtkWidget*)SF->wScoringFacility);
		const char
			*j = SF->channels.front().recording.subject(),
			*d = SF->channels.front().recording.session(),
			*e = SF->channels.front().recording.episode();
		agh::CSubject& J = AghCC->subject_by_x(j);
		auto& EE = J.measurements[d].episodes;
		// auto E = find( EE.begin(), EE.end(), e);
		// guaranteed to have next(E)

		delete SF;

		SF = new SScoringFacility( J, d,
					   next( find( EE.begin(), EE.end(), e)) -> name());
		gtk_widget_show_all( (GtkWidget*)SF->wScoringFacility);
		set_cursor_busy( false, (GtkWidget*)SF->wScoringFacility);
	}



// ------- cleanup

	gboolean
	wScoringFacility_delete_event_cb( GtkWidget *widget,
					  GdkEvent  *event,
					  gpointer   userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		// not sure resurrection will succeed, tho
		delete SF;
		gtk_widget_queue_draw( (GtkWidget*)cMeasurements);

		return TRUE; // to stop other handlers from being invoked for the event
	}





// -------- colours


	void
	bColourNONE_color_set_cb( GtkColorButton *widget,
				  gpointer        userdata)
	{
		CwB[TColour::score_none].acquire();
	}

	void
	bColourNREM1_color_set_cb( GtkColorButton *widget,
				   gpointer        userdata)
	{
		CwB[TColour::score_nrem1].acquire();
	}


	void
	bColourNREM2_color_set_cb( GtkColorButton *widget,
				   gpointer        userdata)
	{
		CwB[TColour::score_nrem2].acquire();
	}


	void
	bColourNREM3_color_set_cb( GtkColorButton *widget,
				   gpointer        userdata)
	{
		CwB[TColour::score_nrem3].acquire();
	}


	void
	bColourNREM4_color_set_cb( GtkColorButton *widget,
				   gpointer        userdata)
	{
		CwB[TColour::score_nrem4].acquire();
	}

	void
	bColourREM_color_set_cb( GtkColorButton *widget,
				 gpointer        userdata)
	{
		CwB[TColour::score_rem].acquire();
	}

	void
	bColourWake_color_set_cb( GtkColorButton *widget,
				  gpointer        userdata)
	{
		CwB[TColour::score_wake].acquire();
	}



	void
	bColourPowerSF_color_set_cb( GtkColorButton *widget,
				     gpointer        userdata)
	{
		CwB[TColour::power_sf].acquire();
	}


	void
	bColourHypnogram_color_set_cb( GtkColorButton *widget,
				       gpointer        userdata)
	{
		CwB[TColour::hypnogram].acquire();
	}

	void
	bColourArtifacts_color_set_cb( GtkColorButton *widget,
				       gpointer        userdata)
	{
		CwB[TColour::artifact].acquire();
	}



	void
	bColourTicksSF_color_set_cb( GtkColorButton *widget,
				     gpointer        userdata)
	{
		CwB[TColour::ticks_sf].acquire();
	}

	void
	bColourLabelsSF_color_set_cb( GtkColorButton *widget,
				      gpointer        userdata)
	{
		CwB[TColour::labels_sf].acquire();
	}

	void
	bColourCursor_color_set_cb( GtkColorButton *widget,
				    gpointer        userdata)
	{
		CwB[TColour::cursor].acquire();
	}


	void
	bColourBandDelta_color_set_cb( GtkColorButton *widget,
				       gpointer        userdata)
	{
		CwB[TColour::band_delta].acquire();
	}
	void
	bColourBandTheta_color_set_cb( GtkColorButton *widget,
				       gpointer        userdata)
	{
		CwB[TColour::band_theta].acquire();
	}
	void
	bColourBandAlpha_color_set_cb( GtkColorButton *widget,
				       gpointer        userdata)
	{
		CwB[TColour::band_alpha].acquire();
	}
	void
	bColourBandBeta_color_set_cb( GtkColorButton *widget,
				      gpointer        userdata)
	{
		CwB[TColour::band_beta].acquire();
	}
	void
	bColourBandGamma_color_set_cb( GtkColorButton *widget,
				       gpointer        userdata)
	{
		CwB[TColour::band_gamma].acquire();
	}

} // extern "C"


} // namespace aghui


// EOF

