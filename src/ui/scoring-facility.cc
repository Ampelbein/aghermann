// ;-*-C++-*- *  Time-stamp: "2011-05-04 03:17:10 hmmr"
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




//#include <cassert>
//#include <math.h>
//#include <sys/stat.h>

#include <initializer_list>
#include <stdexcept>
#include <fstream>

#include <cairo/cairo-svg.h>
#include <samplerate.h>

#include "libexstrom/exstrom.hh"
#include "misc.hh"
#include "ui.hh"
#include "settings.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {

	GtkListStore
		*mScoringPageSize;

	GtkMenu
		*mSFPage,
		*mSFPageSelection,
//		*mSFPageSelectionInspectChannels,
		*mSFPower,
		*mSFScore,
		*mSFSpectrum;
	GtkColorButton
		*bColourNONE,
		*bColourNREM1,
		*bColourNREM2,
		*bColourNREM3,
		*bColourNREM4,
		*bColourREM,
		*bColourWake,
		*bColourPowerSF,
		*bColourEMG,
		*bColourHypnogram,
		*bColourArtifacts,
		*bColourTicksSF,
		*bColourLabelsSF,
		*bColourCursor,

		*bColourBandDelta,
		*bColourBandTheta,
		*bColourBandAlpha,
		*bColourBandBeta,
		*bColourBandGamma;


