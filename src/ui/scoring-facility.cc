// ;-*-C++-*- *  Time-stamp: "2011-07-08 03:16:09 hmmr"
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

#include "../libexstrom/exstrom.hh"
#include "../libagh/misc.hh"
#include "misc.hh"
#include "ui.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;


size_t	aghui::SScoringFacility::IntersignalSpace = 120,
	aghui::SScoringFacility::SpectrumWidth = 100,
	aghui::SScoringFacility::HypnogramHeight = 80,
	aghui::SScoringFacility::EMGProfileHeight = 30;
float	aghui::SScoringFacility::NeighPagePeek = 5.;



bool
aghui::SScoringFacility::SChannel::validate_filters()
{
	if ( low_pass.cutoff >= 0. && low_pass.order < 6
	     && high_pass.cutoff >= 0. && high_pass.order < 6
	     && ((low_pass.cutoff > 0. && high_pass.cutoff > 0. && high_pass.cutoff < low_pass.cutoff)
		 || high_pass.cutoff == 0. || low_pass.cutoff == 0.) )
		return true;
	low_pass.cutoff = high_pass.cutoff = 0;
	low_pass.order = high_pass.order = 1;
	return false;
}

void
aghui::SScoringFacility::SChannel::compute_lowpass( float _cutoff, unsigned _order)
{
	if ( signal_lowpass.data.size() == 0 ||
	     signal_lowpass.cutoff != _cutoff || signal_lowpass.order != _order )
		signal_lowpass.data =
			valarray<float> (exstrom::low_pass( signal_filtered, samplerate(),
							    signal_lowpass.cutoff = _cutoff,
							    signal_lowpass.order = _order, true));
}


void
aghui::SScoringFacility::SChannel::compute_tightness( unsigned _tightness)
{
	if ( signal_envelope.lower.size() == 0 ||
	     signal_envelope.tightness != _tightness )
		sigproc::envelope( signal_filtered,
				   signal_envelope.tightness = _tightness, samplerate(),
				   1./samplerate(),
				   signal_envelope.lower,
				   signal_envelope.upper); // don't need anchor points, nor their count
}

void
aghui::SScoringFacility::SChannel::compute_dzcdf( float _step, float _sigma, unsigned _smooth)
{
	if ( signal_dzcdf.data.size() == 0 ||
	     signal_dzcdf.step != _step || signal_dzcdf.sigma != _sigma || signal_dzcdf.smooth != _smooth )
		signal_dzcdf.data =
			sigproc::dzcdf( signal_filtered, samplerate(),
					signal_dzcdf.step = _step,
					signal_dzcdf.sigma = _sigma,
					signal_dzcdf.smooth = _smooth);
}







// class SScoringFacility::SChannel

