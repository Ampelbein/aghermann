// ;-*-C++-*-
/*
 *       File name:  ui/mw/mw.cc
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

//#include <vte/vte.h>

#include "common/config-validate.hh"
#include "libsigfile/page-metrics-base.hh"
#include "expdesign/primaries.hh"
#include "libsigfile/artifacts.hh"
#include "model/beersma.hh"
#include "ui/misc.hh"
#include "ui/sf/sf.hh"
#include "ui/mf/mf.hh"
#include "mw.hh"
#include "mw_cb.hh"

using namespace std;
using namespace aghui;




aghui::SExpDesignUI::SSubjectPresentation::
SSubjectPresentation (agh::CSubject& _j,
		      SGroupPresentation& parent)
      : csubject (_j),
	using_episode (nullptr),
	is_focused (false),
	_p (parent),
	da (nullptr)
{
	cscourse = nullptr;
	create_cscourse();
}


void
aghui::SExpDesignUI::SSubjectPresentation::
create_cscourse()
{
	if ( cscourse )
		delete cscourse;
	try {
		cscourse =
			new agh::CSCourse (
				csubject, *_p._p._AghDi, *_p._p._AghTi,
				agh::SSCourseParamSet {
					_p._p.display_profile_type,
					_p._p.operating_range_from, _p._p.operating_range_upto,
					0., 0, false});
		tl_start = csubject.measurements[*_p._p._AghDi].episodes.front().start_rel;
	} catch (...) {  // can be invalid_argument (no recording in such session/channel) or some TSimPrepError
		cscourse = nullptr;
		fprintf( stderr, "SSubjectPresentation::SSubjectPresentation(): subject %s has no recordings in session %s channel %s\n",
			 csubject.name(), _p._p.AghD(), _p._p.AghT());
	}
}

aghui::SExpDesignUI::SSubjectPresentation::
~SSubjectPresentation ()
{
	if ( cscourse )
		delete cscourse;
}





aghui::SExpDesignUI::SSubjectPresentation*
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



const char
	*const aghui::SExpDesignUI::FreqBandNames[(size_t)sigfile::TBand::_total] = {
	"Delta", "Theta", "Alpha", "Beta", "Gamma",
};

const array<unsigned, 4>
	aghui::SExpDesignUI::FFTPageSizeValues = {{4, 20, 30, 60}};
const array<double, 3>
	aghui::SExpDesignUI::FFTBinSizeValues = {{.1, .25, .5}};



aghui::SExpDesignUI::
SExpDesignUI (aghui::SSessionChooser *parent,
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
	close_this_SF_now (nullptr),
	display_profile_type (sigfile::TMetricType::Psd),
	operating_range_from (2.),
	operating_range_upto (3.),
	uc_accuracy_factor (1.),
	pagesize_item (2),
	binsize_item (1),
	ext_score_codes (
		{{{" -0"}, {"1"}, {"2"}, {"3"}, {"4"}, {"6Rr8"}, {"Ww5"}}}
	),
	freq_bands {
		{  1.5,  4.0 },
		{  4.0,  8.0 },
		{  8.0, 12.0 },
		{ 15.0, 30.0 },
		{ 30.0, 40.0 },
	},
	profile_scale_psd (0.),
	profile_scale_mc (0.),
	autoscale (false),
	smooth_profile (1),
	timeline_height (80),
	timeline_pph (30),
	browse_command ("thunar"),
	config_keys_s ({
		confval::SValidator<string>("WindowGeometry.Main",		&_geometry_placeholder),
		confval::SValidator<string>("Common.CurrentSession",		&_aghdd_placeholder),
		confval::SValidator<string>("Common.CurrentChannel",		&_aghtt_placeholder),
		confval::SValidator<string>("Measurements.BrowseCommand",	&browse_command),
		confval::SValidator<string>("LastUsedVersion",			&last_used_version),
	}),
	config_keys_d ({
		confval::SValidator<int>("Measurements.DisplayProfileMode",	(int*)&display_profile_type,			confval::SValidator<int>::SVFRangeIn ( 0,   1)),
		confval::SValidator<int>("Measurements.SmoothSide",		(int*)&smooth_profile,				confval::SValidator<int>::SVFRangeIn ( 1,  20)),
		confval::SValidator<int>("Measurements.TimelineHeight",		(int*)&timeline_height,				confval::SValidator<int>::SVFRangeIn (10, 600)),
		confval::SValidator<int>("Measurements.TimelinePPH",		(int*)&timeline_pph,				confval::SValidator<int>::SVFRangeIn (10, 600)),
		confval::SValidator<int>("ScoringFacility.IntersignalSpace",	(int*)&SScoringFacility::IntersignalSpace,	confval::SValidator<int>::SVFRangeIn (10, 800)),
		confval::SValidator<int>("ScoringFacility.HypnogramHeight",	(int*)&SScoringFacility::HypnogramHeight,	confval::SValidator<int>::SVFRangeIn (10, 300)),
		confval::SValidator<int>("ModelRun.SWASmoothOver",		(int*)&SModelrunFacility::swa_smoothover,	confval::SValidator<int>::SVFRangeIn ( 1,   5)),
	}),
	config_keys_g ({
		confval::SValidator<double>("UltradianCycleDetectionAccuracy",	&uc_accuracy_factor,			confval::SValidator<double>::SVFRangeIn (0.5, 20.)),
		confval::SValidator<double>("Measurements.ProfileScalePSD",	&profile_scale_psd,			confval::SValidator<double>::SVFRangeIn (0., 1e10)), // can be 0, will trigger autoscale
		confval::SValidator<double>("Measurements.ProfileScaleMC",	&profile_scale_mc,			confval::SValidator<double>::SVFRangeIn (0., 1e10)),
		confval::SValidator<double>("Common.OperatingRangeFrom",	&operating_range_from,			confval::SValidator<double>::SVFRangeIn (0., 20.)),
		confval::SValidator<double>("Common.OperatingRangeUpto",	&operating_range_upto,			confval::SValidator<double>::SVFRangeIn (0., 20.)),
	})
{
	nodestroy_by_cb = true;

	set_wMainWindow_interactive( false);
	gtk_widget_show_all( (GtkWidget*)wMainWindow);

	if ( not dir.empty() and not agh::fs::exists_and_is_writable( dir) )
		throw invalid_argument (string("Experiment directory ") + dir + " does not exist or is not writable");

	if ( dir.empty() ) { // again? only happens when user has moved a previously valid expdir between sessions
		string sure_dir = string (getenv("HOME")) + "/EmptyDummy";
		if ( agh::fs::mkdir_with_parents( sure_dir) != 0 )
			throw invalid_argument ("The last used experiment directory does not exist (or is not writable),"
						" and a dummy fallback directory could not be created in your $HOME."
						" Whatever the reason, this is really too bad: I can't fix it for you.");
	}
	ED = new agh::CExpDesign (dir,
				  bind( &SExpDesignUI::sb_main_progress_indicator, this,
					placeholders::_1, placeholders::_2, placeholders::_3));
	load_artifact_detection_profiles();
	if ( global_artifact_detection_profiles.empty() )
		global_artifact_detection_profiles["default"] = sigfile::SArtifactDetectionPP ();

	nodestroy_by_cb = false;

	// bind fields to widgets
	// tab 1
	W_V1.reg( eSMPMaxThreads, &ED->num_threads);
	W_V1.reg( eArtifDampenWindowType, (int*)&ED->af_dampen_window_type);
	W_V1.reg( eArtifDampenFactor, &ED->af_dampen_factor);
	W_V1.reg( eFFTParamsWindowType, (int*)&ED->fft_params.welch_window_type);
	W_V1.reg( eMCParamIIRBackpolate, &ED->mc_params.iir_backpolate);
	W_V1.reg( eMCParamMCGain, &ED->mc_params.mc_gain);
	W_V1.reg( eMCParamBandWidth, &ED->mc_params.bandwidth);

	W_V1.reg( eFFTParamsPageSize, &pagesize_item);
	W_V1.reg( eFFTParamsBinSize, &binsize_item);
	W_V1.reg( eUltradianCycleDetectionAccuracy, &uc_accuracy_factor);
	for ( size_t i = 0; i < sigfile::SPage::TScore::_total; ++i )
		W_V1.reg( eScoreCode[i], &ext_score_codes[i]);
	for ( size_t i = 0; i < sigfile::TBand::_total; ++i ) {
		W_V1.reg( eBand[i][0], &freq_bands[i][0]);
		W_V1.reg( eBand[i][1], &freq_bands[i][1]);
	}
	W_V1.reg( eDAMsmtPPH, (int*)&timeline_pph);
	W_V1.reg( eDAMsmtTLHeight, (int*)&timeline_height);
	W_V1.reg( eDAPageHeight, (int*)&SScoringFacility::IntersignalSpace);
	W_V1.reg( eDAHypnogramHeight, (int*)&SScoringFacility::HypnogramHeight);
	W_V1.reg( eDAEMGHeight, (int*)&SScoringFacility::EMGProfileHeight);
	W_V1.reg( eBrowseCommand, &browse_command);

	// set _saved, too
	fft_params_welch_window_type_saved	= ED->fft_params.welch_window_type;
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

	W_V2.reg( eCtlParamDBAmendment1, &ED->ctl_params0.DBAmendment1);
	W_V2.reg( eCtlParamDBAmendment2, &ED->ctl_params0.DBAmendment2);
	W_V2.reg( eCtlParamAZAmendment1, &ED->ctl_params0.AZAmendment1);
	W_V2.reg( eCtlParamAZAmendment2, &ED->ctl_params0.AZAmendment2);
	W_V2.reg( eCtlParamNSWAPpBeforeSimStart, (int*)&ED->ctl_params0.swa_laden_pages_before_SWA_0);
	W_V2.reg( eCtlParamReqScoredPercent, &ED->ctl_params0.req_percent_scored);
	W_V2.reg( eCtlParamScoreUnscoredAsWake, &ED->ctl_params0.ScoreUnscoredAsWake);

	// tunables are isolated so they can be reset separately
	for ( size_t t = 0; t < agh::ach::TTunable::_basic_tunables; ++t ) {
		W_Vtunables.reg( eTunable[t][0], &ED->tunables0 [t]);
		W_Vtunables.reg( eTunable[t][1], &ED->tlo       [t]);
		W_Vtunables.reg( eTunable[t][2], &ED->thi       [t]);
		W_Vtunables.reg( eTunable[t][3], &ED->tstep     [t]);
	}

	populate( true);

	set_wMainWindow_interactive( true, false);
}

void
aghui::SExpDesignUI::
load_artifact_detection_profiles()
{
	FILE *domien = fopen( (ED->session_dir() + "/.AD_profiles").c_str(), "r");
	if ( domien ) {
		while ( !feof (domien) ) {
			sigfile::SArtifactDetectionPP P;
			DEF_UNIQUE_CHARP (_);
			int int_estimate_E, int_use_range;
// at least gcc 4.7.2 fails to recognize "%as" (dynamic allocation), so
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic push
			if ( 16 ==
			     fscanf( domien, "%a[^\n]\n%la  %la %la  %la %la %la  %la %la  %la %la %la "
				     "%zu %zu %d %d",
				     &_,
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
				global_artifact_detection_profiles[_] = P;
#pragma GCC diagnostic pop
			} else
				break;
		}
		fclose( domien);
	}
}

void
aghui::SExpDesignUI::
save_artifact_detection_profiles() const
{
	// libconfig::Config conf;
	// auto&	root = conf.getRoot();
	// auto&	profiles = root.add("profiles", libconfig::Setting::Type::TypeArray);
	if ( global_artifact_detection_profiles.size() == 0 )
		return;

	FILE *domien = fopen( (ED->session_dir() + "/.AD_profiles").c_str(), "w");
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
aghui::SExpDesignUI::
figure_pagesize_item()
{
	size_t i = 0;
	while ( FFTPageSizeValues[i] < ED->fft_params.pagesize )
		++i;
	return i;
}
size_t
__attribute__ ((pure))
aghui::SExpDesignUI::
figure_binsize_item()
{
	size_t i = 0;
	while ( FFTPageSizeValues[i] < ED->fft_params.pagesize )
		++i;
	return i;
}



aghui::SExpDesignUI::
~SExpDesignUI ()
{
	save_settings();
	save_artifact_detection_profiles();
	delete ED;
}






void
aghui::SExpDesignUI::
do_rescan_tree( bool with_update)
{
	aghui::SBusyBlock bb (wMainWindow);

	depopulate( false);
	ED -> sync();
	if ( with_update )
		ED -> scan_tree( bind (&SExpDesignUI::sb_main_progress_indicator, this,
				       placeholders::_1, placeholders::_2, placeholders::_3));
	else
		ED -> scan_tree();
	populate( false);

	gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
	gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral,
			    "Scanning complete");
}


void
aghui::SExpDesignUI::
do_purge_computed()
{
	aghui::SBusyBlock bb (wMainWindow);

	snprintf_buf( "find '%s' \\( -name '.*.psd' -or -name '.*.mc' \\) -delete",
		      ED->session_dir().c_str());
	if ( system( __buf__) ) {
		fprintf( stderr, "Command '%s' returned a non-zero status. This is suspicious.\n", __buf__);
		gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
		gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral,
				    "Failed to purge cache files. This is odd.");
	}

	gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
	gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral,
			    "Purged computed files cache");
}


void
aghui::SExpDesignUI::
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
	agh::beersma::ultradian_cycles(
		M,
		{ display_profile_type,
		  operating_range_from, operating_range_upto,
		  .1, siman_params});
}




void
aghui::SExpDesignUI::
adjust_op_freq_spinbuttons()
{
	suppress_redraw = true;

	switch ( display_profile_type ) {
	case sigfile::Psd:
		gtk_adjustment_set_step_increment( jMsmtOpFreqFrom,  ED->fft_params.binsize);
		gtk_adjustment_set_step_increment( jMsmtOpFreqWidth, ED->fft_params.binsize);
		if ( not used_eeg_samplerates.empty() )
			gtk_adjustment_set_upper(
				jMsmtOpFreqFrom,
				ED->fft_params.binsize * (ED->fft_params.compute_n_bins( used_eeg_samplerates.back()) - 1));

		gtk_widget_set_sensitive( (GtkWidget*)eMsmtOpFreqWidth, TRUE);
	    break;
	case sigfile::Mc:
		gtk_adjustment_set_step_increment( jMsmtOpFreqFrom, ED->mc_params.bandwidth);
		gtk_spin_button_set_value( eMsmtOpFreqWidth, ED->mc_params.bandwidth);
		if ( not used_eeg_samplerates.empty() )
			gtk_adjustment_set_upper(
				jMsmtOpFreqFrom,
				ED->mc_params.freq_from
				+ ED->mc_params.bandwidth * (ED->mc_params.compute_n_bins( used_eeg_samplerates.back()) - 1));

		gtk_widget_set_sensitive( (GtkWidget*)eMsmtOpFreqWidth, FALSE);
	    break;
	case sigfile::invalid:
	    break;
	}

	suppress_redraw = false;
}



void
aghui::SExpDesignUI::
calculate_profile_scale()
{
	double	avg_profile_height = 0.;
	size_t	valid_episodes = 0;
	for ( auto& G : groups )
		for ( auto &J : G )
			if ( J.cscourse && !J.cscourse->mm_list().empty() ) {
				avg_profile_height += J.cscourse->metric_avg();
				++valid_episodes;
			}
	avg_profile_height /= valid_episodes;

	switch ( display_profile_type ) {
	case sigfile::TMetricType::Psd:
		profile_scale_psd = timeline_height / avg_profile_height * .3;
	    break;
	case sigfile::TMetricType::Mc:
		profile_scale_mc = timeline_height / avg_profile_height * .3;
	    break;
	case sigfile::invalid:
	    break;
	}
}



void
aghui::SExpDesignUI::
show_changelog()
{
	gtk_widget_show_all( (GtkWidget*)wAbout);
	gtk_notebook_set_current_page( cAboutTabs, 2);
}



void
aghui::SExpDesignUI::
show_empty_experiment_blurb()
{
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	const char *blurb =
		"<b><big>Empty experiment\n</big></b>\n"
		"When you have your recordings ready as a set of .edf files,\n"
		"• Create your experiment tree as follows: <i>Experiment/Group/Subject/Session</i>;\n"
		"• Have your EDF sources named <i>Episode</i>.edf, and placed in the corresponding <i>Session</i> directory, or\n"
		"• Drag-and-Drop any EDF sources onto this window and identify and place them individually.\n\n"
		"Once set up, either:\n"
		"• select <b>Experiment→Change</b> and select the top directory of the (newly created) experiment tree, or\n"
		"• select <b>Experiment→Rescan Tree</b> if this is the tree you have just populated.\n"
		"\n"
		"Or, If you have none yet, here is a <a href=\"http://johnhommer.com/academic/code/aghermann/Experiment.tar.bz2\">set of EEG data</a>, for a primer;"
		" push the button below to download it into the current directory:";
	GtkLabel *blurb_label = (GtkLabel*)gtk_label_new( "");
	gtk_label_set_markup( blurb_label, blurb);

	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)blurb_label,
			    TRUE, TRUE, 0);
	GtkWidget *bDownload = gtk_button_new_with_label("  Download  ");
	g_object_set( (GObject*)bDownload,
		      "expand", FALSE,
		      "halign", GTK_ALIGN_CENTER,
		      NULL);
	g_signal_connect( bDownload, "clicked",
			  (GCallback)bDownload_clicked_cb,
			  this);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    bDownload,
			    FALSE, FALSE, 0);

	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)gtk_image_new_from_file(
					    PACKAGE_DATADIR "/" PACKAGE "/idle-bg.svg"),
			    TRUE, FALSE, 0);

	gtk_widget_show_all( (GtkWidget*)cMeasurements);
}


extern "C" void
bDownload_clicked_cb( GtkButton* button, gpointer userdata)
{
	auto EDp = (SExpDesignUI*)userdata;
	EDp->try_download();
}

int
aghui::SExpDesignUI::
try_download()
{
	const char
		*url = "http://johnhommer.com/academic/code/aghermann/Experiment.tar.bz2",
		*archive_file = "Experiment.tar.bz2";
	snprintf_buf( "xterm -e sh -c "
		      "'cd \"%s\" && "
		      " wget -c \"%s\" && "
		      " tar xjf \"%s\" && "
		      " rm -f \"%s\" && "
		      " echo \"Sample data set downloaded and unpacked\" && "
		      " read -p \"Press <Enter> to close this window...\"'",
		      ED->session_dir().c_str(), url, archive_file, archive_file);
	aghui::SBusyBlock bb (wMainWindow);

	if ( system( __buf__) ) {
		gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
		gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral,
				    "Download failed for some reason");
	}
	do_rescan_tree( true);
	populate( true);
	// gtk_container_foreach( (GtkContainer*)cMeasurements,
	// 		       (GtkCallback) gtk_widget_destroy,
	// 		       NULL);
	// GtkWidget *tTerm = vte_terminal_new();
	// gtk_box_pack_start( (GtkBox*)cMeasurements,
	// 		    tTerm,
	// 		    TRUE, FALSE, 0);
	// GPid download_process_pid;
	// char *argv[] = {
	// 	"ls",
	// 	".",
	// };
	// vte_terminal_fork_command_full(
	// 	(VteTerminal*)tTerm,
	// 	VTE_PTY_DEFAULT,
	// 	ED->session_dir(),
	// 	argv,
	// 	NULL, // char **envv,
	// 	(GSpawnFlags)0, // GSpawnFlags spawn_flags,
	// 	NULL, // GSpawnChildSetupFunc child_setup,
	// 	NULL, // gpointer child_setup_data,
	// 	&download_process_pid,
	// 	NULL); // GError **error);
	return 0;
}


void
aghui::SExpDesignUI::
buf_on_main_status_bar()
{
	gtk_statusbar_pop( sbMainStatusBar, sbMainContextIdGeneral);
	gtk_statusbar_push( sbMainStatusBar, sbMainContextIdGeneral, __buf__);
}

void
aghui::SExpDesignUI::
sb_main_progress_indicator( const char* current, size_t n, size_t i)
{
	snprintf_buf( "(%zu of %zu) %s", i, n, current);
	buf_on_main_status_bar();
	gdk_window_process_updates(
		gtk_widget_get_parent_window(
			(GtkWidget*)sbMainStatusBar),
		FALSE);
	//gtk_flush();
}




void
aghui::SExpDesignUI::
update_subject_details_interactively( agh::CSubject& J)
{
	gtk_entry_set_text( eSubjectDetailsName, J.full_name.c_str());
	gtk_spin_button_set_value( eSubjectDetailsAge, J.age);
	gtk_toggle_button_set_active( (J.gender == agh::CSubject::TGender::male)
				      ? (GtkToggleButton*)eSubjectDetailsGenderMale
				      : (GtkToggleButton*)eSubjectDetailsGenderFemale,
				      TRUE);
	gtk_entry_set_text( eSubjectDetailsComment, J.comment.c_str());

	if ( gtk_dialog_run( (GtkDialog*)wSubjectDetails) == -5 ) {
		J.full_name.assign( gtk_entry_get_text( eSubjectDetailsName));
		J.age = gtk_spin_button_get_value( eSubjectDetailsAge);
		J.gender =
			gtk_toggle_button_get_active( (GtkToggleButton*)eSubjectDetailsGenderMale)
			? agh::CSubject::TGender::male
			: agh::CSubject::TGender::female;
		J.comment.assign( gtk_entry_get_text( eSubjectDetailsComment));
	}
}


// eof