namespace sf {

// saved variables

SGeometry
	GeometryScoringFac;

bool	UseSigAnOnNonEEGChannels = false;

unsigned
	WidgetPageHeight = 130,
	WidgetSpectrumWidth = 100,
	WidgetPowerProfileHeight = 65,
	WidgetEMGProfileHeight = 26;


// module variables



inline namespace {

} // inline namespace



// struct member functions


// class SScoringFacility::SChannel

SScoringFacility::SChannel::SChannel( agh::CRecording& r,
				      SScoringFacility& parent)
      : name (r.channel()),
	type (r.signal_type()),
	recording (r),
	sf (parent),
	_h (recording.F().which_channel(name)),
	_ssignal (recording.F()[_h]),
	_resample_buffer (NULL),
	_resample_buffer_size (0)
{
	get_signal_original();
	get_signal_filtered();

	signal_display_scale =
		isfinite( sf.sane_signal_display_scale)
		? sf.sane_signal_display_scale
		: calibrate_display_scale( signal_filtered,
					   sf.vpagesize() * samplerate() * min (recording.F().length(), (size_t)10),
					   WidgetPageHeight / 2);

	if ( settings::UseSigAnOnNonEEGChannels || strcmp( type, "EEG") == 0 ) {
	      // and signal course
		// snprintf_buf( "(%zu/%zu) %s: low-pass...", h+1, __n_all_channels, HH[h].name);
		// BUF_ON_STATUS_BAR;
		signal_lowpass = SSFLowPassCourse (settings::BWFOrder, settings::BWFCutoff,
						   signal_filtered, samplerate());

	      // and envelope and breadth
		// snprintf_buf( "(%zu/%zu) %s: envelope...", h+1, __n_all_channels, HH[h].name);
		// BUF_ON_STATUS_BAR;
		signal_breadth = SSFEnvelope (settings::EnvTightness,
					      signal_filtered, samplerate());

	      // and dzcdf
		// snprintf_buf( "(%zu/%zu) %s: zerocrossings...", h+1, __n_all_channels, HH[h].name);
		// BUF_ON_STATUS_BAR;
		signal_dzcdf = SSFDzcdf (settings::DZCDFStep,
					 settings::DZCDFSigma,
					 settings::DZCDFSmooth,
					 signal_filtered, samplerate());
	}

      // power and spectrum
	if ( signal_type_is_fftable( type) ) {

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
			isfinite( sf.sane_power_display_scale)
			? sf.sane_power_display_scale
			: calibrate_display_scale( power_in_bands[(size_t)TBand::delta],
						   power_in_bands[(size_t)TBand::delta].size(),
						   WidgetPageHeight);
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

		 emg_scale = WidgetEMGProfileHeight/2 / largest;
	}

	percent_dirty = calculate_dirty_percent();

	draw_processed_signal = true;
	draw_original_signal = false;
	draw_dzcdf = draw_envelope = false;


      // widgetz!

      // expander and vbox
	gchar *h_escaped = g_markup_escape_text( name, -1);
	snprintf_buf( "%s <b>%s</b>", type, h_escaped);
	g_free( h_escaped);
	expander = (GtkExpander*) gtk_expander_new( __buf__);
	gtk_expander_set_use_markup( expander, TRUE);

	gtk_box_pack_start( (GtkBox*)sf.cScoringFacPageViews,
			    (GtkWidget*)expander, TRUE, TRUE, 0);
	gtk_expander_set_expanded( expander,
				   TRUE);
	gtk_container_add( (GtkContainer*)expander,
			   (GtkWidget*)(vbox = (GtkVBox*) (gtk_vbox_new( FALSE, 0))));

      // page view
	gtk_container_add( (GtkContainer*)vbox,
			   (GtkWidget*) (da_page = (GtkDrawingArea*) (gtk_drawing_area_new())));
	g_object_set( (GObject*)da_page,
		      "app-paintable", TRUE,
		      "height-request", WidgetPageHeight,
		      NULL);
	g_signal_connect_after( da_page, "expose-event",
				G_CALLBACK (daScoringFacPageView_expose_event_cb),
				(gpointer)this);
	g_signal_connect_after( da_page, "button-press-event",
				G_CALLBACK (daScoringFacPageView_button_press_event_cb),
				(gpointer)this);
	g_signal_connect_after( da_page, "button-release-event",
				G_CALLBACK (daScoringFacPageView_button_release_event_cb),
				(gpointer)this);
	g_signal_connect_after( da_page, "motion-notify-event",
				G_CALLBACK (daScoringFacPageView_motion_notify_event_cb),
				(gpointer)this);
	g_signal_connect_after( da_page, "scroll-event",
				G_CALLBACK (daScoringFacPageView_scroll_event_cb),
				(gpointer)this);
	g_signal_connect_after( da_page, "configure-event",
				G_CALLBACK (da_page_configure_event_cb),
				(gpointer)this);
	gtk_widget_add_events( (GtkWidget*)da_page,
			       (GdkEventMask)
			       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			       GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_DRAG_MOTION);

	if ( signal_type_is_fftable( type) ) {
	      // power pane
		GtkWidget *hbox;
		gtk_container_add( (GtkContainer*)vbox,
				   hbox = gtk_hbox_new( FALSE, 0));
		gtk_container_add( (GtkContainer*)hbox,
				   (GtkWidget*) (da_power = (GtkDrawingArea*) (gtk_drawing_area_new())));
		gtk_container_add_with_properties( (GtkContainer*) (hbox),
						   (GtkWidget*) (da_spectrum = (GtkDrawingArea*) (gtk_drawing_area_new())),
						   "expand", FALSE,
						   NULL);
		g_object_set( (GObject*)da_power,
			      "app-paintable", TRUE,
			      "height-request", WidgetPowerProfileHeight,
			      NULL);
		g_signal_connect_after( da_power, "expose-event",
					G_CALLBACK (daScoringFacPSDProfileView_expose_event_cb),
					(gpointer)this);
		g_signal_connect_after( da_power, "button-press-event",
					G_CALLBACK (daScoringFacPSDProfileView_button_press_event_cb),
					(gpointer)this);
		g_signal_connect_after( da_power, "scroll-event",
					G_CALLBACK (daScoringFacPSDProfileView_scroll_event_cb),
					(gpointer)this);
		g_signal_connect_after( da_power, "configure-event",
					G_CALLBACK (da_power_configure_event_cb),
					(gpointer)this);
		gtk_widget_add_events( (GtkWidget*)da_power,
				       (GdkEventMask) GDK_BUTTON_PRESS_MASK);

	      // spectrum pane
		g_object_set( (GObject*)da_spectrum,
			      "app-paintable", TRUE,
			      "width-request", WidgetSpectrumWidth,
			      NULL);
		// gtk_widget_modify_fg( da_spectrum, GTK_STATE_NORMAL, &__fg1__[cSPECTRUM]);
		// gtk_widget_modify_bg( da_spectrum, GTK_STATE_NORMAL, &__bg1__[cSPECTRUM]);

		g_signal_connect_after( da_spectrum, "expose-event",
					G_CALLBACK (daScoringFacSpectrumView_expose_event_cb),
					(gpointer)this);
		g_signal_connect_after( da_spectrum, "button-press-event",
					G_CALLBACK (daScoringFacSpectrumView_button_press_event_cb),
					(gpointer)this);
		g_signal_connect_after( da_spectrum, "scroll-event",
					G_CALLBACK (daScoringFacSpectrumView_scroll_event_cb),
					(gpointer)this);
		g_signal_connect_after( da_spectrum, "configure-event",
					G_CALLBACK (da_spectrum_configure_event_cb),
					(gpointer)this);
		gtk_widget_add_events( (GtkWidget*)da_spectrum, (GdkEventMask) GDK_BUTTON_PRESS_MASK);

	} else
		da_power = da_spectrum = NULL;

	if ( strcmp( type, "EMG") == 0 ) {
		 GtkWidget *hbox, *da_void;
		 gtk_container_add( (GtkContainer*)vbox,
				    hbox = gtk_hbox_new( FALSE, 0));
		 gtk_container_add( (GtkContainer*) (hbox),
				    (GtkWidget*) (da_emg_profile = (GtkDrawingArea*) (gtk_drawing_area_new())));
		 gtk_container_add_with_properties( (GtkContainer*)hbox,
						    da_void = gtk_drawing_area_new(),
						    "expand", FALSE,
						    NULL);
		 g_object_set( (GObject*)da_emg_profile,
			       "app-paintable", TRUE,
			       "height-request", WidgetEMGProfileHeight,
			       NULL);
		 g_object_set( (GObject*)da_void,
			       "width-request", WidgetSpectrumWidth,
			       NULL);
		 g_signal_connect_after( da_emg_profile, "expose-event",
					 G_CALLBACK (daScoringFacEMGProfileView_expose_event_cb),
					 (gpointer)this);
		 g_signal_connect_after( da_emg_profile, "button-press-event",
					 G_CALLBACK (daScoringFacEMGProfileView_button_press_event_cb),
					 (gpointer)this);
		 g_signal_connect_after( da_emg_profile, "scroll-event",
					 G_CALLBACK (daScoringFacEMGProfileView_scroll_event_cb),
					 (gpointer)this);
		 g_signal_connect_after( da_emg_profile, "configure-event",
					 G_CALLBACK (da_emg_profile_configure_event_cb),
					 (gpointer)this);
		 gtk_widget_add_events( (GtkWidget*)da_emg_profile,
					(GdkEventMask) GDK_BUTTON_PRESS_MASK);
	 } else {
		 da_emg_profile = NULL;
	 }

      // // add channel under mSFPageSelectionInspectChannels
      // 	gtk_container_add( GTK_CONTAINER (mSFPageSelectionInspectChannels),
      // 			   GTK_WIDGET (menu_item = GTK_MENU_ITEM (gtk_check_menu_item_new_with_label( name))));
      // 	g_object_set( G_OBJECT (menu_item),
      // 		      "visible", TRUE,
      // 		      NULL);
}

SScoringFacility::SChannel::~SChannel()
{
	free( (void*)_resample_buffer);
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
SScoringFacility::SChannel::mark_region_as_artifact( size_t start, size_t end,
						     bool do_mark)
{
	if ( do_mark )
		recording.F()[name].mark_artifact( start, end);
	else
		recording.F()[name].clear_artifact( start, end);

	calculate_dirty_percent();

	get_signal_filtered();

	if ( have_power() ) {
		get_power();
		get_power_in_bands();
		get_spectrum( sf.cur_page());

		gtk_widget_queue_draw( (GtkWidget*)da_power);
		gtk_widget_queue_draw( (GtkWidget*)da_spectrum);
	}
	draw_page();
}




void
SScoringFacility::SChannel::draw_signal( const valarray<float>& signal,
					 size_t start, size_t end,
					 unsigned width, int vdisp, float display_scale,
					 cairo_t *cr)
{
	if ( use_resample ) {
		if ( _resample_buffer_size != width )
			_resample_buffer = (float*)realloc( _resample_buffer,
							    (_resample_buffer_size = width) * sizeof(float));
		SRC_DATA samples;
		samples.data_in      = const_cast<float*>(&signal[start]);
		samples.input_frames = end - start;
		samples.data_out     = _resample_buffer;
		samples.src_ratio    = (double)samples.output_frames / samples.input_frames;

		if ( src_simple( &samples, SRC_SINC_FASTEST /*SRC_LINEAR*/, 1) )
			;

		size_t i;
		cairo_move_to( cr, 0,
			       - samples.data_out[0]
			       * display_scale
			       + vdisp);
		for ( i = 0; i < width; ++i )
			cairo_line_to( cr, i,
				       - samples.data_out[i]
				       * display_scale
				       + vdisp);

		free( (void*)samples.data_out);

	} else {
		size_t i;
		cairo_move_to( cr, 0,
			       - signal[ sf.cur_vpage_start() * samplerate() ]
			       * display_scale
			       + vdisp);
		size_t length = end - start;
		for ( i = 0; i < length; ++i ) {
			cairo_line_to( cr, ((float)i)/length * width,
				       - signal[ start + i ]
				       * display_scale
				       + vdisp);
		}
	}
}





// class SScoringFacility


SScoringFacility::SScoringFacility( agh::CSubject& J,
				    const string& D, const string& E)
      : draw_crosshair (false),
	draw_power (true),
	crosshair_at (10),
	marking_in_widget (NULL),
	selection_start (0),
	selection_end (0),
	using_channel (NULL),
	unfazer_mode (TUnfazerMode::none),
	unfazer_offending_channel (NULL),
	unfazer_factor (0.1),
	_cur_page (0),
	_cur_vpage (0),
	pagesize_item (4)
{
	set_cursor_busy( true, (GtkWidget*)wMainWindow);

	if ( construct_widgets() )
		throw runtime_error( "SScoringFacility::SScoringFacility(): Failed to construct own wisgets");

      // get display scales
	{
		ifstream ifs (make_fname__common( channels.front().recording.F().filename(), true) + ".displayscale");
		if ( not ifs.good() ||
		     (ifs >> sane_signal_display_scale >> sane_power_display_scale, ifs.gcount() == 0) )
			sane_signal_display_scale = sane_power_display_scale = NAN;
	}
	// sane values, now set, will be used in SChannel ctors

      // iterate all of AghHH, mark our channels
	for ( auto H = AghHH.begin(); H != AghHH.end(); ++H ) {
		snprintf_buf( "Reading and processing channel %s...", H->c_str());
		buf_on_status_bar();
		try {
			channels.emplace_back( J.measurements.at(D)[E].recordings.at(*H), *this);
		} catch (...) {
		}
	}

	if ( channels.size() == 0 )
		throw invalid_argument( string("no channels found for combination (") + J.name() + ", " + D + ", " + E + ")");

      // histogram -> scores
	const CEDFFile& F = channels.begin()->recording.F();
	hypnogram.resize( F.agh::CHypnogram::length());
	for ( size_t p = 0; p < F.CHypnogram::length(); ++p )
		hypnogram[p] = F.nth_page(p).score_code();

      // count n_eeg_channels
	n_eeg_channels =
		count_if( channels.begin(), channels.end(),
			  [] (const SChannel& h)
			  {
				  return strcmp( h.type, "EEG") == 0;
			  });

       // // finish mSFPageSelectionInspectChannels
       // 	GtkWidget *iSFPageSelectionInspectMany = gtk_menu_item_new_with_label( "Inspect these");
       // 	gtk_container_add( GTK_CONTAINER (mSFPageSelectionInspectChannels),
       // 			   iSFPageSelectionInspectMany);
       // 	g_object_set( (GObject*)(iSFPageSelectionInspectMany),
       // 		      "visible", TRUE,
       // 		      NULL);
       // 	g_signal_connect_after( iSFPageSelectionInspectMany, "select",  // but why the hell not "activate"?? GTK+ <3<3<#<#,3,3
       // 				G_CALLBACK (iSFPageSelectionInspectMany_activate_cb),
       // 				NULL);

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
	g_object_set( (GObject*)cScoringFacSleepStageStats,
		      "width-request", WidgetSpectrumWidth,
		      NULL);

	// grey out phasediff button if there are fewer than 2 EEG channels
	gtk_widget_set_sensitive( (GtkWidget*)bScoringFacShowPhaseDiffDialog, (n_eeg_channels >= 2));

	// desensitize iSFAcceptAndTakeNext unless there are more episodes
	gtk_widget_set_sensitive( (GtkWidget*)(iSFAcceptAndTakeNext),
				  J.measurements.at(D).episodes.back().name() != E);

	// draw all
	suppress_redraw = true;
	gtk_combo_box_set_active( (GtkComboBox*)(eScoringFacPageSize),
				  pagesize_is_right());

	gtk_spin_button_set_value( eScoringFacCurrentPage,
				   1);
	suppress_redraw = false;
	g_signal_emit_by_name( eScoringFacPageSize, "changed");
	//	gtk_widget_queue_draw( cMeasurements);

	gtk_statusbar_pop( sbMainStatusBar, sbContextIdGeneral);
	set_cursor_busy( false, (GtkWidget*)(wMainWindow));

	calculate_scored_percent();
	repaint_score_stats();

	gtk_window_set_default_size( wScoringFacility,
				     gdk_screen_get_width( gdk_screen_get_default()) * .93,
				     gdk_screen_get_height( gdk_screen_get_default()) * .92);
	gtk_widget_show_all( (GtkWidget*)(wScoringFacility));
}


SScoringFacility::~SScoringFacility()
{
	// save display scales
	{
		ofstream ofs (make_fname__common( channels.front().recording.F().filename(), true) + ".displayscale");
		if ( ofs.good() )
			ofs << sane_signal_display_scale << sane_power_display_scale;
	}

	// destroy widgets
	gtk_container_foreach( (GtkContainer*)cScoringFacPageViews,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	// gtk_container_foreach( (GtkContainer*)mSFPageSelectionInspectChannels,
	// 		       (GtkCallback) gtk_widget_destroy,
	// 		       NULL);
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
				if ( H->have_power() )
					H->spectrum = H->recording.power_spectrum<float>( _cur_page);
		}

		auto	cur_stage = cur_page_score();
		snprintf_buf( "<b><big>%s</big></b>", agh::SPage::score_name(cur_stage));
		gtk_label_set_markup( lScoringFacCurrentStage, __buf__);

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

		queue_redraw_all();
	}
	return _cur_vpage;
}

void
SScoringFacility::set_pagesize( int item)
{
	if ( item == pagesize_item || item > (int)settings::DisplayPageSizeValues.size() )
		return;
	pagesize_item = item;
	_cur_vpage = p2ap(_cur_page);

	gtk_spin_button_set_range( eScoringFacCurrentPage, 1, total_vpages());
	gtk_spin_button_set_value( eScoringFacCurrentPage, _cur_vpage+1);

	snprintf_buf( "<small>of</small> %zu", total_vpages());
	gtk_label_set_markup( lScoringFacTotalPages, __buf__);

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
		repaint_score_stats();
	}
}