aghui::SScoringFacility::SChannel::SChannel( agh::CRecording& r,
					     SScoringFacility& parent,
					     size_t y0)
      : name (r.channel()),
	type (r.signal_type()),
	recording (r),
	_p (parent),
	low_pass ({INFINITY, (unsigned)-1}),
	high_pass ({INFINITY, (unsigned)-1}),
	zeroy (y0),
	hidden (false),
	draw_original_signal (false),
	draw_filtered_signal (true),
	draw_power (true),
	draw_bands (true),
	draw_spectrum_absolute (true),
	use_resample (true),
	marquee_start (0.),
	marquee_end (0.),
	selection_start_time (0.),
	selection_end_time (0.),
	selection_start (0),
	selection_end (0),
	_h (recording.F().which_channel(name)),
	_ssignal (recording.F()[_h])
{
      // load any previously saved filters
	{
		ifstream ifs (agh::make_fname__common( recording.F().filename(), true) + '-' + name + ".filters");
		if ( ifs.good() ) {
			ifs >> low_pass.order >> low_pass.cutoff >> high_pass.order >> high_pass.cutoff;
			validate_filters();
		}
	}
	get_signal_original();
	get_signal_filtered();

	signal_display_scale =
		calibrate_display_scale( signal_filtered,
					 _p.vpagesize() * samplerate() * min (recording.F().length(), (size_t)10),
					 _p.interchannel_gap / 2);

      // power and spectrum
	if ( agh::SChannel::signal_type_is_fftable( type) ) {

		// snprintf_buf( "(%zu/%zu) %s: power...", h+1, __n_all_channels, HH[h].name);
		// BUF_ON_STATUS_BAR;

		// power in a single bin
		from = _p._p.operating_range_from;
		upto = _p._p.operating_range_upto;
		get_power();
	      // power spectrum (for the first page)
		n_bins = last_spectrum_bin = recording.n_bins();
		get_spectrum( 0);
		// will be reassigned in REDRAW_ALL
		spectrum_upper_freq = n_bins * recording.binsize();

	      // power in bands
		agh::TBand n_bands = agh::TBand::delta;
		while ( n_bands != agh::TBand::_total )
			if ( _p._p.freq_bands[(size_t)n_bands][0] >= spectrum_upper_freq )
				break;
			else
				next(n_bands);
		uppermost_band = prev(n_bands);
		get_power_in_bands();

	      // delta comes first, calibrate display scale against it
		power_display_scale =
			calibrate_display_scale( power_in_bands[(size_t)agh::TBand::delta],
						 power_in_bands[(size_t)agh::TBand::delta].size(),
						 _p.interchannel_gap/2.);
	      // switches
		draw_spectrum_absolute = true;
		draw_bands = true;
		focused_band = agh::TBand::delta; // delta
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
				       (signal_original[ slice (i * _p.pagesize() * samplerate(),
								(i+1) * _p.pagesize() * samplerate(), 1) ])).max();
			 if ( largest < current )
				 largest = current;
		 }

		 emg_scale = EMGProfileHeight/2 / largest;
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

aghui::SScoringFacility::SChannel::~SChannel()
{
	ofstream ofs (agh::make_fname__common( recording.F().filename(), true) + '-' + name + ".filters");
	if ( ofs.good() )
		ofs << low_pass.order << ' ' << low_pass.cutoff << endl
		    << high_pass.order << ' ' << high_pass.cutoff;
}

void
aghui::SScoringFacility::SChannel::get_signal_original()
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
aghui::SScoringFacility::SChannel::get_signal_filtered()
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
aghui::SScoringFacility::SChannel::calibrate_display_scale( const valarray<float>& signal,
						     size_t over, float fit)
{
	float max_over = 0.;
	for ( size_t i = 0; i < over; ++i )
		if ( max_over < signal[i] )
			max_over = signal[i];
	return fit / max_over;
}


float
aghui::SScoringFacility::SChannel::calculate_dirty_percent()
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
aghui::SScoringFacility::SChannel::mark_region_as_artifact( bool do_mark)
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
		get_spectrum( _p.cur_page());
	}
	gtk_widget_queue_draw( (GtkWidget*)_p.daScoringFacMontage);
}


void
aghui::SScoringFacility::SChannel::mark_region_as_pattern()
{
	_p.find_dialog.load_pattern( *this);
	gtk_widget_show_all( (GtkWidget*)_p.find_dialog.wPattern);
}





// class aghui::SScoringFacility

const array<unsigned, 8>
	aghui::SScoringFacility::DisplayPageSizeValues = {
	{5, 10, 15, 20, 30, 60, 60*3, 60*5}
};

size_t
aghui::SScoringFacility::figure_display_pagesize_item( size_t seconds)
{
	size_t i = 0;
	while ( i < DisplayPageSizeValues.size()-1 && DisplayPageSizeValues[i] < seconds )
		++i;
	return i;
}


