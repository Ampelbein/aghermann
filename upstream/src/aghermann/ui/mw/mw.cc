/*
 *       File name:  aghermann/ui/mw/mw.cc
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
#include <signal.h>

#include "common/config-validate.hh"
#include "libmetrics/page-metrics-base.hh"
#include "libmetrics/mc-artifacts.hh"
#include "libmetrics/bands.hh"
//#include "aghermann/expdesign/expdesign.hh"
#include "aghermann/model/beersma.hh"
#include "aghermann/ui/misc.hh"
#include "aghermann/ui/sf/sf.hh"
#include "aghermann/ui/mf/mf.hh"
#include "mw.hh"
#include "mw_cb.hh"

using namespace std;
using namespace agh::ui;


SExpDesignUI::SSubjectPresentation::
SSubjectPresentation (agh::CSubject& _j,
		      SGroupPresentation& parent)
      : csubject (_j),
	using_episode (nullptr),
	is_focused (false),
	_p (parent),
	da (nullptr)
{
	cprofile = nullptr;
	create_cprofile();
}


agh::SProfileParamSet
SExpDesignUI::
make_active_profile_paramset() const
{
	switch ( display_profile_type ) {
	case metrics::TType::psd:
		return agh::SProfileParamSet (
			agh::SProfileParamSet::PSD {active_profile_psd_freq_from, active_profile_psd_freq_upto}
			);
	case metrics::TType::swu:
		return agh::SProfileParamSet (
			agh::SProfileParamSet::SWU {active_profile_swu_f0}
			);
	case metrics::TType::mc:
		return agh::SProfileParamSet (
			agh::SProfileParamSet::MC {active_profile_mc_f0}
			);
	default:
		throw runtime_error ("Which profile is this?");
	}
}


void
SExpDesignUI::SSubjectPresentation::
create_cprofile()
{
	if ( cprofile )
		delete cprofile;
	try {
		agh::SProfileParamSet Pp;
		cprofile =
			new agh::CProfile (
				csubject, *_p._p._AghDi, *_p._p._AghTi,
				_p._p.make_active_profile_paramset());
		tl_start = csubject.measurements[*_p._p._AghDi].episodes.front().start_rel;
	} catch (...) {  // can be invalid_argument (no recording in such session/channel) or some TSimPrepError
		cprofile = nullptr;
		fprintf( stderr, "SSubjectPresentation::SSubjectPresentation(): subject \"%s\" has no recordings in session \"%s\" channel \"%s\"\n",
			 csubject.id.c_str(), _p._p.AghD(), _p._p.AghT());
	}
}

SExpDesignUI::SSubjectPresentation::
~SSubjectPresentation ()
{
	if ( cprofile )
		delete cprofile;
}





SExpDesignUI::SSubjectPresentation*
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



const array<unsigned, 4>
	SExpDesignUI::FFTPageSizeValues = {{4, 20, 30, 60}};
const array<double, 3>
	SExpDesignUI::FFTBinSizeValues = {{.1, .25, .5}};

double SExpDesignUI::scroll_factor = 1.05;

using agh::confval::SValidator;

SExpDesignUI::
SExpDesignUI (SSessionChooser *parent,
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
	dl_pid (-1),
	close_this_SF_now (nullptr),
	display_profile_type (metrics::TType::psd),
	active_profile_psd_freq_from (2.),
	active_profile_psd_freq_upto (3.),
	active_profile_swu_f0 (.5),
	active_profile_mc_f0 (.5),
	only_plain_global_annotations (true),
	strict_subject_id_checks (false),
	suppress_scan_report (false),
	uc_accuracy_factor (1.),
	pagesize_item (2),
	binsize_item (1),
	ext_score_codes ({
		{{" -0"}, {"1"}, {"2"}, {"3"}, {"4"}, {"6Rr8"}, {"Ww5"}}
	}),
	profile_scale_psd (0.),
	profile_scale_swu (0.),
	profile_scale_mc (0.),
	autoscale (false),
	smooth_profile (1),
	tl_height (80),
	tl_pph (30),
	sort_by (TSubjectSortBy::name),
	sort_ascending (true),
	sort_segregate (false),
	browse_command ("thunar"),
	config_keys_s ({
		SValidator<string>("WindowGeometry.Main",		&_geometry_placeholder),
		SValidator<string>("Common.CurrentSession",		&_aghdd_placeholder),
		SValidator<string>("Common.CurrentChannel",		&_aghtt_placeholder),
		SValidator<string>("Measurements.BrowseCommand",	&browse_command),
	}),
	config_keys_b ({
		SValidator<bool>("Common.OnlyPlainAnnotations",		&only_plain_global_annotations),
		SValidator<bool>("ScanTree.StrictSubjectIdCheck",	&strict_subject_id_checks),
		SValidator<bool>("ScanTree.SuppressScanReport",		&suppress_scan_report),
	}),
	config_keys_d ({
		SValidator<int>("Common.Sort.By",		        (int*)&sort_by,		                        SValidator<int>::SVFRangeIn ( 0,   3)),
		SValidator<int>("Common.Sort.Ascending",	        (int*)&sort_ascending,	                        SValidator<int>::SVFRangeIn ( 0,   1)),
		SValidator<int>("Common.Sort.Segregate",	        (int*)&sort_segregate,	                        SValidator<int>::SVFRangeIn ( 0,   1)),
		SValidator<int>("Measurements.DisplayProfileType",	(int*)&display_profile_type,			SValidator<int>::SVFRangeIn ( 0,   3)),
		SValidator<int>("Measurements.SmoothSide",		(int*)&smooth_profile,				SValidator<int>::SVFRangeIn ( 1,  20)),
		SValidator<int>("Measurements.TimelineHeight",		(int*)&tl_height,				SValidator<int>::SVFRangeIn (10, 600)),
		SValidator<int>("Measurements.TimelinePPH",		(int*)&tl_pph,					SValidator<int>::SVFRangeIn (10, 600)),
		SValidator<int>("ScoringFacility.IntersignalSpace",	(int*)&SScoringFacility::IntersignalSpace,	SValidator<int>::SVFRangeIn (10, 800)),
		SValidator<int>("ScoringFacility.HypnogramHeight",	(int*)&SScoringFacility::HypnogramHeight,	SValidator<int>::SVFRangeIn (10, 300)),
		SValidator<int>("ModelRun.SWASmoothOver",		(int*)&SModelrunFacility::swa_smoothover,	SValidator<int>::SVFRangeIn ( 1,   5)),
	}),
	config_keys_g ({
		SValidator<double>("UltradianCycleDetectionAccuracy",	&uc_accuracy_factor,				SValidator<double>::SVFRangeIn (0.5, 20.)),
		SValidator<double>("Measurements.ProfileScalePSD",	&profile_scale_psd,				SValidator<double>::SVFRangeIn (0., 1e10)), // can be 0, will trigger autoscale
		SValidator<double>("Measurements.ProfileScaleSWU",	&profile_scale_swu,				SValidator<double>::SVFRangeIn (0., 1e10)),
		SValidator<double>("Measurements.ProfileScaleMC",	&profile_scale_mc,				SValidator<double>::SVFRangeIn (0., 1e10)),
		SValidator<double>("Profiles.PSD.FreqFrom",		&active_profile_psd_freq_from,			SValidator<double>::SVFRangeIn (0., 20.)),
		SValidator<double>("Profiles.PSD.FreqUpto",		&active_profile_psd_freq_upto,			SValidator<double>::SVFRangeIn (0., 20.)),
		SValidator<double>("Profiles.SWU.F0",			&active_profile_swu_f0,				SValidator<double>::SVFRangeIn (0., 20.)),
		SValidator<double>("Profiles.MC.F0",			&active_profile_mc_f0,				SValidator<double>::SVFRangeIn (0., 20.)),
		SValidator<double>("Common.ScrollFactor",		&scroll_factor,					SValidator<double>::SVFRangeIn (1.001, 1.5)),
	})
{
	nodestroy_by_cb = true;
	suppress_redraw = true;

	set_wMainWindow_interactive( false);
	set_controls_for_empty_experiment( true, false);
	gtk_widget_show( (GtkWidget*)wMainWindow);

	if ( not dir.empty() and not agh::fs::exists_and_is_writable( dir) )
		throw invalid_argument (string("Experiment directory ") + dir + " does not exist or is not writable");

	string sure_dir;
	if ( dir.empty() ) { // again? only happens when user has moved a previously valid expdir between sessions
		sure_dir = string (getenv("HOME")) + "/EmptyDummy";
		if ( agh::fs::mkdir_with_parents( sure_dir) != 0 )
			throw invalid_argument ("The last used experiment directory does not exist (or is not writable),"
						" and a dummy fallback directory could not be created in your $HOME."
						" Whatever the reason, this is really too bad: I can't fix it for you.");
	} else
		sure_dir = dir;
	ED = new agh::CExpDesign (sure_dir,
				  bind( &SExpDesignUI::sb_main_progress_indicator, this,
					placeholders::_1, placeholders::_2, placeholders::_3,
					ui::TGtkRefreshMode::gtk));
	load_artifact_detection_profiles();
	if ( global_artifact_detection_profiles.empty() )
		global_artifact_detection_profiles["default"] = metrics::mc::SArtifactDetectionPP ();

	nodestroy_by_cb = false;

	// bind fields to widgets
	// tab 1
	W_V1.reg( eSMPMaxThreads,		&ED->num_threads);
	W_V1.reg( eScanTreeStrict,		&ED->strict_subject_id_checks);
	W_V1.reg( eArtifDampenWindowType, (int*)&ED->af_dampen_window_type);
	W_V1.reg( eArtifDampenFactor,		&ED->af_dampen_factor);
	W_V1.reg( eFFTParamsWindowType,   (int*)&ED->fft_params.welch_window_type);
	W_V1.reg( eMCParamIIRBackpolate,	&ED->mc_params.iir_backpolate);
	W_V1.reg( eMCParamMCGain,		&ED->mc_params.mc_gain);
	W_V1.reg( eMCParamBandWidth,		&ED->mc_params.bandwidth);
	W_V1.reg( eMCParamFreqInc,		&ED->mc_params.freq_inc);
	W_V1.reg( eMCParamNBins,		&ED->mc_params.n_bins);
	W_V1.reg( eSWUParamMinUpswingDuration,	&ED->swu_params.min_upswing_duration);

	W_V1.reg( eScanTreeSuppressReport,	&suppress_scan_report);
	W_V1.reg( eFFTParamsPageSize,		&pagesize_item);
	W_V1.reg( eFFTParamsBinSize,		&binsize_item);
	W_V1.reg( eFFTParamsPlanType,	  (int*)&ED->fft_params.plan_type);
	W_V1.reg( eUltradianCycleDetectionAccuracy, &uc_accuracy_factor);
	for ( size_t i = 0; i < sigfile::SPage::TScore::TScore_total; ++i )
		W_V1.reg( eScoreCode[i], &ext_score_codes[i]);
	for ( size_t i = 0; i < metrics::TBand::TBand_total; ++i ) {
		W_V1.reg( eBand[i][0], &ED->freq_bands[i][0]);
		W_V1.reg( eBand[i][1], &ED->freq_bands[i][1]);
	}
	W_V1.reg( eDAMsmtPPH, (int*)&tl_pph);
	W_V1.reg( eDAMsmtTLHeight, (int*)&tl_height);
	W_V1.reg( eDAPageHeight, (int*)&SScoringFacility::IntersignalSpace);
	W_V1.reg( eDAHypnogramHeight, (int*)&SScoringFacility::HypnogramHeight);
	W_V1.reg( eDAEMGHeight, (int*)&SScoringFacility::EMGProfileHeight);
	W_V1.reg( eBrowseCommand, &browse_command);
	W_V1.reg( eScrollSpeedFactor, &scroll_factor);

	// set _saved, too
	fft_params_welch_window_type_saved	= ED->fft_params.welch_window_type;
	fft_params_plan_type_saved		= ED->fft_params.plan_type;
	af_dampen_window_type_saved		= ED->af_dampen_window_type;
	af_dampen_factor_saved			= ED->af_dampen_factor;
	mc_params_saved				= ED->mc_params;
	pagesize_item_saved = pagesize_item	= figure_pagesize_item();
	binsize_item_saved = binsize_item 	= figure_binsize_item();

	// tab 2
	W_V2.reg( eCtlParamAnnlNTries,		&ED->ctl_params0.siman_params.n_tries);
	W_V2.reg( eCtlParamAnnlItersFixedT,	&ED->ctl_params0.siman_params.iters_fixed_T);
	W_V2.reg( eCtlParamAnnlStepSize,	&ED->ctl_params0.siman_params.step_size);
	W_V2.reg( eCtlParamAnnlBoltzmannk,	&ED->ctl_params0.siman_params.k);
	W_V2.reg( eCtlParamAnnlDampingMu,	&ED->ctl_params0.siman_params.mu_t);
	W_V2.reg( eCtlParamAnnlTMinMantissa,	&ctl_params0_siman_params_t_min_mantissa);
	W_V2.reg( eCtlParamAnnlTMinExponent,	&ctl_params0_siman_params_t_min_exponent);
	W_V2.reg( eCtlParamAnnlTInitialMantissa,&ctl_params0_siman_params_t_initial_mantissa);
	W_V2.reg( eCtlParamAnnlTInitialExponent,&ctl_params0_siman_params_t_initial_exponent);

	W_V2.reg( eCtlParamDBAmendment1,		&ED->ctl_params0.DBAmendment1);
	W_V2.reg( eCtlParamDBAmendment2,		&ED->ctl_params0.DBAmendment2);
	W_V2.reg( eCtlParamAZAmendment1,		&ED->ctl_params0.AZAmendment1);
	W_V2.reg( eCtlParamNSWAPpBeforeSimStart,  (int*)&ED->swa_laden_pages_before_SWA_0);
	W_V2.reg( eCtlParamReqScoredPercent,		&ED->req_percent_scored);
	W_V2.reg( eCtlParamScoreUnscoredAsWake,		&ED->score_unscored_as_wake);

	// tunables are isolated so they can be reset separately
	for ( size_t t = 0; t < agh::ach::TTunable::_basic_tunables; ++t ) {
		W_Vtunables.reg( eTunable[t][0], &ED->tunables0 [t]);
		W_Vtunables.reg( eTunable[t][1], &ED->tlo       [t]);
		W_Vtunables.reg( eTunable[t][2], &ED->thi       [t]);
		W_Vtunables.reg( eTunable[t][3], &ED->tstep     [t]);
	}

	// for ( size_t i = metrics::psd::TBand::delta; i < metrics::psd::TBand::TBand_total; ++i ) {
	// 	gtk_spin_button_set_value( eBand[i][0], ED->freq_bands[i][0]);
	// 	gtk_spin_button_set_value( eBand[i][1], ED->freq_bands[i][1]);
	// }

	populate( true);

	// set check and radio menuitems in global menu
	switch ( sort_by ) {
	case TSubjectSortBy::name:		gtk_check_menu_item_set_active( (GtkCheckMenuItem*)iExpSubjectSortName,		TRUE); break;
	case TSubjectSortBy::age:		gtk_check_menu_item_set_active( (GtkCheckMenuItem*)iExpSubjectSortAge,		TRUE); break;
	case TSubjectSortBy::admission_date:	gtk_check_menu_item_set_active( (GtkCheckMenuItem*)iExpSubjectSortAdmissionDate,TRUE); break;
	case TSubjectSortBy::avg_profile_power:	gtk_check_menu_item_set_active( (GtkCheckMenuItem*)iExpSubjectSortAvgPower,	TRUE); break;
	}
	gtk_check_menu_item_set_active( iExpSubjectSortAscending, sort_ascending);
	gtk_check_menu_item_set_active( iExpSubjectSortSegregate, sort_segregate);

	set_wMainWindow_interactive( true, false);
	suppress_redraw = false;
}

void
SExpDesignUI::
load_artifact_detection_profiles()
{
	FILE *domien = fopen( (string(ED->session_dir()) + "/.AD_profiles").c_str(), "r");
	if ( domien ) {
		while ( !feof (domien) ) {
			metrics::mc::SArtifactDetectionPP P;
			char *name = nullptr;
			int int_estimate_E, int_use_range;
// at least gcc 4.7.2 fails to recognize "%as" (dynamic allocation), so
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic push
			if ( 16 ==
			     fscanf( domien, "%a[^\n]\n%la  %la %la  %la %la %la  %la %la  %la %la %la "
				     "%zu %zu %d %d",
				     &name,
				     &P.scope,
				     &P.upper_thr, &P.lower_thr,
				     &P.f0, &P.fc, &P.bandwidth,
				     &P.mc_gain, &P.iir_backpolate,
				     &P.E, &P.dmin, &P.dmax,
				     &P.sssu_hist_size,
				     &P.smooth_side,
				     &int_estimate_E,
				     &int_use_range) ) {

				P.estimate_E = (bool)int_estimate_E;
				P.use_range = (bool)int_use_range;
				global_artifact_detection_profiles[name] = P;
#pragma GCC diagnostic pop
			} else
				break;

			free( (void*)name);
		}
		fclose( domien);
	}
}

void
SExpDesignUI::
save_artifact_detection_profiles() const
{
	// libconfig::Config conf;
	// auto&	root = conf.getRoot();
	// auto&	profiles = root.add("profiles", libconfig::Setting::Type::TypeArray);
	if ( global_artifact_detection_profiles.size() == 0 )
		return;

	FILE *domien = fopen( (string(ED->session_dir()) + "/.AD_profiles").c_str(), "w");
	if ( domien ) {
		for ( auto &P : global_artifact_detection_profiles ) {
			fprintf( domien, "%s\n", P.first.c_str());
			fprintf( domien, "%a  %a %a  %a %a %a  %a %a  %a %a %a "
				 "%zu %zu %d %d\n",
				 P.second.scope,
				 P.second.upper_thr, P.second.lower_thr,
				 P.second.f0, P.second.fc, P.second.bandwidth,
				 P.second.mc_gain, P.second.iir_backpolate,
				 P.second.E, P.second.dmin, P.second.dmax,
				 P.second.sssu_hist_size,
				 P.second.smooth_side,
				 (int)P.second.estimate_E,
				 (int)P.second.use_range);

		}
		fclose( domien);
	} else
		fprintf( stderr, "failed to open $EXPROOT/.AD_profiles for writing\n");
}


size_t
__attribute__ ((pure))
SExpDesignUI::
figure_pagesize_item()
{
	size_t i = 0;
	while ( FFTPageSizeValues[i] < ED->fft_params.pagesize )
		++i;
	return i;
}
size_t
__attribute__ ((pure))
SExpDesignUI::
figure_binsize_item()
{
	size_t i = 0;
	while ( FFTBinSizeValues[i] < ED->fft_params.binsize )
		++i;
	return i;
}



SExpDesignUI::
~SExpDesignUI ()
{
	if ( dl_pid > 0 )
		kill( dl_pid, SIGTERM);

	save_settings();
	save_artifact_detection_profiles();
	auto sielle = open_scoring_facilities;
	// let SF dtor reach back and erase its own mention from open_scoring_facilities
	for ( auto& SF : sielle )
		delete SF;

	delete ED;
}






void
SExpDesignUI::
do_rescan_tree( const bool with_update)
{
	ui::SBusyBlock bb (wMainWindow);

	depopulate( false);
	ED -> sync();
	if ( with_update )
		ED -> scan_tree( bind (&SExpDesignUI::sb_main_progress_indicator, this,
				       placeholders::_1, placeholders::_2, placeholders::_3,
				       ui::TGtkRefreshMode::gdk));
	else
		ED -> scan_tree();
	populate( false);
}


void
SExpDesignUI::
do_purge_computed()
{
	SBusyBlock bb (wMainWindow);

	if ( ED->purge_cached_profiles() ) {
		fprintf( stderr, "Command '%s' returned a non-zero status. This is suspicious.\n", ui::global::buf);
		sb_message( "Failed to purge cache files. This is odd.");
	}

	sb_message( "Purged computed files cache");
}


void
SExpDesignUI::
do_detect_ultradian_cycle( agh::CRecording& M)
{
	gsl_siman_params_t siman_params;
	// specially for ubuntu
	siman_params.n_tries		=   (int)(5 * uc_accuracy_factor);
	siman_params.iters_fixed_T	=   (int)(10 * uc_accuracy_factor);
	siman_params.step_size		=    4;
	siman_params.k			=    1.0;
	siman_params.t_initial  	=   10 * uc_accuracy_factor;
	siman_params.mu_t		=    1.003;
	siman_params.t_min		=    5e-2;

	agh::beersma::SUltradianCycle
		L = agh::beersma::ultradian_cycles(
			M, {make_active_profile_paramset(), .1, siman_params});

	if ( M.uc_params )
		delete M.uc_params;
	M.uc_params = new agh::beersma::SUltradianCycle (L);
}




void
SExpDesignUI::
adjust_op_freq_spinbuttons()
{
	suppress_redraw = true;

	gtk_adjustment_set_step_increment( jMsmtProfileParamsPSDFreqFrom,  ED->fft_params.binsize);
	gtk_adjustment_set_step_increment( jMsmtProfileParamsPSDFreqWidth, ED->fft_params.binsize);
	if ( not used_eeg_samplerates.empty() )
		gtk_adjustment_set_upper(
			jMsmtProfileParamsPSDFreqFrom,
			ED->fft_params.binsize * (ED->fft_params.compute_n_bins( used_eeg_samplerates.back()) - 1));

	gtk_adjustment_set_step_increment( jMsmtProfileParamsMCF0,
					   ED->mc_params.freq_inc); // matches the default in metrics/mc.cc
	gtk_adjustment_set_upper( jMsmtProfileParamsMCF0,
				  ED->mc_params.compute_n_bins(pagesize()) *
				  ED->mc_params.freq_inc);

	suppress_redraw = false;
}



double
SExpDesignUI::
calculate_profile_scale()
{
	double	avg_profile_height = 0.;
	size_t	valid_episodes = 0;
	for ( auto& G : groups )
		for ( auto &J : G )
			if ( J.cprofile && !J.cprofile->mm_list().empty() ) {
				auto this_episode_avg = J.cprofile->metric_avg();
				if ( isfinite(this_episode_avg) ) {
					avg_profile_height += this_episode_avg;
					++valid_episodes;
				}
			}
	double value =
		unlikely (valid_episodes == 0)
		? 1.
		: tl_height / valid_episodes / avg_profile_height * .9;

	switch ( display_profile_type ) {
	case metrics::TType::psd:
		profile_scale_psd = value;
	    break;
	case metrics::TType::swu:
		profile_scale_swu = value;
	    break;
	case metrics::TType::mc:
		profile_scale_mc  = value;
	    break;
	default:
	    break;
	}

	return value;
}



void
SExpDesignUI::
show_changelog()
{
	gtk_widget_show_all( (GtkWidget*)wAbout);
	gtk_notebook_set_current_page( cAboutTabs, 1);
}





void
SExpDesignUI::
sb_message( const string& msg) const
{
	gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
	gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral, msg.c_str());
}

void
SExpDesignUI::
sb_clear() const
{
	gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
}



void
SExpDesignUI::
sb_main_progress_indicator( const string& current,
			    const size_t n, const size_t i,
			    const TGtkRefreshMode mode)
{
	sb_message( agh::str::sasprintf( "(%zu of %zu) %s", i, n, current.c_str()));

	switch ( mode ) {
	case TGtkRefreshMode::gtk:
		gtk_flush();  // this brings down the whole of GTK if called from a callback
		break;
	case TGtkRefreshMode::gdk:
		gdk_window_process_all_updates(); // this, however, fails to do the update if called *not* from a callback
		break;
	}
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