size_t
SScoringFacility::marquee_to_selection()
{
	float	x1 = marquee_start,
		x2 = marquee_virtual_end;
	if ( x1 > x2 ) { float _ = x1; x1 = x2, x2 = _; }
	if ( x1 < 0. ) x1 = 0.;

	selection_start = (_cur_vpage + x1/using_channel->da_page_wd) * vpagesize() * using_channel->samplerate();
	selection_end   = (_cur_vpage + x2/using_channel->da_page_wd) * vpagesize() * using_channel->samplerate();
	if ( selection_start > using_channel->n_samples() )
		return 0;
	if ( selection_end > using_channel->n_samples() )
		selection_end = using_channel->n_samples();

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
SScoringFacility::repaint_score_stats()
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
//		g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
	for ( auto H = channels.begin(); H != channels.end(); ++H ) {
		if ( gtk_expander_get_expanded( H->expander) )
			gtk_widget_queue_draw( (GtkWidget*)H->da_page);
		if ( H->have_power() ) {
			gtk_widget_queue_draw( (GtkWidget*)H->da_power);
			gtk_widget_queue_draw( (GtkWidget*)H->da_spectrum);
		}
		if ( H->da_emg_profile )
			gtk_widget_queue_draw( (GtkWidget*)H->da_emg_profile);
	}
	gtk_widget_queue_draw( (GtkWidget*)daScoringFacHypnogram);
}


