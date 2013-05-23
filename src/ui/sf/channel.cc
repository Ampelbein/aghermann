/*
 *       File name:  ui/sf/channel.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-05-29
 *
 *         Purpose:  scoring facility SScoringFacility::SChannel methods
 *
 *         License:  GPL
 */


#include <type_traits>
#include "common/lang.hh"
#include "common/config-validate.hh"
#include "sigproc/exstrom.hh"
#include "ui/globals.hh"
#include "sf.hh"
#include "d/artifacts.hh"
#include "d/patterns.hh"

using namespace std;

pattern::SPatternPPack<TFloat>
	aghui::SScoringFacility::SChannel::pattern_params =
		{.25,  0., 1.5, 1,  .1, .5, 3};

using agh::confval::SValidator;

aghui::SScoringFacility::SChannel::
SChannel (agh::CRecording& r,
	  SScoringFacility& parent,
	  size_t y0,
	  int seq)
      : crecording (r),
	filters (r.F().filters(h())),
	annotations (r.F().annotations(h())),
	artifacts (r.F().artifacts(r.h())),
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
	draw_phasic_eyeblink (true),
	apply_reconstituted (false),
	config_keys_b ({
		SValidator<bool>( string("h")+to_string(seq) + ".hidden",			&hidden),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_zeroline",		&draw_zeroline),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_original_signal",		&draw_original_signal),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_filtered_signal",		&draw_filtered_signal),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_emg",			&draw_emg),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_psd",			&draw_psd),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_swu",			&draw_swu),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_psd_bands",		&draw_psd_bands),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_spectrum",		&draw_spectrum),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_mc",			&draw_mc),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_phasic_spindle",		&draw_phasic_spindle),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_phasic_Kcomplex",		&draw_phasic_Kcomplex),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_phasic_eyeblink",		&draw_phasic_eyeblink),
		SValidator<bool>( string("h")+to_string(seq) + ".autoscale_profile",		&autoscale_profile),
		SValidator<bool>( string("h")+to_string(seq) + ".resample_signal",		&resample_signal),
		SValidator<bool>( string("h")+to_string(seq) + ".resample_power",		&resample_power),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_selection_course",	&draw_selection_course),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_selection_envelope",	&draw_selection_envelope),
		SValidator<bool>( string("h")+to_string(seq) + ".draw_selection_dzcdf",		&draw_selection_dzcdf),
	}),
	config_keys_g ({
		SValidator<double>( string("h")+to_string(seq) + ".zeroy",			&zeroy,			SValidator<double>::SVFRangeIn (-100., 4000.)),
		SValidator<double>( string("h")+to_string(seq) + ".selection_start_time",	&selection_start_time),
		SValidator<double>( string("h")+to_string(seq) + ".selection_end_time",		&selection_end_time),
		SValidator<double>( string("h")+to_string(seq) + ".signal_display_scale",	&signal_display_scale,	SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
		SValidator<double>( string("h")+to_string(seq) + ".psd_display_scale",		&psd.display_scale,	SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
		SValidator<double>( string("h")+to_string(seq) + ".swu_display_scale",		&swu.display_scale,	SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
		SValidator<double>( string("h")+to_string(seq) + ".mc_display_scale",		&mc.display_scale,	SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
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

      // irrespective (grown out of EMG, eventually for universal use)
	// get_raw_profile(); // too heavy; make it on-demand

      // psd power and spectrum, mc
	if ( schannel().is_fftable() ) {
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

	} else if ( schannel().type() == sigfile::SChannel::TType::emg )
		get_raw_profile();

      // let it be so to avoid libconfig::readFile throwing exceptions
	psd.display_scale = mc.display_scale = swu.display_scale =
		DBL_MIN;

	percent_dirty = calculate_dirty_percent();
}






void
aghui::SScoringFacility::SChannel::
get_signal_original()
{
	signal_original =
		crecording.F().get_signal_original( h());
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
		crecording.F().get_signal_filtered( h());
	// filtered is already zeromean as shipped
	drop_cached_signal_properties();
}






list<sigfile::SAnnotation*>
aghui::SScoringFacility::SChannel::
in_annotations( const double time) const
{
	// select this channel's annotations
	list<sigfile::SAnnotation*>
		ret;
	for ( auto &A : annotations )
		if ( agh::alg::overlap(
			     A.span.a, A.span.z,
			     time, time) )
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


void
aghui::SScoringFacility::SChannel::
get_raw_profile()
{
	raw_profile = sigproc::raw_signal_profile<TFloat>(
		{signal_filtered, samplerate()},
		1., 3.);
}


tuple<metrics::TType, valarray<TFloat>&>
aghui::SScoringFacility::SChannel::
which_profile( const metrics::TType metric)
{
	switch ( schannel().type() ) {
	case sigfile::SChannel::TType::eeg:
		switch ( metric ) {
		case metrics::TType::mc:
			return tuple<metrics::TType, valarray<TFloat>&>(metric, mc.course);
		case metrics::TType::psd:
			return tuple<metrics::TType, valarray<TFloat>&>(metric, psd.course);
		case metrics::TType::swu:
			return tuple<metrics::TType, valarray<TFloat>&>(metric, swu.course);
		case metrics::TType::raw:
			if ( raw_profile.size() == 0 )
				get_raw_profile();
			return tuple<metrics::TType, valarray<TFloat>&>(metrics::TType::raw, raw_profile);
		}
	default:
		if ( raw_profile.size() == 0 )
			get_raw_profile();
		return tuple<metrics::TType, valarray<TFloat>&>(metrics::TType::raw, raw_profile);
	}
}


void
aghui::SScoringFacility::SChannel::
get_spectrum( const size_t p)
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

	swu.display_scale =
		agh::alg::calibrate_display_scale(
			swu.course,
			swu.course.size(),
			_p.interchannel_gap/2.);
}



float
aghui::SScoringFacility::SChannel::
calculate_dirty_percent()
{
	size_t total = 0; // in samples
	for ( auto &A : artifacts() )
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
			marked[p] * P.scope, (marked[p]+1) * P.scope);

	calculate_dirty_percent();
	get_signal_filtered();
	if ( schannel().type() == sigfile::SChannel::TType::eeg ) {
		get_psd_course();
		get_psd_in_bands();
		get_spectrum( _p.cur_page());
		get_swu_course();
		get_mc_course();
		get_raw_profile();

		// if ( this == channel currently displayed on measurements overview )
		if ( strcmp( name(), _p._p.AghH()) == 0 )
			_p.redraw_ssubject_timeline();
	}
}


pair<double, double>
aghui::SScoringFacility::SChannel::
mark_flat_regions_as_artifacts( const double minsize, const double pad)
{
	double	total_before = artifacts.total();
	size_t	marked_here = 0;
	auto d =
		sigproc::derivative( signal_original);
	size_t	last_j = 0;
	for ( size_t i = 0; i < d.size(); ++i )
		if ( d[i] == 0. ) {
			size_t j = i;
			while ( j < d.size() && d[j] == 0. )
				++j;
			if ( j-i > minsize * samplerate() ) {
				size_t extend_from = (i - last_j < .1) ? last_j : i;
				artifacts.mark_artifact(
					(double)extend_from/samplerate() - pad,
					(double)j/samplerate() + pad);
				marked_here += (j - extend_from);
				last_j = j;
			}
			i = j;
		}

	calculate_dirty_percent();
	get_signal_filtered();
	if ( schannel().type() == sigfile::SChannel::TType::eeg ) {
		get_psd_course();
		get_psd_in_bands();
		get_spectrum( _p.cur_page());
		get_swu_course();
		get_mc_course();

		// if ( this == channel currently displayed on measurements overview )
		if ( strcmp( name(), _p._p.AghH()) == 0 )
			_p.redraw_ssubject_timeline();
	}

	return { (double)marked_here/samplerate(),
		 (double)(artifacts.total() - total_before) };
}




void
aghui::SScoringFacility::SChannel::
mark_region_as_artifact( const bool do_mark)
{
	if ( do_mark )
		artifacts.mark_artifact(
			selection_start_time,
			selection_end_time);
	else
		artifacts.clear_artifact(
			selection_start_time,
			selection_end_time);

	calculate_dirty_percent();

	get_signal_filtered();

	if ( schannel().type() == sigfile::SChannel::TType::eeg ) {
		get_psd_course();
		get_psd_in_bands();
		get_spectrum( _p.cur_page());
		get_swu_course();
		get_mc_course();

		if ( strcmp( name(), _p._p.AghH()) == 0 )
			_p.redraw_ssubject_timeline();
	}
}

void
aghui::SScoringFacility::SChannel::
mark_region_as_annotation( const string& label,
			   const sigfile::SAnnotation::TType type)
{
	sigfile::mark_annotation(
		annotations,
		selection_start_time, selection_end_time,
		label,
		type);
}


void
aghui::SScoringFacility::SChannel::
mark_region_as_pattern()
{
	if ( _p.patterns_d().import_from_selection( *this) == 0 )
		gtk_widget_show( (GtkWidget*)_p.patterns_d().wSFFD);
}



void
aghui::SScoringFacility::SChannel::
update_channel_menu_items( const double x)
{
	_p.suppress_redraw = true;

	bool	need_filtered = (have_low_pass() or have_high_pass() or have_notch_filter())
		or (not artifacts().empty());

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageShowOriginal, need_filtered);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageShowProcessed, need_filtered);

	gtk_check_menu_item_set_active( _p.iSFPageShowOriginal,  draw_original_signal);
	gtk_check_menu_item_set_active( _p.iSFPageShowProcessed, draw_filtered_signal);
	gtk_check_menu_item_set_active( _p.iSFPageUseResample,   resample_signal);
	gtk_check_menu_item_set_active( _p.iSFPageDrawZeroline,  draw_zeroline);

	gtk_check_menu_item_set_active( _p.iSFPageDrawPSDProfile,  draw_psd);
	gtk_check_menu_item_set_active( _p.iSFPageDrawPSDSpectrum, draw_spectrum);
	gtk_check_menu_item_set_active( _p.iSFPageDrawMCProfile,   draw_mc);
	gtk_check_menu_item_set_active( _p.iSFPageDrawSWUProfile,  draw_swu);

	gtk_check_menu_item_set_active( _p.iSFPageDrawPhasicSpindles,   draw_phasic_spindle);
	gtk_check_menu_item_set_active( _p.iSFPageDrawPhasicKComplexes, draw_phasic_Kcomplex);
	gtk_check_menu_item_set_active( _p.iSFPageDrawPhasicEyeBlinks,  draw_phasic_eyeblink);

	gtk_check_menu_item_set_active( _p.iSFPageSelectionDrawCourse,   draw_selection_course);
	gtk_check_menu_item_set_active( _p.iSFPageSelectionDrawEnvelope, draw_selection_envelope);
	gtk_check_menu_item_set_active( _p.iSFPageSelectionDrawDzxdf,    draw_selection_dzcdf);

	bool	is_eeg = (schannel().type() == sigfile::SChannel::TType::eeg),
		is_emg = (schannel().type() == sigfile::SChannel::TType::emg),
		have_profile = is_eeg or is_emg;
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageProfilesSubmenuSeparator, have_profile);
	gtk_widget_set_visible( (GtkWidget*)_p.iiSFPageProfiles,                have_profile);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawPSDProfile,  is_eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawPSDSpectrum, is_eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawSWUProfile,  is_eeg);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageArtifactsDetect, is_eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawMCProfile,   is_eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawEMGProfile,  is_emg);

	double cpos = _p.time_at_click( x);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageHidden, _p.n_hidden > 0);

	bool have_any = not annotations.empty();
	bool over_any = not (_p.over_annotations = in_annotations( cpos)) . empty();
	gtk_widget_set_visible( (GtkWidget*)_p.iiSFPageAnnotation, have_any);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageAnnotationEdit, over_any);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageAnnotationDelete, over_any);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageAnnotationSeparator, over_any);

	_p.suppress_redraw = false;
}

void
aghui::SScoringFacility::SChannel::
update_power_menu_items()
{
	_p.suppress_redraw = true;
	gtk_check_menu_item_set_active( _p.iSFPageDrawEMGProfile, (gboolean)draw_emg);
	gtk_check_menu_item_set_active( _p.iSFPowerDrawBands,     (gboolean)draw_psd_bands);
	gtk_check_menu_item_set_active( _p.iSFPowerSmooth,        (gboolean)resample_power);
	gtk_check_menu_item_set_active( _p.iSFPowerAutoscale,     (gboolean)autoscale_profile);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPowerDrawBands,
				(schannel().type() == sigfile::SChannel::TType::eeg &&
				 draw_psd));
	_p.suppress_redraw = false;
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
put_selection( const size_t a, const size_t e)
{
	selection_start = a, selection_end = e;
	selection_start_time = (double)a / samplerate();
	selection_end_time = (double)e / samplerate();
	_put_selection();
}

void
aghui::SScoringFacility::SChannel::
put_selection( const double a, const double e)
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
		_p.artifacts_d().W_V.down();
		auto& P = _p.artifacts_d().P;
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


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