aghui::SScoringFacility::SScoringFacility( agh::CSubject& J,
					   const string& D, const string& E,
					   aghui::SExpDesignUI& parent)
      : _p (parent),
	_csubject (J),
	_sepisode (J.measurements.at(D)[E]),
	draw_crosshair (false),
	draw_power (false),
	marking_now (false),
	shuffling_channels_now (false),
	draw_spp (true),
	skirting_run_per1 (NeighPagePeek),
	crosshair_at (10),
	using_channel (NULL),
	interchannel_gap (IntersignalSpace),
	n_hidden (0),
	unfazer_mode (TUnfazerMode::none),
	unfazer_offending_channel (NULL),
	unfazer_factor (0.1),
	find_dialog (*this),
	filters_dialog (*this),
	phasediff_dialog (*this),
	_cur_page (0),
	_cur_vpage (0),
	pagesize_item (figure_display_pagesize_item( parent.pagesize()))
{
	set_cursor_busy( true, (GtkWidget*)_p.wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)_p.wMainWindow, FALSE);

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
	//  we do it all mostly ourself, except for some delete-event binding to gtk_true()

      // iterate all of AghHH, mark our channels
	size_t y = interchannel_gap / 2.;
	for ( auto H = _p.AghHH.begin(); H != _p.AghHH.end(); ++H ) {
		snprintf_buf( "Reading and processing channel %s...", H->c_str());
		_p.buf_on_status_bar();
		try {
			channels.emplace_back( _sepisode.recordings.at(*H),
					       *this,
					       y);
			y += interchannel_gap;
		} catch (...) {
		}
	}
	da_ht = montage_est_height();

      // load montage
	{
		ifstream ifs (agh::make_fname__common( channels.front().recording.F().filename(), true) + ".montage");
		if ( ifs.good() ) {
			ifs >> draw_crosshair >> draw_power >> draw_spp
			    >> sane_signal_display_scale >> sane_power_display_scale
			    // >> skirting_run_per1
			    >> interchannel_gap
			    >> n_hidden;
			for_each( channels.begin(), channels.end(),
				  [&] ( SChannel& h)
				  {
					  // ofs >> h.name;
					  ifs >> h.hidden
					      >> h.draw_original_signal
					      >> h.draw_filtered_signal
					      >> h.draw_power >> h.draw_bands >> h.draw_spectrum_absolute
					      >> h.use_resample
					      >> h.zeroy
					      >> h.selection_start_time >> h.selection_end_time
					      >> h.signal_display_scale >> h.power_display_scale >> h.emg_scale;
					  h.selection_start = h.selection_start_time * h.samplerate();
					  h.selection_end = h.selection_end_time * h.samplerate();
				  });
		}
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
		      "height-request", HypnogramHeight,
		      NULL);
	g_object_set( (GObject*)daScoringFacMontage,
		      "height-request", da_ht,
		      NULL);


	// grey out phasediff button if there are fewer than 2 EEG channels
	gtk_widget_set_sensitive( (GtkWidget*)bScoringFacShowPhaseDiffDialog, (n_eeg_channels >= 2));

	// desensitize iSFAcceptAndTakeNext unless there are more episodes
	gtk_widget_set_sensitive( (GtkWidget*)iSFAcceptAndTakeNext,
				  J.measurements.at(D).episodes.back().name() != E);
	// (de)sensitize various toolbar toggle buttons
	gtk_toggle_button_set_active( bScoringFacDrawPower,
				      (gboolean)draw_power);
	gtk_toggle_button_set_active( bScoringFacDrawCrosshair,
				      (gboolean)draw_crosshair);

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
	gtk_statusbar_pop( _p.sbMainStatusBar, _p.sbContextIdGeneral);
	set_cursor_busy( false, (GtkWidget*)_p.wMainWindow);

	{
		int bar_height;
		gtk_widget_get_size_request( (GtkWidget*)cScoringFacControlBar, NULL, &bar_height);
		int optimal_win_height = min(
			(int)HypnogramHeight + bar_height + da_ht + 70,
			(int)(gdk_screen_get_height( gdk_screen_get_default()) * .92));
		gtk_window_set_default_size( wScoringFacility,
					     gdk_screen_get_width( gdk_screen_get_default()) * .90,
					     optimal_win_height);
	}
	gtk_widget_show_all( (GtkWidget*)(wScoringFacility));

	set_cursor_busy( false, (GtkWidget*)_p.wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)_p.wMainWindow, TRUE);
}