int
SScoringFacility::construct_widgets()
{
	 GtkCellRenderer *renderer;

	 if ( !(AGH_GBGETOBJ (GtkWindow,	wScoringFacility)) ||
	      !(AGH_GBGETOBJ (GtkComboBox,	eScoringFacPageSize)) ||
	      !(AGH_GBGETOBJ (GtkVBox,		cScoringFacPageViews)) ||
	      !(AGH_GBGETOBJ (GtkDrawingArea,	daScoringFacHypnogram)) ||
	      !(AGH_GBGETOBJ (GtkButton,	bScoringFacBack)) ||
	      !(AGH_GBGETOBJ (GtkButton,	bScoringFacForward)) ||
	      !(AGH_GBGETOBJ (GtkLabel,		lScoringFacTotalPages)) ||
	      !(AGH_GBGETOBJ (GtkSpinButton,	eScoringFacCurrentPage)) ||
	      !(AGH_GBGETOBJ (GtkLabel,		lScoringFacClockTime)) ||
	      !(AGH_GBGETOBJ (GtkLabel,		lScoringFacCurrentStage)) ||
	      !(AGH_GBGETOBJ (GtkLabel,		lScoringFacCurrentPos)) ||
	      !(AGH_GBGETOBJ (GtkLabel,		lScoringFacPercentScored)) ||
	      !(AGH_GBGETOBJ (GtkLabel,		lScoreStatsNREMPercent)) ||
	      !(AGH_GBGETOBJ (GtkLabel,		lScoreStatsREMPercent)) ||
	      !(AGH_GBGETOBJ (GtkLabel,		lScoreStatsWakePercent)) ||

	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreClear)) ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreNREM1)) ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreNREM2)) ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreNREM3)) ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreNREM4)) ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreREM))   ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreWake))  ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreMVT))   ||

	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreGotoPrevUnscored)) ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreGotoNextUnscored)) ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreGotoPrevArtifact)) ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bScoreGotoNextArtifact)) ||
	      !(AGH_GBGETOBJ (GtkToolButton,	bSFAccept)) ||

	      !(AGH_GBGETOBJ (GtkToggleButton,	bScoringFacShowFindDialog)) ||
	      !(AGH_GBGETOBJ (GtkToggleButton,	bScoringFacShowPhaseDiffDialog)) ||
	      !(AGH_GBGETOBJ (GtkToggleButton,	bScoringFacDrawPower)) ||
	      !(AGH_GBGETOBJ (GtkToggleButton,	bScoringFacDrawCrosshair)) ||

	      !(AGH_GBGETOBJ (GtkTable,		cScoringFacSleepStageStats)) ||
	      !(AGH_GBGETOBJ (GtkLabel,		lScoringFacHint)) ||
	      !(AGH_GBGETOBJ (GtkStatusbar,	sbSF)) )
		 return -1;

	 gtk_combo_box_set_model( (GtkComboBox*)(eScoringFacPageSize),
				  (GtkTreeModel*)(mScoringPageSize));

	 renderer = gtk_cell_renderer_text_new();
	 gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eScoringFacPageSize), renderer, FALSE);
	 gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eScoringFacPageSize), renderer,
					 "text", 0,
					 NULL);
      // ------- menus
	 if ( !(AGH_GBGETOBJ (GtkMenu, mSFPage)) ||
	      !(AGH_GBGETOBJ (GtkMenu, mSFPageSelection)) ||
//	      !(AGH_GBGETOBJ (GtkMenu, mSFPageSelectionInspectChannels)) ||
	      !(AGH_GBGETOBJ (GtkMenu, mSFPower)) ||
	      !(AGH_GBGETOBJ (GtkMenu, mSFScore)) ||
	      !(AGH_GBGETOBJ (GtkMenu, mSFSpectrum)) )
		 return -1;

	 if ( !(AGH_GBGETOBJ (GtkCheckMenuItem, iSFPageShowOriginal)) ||
	      !(AGH_GBGETOBJ (GtkCheckMenuItem, iSFPageShowProcessed)) ||
	      !(AGH_GBGETOBJ (GtkCheckMenuItem, iSFPageShowDZCDF)) ||
	      !(AGH_GBGETOBJ (GtkCheckMenuItem, iSFPageShowEnvelope)) ||
	      !(AGH_GBGETOBJ (GtkMenuItem,	iSFPageUnfazer)) ||
	      !(AGH_GBGETOBJ (GtkMenuItem,	iSFPageSelectionMarkArtifact)) ||
	      !(AGH_GBGETOBJ (GtkMenuItem,	iSFPageSelectionClearArtifact)) ||
	      !(AGH_GBGETOBJ (GtkMenuItem,	iSFPageSaveAs)) ||
	      !(AGH_GBGETOBJ (GtkMenuItem,	iSFPageExportSignal)) ||
	      !(AGH_GBGETOBJ (GtkMenuItem,	iSFPageUseThisScale)) ||
	      !(AGH_GBGETOBJ (GtkMenuItem,	iSFAcceptAndTakeNext)) )
		 return -1;

	// orient control widget callbacks
	g_signal_connect_after( eScoringFacPageSize, "changed",
				G_CALLBACK (eScoringFacPageSize_changed_cb),
				(gpointer)this);
	g_signal_connect_after( eScoringFacCurrentPage, "value-changed",
				G_CALLBACK (eScoringFacCurrentPage_value_changed_cb),
				(gpointer)this);


	g_signal_connect_after( bScoreClear, "clicked",
				G_CALLBACK (bScoreClear_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoreNREM1, "clicked",
				G_CALLBACK (bScoreNREM1_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoreNREM2, "clicked",
				G_CALLBACK (bScoreNREM2_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoreNREM3, "clicked",
				G_CALLBACK (bScoreNREM3_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoreNREM4, "clicked",
				G_CALLBACK (bScoreNREM4_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoreREM, "clicked",
				G_CALLBACK (bScoreREM_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoreWake, "clicked",
				G_CALLBACK (bScoreWake_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoreMVT, "clicked",
				G_CALLBACK (bScoreMVT_clicked_cb),
				(gpointer)this);

	g_signal_connect_after( bScoringFacForward, "clicked",
				G_CALLBACK (bScoringFacForward_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoringFacBack, "clicked",
				G_CALLBACK (bScoringFacBack_clicked_cb),
				(gpointer)this);

	g_signal_connect_after( bScoreGotoNextUnscored, "clicked",
				G_CALLBACK (bScoreGotoNextUnscored_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoreGotoPrevUnscored, "clicked",
				G_CALLBACK (bScoreGotoPrevUnscored_clicked_cb),
				(gpointer)this);

	g_signal_connect_after( bScoreGotoNextArtifact, "clicked",
				G_CALLBACK (bScoreGotoNextArtifact_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bScoreGotoPrevArtifact, "clicked",
				G_CALLBACK (bScoreGotoPrevArtifact_clicked_cb),
				(gpointer)this);

	g_signal_connect_after( bScoringFacDrawPower, "toggled",
				G_CALLBACK (bScoringFacDrawPower_toggled_cb),
				(gpointer)this);
	g_signal_connect_after( bScoringFacDrawCrosshair, "toggled",
				G_CALLBACK (bScoringFacDrawCrosshair_toggled_cb),
				(gpointer)this);

	g_signal_connect_after( bScoringFacShowFindDialog, "toggled",
				G_CALLBACK (bScoringFacShowFindDialog_toggled_cb),
				(gpointer)this);
	g_signal_connect_after( bScoringFacShowPhaseDiffDialog, "toggled",
				G_CALLBACK (bScoringFacShowPhaseDiffDialog_toggled_cb),
				(gpointer)this);

	g_signal_connect_after( bSFAccept, "clicked",
				G_CALLBACK (bSFAccept_clicked_cb),
				(gpointer)this);

      // menus
	g_signal_connect_after( mSFPage, "show",
				G_CALLBACK (mSFPage_show_cb),
				(gpointer)this);

	g_signal_connect_after( iSFPageShowOriginal, "toggled",
				G_CALLBACK (iSFPageShowOriginal_toggled_cb),
				(gpointer)this);
	g_signal_connect_after( iSFPageShowProcessed, "toggled",
				G_CALLBACK (iSFPageShowProcessed_toggled_cb),
				(gpointer)this);
	g_signal_connect_after( iSFPageShowEnvelope, "toggled",
				G_CALLBACK (iSFPageShowEnvelope_toggled_cb),
				(gpointer)this);
	g_signal_connect_after( iSFPageShowDZCDF, "toggled",
				G_CALLBACK (iSFPageShowDZCDF_toggled_cb),
				(gpointer)this);

	g_signal_connect_after( iSFPageSelectionMarkArtifact, "activate",
				G_CALLBACK (iSFPageSelectionMarkArtifact_activate_cb),
				(gpointer)this);
	g_signal_connect_after( iSFPageSelectionClearArtifact, "activate",
				G_CALLBACK (iSFPageSelectionClearArtifact_activate_cb),
				(gpointer)this);

	g_signal_connect_after( iSFPageUnfazer, "activate",
				G_CALLBACK (iSFPageUnfazer_activate_cb),
				(gpointer)this);
	g_signal_connect_after( iSFPageSaveAs, "activate",
				G_CALLBACK (iSFPageSaveAs_activate_cb),
				(gpointer)this);
	g_signal_connect_after( iSFPageExportSignal, "activate",
				G_CALLBACK (iSFPageExportSignal_activate_cb),
				(gpointer)this);
	g_signal_connect_after( iSFPageUseThisScale, "activate",
				G_CALLBACK (iSFPageUseThisScale_activate_cb),
				(gpointer)this);



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
construct( GtkBuilder *builder)
{
      // ------ colours
	 if ( !(AGH_GBGETOBJ (GtkColorButton, bColourNONE)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourNREM1)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourNREM2)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourNREM3)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourNREM4)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourREM)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourWake)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourPowerSF)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourEMG)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourHypnogram)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourArtifacts)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourTicksSF)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourLabelsSF)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourCursor)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourBandDelta)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourBandTheta)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourBandAlpha)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourBandBeta)) ||
	      !(AGH_GBGETOBJ (GtkColorButton, bColourBandGamma)) )
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


	gboolean
	da_page_configure_event_cb( GtkWidget *widget,
				    GdkEventConfigure  *event,
				    gpointer   userdata)
	{
		if ( event->type == GDK_CONFIGURE ) {
			auto& Ch = *(SScoringFacility::SChannel*)userdata;
			Ch.da_page_ht = event->height;
			Ch.da_page_wd = event->width;
		}
		return FALSE;
	}

	gboolean
	da_power_configure_event_cb( GtkWidget *widget,
				     GdkEventConfigure  *event,
				     gpointer   userdata)
	{
		if ( event->type == GDK_CONFIGURE ) {
			auto& Ch = *(SScoringFacility::SChannel*)userdata;
			Ch.da_power_ht = event->height;
			Ch.da_power_wd = event->width;
		}
		return FALSE;
	}

	gboolean
	da_spectrum_configure_event_cb( GtkWidget *widget,
					GdkEventConfigure  *event,
					gpointer   userdata)
	{
		if ( event->type == GDK_CONFIGURE ) {
			auto& Ch = *(SScoringFacility::SChannel*)userdata;
			Ch.da_spectrum_ht = event->height;
			Ch.da_spectrum_wd = event->width;
		}
		return FALSE;
	}

	gboolean
	da_emg_profile_configure_event_cb( GtkWidget *widget,
					   GdkEventConfigure  *event,
					   gpointer   userdata)
	{
		if ( event->type == GDK_CONFIGURE ) {
			auto& Ch = *(SScoringFacility::SChannel*)userdata;
			Ch.da_emg_profile_ht = event->height;
			Ch.da_emg_profile_wd = event->width;
		}
		return FALSE;
	}





