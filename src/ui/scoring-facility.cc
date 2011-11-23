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




#include <initializer_list>
#include <stdexcept>
#include <fstream>

#include "../libexstrom/exstrom.hh"
#include "../misc.hh"
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



void
aghui::SScoringFacility::SChannel::compute_lowpass( float _cutoff, unsigned _order)
{
	if ( signal_lowpass.data.size() == 0 ||
	     signal_lowpass.cutoff != _cutoff || signal_lowpass.order != _order )
		signal_lowpass.data =
			valarray<TFloat> (exstrom::low_pass( signal_filtered, samplerate(),
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
			sigproc::dzcdf<TFloat>( signal_filtered, samplerate(),
						signal_dzcdf.step = _step,
						signal_dzcdf.sigma = _sigma,
						signal_dzcdf.smooth = _smooth);
}



list<sigfile::SAnnotation*>
aghui::SScoringFacility::SChannel::in_annotations( double time) const
{
	// select this channel's annotations
	auto& annotations = crecording.F().annotations(name);
	list<sigfile::SAnnotation*>
		ret;
	size_t pos = time * crecording.F().samplerate(name);
	for ( auto &A : annotations )
		if ( overlap( A.span.first, A.span.second,
			      pos, pos) )
			ret.push_back( &A);
	return ret;
}


void
aghui::SScoringFacility::SChannel::get_power( bool force)
{
	auto tmp = (crecording.obtain_power( force),
		    crecording.power_course<TFloat>( from, upto));
	auto xi = vector<size_t> (tmp.size());
	for ( size_t i = 0; i < tmp.size(); ++i )
		xi[i] = i;
	power = sigproc::interpolate( xi, 3600/_p.pagesize(), tmp, 2./3600);
}


void
aghui::SScoringFacility::SChannel::get_spectrum( size_t p)
{
	spectrum = crecording.power_spectrum<TFloat>( p);
}
void
aghui::SScoringFacility::SChannel::get_power_in_bands( bool force)
{
	crecording.obtain_power( force);
	auto xi = vector<size_t> (crecording.n_pages());
	for ( size_t i = 0; i < xi.size(); ++i )
		xi[i] = i;
	for ( size_t b = 0; b < (size_t)uppermost_band; ++b ) {
		auto	_from = _p._p.freq_bands[b][0],
			_upto = _p._p.freq_bands[b][1];
		auto tmp = crecording.power_course<TFloat>( _from, _upto);
		power_in_bands[b] =
			sigproc::interpolate( xi, 3600/_p.pagesize(),
					      tmp,
					      2./3600);
	}
}


// class SScoringFacility::SChannel

aghui::SScoringFacility::SChannel::SChannel( agh::CRecording& r,
					     SScoringFacility& parent,
					     size_t y0)
      : name (r.channel()),
	type (r.signal_type()),
	crecording (r),
	filters (r.F().filters(name)),
	annotations (r.F().annotations(name)),
	artifacts (r.F().artifacts(name)),
	_p (parent),
	zeroy (y0),
	hidden (false),
	draw_zeroline (true),
	draw_original_signal (false),
	draw_filtered_signal (true),
	zeromean_original (true),
	draw_power (true),
	draw_bands (true),
	draw_spectrum_absolute (true),
	use_resample (true),
	apply_reconstituted (false),
	marquee_start (0.),
	marquee_end (0.),
	selection_start_time (0.),
	selection_end_time (0.),
	selection_start (0),
	selection_end (0),
	_h (crecording.F().channel_id(name))
{
	get_signal_original();
	get_signal_filtered();

	signal_display_scale =
		calibrate_display_scale( signal_filtered,
					 _p.vpagesize() * samplerate() * min (crecording.F().length(), (size_t)10),
					 _p.interchannel_gap / 2);

      // power and spectrum
	if ( sigfile::SChannel::signal_type_is_fftable( type) ) {
	      // power in a single bin
		from = _p._p.operating_range_from;
		upto = _p._p.operating_range_upto;
		get_power();
	      // power spectrum (for the first page)
		n_bins = last_spectrum_bin = crecording.n_bins();
		get_spectrum( 0);
		// will be reassigned in REDRAW_ALL
		spectrum_upper_freq = n_bins * crecording.bin_size;

	      // power in bands
		size_t n_bands = sigfile::TBand::delta;
		while ( n_bands != sigfile::TBand::_total )
			if ( _p._p.freq_bands[(size_t)n_bands][0] >= spectrum_upper_freq )
				break;
			else
				++n_bands;
		uppermost_band = (sigfile::TBand)(n_bands-1);
		get_power_in_bands();

	      // delta comes first, calibrate display scale against it
		power_display_scale =
			calibrate_display_scale( power_in_bands[sigfile::TBand::delta],
						 power_in_bands[sigfile::TBand::delta].size(),
						 _p.interchannel_gap/2.);
	      // switches
		draw_spectrum_absolute = true;
		draw_bands = true;
		focused_band = sigfile::TBand::delta;

	} else if ( type == sigfile::SChannel::TType::emg ) {
		valarray<TFloat> env_u, env_l;
		sigproc::envelope( signal_original,
				   5, samplerate(), 1.,
				   env_l, env_u);
		emg_profile.resize( env_l.size());
		emg_profile = env_u - env_l;
	}

	percent_dirty = calculate_dirty_percent();


      // widgetz!

      // //expander and vbox
	{
		gchar *h_escaped = g_markup_escape_text( name, -1);
		snprintf_buf( "%s <b>%s</b>",
			      sigfile::SChannel::kemp_signal_types[type],
			      h_escaped);
		g_free( h_escaped);
	}
      // page view
	;
}

aghui::SScoringFacility::SChannel::~SChannel()
{
}


float
__attribute__ ((pure))
aghui::SScoringFacility::SChannel::calibrate_display_scale( const valarray<TFloat>& signal,
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
	auto& af = crecording.F().artifacts(_h);
	for ( auto &A : af() )
		total += A.second - A.first;
	return percent_dirty = (float)total / n_samples();
}




void
aghui::SScoringFacility::SChannel::mark_region_as_artifact( bool do_mark)
{
	if ( do_mark )
		crecording.F().artifacts(_h).mark_artifact( selection_start, selection_end);
	else
		crecording.F().artifacts(_h).clear_artifact( selection_start, selection_end);

	calculate_dirty_percent();

	get_signal_filtered();

	if ( type == sigfile::SChannel::TType::eeg ) {
		get_power();
		get_power_in_bands();
		get_spectrum( _p.cur_page());
	}
	gtk_widget_queue_draw( (GtkWidget*)_p.daSFMontage);
}

void
aghui::SScoringFacility::SChannel::mark_region_as_annotation( const char *label)
{
	crecording.F().annotations(_h).emplace_back( selection_start, selection_end,
						     label);
	gtk_widget_queue_draw( (GtkWidget*)_p.daSFMontage);
}


void
aghui::SScoringFacility::SChannel::mark_region_as_pattern()
{
	_p.find_dialog.load_pattern( *this);
	gtk_widget_show_all( (GtkWidget*)_p.find_dialog.wPattern);
}



void
aghui::SScoringFacility::SChannel::update_channel_check_menu_items()
{
	gtk_check_menu_item_set_active( _p.iSFPageShowOriginal,
					(gboolean)draw_original_signal);
	gtk_check_menu_item_set_active( _p.iSFPageShowProcessed,
					(gboolean)draw_filtered_signal);
	gtk_check_menu_item_set_active( _p.iSFPageUseResample,
					(gboolean)use_resample);
	gtk_check_menu_item_set_active( _p.iSFPageDrawZeroline,
					(gboolean)draw_zeroline);

	gtk_check_menu_item_set_active( _p.iSFPageDrawPSDProfile,
					(gboolean)draw_power);
	gtk_check_menu_item_set_active( _p.iSFPageDrawEMGProfile,
					(gboolean)draw_emg);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawPSDProfile,
				type == sigfile::SChannel::TType::eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawEMGProfile,
				type == sigfile::SChannel::TType::emg);
}


// class aghui::SScoringFacility

const array<unsigned, 9>
	aghui::SScoringFacility::DisplayPageSizeValues = {
	{4, 5, 10, 15, 20, 30, 60, 60*3, 60*5}
};

size_t
__attribute__ ((pure))
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
	_session (D),
	_sepisode (J.measurements.at(D)[E]),
	ica (NULL),
	mode (TMode::scoring),
	crosshair_at (10),
	draw_crosshair (false),
	draw_spp (true),
	pagesize_item (figure_display_pagesize_item( parent.pagesize())),
	_cur_page (0),
	_cur_vpage (0),
	skirting_run_per1 (.04),
	interchannel_gap (IntersignalSpace),
	n_hidden (0),
	find_dialog (*this),
	filters_dialog (*this),
	phasediff_dialog (*this),
	using_channel (NULL)
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
	for ( auto &H : _p.AghHH ) {
		snprintf_buf( "Reading and processing channel %s...", H.c_str());
		_p.buf_on_status_bar();
		try {
			channels.emplace_back( _sepisode.recordings.at(H),
					       *this, y);
			y += interchannel_gap;
		} catch (...) {
		}
	}
	da_ht = montage_est_height();

      // load montage
	{
		ifstream ifs (sigfile::make_fname__common( channels.front().crecording.F().filename(), true) + ".montage");
		if ( ifs.good() ) {
			ifs >> draw_crosshair >> draw_spp
			    >> sane_signal_display_scale >> sane_power_display_scale
			    // >> skirting_run_per1
			    >> interchannel_gap
			    >> n_hidden;
			for_each( channels.begin(), channels.end(),
				  [&] ( SChannel& h)
				  {
					  // ofs >> h.name;
					  ifs >> h.hidden
					      >> h.draw_zeroline
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
				  return h.type == sigfile::SChannel::TType::eeg;
			  });

      // recalculate (average) signal and power display scales
	if ( isfinite( sane_signal_display_scale) ) {
		;  // we've got it saved previously
	} else {
		sane_signal_display_scale = sane_power_display_scale = 0.;
		size_t n_with_power = 0;
		for ( auto &h : channels ) {
			sane_signal_display_scale += h.signal_display_scale;
			if ( h.type == sigfile::SChannel::TType::eeg ) {
				++n_with_power;
				sane_power_display_scale += h.power_display_scale;
			}
		}
		sane_signal_display_scale /= channels.size();
		sane_power_display_scale /= n_with_power;
		for ( auto &h : channels ) {
			h.signal_display_scale = sane_signal_display_scale;
			if ( h.type == sigfile::SChannel::TType::eeg )
				h.power_display_scale = sane_power_display_scale;
		}
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
		      "height-request", da_ht,
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

	repaint_score_stats();

	gtk_combo_box_set_active( (GtkComboBox*)(eSFPageSize),
				  pagesize_item);

	gtk_spin_button_set_value( eSFCurrentPage,
				   1);
	suppress_redraw = false;
	g_signal_emit_by_name( eSFPageSize, "changed");

	// tell main window we are done (so it can start another instance of scoring facility)
	gtk_statusbar_pop( _p.sbMainStatusBar, _p.sbContextIdGeneral);

	{
		int bar_height;
		gtk_widget_get_size_request( (GtkWidget*)cSFControlBar, NULL, &bar_height);
		int optimal_win_height = min(
			(int)HypnogramHeight + bar_height + da_ht + 100,
			(int)(gdk_screen_get_height( gdk_screen_get_default()) * .92));
		gtk_window_set_default_size( wScoringFacility,
					     gdk_screen_get_width( gdk_screen_get_default()) * .90,
					     optimal_win_height);
	}
	gtk_widget_show_all( (GtkWidget*)(wScoringFacility));
	// display proper control bar and set tooltip
	gtk_widget_set_visible( (GtkWidget*)cSFScoringModeContainer, TRUE);
	gtk_widget_set_visible( (GtkWidget*)cSFICAModeContainer, FALSE);
	set_tooltip( TTipIdx::scoring_mode);

	set_cursor_busy( false, (GtkWidget*)_p.wMainWindow);
	gtk_widget_set_sensitive( (GtkWidget*)_p.wMainWindow, TRUE);

      // advise parent we are open
	_p.open_scoring_facilities.push_front( this);
	bool enable_expd_destructive_controls =
		_p.open_scoring_facilities.begin() == _p.open_scoring_facilities.end();
	gtk_widget_set_sensitive( (GtkWidget*)_p.bScanTree,
				  enable_expd_destructive_controls);
	gtk_widget_set_sensitive( (GtkWidget*)_p.bExpChange,
				  enable_expd_destructive_controls);
	gtk_widget_set_visible( (GtkWidget*)_p.tSettings,
				enable_expd_destructive_controls);
}


aghui::SScoringFacility::~SScoringFacility()
{
	if ( ica )
		delete ica;

	// put scores
	put_hypnogram();

	// save display scales
	{
		ofstream ofs (sigfile::make_fname__common( channels.front().crecording.F().filename(), true) + ".montage");
		if ( ofs.good() ) {
			ofs << draw_crosshair << ' ' << draw_spp << ' '
			    << sane_signal_display_scale << ' ' << sane_power_display_scale << ' '
			    // << skirting_run_per1 << ' '
			    << interchannel_gap << ' '
			    << n_hidden << ' ' << endl;
			for ( auto &H : channels )
				ofs << H.hidden << ' '
				    << H.draw_zeroline << ' ' << H.draw_original_signal << ' '
				    << H.draw_filtered_signal << ' '
				    << H.draw_power << ' ' << H.draw_bands << ' ' << H.draw_spectrum_absolute << ' '
				    << H.use_resample << ' '
				    << H.zeroy << ' '
				    << H.selection_start_time << ' ' << H.selection_end_time << ' '
				    << H.signal_display_scale << ' ' << H.power_display_scale << ' ' << H.emg_scale << ' ' << endl;
		}
	}

	// destroy widgets
	gtk_widget_destroy( (GtkWidget*)wScoringFacility);
	g_object_unref( (GObject*)builder);

	_p.open_scoring_facilities.remove( this);
	bool enable_expd_destructive_controls =
		_p.open_scoring_facilities.begin() == _p.open_scoring_facilities.end();
	gtk_widget_set_sensitive( (GtkWidget*)_p.bScanTree,
				  enable_expd_destructive_controls);
	gtk_widget_set_sensitive( (GtkWidget*)_p.bExpChange,
				  enable_expd_destructive_controls);
	gtk_widget_set_visible( (GtkWidget*)_p.tSettings,
				enable_expd_destructive_controls);
}




void
aghui::SScoringFacility::get_hypnogram()
{
	// just get from the first source,
	// trust other sources are no different
	auto &F = _sepisode.sources.front();
	hypnogram.resize( F.sigfile::CHypnogram::length());
	for ( size_t p = 0; p < F.length(); ++p )
		hypnogram[p] = F[p].score_code();
}
void
aghui::SScoringFacility::put_hypnogram()
{
	// but put to all
	for( auto &F : _sepisode.sources )
		for ( size_t p = 0; p < F.sigfile::CHypnogram::length(); ++p )
			F[p].mark( hypnogram[p]);
}




void
aghui::SScoringFacility::calculate_scored_percent()
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
	if ( p < total_vpages() ) {
		_cur_vpage = p;

		if ( ap2p(p) != _cur_page ) { // vpage changed but page is same
			_cur_page = ap2p(p);
			// spectrum isn't displayed currently
			// for ( auto H = channels.begin(); H != channels.end(); ++H )
			// 	if ( H->draw_power && strcmp(H->type, "EEG") == 0 )
			// 		H->spectrum = H->crecording.power_spectrum<TFloat>( _cur_page);
		}

		// auto	cur_stage = cur_page_score();
		// snprintf_buf( "<b><big>%s</big></b>", agh::SPage::score_name(cur_stage));
		// gtk_label_set_markup( lSFCurrentStage, __buf__);

		auto	cur_pos = cur_vpage_start(); // in sec
		size_t	cur_pos_hr  =  cur_pos / 3600,
			cur_pos_min = (cur_pos - cur_pos_hr * 3600) / 60,
			cur_pos_sec =  cur_pos % 60;

		snprintf_buf( "<b>%2zu:%02zu:%02zu</b>", cur_pos_hr, cur_pos_min, cur_pos_sec);
		gtk_label_set_markup( lSFCurrentPos, __buf__);

		time_t time_at_cur_pos = start_time() + cur_pos;
		char tmp[10];
		strftime( tmp, 9, "%H:%M:%S", localtime( &time_at_cur_pos));
		snprintf_buf( "<b>%s</b>", tmp);
		gtk_label_set_markup( lSFClockTime, __buf__);

		gtk_spin_button_set_value( eSFCurrentPage, _cur_vpage+1);

		gtk_widget_set_sensitive( (GtkWidget*)bSFForward, _cur_vpage < total_vpages()-1);
		gtk_widget_set_sensitive( (GtkWidget*)bSFBack, _cur_vpage > 0);

		queue_redraw_all();
	}
	return _cur_vpage;
}

void
aghui::SScoringFacility::set_pagesize( int item)
{
	if ( item > (int)DisplayPageSizeValues.size() )
		return;
	pagesize_item = item;
	_cur_vpage = p2ap(_cur_page);

	gtk_adjustment_set_upper( jPageNo, total_vpages());
	gtk_spin_button_set_value( eSFCurrentPage, _cur_vpage+1);

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

	snprintf_buf( "/%zu", total_vpages());
	gtk_label_set_markup( lSFTotalPages, __buf__);

	queue_redraw_all();
}



void
aghui::SScoringFacility::do_score_forward( char score_ch)
{
	hypnogram[_cur_page] = score_ch;
	calculate_scored_percent();
	repaint_score_stats();
	set_cur_page( _cur_page+1);
}
void
aghui::SScoringFacility::do_score_back( char score_ch)
{
	hypnogram[_cur_page] = score_ch;
	calculate_scored_percent();
	repaint_score_stats();
	set_cur_page( _cur_page-1);
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
	for ( auto &H : channels ) {
		auto& Aa = H.crecording.F().artifacts(H._h)();
		auto spp = vpagesize() * H.samplerate();
		if ( any_of( Aa.begin(), Aa.end(),
			     [&] (const sigfile::TRegion& span)
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
	gtk_label_set_markup( lSFPercentScored, __buf__);

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
	gtk_widget_queue_draw( (GtkWidget*)daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)daSFHypnogram);
	repaint_score_stats();
}




aghui::SScoringFacility::SChannel*
__attribute__ ((pure))
aghui::SScoringFacility::channel_near( int y)
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
		gtk_widget_set_size_request( (GtkWidget*)daSFMontage,
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

	gtk_widget_set_size_request( (GtkWidget*)daSFMontage,
				     -1, thomas.back().ch->zeroy + mean_gap/2);
}


void
aghui::SScoringFacility::expand_by_factor( double fac)
{
	for ( auto &H : channels ) {
		H.signal_display_scale *= fac;
		H.power_display_scale *= fac;
		H.zeroy *= fac;
	}
	interchannel_gap *= fac;
}



sigfile::SAnnotation*
aghui::SScoringFacility::interactively_choose_annotation() const
{
	// do some on-the-fly construcion
	gtk_combo_box_set_model( eAnnotationSelectorWhich, NULL);
	gtk_list_store_clear( mAnnotationsAtCursor);
	GtkTreeIter iter;
	for ( auto &A : over_annotations ) {
		gtk_list_store_append( mAnnotationsAtCursor, &iter);
		gtk_list_store_set( mAnnotationsAtCursor, &iter,
				    0, A->label.c_str(),
				    -1);
	}
	gtk_combo_box_set_model( eAnnotationSelectorWhich, (GtkTreeModel*)mAnnotationsAtCursor);

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( wAnnotationSelector) ) {
		const char *selected_label = gtk_combo_box_get_active_id( eAnnotationSelectorWhich);
		if ( selected_label == NULL )
			return NULL;
		for ( auto &A : over_annotations )
			if ( A->label == selected_label )
				return A;
	}
	return NULL;
}




int
aghui::SScoringFacility::construct_widgets()
{
	GtkCellRenderer *renderer;

	if ( !(AGH_GBGETOBJ3 (builder, GtkWindow,		wScoringFacility)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFHint)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eSFPageSize)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkAdjustment,		jPageNo)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFTotalPages)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFCurrentPage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFClockTime)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFCurrentPos)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkExpander,		cSFHypnogram)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkHBox,			cSFControlBar)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkBox,			cSFScoringModeContainer)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkBox,			cSFICAModeContainer)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFBack)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFForward)) ||

	     // 1. scoring
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreClear)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreNREM1)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreNREM2)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreNREM3)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreNREM4)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreREM))   ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreWake))  ||
	     !(AGH_GBGETOBJ3 (builder, GtkTable,		cSFSleepStageStats)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lSFPercentScored)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoreStatsNREMPercent)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoreStatsREMPercent)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,		lScoreStatsWakePercent)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreGotoPrevUnscored)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreGotoNextUnscored)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreGotoPrevArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bScoreGotoNextArtifact)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFShowFindDialog)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFShowPhaseDiffDialog)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFDrawCrosshair)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFRunICA)) ||

	     // 2. ICA
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eSFICARemixMode)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eSFICANonlinearity)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eSFICAApproach)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkListStore,		mSFICARemixMode)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkListStore,		mSFICANonlinearity)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkListStore,		mSFICAApproach)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,		eSFICAFineTune)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,		eSFICAStabilizationMode)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAa1)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAa2)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAmu)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAepsilon)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICANofICs)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkAdjustment,		jSFICANofICs)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAEigVecFirst)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAEigVecLast)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkAdjustment,		jSFICAEigVecFirst)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkAdjustment,		jSFICAEigVecLast)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICASampleSizePercent)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkSpinButton,		eSFICAMaxIterations)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFICATry)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFICAPreview)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToggleButton,		bSFICAShowMatrix)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFICAApply)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,		bSFICACancel)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDialog,		wSFICAMatrix)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkTextView,		tSFICAMatrix)) ||

	     // rest
	     !(AGH_GBGETOBJ3 (builder, GtkDrawingArea,		daSFMontage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDrawingArea,		daSFHypnogram)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuToolButton,	bSFAccept)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu,			mSFAccept)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkStatusbar,		sbSF)) )
		return -1;

	gtk_combo_box_set_model( eSFPageSize, // reuse the one previously constructed in SExpDesignUI
				 (GtkTreeModel*)_p.mScoringPageSize);
	gtk_combo_box_set_id_column( eSFPageSize, 0);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFPageSize, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFPageSize, renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( eSFICANonlinearity,
				 (GtkTreeModel*)mSFICANonlinearity);
	gtk_combo_box_set_id_column( eSFICANonlinearity, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFICANonlinearity, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFICANonlinearity, renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( eSFICAApproach,
				 (GtkTreeModel*)mSFICAApproach);
	gtk_combo_box_set_id_column( eSFICAApproach, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFICAApproach, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFICAApproach, renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( eSFICARemixMode,
				 (GtkTreeModel*)mSFICARemixMode);
	gtk_combo_box_set_id_column( eSFICARemixMode, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFICARemixMode, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFICARemixMode, renderer,
					"text", 0,
					NULL);

	auto tabarray = pango_tab_array_new( 20, FALSE);  // 20 channels is good enough
	for ( int t = 1; t < 20; ++t )
		pango_tab_array_set_tab( tabarray, t-1, PANGO_TAB_LEFT, t * 12);
	g_object_set( tSFICAMatrix,
		      "tabs", tabarray,
		      NULL);

	// ------- menus
	if ( !(AGH_GBGETOBJ3 (builder, GtkLabel, 		lSFOverChannel)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFICAPage)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPageSelection)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPageAnnotation)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPageHidden)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFPower)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenu, 		mSFScore)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageShowOriginal)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageShowProcessed)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageUseResample)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem,	iSFPageDrawZeroline)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem, 	iSFPageDrawPSDProfile)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckMenuItem, 	iSFPageDrawEMGProfile)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageFilter)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSaveAs)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageExportSignal)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageUseThisScale)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageClearArtifacts)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageHide)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem, 		iSFPageHidden)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem, 		iSFPageSpaceEvenly)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageAnnotationSeparator)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageAnnotationDelete)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageAnnotationEdit)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionMarkArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionClearArtifact)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionFindPattern)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPageSelectionAnnotate)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerExportRange)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerExportAll)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFPowerUseThisScale)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreAssist)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreImport)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreExport)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFScoreClear)) ||

	     !(AGH_GBGETOBJ3 (builder, GtkMenuItem,		iSFAcceptAndTakeNext)) )
		return -1;

	gtk_menu_tool_button_set_menu( bSFAccept, (GtkWidget*)mSFAccept);

	gtk_menu_item_set_submenu( iSFPageHidden, (GtkWidget*)mSFPageHidden);

	// petty dialogs
	if ( !(AGH_GBGETOBJ3 (builder, GtkDialog,		wAnnotationLabel)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkEntry,		eAnnotationLabel)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDialog,		wAnnotationSelector)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkComboBox,		eAnnotationSelectorWhich)) )
		return -1;

	mAnnotationsAtCursor = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_combo_box_set_model( eAnnotationSelectorWhich,
				 (GtkTreeModel*)mAnnotationsAtCursor);
	gtk_combo_box_set_id_column( eAnnotationSelectorWhich, 0);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eAnnotationSelectorWhich, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eAnnotationSelectorWhich, renderer,
					"text", 0,
					NULL);


      // orient control widget callbacks
	g_signal_connect( eSFPageSize, "changed",
			  (GCallback)eSFPageSize_changed_cb,
			  this);
	g_signal_connect( eSFCurrentPage, "value-changed",
			  (GCallback)eSFCurrentPage_value_changed_cb,
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

	g_signal_connect( bSFForward, "clicked",
			  (GCallback)bSFForward_clicked_cb,
			  this);
	g_signal_connect( bSFBack, "clicked",
			  (GCallback)bSFBack_clicked_cb,
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

	g_signal_connect( bSFRunICA, "clicked",
			  (GCallback)bSFRunICA_clicked_cb,
			  this);
	// g_signal_connect( bSFResetMontage, "clicked",
	// 		  (GCallback)bSFResetMontage_clicked_cb,
	// 		  this);


	g_signal_connect( bSFDrawCrosshair, "toggled",
			  (GCallback)bSFDrawCrosshair_toggled_cb,
			  this);

	g_signal_connect( bSFShowFindDialog, "toggled",
			  (GCallback)bSFShowFindDialog_toggled_cb,
			  this);
	g_signal_connect( bSFShowPhaseDiffDialog, "toggled",
			  (GCallback)bSFShowPhaseDiffDialog_toggled_cb,
			  this);

	g_signal_connect( eSFICARemixMode, "changed",
			  (GCallback)eSFICARemixMode_changed_cb,
			  this);
	g_signal_connect( eSFICANonlinearity, "changed",
			  (GCallback)eSFICANonlinearity_changed_cb,
			  this);
	g_signal_connect( eSFICAApproach, "changed",
			  (GCallback)eSFICAApproach_changed_cb,
			  this);
	g_signal_connect( eSFICAFineTune, "toggled",
			  (GCallback)eSFICAFineTune_toggled_cb,
			  this);
	g_signal_connect( eSFICAStabilizationMode, "toggled",
			  (GCallback)eSFICAStabilizationMode_toggled_cb,
			  this);
	g_signal_connect( eSFICAa1, "value-changed",
			  (GCallback)eSFICAa1_value_changed_cb,
			  this);
	g_signal_connect( eSFICAa2, "value-changed",
			  (GCallback)eSFICAa2_value_changed_cb,
			  this);
	g_signal_connect( eSFICAmu, "value-changed",
			  (GCallback)eSFICAmu_value_changed_cb,
			  this);
	g_signal_connect( eSFICAepsilon, "value-changed",
			  (GCallback)eSFICAepsilon_value_changed_cb,
			  this);
	g_signal_connect( eSFICANofICs, "value-changed",
			  (GCallback)eSFICANofICs_value_changed_cb,
			  this);
	g_signal_connect( eSFICAEigVecFirst, "value-changed",
			  (GCallback)eSFICAEigVecFirst_value_changed_cb,
			  this);
	g_signal_connect( eSFICAEigVecLast, "value-changed",
			  (GCallback)eSFICAEigVecLast_value_changed_cb,
			  this);
	g_signal_connect( eSFICASampleSizePercent, "value-changed",
			  (GCallback)eSFICASampleSizePercent_value_changed_cb,
			  this);
	g_signal_connect( eSFICAMaxIterations, "value-changed",
			  (GCallback)eSFICAMaxIterations_value_changed_cb,
			  this);

	g_signal_connect( bSFICATry, "clicked",
			  (GCallback)bSFICATry_clicked_cb,
			  this);
	g_signal_connect( bSFICAPreview, "toggled",
			  (GCallback)bSFICAPreview_toggled_cb,
			  this);
	g_signal_connect( bSFICAShowMatrix, "toggled",
			  (GCallback)bSFICAShowMatrix_toggled_cb,
			  this);
	g_signal_connect( wSFICAMatrix, "hide",
			  G_CALLBACK (wSFICAMatrix_hide_cb),
			  this);

	g_signal_connect( bSFICAApply, "clicked",
			  (GCallback)bSFICAApply_clicked_cb,
			  this);
	g_signal_connect( bSFICACancel, "clicked",
			  (GCallback)bSFICACancel_clicked_cb,
			  this);


	g_signal_connect( bSFAccept, "clicked",
			  (GCallback)bSFAccept_clicked_cb,
			  this);
	g_signal_connect( iSFAcceptAndTakeNext, "activate",
			  (GCallback)iSFAcceptAndTakeNext_activate_cb,
			  this);

	g_signal_connect( wScoringFacility, "delete-event",
			  (GCallback)wScoringFacility_delete_event_cb,
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
	g_signal_connect( iSFPageDrawZeroline, "toggled",
			  (GCallback)iSFPageDrawZeroline_toggled_cb,
			  this);

	g_signal_connect( iSFPageAnnotationDelete, "activate",
			  (GCallback)iSFPageAnnotationDelete_activate_cb,
			  this);
	g_signal_connect( iSFPageAnnotationEdit, "activate",
			  (GCallback)iSFPageAnnotationEdit_activate_cb,
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
	g_signal_connect( iSFPageSelectionAnnotate, "activate",
			  (GCallback)iSFPageSelectionAnnotate_activate_cb,
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

	g_signal_connect( iSFPageSpaceEvenly, "activate",
			  (GCallback)iSFPageSpaceEvenly_activate_cb,
			  this);
	g_signal_connect( iSFPageDrawPSDProfile, "toggled",
			  (GCallback)iSFPageDrawPSDProfile_toggled_cb,
			  this);
	g_signal_connect( iSFPageDrawEMGProfile, "toggled",
			  (GCallback)iSFPageDrawEMGProfile_toggled_cb,
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

	g_signal_connect( daSFMontage, "draw",
			  (GCallback)daSFMontage_draw_cb,
			  this);
	g_signal_connect( daSFMontage, "configure-event",
			  (GCallback)daSFMontage_configure_event_cb,
			  this);
	g_signal_connect( daSFMontage, "button-press-event",
			  (GCallback)daSFMontage_button_press_event_cb,
			  this);
	g_signal_connect( daSFMontage, "button-release-event",
			  (GCallback)daSFMontage_button_release_event_cb,
			  this);
	g_signal_connect( daSFMontage, "scroll-event",
			  (GCallback)daSFMontage_scroll_event_cb,
			  this);
	g_signal_connect( daSFMontage, "motion-notify-event",
			  (GCallback)daSFMontage_motion_notify_event_cb,
			  this);
	g_signal_connect( daSFMontage, "leave-notify-event",
			  (GCallback)daSFMontage_leave_notify_event_cb,
			  this);

	g_signal_connect( daSFHypnogram, "draw",
			  (GCallback)daSFHypnogram_draw_cb,
			  this);
	// g_signal_connect_after( daSFHypnogram, "configure-event",
	// 			(GCallback)daSFHypnogram_configure_event_cb,
	// 			this);
	g_signal_connect( daSFHypnogram, "button-press-event",
			  (GCallback)daSFHypnogram_button_press_event_cb,
			  this);
	return 0;
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
	" <i>on PSD:</i>\n"
	"  Click1:	position cursor;\n"
	"  Click2:	bands/discrete 1Hz bins.\n"
	"  Wheel:	cycle focused band\n"
	"		/ in-/decrement bin;\n"
	"  Shift+Wheel:	in-/decrement scale.\n"
	"\n"
	"<b>Hypnogram:</b>\n"
	"  Click1:	position cursor;\n"
	"  Click3:	context menu.",

	"<b>ICA:</b>\n"
	"  Wheel:	adjust display scale;\n"
	"  Click1:	\"apply\" toggle;\n"
	"  Click3:	IC map context menu.\n",
};







// EOF