aghui::SScoringFacility::~SScoringFacility()
{
	// put scores
	put_hypnogram();

	// save display scales
	{
		ofstream ofs (agh::make_fname__common( channels.front().recording.F().filename(), true) + ".montage");
		if ( ofs.good() ) {
			ofs << draw_crosshair << ' ' << draw_power << ' ' << draw_spp << ' '
			    << sane_signal_display_scale << ' ' << sane_power_display_scale << ' '
			    // << skirting_run_per1 << ' '
			    << interchannel_gap << ' '
			    << n_hidden << ' ' << endl;
			for_each( channels.begin(), channels.end(),
				  [&] ( SChannel& h)
				  {
					  // ofs << h.name;
					  ofs << h.hidden << ' ' << h.draw_original_signal << ' '
					      << h.draw_filtered_signal << ' '
					      << h.draw_power << ' ' << h.draw_bands << ' ' << h.draw_spectrum_absolute << ' '
					      << h.use_resample << ' '
					      << h.zeroy << ' '
					      << h.selection_start_time << ' ' << h.selection_end_time << ' '
					      << h.signal_display_scale << ' ' << h.power_display_scale << ' ' << h.emg_scale << ' ' << endl;
				  });
		}
	}

	// destroy widgets
	gtk_widget_destroy( (GtkWidget*)wScoringFacility);
	g_object_unref( (GObject*)builder);
}






void
aghui::SScoringFacility::get_hypnogram()
{
	// just get from the first source,
	// trust other sources are no different
	const agh::CEDFFile& F = channels.begin()->recording.F();
	hypnogram.resize( F.agh::CHypnogram::length());
	for ( size_t p = 0; p < F.CHypnogram::length(); ++p )
		hypnogram[p] = F.nth_page(p).score_code();
}
void
aghui::SScoringFacility::put_hypnogram()
{
	// but put to all
	for_each( _sepisode.sources.begin(), _sepisode.sources.end(),
		  [&] ( agh::CEDFFile& F)
		  {
			  for ( size_t p = 0; p < F.CHypnogram::length(); ++p )
				  F.nth_page(p).mark( hypnogram[p]);
		  });
}







size_t
aghui::SScoringFacility::set_cur_page( size_t p)
{
	if ( p < total_pages() ) {
		_cur_page = p;
		_cur_vpage = p2ap(p);
	}
	queue_redraw_all();
	return _cur_page;
}
size_t
aghui::SScoringFacility::set_cur_vpage( size_t p)
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

		gtk_widget_set_sensitive( (GtkWidget*)bScoringFacForward, _cur_vpage < total_vpages()-1);
		gtk_widget_set_sensitive( (GtkWidget*)bScoringFacBack, _cur_vpage > 0);

		queue_redraw_all();
	}
	return _cur_vpage;
}

void
aghui::SScoringFacility::set_pagesize( int item)
{
	if ( item == pagesize_item || item > (int)DisplayPageSizeValues.size() )
		return;
	pagesize_item = item;
	_cur_vpage = p2ap(_cur_page);

	gtk_adjustment_set_upper( jPageNo, total_vpages());
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

	snprintf_buf( "<small>of</small> %zu", total_vpages());
	gtk_label_set_markup( lScoringFacTotalPages, __buf__);

	queue_redraw_all();
}



void
aghui::SScoringFacility::do_score_forward( char score_ch)
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
aghui::SScoringFacility::SChannel::marquee_to_selection()
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
aghui::SScoringFacility::page_has_artifacts( size_t p)
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
aghui::SScoringFacility::repaint_score_stats() const
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
aghui::SScoringFacility::queue_redraw_all() const
{
	if ( suppress_redraw )
		return;
	gtk_widget_queue_draw( (GtkWidget*)daScoringFacMontage);
	gtk_widget_queue_draw( (GtkWidget*)daScoringFacHypnogram);
	repaint_score_stats();
}