// ---------- page value_changed


	void
	eScoringFacPageSize_changed_cb( GtkComboBox *widget, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		gint item = gtk_combo_box_get_active( (GtkComboBox*)widget);
		SF->set_pagesize( item); // -1 is fine here
	}

	void
	eScoringFacCurrentPage_value_changed_cb( GtkSpinButton *spinbutton, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		SF->set_cur_vpage( gtk_spin_button_get_value( SF->eScoringFacCurrentPage) - 1);
	}








// -------------- various buttons






	void bScoreClear_clicked_cb( GtkButton *button, gpointer user_data)  { ((SScoringFacility*)user_data)->do_score_forward( agh::SPage::score_code(TScore::none)); }
	void bScoreNREM1_clicked_cb( GtkButton *button, gpointer user_data)  { ((SScoringFacility*)user_data)->do_score_forward( agh::SPage::score_code(TScore::nrem1)); }
	void bScoreNREM2_clicked_cb( GtkButton *button, gpointer user_data)  { ((SScoringFacility*)user_data)->do_score_forward( agh::SPage::score_code(TScore::nrem2)); }
	void bScoreNREM3_clicked_cb( GtkButton *button, gpointer user_data)  { ((SScoringFacility*)user_data)->do_score_forward( agh::SPage::score_code(TScore::nrem3)); }
	void bScoreNREM4_clicked_cb( GtkButton *button, gpointer user_data)  { ((SScoringFacility*)user_data)->do_score_forward( agh::SPage::score_code(TScore::nrem4)); }
	void bScoreREM_clicked_cb  ( GtkButton *button, gpointer user_data)  { ((SScoringFacility*)user_data)->do_score_forward( agh::SPage::score_code(TScore::rem)); }
	void bScoreWake_clicked_cb ( GtkButton *button, gpointer user_data)  { ((SScoringFacility*)user_data)->do_score_forward( agh::SPage::score_code(TScore::wake)); }
	void bScoreMVT_clicked_cb  ( GtkButton *button, gpointer user_data)  { ((SScoringFacility*)user_data)->do_score_forward( agh::SPage::score_code(TScore::mvt)); }





	void
	bScoringFacForward_clicked_cb( GtkButton *button, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		SF->set_cur_vpage( SF->cur_vpage() + 1);
	}

	void
	bScoringFacBack_clicked_cb( GtkButton *button, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		SF->set_cur_vpage( SF->cur_vpage() - 1);
	}




	void
	bScoreGotoPrevUnscored_clicked_cb( GtkButton *button, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
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
	bScoreGotoNextUnscored_clicked_cb( GtkButton *button, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
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
	bScoreGotoPrevArtifact_clicked_cb( GtkButton *button, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
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
	bScoreGotoNextArtifact_clicked_cb( GtkButton *button, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
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
	bScoringFacDrawPower_toggled_cb( GtkToggleButton *button, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		SF->draw_power = !SF->draw_power;
		for ( auto H = SF->channels.begin(); H != SF->channels.end(); ++H )
			if ( H->have_power() ) {
				g_object_set( (GObject*)H->da_power,
					      "visible", SF->draw_power ? TRUE : FALSE,
					      NULL);
				g_object_set( (GObject*)H->da_spectrum,
					      "visible", SF->draw_power ? TRUE : FALSE,
					      NULL);
			}
		SF->queue_redraw_all();
	}

	void
	bScoringFacDrawCrosshair_toggled_cb( GtkToggleButton *button, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		SF->draw_crosshair = !SF->draw_crosshair;
		SF->queue_redraw_all();
	}





	void
	bScoringFacShowFindDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		if ( gtk_toggle_button_get_active( togglebutton) ) {
			gtk_widget_show_all( (GtkWidget*)SF->wPattern);
		} else
			gtk_widget_hide( (GtkWidget*)SF->wPattern);
	}



	void
	bScoringFacShowPhaseDiffDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		if ( gtk_toggle_button_get_active( togglebutton) ) {
			gtk_widget_show_all( (GtkWidget*)SF->wPhaseDiff);
		} else
			gtk_widget_hide( (GtkWidget*)SF->wPhaseDiff);
	}







// -- PageSelection


	void
	iSFPageSelectionMarkArtifact_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		if ( SF->marquee_to_selection() > 0 )
			SF->using_channel->mark_region_as_artifact( SF->selection_start, SF->selection_end, true);
	}

	void
	iSFPageSelectionClearArtifact_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		if ( SF->marquee_to_selection() > 0 )
			SF->using_channel->mark_region_as_artifact( SF->selection_start, SF->selection_end, false);
	}




	void
	bSFAccept_clicked_cb( GtkButton *button, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		gtk_widget_hide( (GtkWidget*)SF->wPattern);
		gtk_widget_hide( (GtkWidget*)SF->wScoringFacility);

		gtk_widget_queue_draw( (GtkWidget*)cMeasurements);

		delete SF;
	}


	void
	iSFAcceptAndTakeNext_activate_cb( GtkMenuItem *menuitem, gpointer user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
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
		gtk_widget_show_all( (GtkWidget*)wScoringFacility);
		set_cursor_busy( false, (GtkWidget*)wScoringFacility);
	}



