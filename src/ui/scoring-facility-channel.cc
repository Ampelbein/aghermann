// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-channel.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-05-29
 *
 *         Purpose:  scoring facility SScoringFacility::SChannel methods
 *
 *         License:  GPL
 */




#include "../common/lang.hh"
#include "../common/config-validate.hh"
#include "../sigproc/exstrom.hh"
#include "globals.hh"
#include "scoring-facility.hh"

using namespace std;


aghui::SScoringFacility::SChannel::
SChannel( agh::CRecording& r,
	  SScoringFacility& parent,
	  size_t y0,
	  char seq)
      : name (r.channel()),
	type (r.signal_type()),
	crecording (r),
	filters (r.F().filters(name)),
	annotations (r.F().annotations(name)),
	artifacts (r.F().artifacts(name)),
	_p (parent),
	zeroy (y0),
	// let them be read or recalculated
	signal_display_scale( NAN),
	hidden (false),
	draw_zeroline (true),
	draw_original_signal (false),
	draw_filtered_signal (true),
	zeromean_original (true),
	draw_psd (true),
	draw_mc (false),
	draw_emg (true),
	draw_bands (true),
	draw_spectrum (true),
	resample_signal (true),
	// resample_power (true), // set based on pages-per-pixel
	draw_selection_course (false),
	draw_selection_envelope (true),
	draw_selection_dzcdf (false),
	apply_reconstituted (false),
	config_keys_b ({
		confval::SValidator<bool>( string(1, seq) + ".hidden",			&hidden),
		confval::SValidator<bool>( string(1, seq) + ".draw_zeroline",		&draw_zeroline),
		confval::SValidator<bool>( string(1, seq) + ".draw_original_signal",	&draw_original_signal),
		confval::SValidator<bool>( string(1, seq) + ".draw_filtered_signal",	&draw_filtered_signal),
		confval::SValidator<bool>( string(1, seq) + ".draw_emg",		&draw_emg),
		confval::SValidator<bool>( string(1, seq) + ".draw_psd",		&draw_psd),
		confval::SValidator<bool>( string(1, seq) + ".draw_bands",		&draw_bands),
		confval::SValidator<bool>( string(1, seq) + ".draw_spectrum",		&draw_spectrum),
		confval::SValidator<bool>( string(1, seq) + ".draw_mc",			&draw_mc),
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
		confval::SValidator<double>( string(1, seq) + ".mc_display_scale",	&mc.display_scale,	confval::SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
		confval::SValidator<double>( string(1, seq) + ".emg_display_scale",	&emg_display_scale,	confval::SValidator<double>::SVFRangeIn (DBL_MIN, INFINITY)),
	}),
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

      // psd power and spectrum, mc
	if ( sigfile::SChannel::signal_type_is_fftable( type) ) {
	      // power in a single bin
		psd.from = _p._p.operating_range_from;
		psd.upto = _p._p.operating_range_upto;
		get_psd_course( false);
	      // power spectrum (for the first page)
		spectrum_bins = last_spectrum_bin = crecording.CBinnedPower::bins();
		get_spectrum( 0);
		// will be reassigned in REDRAW_ALL
		spectrum_upper_freq = spectrum_bins * crecording.binsize;

	      // power in bands
		size_t n_bands = 0;
		while ( n_bands != sigfile::TBand::_total )
			if ( _p._p.freq_bands[n_bands][0] >= spectrum_upper_freq )
				break;
			else
				++n_bands;
		psd.uppermost_band = (n_bands-1);
		get_psd_in_bands( false);

	      // mc profile
		mc.bin = (_p._p.operating_range_from - sigfile::SMCParamSet::freq_from) / crecording.bandwidth;
		get_mc_course( false);

	      // delta comes first, calibrate display scale against it
		//update_profile_display_scales();
		// don't: interchannel_gap is rubbish yet
		psd.focused_band = sigfile::TBand::delta;

	} else if ( type == sigfile::SChannel::TType::emg ) {
		valarray<TFloat> env_u, env_l;
		sigproc::envelope( signal_original,
				   5, samplerate(), 1.,
				   env_l, env_u);
		emg_profile.resize( env_l.size());
		emg_profile = env_u - env_l;
	}

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
	// signal_filtered_resampled =
	// 	sigproc::resample( signal_filtered, 0, signal_filtered.size(),
	// 			   signal_filtered.size() / spp());
	// filtered is already zeromean as shipped
}


