// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-channel.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-05-29
 *
 *         Purpose:  scoring facility SScoringFacility::SChannel methods
 *
 *         License:  GPL
 */




#include "common/lang.hh"
#include "common/config-validate.hh"
#include "sigproc/exstrom.hh"
#include "ui/globals.hh"
#include "sf.hh"

using namespace std;


aghui::SScoringFacility::SChannel::
SChannel( agh::CRecording& r,
	  SScoringFacility& parent,
	  size_t y0,
	  char seq)
      : name (r.channel()),
	type (r.signal_type()),
	crecording (r),
	_h (r.F().channel_id(name)),
	filters (r.F().filters(name)),
	annotations (r.F().annotations(name)),
	artifacts (r.F().artifacts(name)),
	_p (parent),
	signal_lowpass  ({signal_filtered, samplerate()}),
	signal_bandpass ({signal_filtered, samplerate()}),
	signal_envelope ({signal_filtered, samplerate()}),
	signal_dzcdf    ({signal_filtered, samplerate()}),
	zeroy (y0),
	// let them be read or recalculated
	signal_display_scale( NAN),
	hidden (false),
	draw_zeroline (true),
	draw_original_signal (false),
	draw_filtered_signal (true),
	zeromean_original (true),
	draw_psd (true),
	draw_swu (false),
	draw_mc (false),
	draw_emg (true),
	draw_psd_bands (true),
	draw_spectrum (true),
	resample_signal (true),
	// resample_power (true), // set based on pages-per-pixel
	draw_selection_course (false),
	draw_selection_envelope (true),
	draw_selection_dzcdf (false),
	draw_phasic_spindle (true),
	draw_phasic_Kcomplex (true),
	apply_reconstituted (false),
	config_keys_b ({
		confval::SValidator<bool>( string(1, seq) + ".hidden",			&hidden),
		confval::SValidator<bool>( string(1, seq) + ".draw_zeroline",		&draw_zeroline),
		confval::SValidator<bool>( string(1, seq) + ".draw_original_signal",	&draw_original_signal),
		confval::SValidator<bool>( string(1, seq) + ".draw_filtered_signal",	&draw_filtered_signal),
		confval::SValidator<bool>( string(1, seq) + ".draw_emg",		&draw_emg),
		confval::SValidator<bool>( string(1, seq) + ".draw_psd",		&draw_psd),
		confval::SValidator<bool>( string(1, seq) + ".draw_swu",		&draw_swu),
		confval::SValidator<bool>( string(1, seq) + ".draw_psd_bands",		&draw_psd_bands),
		confval::SValidator<bool>( string(1, seq) + ".draw_spectrum",		&draw_spectrum),
		confval::SValidator<bool>( string(1, seq) + ".draw_mc",			&draw_mc),
		confval::SValidator<bool>( string(1, seq) + ".draw_phasic_spindle",	&draw_phasic_spindle),
		confval::SValidator<bool>( string(1, seq) + ".draw_phasic_Kcomplex",	&draw_phasic_Kcomplex),
		confval::SValidator<bool>( string(1, seq) + ".autoscale_profile",	&autoscale_profile),
		confval::SValidator<bool>( string(1, seq) + ".resample_signal",		&resample_signal),
		confval::SValidator<bool>( string(1, seq) + ".resample_power",		&resample_power),
		confval::SValidator<bool>( string(1, seq) + ".draw_selection_course",	&draw_selection_course),
		confval::SValidator<bool>( string(1, seq) + ".draw_selection_envelope",	&draw_selection_envelope),
		confval::SValidator<bool>( string(1, seq) + ".draw_selection_dzcdf",	&draw_selection_dzcdf),
	}),
	config_keys_g ({
		confval::SValidator<double>( string(1, seq) + ".zeroy",			&zeroy,			confval::SValidator<double>::SVFRangeIn (-100., 4000.)),
		confval::SValidator<double>( string(1, seq) + ".selection_start_time",	&selection_start_time),
		confval::SValidator<double>( string(1, seq) + ".selection_end_time",	&selection_end_time),
		confval::SValidator<double>( string(1, seq) + ".signal_display_scale",	&signal_display_scale,	confval::SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
		confval::SValidator<double>( string(1, seq) + ".psd_display_scale",	&psd.display_scale,	confval::SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
		confval::SValidator<double>( string(1, seq) + ".swu_display_scale",	&swu.display_scale,	confval::SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
		confval::SValidator<double>( string(1, seq) + ".mc_display_scale",	&mc.display_scale,	confval::SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
		confval::SValidator<double>( string(1, seq) + ".emg_display_scale",	&emg_display_scale,	confval::SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
	}),
	marquee_start (0.),
	marquee_end (0.),
	selection_start_time (0.),
	selection_end_time (0.),
	selection_start (0),
	selection_end (0)
{
	get_signal_original();
	get_signal_filtered();

      // psd power and spectrum, mc
	if ( sigfile::SChannel::signal_type_is_fftable( type) ) {
	      // power in a single bin
		psd.from = _p._p.active_profile_psd_freq_from;
		psd.upto = _p._p.active_profile_psd_freq_upto;
		get_psd_course();
	      // power spectrum (for the first page)
		spectrum_bins = last_spectrum_bin = crecording.psd_profile.bins();
		get_spectrum( 0);
		// will be reassigned in REDRAW_ALL
		spectrum_upper_freq = spectrum_bins * crecording.psd_profile.Pp.binsize;

	      // power in bands
		size_t n_bands = 0;
		while ( n_bands != metrics::psd::TBand::TBand_total )
			if ( _p._p.freq_bands[n_bands][0] >= spectrum_upper_freq )
				break;
			else
				++n_bands;
		psd.uppermost_band = (n_bands-1);
		get_psd_in_bands();

	      // swu profile
		swu.f0 = _p._p.active_profile_swu_f0;
		get_swu_course();

	      // mc profile
		mc.f0 = _p._p.active_profile_mc_f0;
		get_mc_course();

	      // delta comes first, calibrate display scale against it
		//update_profile_display_scales();
		// don't: interchannel_gap is rubbish yet
		psd.focused_band = metrics::psd::TBand::delta;

	} else if ( type == sigfile::SChannel::TType::emg ) {
		valarray<TFloat> env_u, env_l;
 		sigproc::envelope( {signal_original, samplerate()},
				   .5, 1.,
				   &env_l, &env_u);
		emg_profile.resize( env_l.size());
		emg_profile = env_u - env_l;
	}

      // prevent exceptions from phasic_events.at
	phasic_events[metrics::phasic::TEventTypes::spindle].clear();
	phasic_events[metrics::phasic::TEventTypes::K_complex].clear();

      // let it be so to avoid libconfig::readFile throwing exceptions
	psd.display_scale = mc.display_scale =
		emg_display_scale = DBL_MIN;

	percent_dirty = calculate_dirty_percent();
}




void
aghui::SScoringFacility::SChannel::
get_signal_original()
{
	signal_original =
		crecording.F().get_signal_original( name);
	// signal_original_resampled =
	// 	sigproc::resample( signal_original, 0, signal_original.size(),
	// 			   signal_original.size() / spp());
	if ( zeromean_original )
		signal_original -=
			signal_original.sum() / signal_original.size();
}

void
aghui::SScoringFacility::SChannel::
get_signal_filtered()
{
	signal_filtered =
		crecording.F().get_signal_filtered( name);
	// filtered is already zeromean as shipped
	drop_cached_signal_properties();
}






list<sigfile::SAnnotation*>
aghui::SScoringFacility::SChannel::
in_annotations( double time) const
{
	// select this channel's annotations
	auto& annotations = crecording.F().annotations(name);
	list<sigfile::SAnnotation*>
		ret;
	size_t pos = time * crecording.F().samplerate(name);
	for ( auto &A : annotations )
		if ( agh::alg::overlap(
			     A.span.a, A.span.z,
			     pos, pos) )
			ret.push_back( &A);
	return ret;
}


void
aghui::SScoringFacility::SChannel::
get_psd_course()
{
	//psd_profile.compute();
	auto tmp = crecording.course(
		agh::make_profile_paramset<metrics::TType::psd>( psd.from, psd.upto));
	if ( resample_power ) {
		auto xi = vector<size_t> (tmp.size());
		for ( size_t i = 0; i < tmp.size(); ++i )
			xi[i] = i;
		psd.course = sigproc::interpolate( xi, 3600/_p.pagesize(), tmp, 3./3600);
	} else
		psd.course = tmp;
}

void
aghui::SScoringFacility::SChannel::
get_psd_in_bands()
{
	crecording.psd_profile.compute();
	if ( resample_power ) {
		auto xi = vector<size_t> (crecording.psd_profile.pages());
		for ( size_t i = 0; i < xi.size(); ++i )
			xi[i] = i;
		for ( size_t b = 0; b <= psd.uppermost_band; ++b ) {
			auto	_from = _p._p.freq_bands[b][0],
				_upto = _p._p.freq_bands[b][1];
			auto tmp = crecording.psd_profile.course( _from, _upto);
			psd.course_in_bands[b] =
				sigproc::interpolate( xi, 3600/_p.pagesize(),
						      tmp,
						      3./3600);
		}
	} else
		for ( size_t b = 0; b <= psd.uppermost_band; ++b ) {
			auto	_from = _p._p.freq_bands[b][0],
				_upto = _p._p.freq_bands[b][1];
			psd.course_in_bands[b] =
				crecording.psd_profile.course( _from, _upto);
		}
}


void
aghui::SScoringFacility::SChannel::
get_swu_course()
{
	//swu_profile.compute();
	auto tmp = crecording.course(
		agh::make_profile_paramset<metrics::TType::swu>( swu.f0));
	if ( resample_power ) {
		auto xi = vector<size_t> (tmp.size());
		for ( size_t i = 0; i < tmp.size(); ++i )
			xi[i] = i;
		swu.course = sigproc::interpolate( xi, 3600/_p.pagesize(), tmp, 3./3600);
	} else
		swu.course = tmp;
}


void
aghui::SScoringFacility::SChannel::
get_mc_course()
{
	//mc_profile.compute();
	auto tmp = crecording.course(
		agh::make_profile_paramset<metrics::TType::mc>( mc.f0));
	if ( resample_power ) {
		auto xi = vector<size_t> (tmp.size());
		for ( size_t i = 0; i < tmp.size(); ++i )
			xi[i] = i;
		mc.course = sigproc::interpolate( xi, 3600/_p.pagesize(), tmp, 3./3600);
	} else
		mc.course = tmp;
}


valarray<TFloat>&
aghui::SScoringFacility::SChannel::
which_profile( metrics::TType type)
{
	switch ( type ) {
	case metrics::TType::mc:
		return get_mc_course(), mc.course;
	case metrics::TType::psd:
		return get_psd_course(), psd.course;
	case metrics::TType::swu:
		return get_swu_course(), swu.course;
	default:
		throw runtime_error ("which profile is it?");
	}
}


void
aghui::SScoringFacility::SChannel::
get_spectrum( size_t p)
{
	spectrum = crecording.psd_profile.spectrum( p);
}
void
aghui::SScoringFacility::SChannel::
get_spectrum()
{
	spectrum = crecording.psd_profile.spectrum( _p.cur_page());
}






void
aghui::SScoringFacility::SChannel::
update_profile_display_scales()
{
	psd.display_scale =
		agh::alg::calibrate_display_scale(
			draw_psd_bands ? psd.course_in_bands[psd.focused_band] : psd.course,
			psd.course.size(),
			_p.interchannel_gap/2.);

	mc.display_scale =
		agh::alg::calibrate_display_scale(
			mc.course,
			mc.course.size(),
			_p.interchannel_gap/2.);
}



float
aghui::SScoringFacility::SChannel::
calculate_dirty_percent()
{
	size_t total = 0; // in samples
	auto& af = crecording.F().artifacts(_h);
	for ( auto &A : af() )
		total += A.size();
	return percent_dirty = (float)total / n_samples();
}




void
aghui::SScoringFacility::SChannel::
detect_artifacts( const metrics::mc::SArtifactDetectionPP& P)
{
	auto marked =
		metrics::mc::detect_artifacts( signal_original, samplerate(), P);
	for ( size_t p = 0; p < marked.size(); ++p )
		artifacts.mark_artifact(
			marked[p] * P.scope * samplerate(), (marked[p]+1) * P.scope * samplerate());

	calculate_dirty_percent();
	get_signal_filtered();
	if ( type == sigfile::SChannel::TType::eeg ) {
		get_psd_course();
		get_psd_in_bands();
		get_spectrum( _p.cur_page());
		get_swu_course();
		get_mc_course();

		// if ( this == channel currently displayed on measurements overview )
		if ( strcmp( name, _p._p.AghH()) == 0 )
			_p.redraw_ssubject_timeline();
	}
}


pair<double, double>
aghui::SScoringFacility::SChannel::
mark_flat_regions_as_artifacts( double minsize, double pad)
{
	size_t	total_before = artifacts.total(),
		marked_here = 0;
	auto d =
		sigproc::derivative( signal_original);
	size_t	last_j = 0;
	for ( size_t i = 0; i < d.size(); ++i )
		if ( d[i] == 0. ) {
			size_t j = i;
			while ( j < d.size() && d[j] == 0. )
				++j;
			if ( j-i > minsize * samplerate() ) {
				size_t extend_from = (i - last_j < .1 * samplerate()) ? last_j : i;
				artifacts.mark_artifact(
					extend_from - pad * samplerate(),
					j + pad * samplerate());
				marked_here += (j - extend_from);
				last_j = j;
			}
			i = j;
		}

	calculate_dirty_percent();
	get_signal_filtered();
	if ( type == sigfile::SChannel::TType::eeg ) {
		get_psd_course();
		get_psd_in_bands();
		get_spectrum( _p.cur_page());
		get_swu_course();
		get_mc_course();

		// if ( this == channel currently displayed on measurements overview )
		if ( strcmp( name, _p._p.AghH()) == 0 )
			_p.redraw_ssubject_timeline();
	}

	return { (double)marked_here/samplerate(),
		 (double)(artifacts.total() - total_before) / samplerate() };
}




void
aghui::SScoringFacility::SChannel::
mark_region_as_artifact( bool do_mark)
{
	if ( do_mark )
		crecording.F().artifacts(_h).mark_artifact( selection_start, selection_end);
	else
		crecording.F().artifacts(_h).clear_artifact( selection_start, selection_end);

	calculate_dirty_percent();

	get_signal_filtered();

	if ( type == sigfile::SChannel::TType::eeg ) {
		get_psd_course();
		get_psd_in_bands();
		get_spectrum( _p.cur_page());
		get_swu_course();
		get_mc_course();

		if ( strcmp( name, _p._p.AghH()) == 0 )
			_p.redraw_ssubject_timeline();
	}
}

void
aghui::SScoringFacility::SChannel::
mark_region_as_annotation( const char *label)
{
	crecording.F().annotations(_h).emplace_back(
		selection_start, selection_end,
		label);
}


void
aghui::SScoringFacility::SChannel::
mark_region_as_pattern()
{
	_p.find_dialog.import_from_selection( *this);
	gtk_widget_show( (GtkWidget*)_p.wSFFD);
}



void
aghui::SScoringFacility::SChannel::
update_channel_check_menu_items()
{
	_p.suppress_redraw = true;

	gtk_check_menu_item_set_active( _p.iSFPageShowOriginal,  (gboolean)draw_original_signal);
	gtk_check_menu_item_set_active( _p.iSFPageShowProcessed, (gboolean)draw_filtered_signal);
	gtk_check_menu_item_set_active( _p.iSFPageUseResample,   (gboolean)resample_signal);
	gtk_check_menu_item_set_active( _p.iSFPageDrawZeroline,  (gboolean)draw_zeroline);

	gtk_check_menu_item_set_active( _p.iSFPageDrawPSDProfile,  (gboolean)draw_psd);
	gtk_check_menu_item_set_active( _p.iSFPageDrawPSDSpectrum, (gboolean)draw_spectrum);
	gtk_check_menu_item_set_active( _p.iSFPageDrawMCProfile,   (gboolean)draw_mc);
	gtk_check_menu_item_set_active( _p.iSFPageDrawSWUProfile,  (gboolean)draw_swu);

	gtk_check_menu_item_set_active( _p.iSFPageSelectionDrawCourse,   (gboolean)draw_selection_course);
	gtk_check_menu_item_set_active( _p.iSFPageSelectionDrawEnvelope, (gboolean)draw_selection_envelope);
	gtk_check_menu_item_set_active( _p.iSFPageSelectionDrawDzxdf,    (gboolean)draw_selection_dzcdf);

	bool	is_eeg = (type == sigfile::SChannel::TType::eeg),
		is_emg = (type == sigfile::SChannel::TType::emg),
		have_profile = is_eeg or is_emg;
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageProfilesSubmenuSeparator, have_profile);
	gtk_widget_set_visible( (GtkWidget*)_p.iiSFPageProfiles,                have_profile);
	gtk_widget_set_visible( (GtkWidget*)_p.iiSFPagePhasicEvents,            have_profile);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawPSDProfile,  is_eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawPSDSpectrum, is_eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawSWUProfile,  is_eeg);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageArtifactsDetect, is_eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawMCProfile,   is_eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawEMGProfile,  is_emg);

	_p.suppress_redraw = false;
}

void
aghui::SScoringFacility::SChannel::
update_power_check_menu_items()
{
	_p.suppress_redraw = true;
	gtk_check_menu_item_set_active( _p.iSFPageDrawEMGProfile, (gboolean)draw_emg);
	gtk_check_menu_item_set_active( _p.iSFPowerDrawBands,     (gboolean)draw_psd_bands);
	gtk_check_menu_item_set_active( _p.iSFPowerSmooth,        (gboolean)resample_power);
	gtk_check_menu_item_set_active( _p.iSFPowerAutoscale,     (gboolean)autoscale_profile);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPowerDrawBands,
				(type == sigfile::SChannel::TType::eeg &&
				 draw_psd));
	_p.suppress_redraw = false;
}


void
aghui::SScoringFacility::SChannel::
selectively_enable_page_menu_items( double x)
{
	double cpos = _p.time_at_click( x);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageHidden, _p.n_hidden > 0);
	bool over_any =
		not (_p.over_annotations = in_annotations( cpos)) . empty();
	gtk_widget_set_visible( (GtkWidget*)_p.iiSFPageAnnotation, over_any);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageAnnotationSeparator, over_any);
}