// ------- cleanup

	gboolean
	wScoringFacility_delete_event_cb( GtkWidget *widget,
					  GdkEvent  *event,
					  gpointer   user_data)
	{
		SScoringFacility* SF = (SScoringFacility*)user_data;
		delete SF;
		// not sure resurrection will succeed, tho

		gtk_widget_hide( (GtkWidget*)wPattern);
		gtk_widget_hide( (GtkWidget*)wScoringFacility);
		gtk_widget_queue_draw( (GtkWidget*)cMeasurements);

		return TRUE; // to stop other handlers from being invoked for the event
	}





// -------- colours


	void
	bColourNONE_color_set_cb( GtkColorButton *widget,
				  gpointer        user_data)
	{
		CwB[TColour::score_none].acquire();
	}

	void
	bColourNREM1_color_set_cb( GtkColorButton *widget,
				   gpointer        user_data)
	{
		CwB[TColour::score_nrem1].acquire();
	}


	void
	bColourNREM2_color_set_cb( GtkColorButton *widget,
				   gpointer        user_data)
	{
		CwB[TColour::score_nrem2].acquire();
	}


	void
	bColourNREM3_color_set_cb( GtkColorButton *widget,
				   gpointer        user_data)
	{
		CwB[TColour::score_nrem3].acquire();
	}


	void
	bColourNREM4_color_set_cb( GtkColorButton *widget,
				   gpointer        user_data)
	{
		CwB[TColour::score_nrem4].acquire();
	}

	void
	bColourREM_color_set_cb( GtkColorButton *widget,
				 gpointer        user_data)
	{
		CwB[TColour::score_rem].acquire();
	}

	void
	bColourWake_color_set_cb( GtkColorButton *widget,
				  gpointer        user_data)
	{
		CwB[TColour::score_wake].acquire();
	}



	void
	bColourPowerSF_color_set_cb( GtkColorButton *widget,
				     gpointer        user_data)
	{
		CwB[TColour::power_sf].acquire();
	}


	void
	bColourHypnogram_color_set_cb( GtkColorButton *widget,
				       gpointer        user_data)
	{
		CwB[TColour::hypnogram].acquire();
	}

	void
	bColourArtifacts_color_set_cb( GtkColorButton *widget,
				       gpointer        user_data)
	{
		CwB[TColour::artifact].acquire();
	}



	void
	bColourTicksSF_color_set_cb( GtkColorButton *widget,
				     gpointer        user_data)
	{
		CwB[TColour::ticks_sf].acquire();
	}

	void
	bColourLabelsSF_color_set_cb( GtkColorButton *widget,
				      gpointer        user_data)
	{
		CwB[TColour::labels_sf].acquire();
	}

	void
	bColourCursor_color_set_cb( GtkColorButton *widget,
				    gpointer        user_data)
	{
		CwB[TColour::cursor].acquire();
	}


	void
	bColourBandDelta_color_set_cb( GtkColorButton *widget,
				       gpointer        user_data)
	{
		CwB[TColour::band_delta].acquire();
	}
	void
	bColourBandTheta_color_set_cb( GtkColorButton *widget,
				       gpointer        user_data)
	{
		CwB[TColour::band_theta].acquire();
	}
	void
	bColourBandAlpha_color_set_cb( GtkColorButton *widget,
				       gpointer        user_data)
	{
		CwB[TColour::band_alpha].acquire();
	}
	void
	bColourBandBeta_color_set_cb( GtkColorButton *widget,
				      gpointer        user_data)
	{
		CwB[TColour::band_beta].acquire();
	}
	void
	bColourBandGamma_color_set_cb( GtkColorButton *widget,
				       gpointer        user_data)
	{
		CwB[TColour::band_gamma].acquire();
	}

} // extern "C"


} // namespace aghui


// EOF