void
aghui::SScoringFacility::SChannel::
compute_lowpass( float _cutoff, unsigned _order)
{
	if ( signal_lowpass.data.size() == 0 ||
	     signal_lowpass.cutoff != _cutoff || signal_lowpass.order != _order )
		signal_lowpass.data =
			exstrom::low_pass( signal_filtered, samplerate(),
					   signal_lowpass.cutoff = _cutoff,
					   signal_lowpass.order = _order, true);
}


void
aghui::SScoringFacility::SChannel::
compute_tightness( unsigned _tightness)
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
aghui::SScoringFacility::SChannel::
compute_dzcdf( float _step, float _sigma, unsigned _smooth)
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
get_psd_course( bool force)
{
	auto tmp = (crecording.sigfile::CBinnedPower::compute( force),
		    crecording.sigfile::CBinnedPower::course<TFloat>( psd.from, psd.upto));
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
get_psd_in_bands( bool force)
{
	crecording.sigfile::CBinnedPower::compute( force);
	if ( resample_power ) {
		auto xi = vector<size_t> (crecording.CBinnedPower::pages());
		for ( size_t i = 0; i < xi.size(); ++i )
			xi[i] = i;
		for ( size_t b = 0; b <= psd.uppermost_band; ++b ) {
			auto	_from = _p._p.freq_bands[b][0],
				_upto = _p._p.freq_bands[b][1];
			auto tmp = crecording.sigfile::CBinnedPower::course<TFloat>( _from, _upto);
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
				crecording.sigfile::CBinnedPower::course<TFloat>( _from, _upto);
		}
}


void
aghui::SScoringFacility::SChannel::
get_mc_course( bool force)
{
	auto tmp = (crecording.sigfile::CBinnedMC::compute( force),
		    crecording.sigfile::CBinnedMC::course<TFloat>( mc.bin));
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
get_spectrum( size_t p)
{
	spectrum = crecording.sigfile::CBinnedPower::spectrum<TFloat>( p);
}
void
aghui::SScoringFacility::SChannel::
get_spectrum()
{
	spectrum = crecording.sigfile::CBinnedPower::spectrum<TFloat>( _p.cur_page());
}






void
aghui::SScoringFacility::SChannel::
update_profile_display_scales()
{
	psd.display_scale =
		agh::alg::calibrate_display_scale(
			draw_bands ? psd.course_in_bands[psd.focused_band] : psd.course,
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
detect_artifacts( SDetectArtifactsParams P)
{
	if ( P.pre_clear )
		crecording.F().artifacts(_h).clear_all();

	auto	sssu =
		sigfile::CBinnedMC::do_sssu_reduction(
			signal_original,
			samplerate(), P.scope,
			P.mc_gain, P.iir_backpolate,
			P.f0, P.fc, P.bandwidth);
	valarray<TFloat>
		sssu_diff =
		{sssu.first - sssu.second};

	sigproc::smooth( sssu_diff, P.smooth_side);

	if ( !isfinite(P.E) )
		P.E = P.use_range
			? sigfile::CBinnedMC::estimate_E(
				sssu_diff,
				P.sssu_hist_size,
				P.dmin, P.dmax)
			: sigfile::CBinnedMC::estimate_E(
				sssu_diff,
				P.sssu_hist_size);

	auto marked =
		sigfile::CBinnedMC::detect_artifacts(
			sssu_diff,
			P.upper_thr, P.lower_thr,
			P.E);
	for ( size_t p = 0; p < marked.size(); ++p )
		crecording.F().artifacts(_h).mark_artifact(
			marked[p] * P.scope * samplerate(), (marked[p]+1) * P.scope * samplerate());

	calculate_dirty_percent();
	get_signal_filtered();
	if ( type == sigfile::SChannel::TType::eeg ) {
		get_psd_course( false);
		get_psd_in_bands( false);
		get_spectrum( _p.cur_page());
		get_mc_course( false);

		// if ( this == channel currently displayed on measurements overview )
		if ( strcmp( name, _p._p.AghH()) == 0 )
			_p.redraw_ssubject_timeline();
	}
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
		get_psd_course( false);
		get_psd_in_bands( false);
		get_spectrum( _p.cur_page());
		get_mc_course( false);

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
	_p.find_dialog.load_pattern( *this);
	gtk_widget_show_all( (GtkWidget*)_p.wPattern);
}



void
aghui::SScoringFacility::SChannel::
update_channel_check_menu_items()
{
	_p.suppress_redraw = true;

	gtk_check_menu_item_set_active( _p.iSFPageShowOriginal,
					(gboolean)draw_original_signal);
	gtk_check_menu_item_set_active( _p.iSFPageShowProcessed,
					(gboolean)draw_filtered_signal);
	gtk_check_menu_item_set_active( _p.iSFPageUseResample,
					(gboolean)resample_signal);
	gtk_check_menu_item_set_active( _p.iSFPageDrawZeroline,
					(gboolean)draw_zeroline);

	gtk_check_menu_item_set_active( _p.iSFPageDrawPSDProfile,
					(gboolean)draw_psd);
	gtk_check_menu_item_set_active( _p.iSFPageDrawPSDSpectrum,
					(gboolean)draw_spectrum);
	gtk_check_menu_item_set_active( _p.iSFPageDrawMCProfile,
					(gboolean)draw_mc);

	gtk_check_menu_item_set_active( _p.iSFPageSelectionDrawCourse,
					(gboolean)draw_selection_course);
	gtk_check_menu_item_set_active( _p.iSFPageSelectionDrawEnvelope,
					(gboolean)draw_selection_envelope);
	gtk_check_menu_item_set_active( _p.iSFPageSelectionDrawDzxdf,
					(gboolean)draw_selection_dzcdf);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawPSDProfile,
				type == sigfile::SChannel::TType::eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawPSDSpectrum,
				type == sigfile::SChannel::TType::eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDetectArtifacts,
				type == sigfile::SChannel::TType::eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawMCProfile,
				type == sigfile::SChannel::TType::eeg);
	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageDrawEMGProfile,
				type == sigfile::SChannel::TType::emg);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPageProfileItemsSeparator,
				type == sigfile::SChannel::TType::eeg || type == sigfile::SChannel::TType::emg);

	_p.suppress_redraw = false;
}

void
aghui::SScoringFacility::SChannel::
update_power_check_menu_items()
{
	_p.suppress_redraw = true;
	gtk_check_menu_item_set_active( _p.iSFPageDrawEMGProfile,
					(gboolean)draw_emg);
	gtk_check_menu_item_set_active( _p.iSFPowerDrawBands,
					(gboolean)draw_bands);
	gtk_check_menu_item_set_active( _p.iSFPowerSmooth,
					(gboolean)resample_power);
	gtk_check_menu_item_set_active( _p.iSFPowerAutoscale,
					(gboolean)autoscale_profile);

	gtk_widget_set_visible( (GtkWidget*)_p.iSFPowerDrawBands,
				(type == sigfile::SChannel::TType::eeg &&
				 draw_psd));
	_p.suppress_redraw = false;
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
		auto P =
			_p.get_mc_params_from_SFAD_widgets();
		auto sssu =
			sigfile::CBinnedMC::do_sssu_reduction(
				signal_filtered[ slice (selection_start, (selection_end - selection_start), 1) ],
				samplerate(), (selection_end - selection_start) / samplerate(),
				P.mc_gain, P.iir_backpolate,
				P.f0, P.fc, P.bandwidth);
		selection_SS = sssu.first[0];
		selection_SU = sssu.second[0];
	}
}



// eof