void
aghui::SScoringFacility::SChannel::
selectively_enable_selection_menu_items()
{
	bool findable =
		(selection_end_time - selection_start_time > .5);
	gtk_widget_set_sensitive( (GtkWidget*)_p.iSFPageSelectionFindPattern, findable);
}


void
aghui::SScoringFacility::SChannel::
put_selection( size_t a, size_t e)
{
	selection_start = a, selection_end = e;
	selection_start_time = (double)a / samplerate();
	selection_end_time = (double)e / samplerate();
	_put_selection();
}

void
aghui::SScoringFacility::SChannel::
put_selection( double a, double e)
{
	selection_start_time = a, selection_end_time = e;
	selection_start = a * samplerate();
	selection_end = e * samplerate();
	_put_selection();
}

void
aghui::SScoringFacility::SChannel::
_put_selection()
{
	if ( selection_end_time - selection_start_time > 1. ) {
		_p.artifact_detection_dialog.W_V.down();
		auto& P = _p.artifact_detection_dialog.P;
		auto sssu =
			metrics::mc::do_sssu_reduction(
				valarray<TFloat> {signal_filtered[ slice (selection_start, (selection_end - selection_start), 1) ]},
				samplerate(), (selection_end - selection_start) / samplerate(),
				P.mc_gain, P.iir_backpolate,
				P.f0, P.fc, P.bandwidth);
		selection_SS = sssu.first[0];
		selection_SU = sssu.second[0];
	}
}



// eof