aghui::SScoringFacility::SChannel*
aghui::SScoringFacility::channel_near( int y)
{
	int nearest = INT_MAX, thisy;
	SChannel* nearest_h = &channels.front();
	for ( auto H = channels.begin(); H != channels.end(); ++H ) {
		if ( H->hidden )
			continue;
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



struct SChHolder {
	aghui::SScoringFacility::SChannel* ch;
	SChHolder( aghui::SScoringFacility::SChannel& ini) : ch (&ini) {}
	bool operator<( const SChHolder& rv) const
		{
			return ch->zeroy < rv.ch->zeroy;
		}
};

int
aghui::SScoringFacility::find_free_space()
{
	vector<SChHolder> thomas;
	for_each( channels.begin(), channels.end(),
		  [&] (SChannel& ch)
		  {
			  if ( not ch.hidden )
				  thomas.push_back( {ch});
		  });
	sort( thomas.begin(), thomas.end());

	int	mean_gap,
		widest_gap = 0,
		widest_after = 0;
	int sum = 0;
	for ( auto ch = channels.begin(); ch != prev(channels.end()); ++ch ) {
		int gap = next(ch)->zeroy - ch->zeroy;
		sum += gap;
		if ( gap > widest_gap ) {
			widest_after = ch->zeroy;
			widest_gap = gap;
		}
	}
	mean_gap = sum / thomas.size()-1;
	if ( widest_gap > mean_gap * 1.5 )
		return widest_after + widest_gap / 2;
	else {
		gtk_widget_set_size_request( (GtkWidget*)daScoringFacMontage,
					     -1, thomas.back().ch->zeroy + 42*2);
		return thomas.back().ch->zeroy + mean_gap;
	}
}

void
aghui::SScoringFacility::space_evenly()
{
	vector<SChHolder> thomas;
	for_each( channels.begin(), channels.end(),
		  [&] (SChannel& ch)
		  {
			  if ( not ch.hidden )
				  thomas.push_back( {ch});
		  });
	sort( thomas.begin(), thomas.end());

	int	mean_gap,
		sum = 0;
	for ( auto ch = channels.begin(); ch != prev(channels.end()); ++ch ) {
		int gap = next(ch)->zeroy - ch->zeroy;
		sum += gap;
	}
	mean_gap = da_ht / thomas.size();

	size_t i = 0;
	for_each( thomas.begin(), thomas.end(),
		  [&] (SChHolder& t)
		  {
			  t.ch->zeroy = mean_gap/2 + mean_gap * i++;
		  });

	gtk_widget_set_size_request( (GtkWidget*)daScoringFacMontage,
				     -1, thomas.back().ch->zeroy + mean_gap/2);
}


void
aghui::SScoringFacility::expand_by_factor( double fac)
{
	for ( auto ch = channels.begin(); ch != channels.end(); ++ch ) {
		ch->signal_display_scale *= fac;
		ch->power_display_scale *= fac;
		ch->zeroy *= fac;
	}
	interchannel_gap *= fac;
}





int
aghui::SScoringFacility::construct_widgets()
{
	GtkCellRenderer *renderer;

	if ( !(AGH_GBGETOBJ3 (builder, GtkWindow,		wScoringFacility)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eScoringFacPageSize)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkAdjustment,		jPageNo)) ||
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

	gtk_combo_box_set_model( eScoringFacPageSize, // reuse the one previously constructed in SExpDesignUI
				 (GtkTreeModel*)_p.mScoringPageSize);
	gtk_combo_box_set_id_column( eScoringFacPageSize, 0);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eScoringFacPageSize, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eScoringFacPageSize, renderer,
					"text", 0,
					NULL);

	// ------- menus
	if ( //!(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFSpectrum)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPageSelection)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPageHidden)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPower)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFScore)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageShowOriginal)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageShowProcessed)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageUseResample)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageUnfazer)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageFilter)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSaveAs)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageExportSignal)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageUseThisScale)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageClearArtifacts)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageHide)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem, 		iSFPageHidden)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem, 		iSFPageSpaceEvenly)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionMarkArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionClearArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionFindPattern)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerExportRange)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerExportAll)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerUseThisScale)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreAssist)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreImport)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreExport)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreClear)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFAcceptAndTakeNext)) )
		return -1;

	// // remove that stupid menuitem6
	// gtk_container_foreach( (GtkContainer*)iSFPageHidden,
	// 		       (GtkCallback) gtk_widget_destroy,
	// 		       NULL);
	gtk_menu_item_set_submenu( iSFPageHidden, (GtkWidget*)mSFPageHidden);

	// orient control widget callbacks
	g_signal_connect( eScoringFacPageSize, "changed",
			  (GCallback)eScoringFacPageSize_changed_cb,
			  this);
	g_signal_connect( eScoringFacCurrentPage, "value-changed",
			  (GCallback)eScoringFacCurrentPage_value_changed_cb,
			  this);

	g_signal_connect( bScoreClear, "clicked",
			  (GCallback)bScoreClear_clicked_cb,
			  this);
	g_signal_connect( bScoreNREM1, "clicked",
			  (GCallback)bScoreNREM1_clicked_cb,
			  this);
	g_signal_connect( bScoreNREM2, "clicked",
			  (GCallback)bScoreNREM2_clicked_cb,
			  this);
	g_signal_connect( bScoreNREM3, "clicked",
			  (GCallback)bScoreNREM3_clicked_cb,
			  this);
	g_signal_connect( bScoreNREM4, "clicked",
			  (GCallback)bScoreNREM4_clicked_cb,
			  this);
	g_signal_connect( bScoreREM, "clicked",
			  (GCallback)bScoreREM_clicked_cb,
			  this);
	g_signal_connect( bScoreWake, "clicked",
			  (GCallback)bScoreWake_clicked_cb,
			  this);
	g_signal_connect( bScoreMVT, "clicked",
			  (GCallback)bScoreMVT_clicked_cb,
			  this);

	g_signal_connect( bScoringFacForward, "clicked",
			  (GCallback)bScoringFacForward_clicked_cb,
			  this);
	g_signal_connect( bScoringFacBack, "clicked",
			  (GCallback)bScoringFacBack_clicked_cb,
			  this);

	g_signal_connect( bScoreGotoNextUnscored, "clicked",
			  (GCallback)bScoreGotoNextUnscored_clicked_cb,
			  this);
	g_signal_connect( bScoreGotoPrevUnscored, "clicked",
			  (GCallback)bScoreGotoPrevUnscored_clicked_cb,
			  this);

	g_signal_connect( bScoreGotoNextArtifact, "clicked",
			  (GCallback)bScoreGotoNextArtifact_clicked_cb,
			  this);
	g_signal_connect( bScoreGotoPrevArtifact, "clicked",
			  (GCallback)bScoreGotoPrevArtifact_clicked_cb,
			  this);

	g_signal_connect( bScoringFacDrawPower, "toggled",
			  (GCallback)bScoringFacDrawPower_toggled_cb,
			  this);
	g_signal_connect( bScoringFacDrawCrosshair, "toggled",
			  (GCallback)bScoringFacDrawCrosshair_toggled_cb,
			  this);

	g_signal_connect( bScoringFacShowFindDialog, "toggled",
			  (GCallback)bScoringFacShowFindDialog_toggled_cb,
			  this);
	g_signal_connect( bScoringFacShowPhaseDiffDialog, "toggled",
			  (GCallback)bScoringFacShowPhaseDiffDialog_toggled_cb,
			  this);

	g_signal_connect( bSFAccept, "clicked",
			  (GCallback)bSFAccept_clicked_cb,
			  this);

	g_signal_connect( wScoringFacility, "delete-event",
			  (GCallback)wScoringFacility_delete_event_cb,
			  this);
	// menus
	g_signal_connect( mSFPage, "show",
			  (GCallback)mSFPage_show_cb,
			  this);

	g_signal_connect( iSFPageShowOriginal, "toggled",
			  (GCallback)iSFPageShowOriginal_toggled_cb,
			  this);
	g_signal_connect( iSFPageShowProcessed, "toggled",
			  (GCallback)iSFPageShowProcessed_toggled_cb,
			  this);
	g_signal_connect( iSFPageUseResample, "toggled",
			  (GCallback)iSFPageUseResample_toggled_cb,
			  this);

	g_signal_connect( iSFPageSelectionMarkArtifact, "activate",
			  (GCallback)iSFPageSelectionMarkArtifact_activate_cb,
			  this);
	g_signal_connect( iSFPageSelectionClearArtifact, "activate",
			  (GCallback)iSFPageSelectionClearArtifact_activate_cb,
			  this);
	g_signal_connect( iSFPageSelectionFindPattern, "activate",
			  (GCallback)iSFPageSelectionFindPattern_activate_cb,
			  this);

	g_signal_connect( iSFPageUnfazer, "activate",
			  (GCallback)iSFPageUnfazer_activate_cb,
			  this);
	g_signal_connect( iSFPageFilter, "activate",
			  (GCallback)iSFPageFilter_activate_cb,
			  this);
	g_signal_connect( iSFPageSaveAs, "activate",
			  (GCallback)iSFPageSaveAs_activate_cb,
			  this);
	g_signal_connect( iSFPageExportSignal, "activate",
			  (GCallback)iSFPageExportSignal_activate_cb,
			  this);
	g_signal_connect( iSFPageUseThisScale, "activate",
			  (GCallback)iSFPageUseThisScale_activate_cb,
			  this);
	g_signal_connect( iSFPageClearArtifacts, "activate",
			  (GCallback)iSFPageClearArtifacts_activate_cb,
			  this);
	g_signal_connect( iSFPageHide, "activate",
			  (GCallback)iSFPageHide_activate_cb,
			  this);
	// g_signal_connect( iSFPageHidden, "select",
	// 		  (GCallback)iSFPageHidden_select_cb,
	// 		  this);
	// g_signal_connect( iSFPageHidden, "deselect",
	// 		  (GCallback)iSFPageHidden_deselect_cb,
	// 		  this);

	g_signal_connect( iSFPageSpaceEvenly, "activate",
			  (GCallback)iSFPageSpaceEvenly_activate_cb,
			  this);

	g_signal_connect( iSFPowerExportRange, "activate",
			  (GCallback)iSFPowerExportRange_activate_cb,
			  this);
	g_signal_connect( iSFPowerExportAll, "activate",
			  (GCallback)iSFPowerExportAll_activate_cb,
			  this);
	g_signal_connect( iSFPowerUseThisScale, "activate",
			  (GCallback)iSFPowerUseThisScale_activate_cb,
			  this);

	g_signal_connect( iSFScoreAssist, "activate",
			  (GCallback)iSFScoreAssist_activate_cb,
			  this);
	g_signal_connect( iSFScoreExport, "activate",
			  (GCallback)iSFScoreExport_activate_cb,
			  this);
	g_signal_connect( iSFScoreImport, "activate",
			  (GCallback)iSFScoreImport_activate_cb,
			  this);
	g_signal_connect( iSFScoreClear, "activate",
			  (GCallback)iSFScoreClear_activate_cb,
			  this);

	g_signal_connect( daScoringFacMontage, "draw",
			  (GCallback)daScoringFacMontage_draw_cb,
			  this);
	g_signal_connect( daScoringFacMontage, "configure-event",
			  (GCallback)daScoringFacMontage_configure_event_cb,
			  this);
	g_signal_connect( daScoringFacMontage, "button-press-event",
			  (GCallback)daScoringFacMontage_button_press_event_cb,
			  this);
	g_signal_connect( daScoringFacMontage, "button-release-event",
			  (GCallback)daScoringFacMontage_button_release_event_cb,
			  this);
	g_signal_connect( daScoringFacMontage, "scroll-event",
			  (GCallback)daScoringFacMontage_scroll_event_cb,
			  this);
	g_signal_connect( daScoringFacMontage, "motion-notify-event",
			  (GCallback)daScoringFacMontage_motion_notify_event_cb,
			  this);

	g_signal_connect( daScoringFacHypnogram, "draw",
			  (GCallback)daScoringFacHypnogram_draw_cb,
			  this);
	// g_signal_connect_after( daScoringFacHypnogram, "configure-event",
	// 			(GCallback)daScoringFacHypnogram_configure_event_cb,
	// 			this);
	g_signal_connect( daScoringFacHypnogram, "button-press-event",
			  (GCallback)daScoringFacHypnogram_button_press_event_cb,
			  this);
	return 0;
}


const char* const
	aghui::SScoringFacility::tooltips[2] = {
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



// EOF

