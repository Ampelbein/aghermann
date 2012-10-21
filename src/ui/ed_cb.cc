// ;-*-C++-*-
/*
 *       File name:  ui/ed_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI widget callbacks
 *
 *         License:  GPL
 */

#include "misc.hh"
#include "sc.hh"
#include "ed.hh"
#include "ed_cb.hh"
#include "sf.hh"


using namespace aghui;

extern "C" {

gboolean
wMainWindow_configure_event_cb( GtkWidget*, GdkEventConfigure *event, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( event->type == GDK_CONFIGURE )
		ED.geometry = {
			event -> x,
			event -> y,
			event -> width,
			event -> height
		};
	return FALSE; // whatever
}

gboolean
wMainWindow_delete_event_cb( GtkWidget*, GdkEvent*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( ED.nodestroy_by_cb )
		return TRUE;

	// if the user closes the main window while in a settings tab,
	// ensure we switch away from it and trigger any saving callbacks
	gtk_notebook_set_current_page( ED.tDesign, 0);
	gtk_notebook_set_current_page( ED.tSimulations, 0);

	iExpClose_activate_cb( NULL, userdata);

	return TRUE; // whatever
}


// tab switch
void
tTaskSelector_switch_page_cb( GtkNotebook*, gpointer, guint page_num, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( page_num == 1 ) {
		//ED.populate( false);
		snprintf_buf( "Session: <b>%s</b>", ED.AghD());
		gtk_label_set_markup( ED.lSimulationsSession, __buf__);
		snprintf_buf( "Channel: <b>%s</b>", ED.AghT());
		gtk_label_set_markup( ED.lSimulationsChannel, __buf__);
		snprintf_buf( "Metric: <b>%s</b>", sigfile::metric_method(ED.display_profile_type));
		gtk_label_set_markup( ED.lSimulationsProfile, __buf__);
		gtk_widget_set_sensitive( (GtkWidget*)ED.iExpClose, FALSE);
		ED.populate_2();
	} else if ( page_num == 0 ) {
		// ED.ED->remove_untried_modruns(); // done in populate_2
		// ED.populate( false);
		gtk_widget_set_sensitive( (GtkWidget*)ED.iExpClose, TRUE);
	}
}



void
iExpRefresh_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.do_rescan_tree( true);
}

void
iExpPurgeComputed_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.do_purge_computed();
}




void
iExpAnnotations_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_dialog_run( ED.wGlobalAnnotations);
}


void
iExpBasicSADetectUltradianCycles_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	aghui::SBusyBlock bb (ED.wMainWindow);

	using namespace agh;
	CExpDesign::TEpisodeFilterFun filter =
		[&ED]( agh::CSubject::SEpisode& E) -> bool
	{
		return E.recordings.find( ED.AghH()) != E.recordings.end();
	};
	CExpDesign::TEpisodeOpFun F =
		[&ED]( agh::CSubject::SEpisode& E)
	{
		ED.do_detect_ultradian_cycle( E.recordings.at( ED.AghH()));
	};
	CExpDesign::TEpisodeReportFun reporter =
		[&ED]( const agh::CJGroup&, const agh::CSubject& J, const string&, const agh::CSubject::SEpisode& E,
		       size_t i, size_t n)
	{
		snprintf_buf(
			"(%zu of %zu) %s/%s/%s", i, n,
			ED.ED->group_of(J), J.name(), E.name());
		ED.buf_on_main_status_bar();
		gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
		gdk_window_process_updates(
			gtk_widget_get_parent_window( (GtkWidget*)ED.cMeasurements),
			TRUE);
	};

	ED.ED->for_all_episodes( F, reporter, filter);
}

void
iExpGloballyDetectArtifacts_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	auto response = gtk_dialog_run( ED.wGlobalArtifactDetection);
	if ( response == GTK_RESPONSE_CANCEL ||
	     response == GTK_RESPONSE_DELETE_EVENT )
		return; // just to save on indents in those lambdas below

	auto& P = ED.global_artifact_detection_profiles[
		gtk_combo_box_get_active_id(ED.eGlobalADProfiles)];
	bool keep_existing = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eGlobalADKeepExisting);

	SBusyBlock bb (ED.wMainWindow);

	using namespace agh;
	CExpDesign::TRecordingOpFun F;
	CExpDesign::TRecordingFilterFun filter;
	CExpDesign::TRecordingReportFun G =
		[&]( const CJGroup&, const CSubject& J, const string& D, const CSubject::SEpisode& E, const CRecording& R,
		     size_t i, size_t total)
		{
			snprintf_buf(
				"(%zu of %zu) Detect artifacts in %s/%s/%s/%s:%s", i, total,
				ED.ED->group_of(J), J.name(), D.c_str(), E.name(), R.F().channel_by_id(R.h()));
			ED.buf_on_main_status_bar();
			gtk_flush();
		};
	switch ( response ) {
	case GTK_RESPONSE_OK:
		F =
		[&]( CRecording& R)
		{
			auto	sr = R.F().samplerate(R.h());
			auto&	af = R.F().artifacts(R.h());

			auto	signal_original
				= R.F().get_signal_original(R.h());

			if ( not keep_existing )
				af.clear_all();
			auto	marked
				= sigfile::detect_artifacts( signal_original, sr, P);
			for ( size_t p = 0; p < marked.size(); ++p )
				af.mark_artifact(
					marked[p] * P.scope * sr,
					(marked[p]+1) * P.scope * sr);
		};
		filter =
		[&]( CRecording& R)
		{
			return R.signal_type() == sigfile::SChannel::TType::eeg;
		};
	    break;
	case 1:
		F =
		[&]( CRecording& R)
		{
			auto& F = R.F();
			for ( auto& H : R.F().channel_list() ) {
				auto&	af = F.artifacts(H.c_str());
				af.clear_all();
			}
		};
		filter =
		[&]( CRecording&)
		{
			return true;
		};
	    break;
	default:
		throw runtime_error ("Fix AD dialog response?");
	}

	forward_list<aghui::SBusyBlock*> bbl;
	for ( auto& SFp : ED.open_scoring_facilities )
		bbl.push_front( new aghui::SBusyBlock (SFp->wScoringFacility));

	ED.ED -> for_all_recordings( F, G, filter);

	for ( auto& SF : ED.open_scoring_facilities ) {
		for ( auto& H : SF->channels )
			if ( H.type == sigfile::SChannel::TType::eeg )
				H.get_signal_filtered();
		SF->queue_redraw_all();
	}

	ED.populate_1();

	for ( auto& bb : bbl )
		delete bb;
}

void
eGlobalADProfiles_changed_cb( GtkComboBox *b, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	gtk_widget_set_sensitive(
		(GtkWidget*)ED.bGlobalADOK,
		ED.global_artifact_detection_profiles.size() > 0);
}



void
iMontageSetDefaults_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( ED.wMontageDefaults) )
		FAFA;
}



void
iExpGloballySetFilters_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	int LPO, HPO, NF;
	double LPC, HPC;
	aghui::SUIVarCollection W_V;
	W_V.reg( ED.eGlobalFiltersLowPassCutoff, &LPC);
	W_V.reg( ED.eGlobalFiltersLowPassOrder, &LPO);
	W_V.reg( ED.eGlobalFiltersHighPassCutoff, &HPC);
	W_V.reg( ED.eGlobalFiltersHighPassOrder, &HPO);
	W_V.reg( ED.eGlobalFiltersNotchFilter, &NF);

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( ED.wGlobalFilters) ) {
		FAFA;
		forward_list<aghui::SBusyBlock*> bbl;
		for ( auto& SFp : ED.open_scoring_facilities )
			bbl.push_front( new aghui::SBusyBlock (SFp->wScoringFacility));
		FAFA;
		W_V.down();
		FAFA;
		for ( auto &G : ED.ED->groups )
			for ( auto &J : G.second )
				for ( auto &D : J.measurements )
					for ( auto &E : D.second.episodes )
						for ( auto &F : E.sources )
							for ( auto &H : F.channel_list() ) {
								auto& ff = F.filters(H.c_str());
								ff.low_pass_cutoff = LPC;
								ff.low_pass_order = LPO;
								ff.high_pass_cutoff = HPC;
								ff.high_pass_order = HPO;
								ff.notch_filter = (sigfile::SFilterPack::TNotchFilter)NF;
							}
		ED.ED->sync();

		for ( auto& SF : ED.open_scoring_facilities ) {
			for ( auto& H : SF->channels )
				if ( H.type == sigfile::SChannel::TType::eeg )
					H.get_signal_filtered();
			SF->queue_redraw_all();
		}
		ED.populate_1();
		for ( auto& bb : bbl )
			delete bb;
	}
}

void
bGlobalMontageResetAll_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	snprintf_buf( "find '%s' -name '.*.montage' -delete",
		      ED.ED->session_dir().c_str());
	if ( system( __buf__) )
		pop_ok_message( ED.wMainWindow, "Command '%s' returned a non-zero status. This is weird.", __buf__);
}



void
iHelpAbout_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_widget_show_all( (GtkWidget*)ED.wAbout);
}

void
iHelpUsage_activate_cb( GtkMenuItem*, gpointer)
{
//	auto& ED = *(SExpDesignUI*)userdata;
	gtk_show_uri( NULL,
		      "http://johnhommer.com/academic/code/aghermann/usage/",
		      GDK_CURRENT_TIME, NULL);
}


inline namespace {

void
before_ED_close( SExpDesignUI& ED)
{
	gtk_window_get_position( ED.wMainWindow, &ED.geometry.x, &ED.geometry.y);
	gtk_window_get_size( ED.wMainWindow, &ED.geometry.w, &ED.geometry.h);
}

} // inline namespace

void
iExpClose_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	before_ED_close( ED);
	g_signal_emit_by_name( ED._p->bSessionChooserClose, "clicked");
}

void
iExpQuit_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	before_ED_close( ED);
	g_signal_emit_by_name( ED._p->bSessionChooserQuit, "clicked");
}





// other main toolbar controls

void
eMsmtProfileAutoscale_toggled_cb( GtkToggleButton* b, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( (ED.autoscale = (bool)gtk_toggle_button_get_active(b)) ) {
		ED.calculate_profile_scale();
		gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
	}
}

void
eMsmtProfileSmooth_value_changed_cb( GtkScaleButton* b, gdouble v, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.smooth_profile = v;
	snprintf_buf( "Smooth: %zu", ED.smooth_profile);
	gtk_button_set_label( (GtkButton*)b, __buf__);
	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}


void
eMsmtProfileType_changed_cb( GtkComboBox* b, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	switch ( gtk_combo_box_get_active( b) ) {
	case 0:
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParams1, TRUE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParams2, FALSE);
		ED.display_profile_type = sigfile::TMetricType::Psd;
	    break;
	case 1:
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParams1, FALSE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParams2, TRUE);
		ED.display_profile_type = sigfile::TMetricType::Mc;
	    break;
	}

	agh::SSCourseParamSet params {
		ED.display_profile_type,
		ED.operating_range_from, ED.operating_range_upto,
		0., 0, false
	};
	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cscourse )
				J.cscourse->create_timeline( params);

	if ( ED.profile_scale_psd == 0. || ED.profile_scale_mc == 0. ||  // don't know which
		ED.autoscale )
		ED.calculate_profile_scale();

	ED.adjust_op_freq_spinbuttons();

	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}





void
eMsmtOpFreqFrom_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.operating_range_from = gtk_spin_button_get_value( spinbutton);
	ED.operating_range_upto = ED.operating_range_from + gtk_spin_button_get_value( ED.eMsmtOpFreqWidth);
	if ( ED.suppress_redraw )
		return;

	agh::SSCourseParamSet params {
		ED.display_profile_type,
		ED.operating_range_from, ED.operating_range_upto,
		0., 0, false
	};
	params._freq_from = ED.operating_range_from;
	params._freq_upto = ED.operating_range_upto;
	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cscourse )
				J.cscourse->create_timeline( params);
	if ( ED.autoscale )
		ED.calculate_profile_scale();

	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}

void
eMsmtOpFreqWidth_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.operating_range_upto = ED.operating_range_from + gtk_spin_button_get_value( spinbutton);
	if ( ED.suppress_redraw )
		return;

	agh::SSCourseParamSet params {
		ED.display_profile_type,
		ED.operating_range_from, ED.operating_range_upto,
		0., 0, false
	};
	params._freq_from = ED.operating_range_from;
	params._freq_upto = ED.operating_range_upto;
	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cscourse )
				J.cscourse->create_timeline( params);
	if ( ED.autoscale )
		ED.calculate_profile_scale();
	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}





void
eMsmtSession_changed_cb( GtkComboBox *combobox, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto oldval = ED._AghDi;
	ED._AghDi = find( ED.AghDD.begin(), ED.AghDD.end(),
			  gtk_combo_box_get_active_id( combobox));
	if ( oldval != ED._AghDi )
		ED.populate_1();
	if ( ED.autoscale )
		ED.calculate_profile_scale();
}

void
eMsmtChannel_changed_cb( GtkComboBox *combobox, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto oldval = ED._AghTi;
	ED._AghTi = find( ED.AghTT.begin(), ED.AghTT.end(),
		       gtk_combo_box_get_active_id( combobox));
	if ( /* _AghTi != AghTT.end() && */ oldval != ED._AghTi )
		ED.populate_1();
	if ( ED.autoscale )
		ED.calculate_profile_scale();
}



// annotations dialog

void
tvGlobalAnnotations_row_activated_cb( GtkTreeView* tree_view,
				      GtkTreePath* path,
				      GtkTreeViewColumn *column,
				      gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	aghui::SExpDesignUI::SAnnotation *ann;
	GtkTreeIter iter;
	gtk_tree_model_get_iter( (GtkTreeModel*)ED.mGlobalAnnotations, &iter, path);
	gtk_tree_model_get( (GtkTreeModel*)ED.mGlobalAnnotations, &iter,
			    ED.mannotations_ref_col, &ann,
			    -1);
	if ( ann == nullptr )
		return;

	gtk_widget_hide( (GtkWidget*)ED.wGlobalAnnotations);
	aghui::SScoringFacility* found = nullptr;
	for ( auto &F : ED.open_scoring_facilities )
		if ( &F->csubject() == &ann->csubject
		     && F->session() == ann->session
		     && &F->sepisode() == &ann->sepisode ) {
			found = F;
			break;
		}
	if ( found ) {
		auto pages = ann->page_span( found->vpagesize());
		gtk_widget_show( (GtkWidget*)found->wScoringFacility);
		found->set_cur_vpage( pages.a, true);
	} else {
		ED.using_subject = ED.subject_presentation_by_csubject( ann->csubject);
		auto SF = new aghui::SScoringFacility( ann->csubject, ann->session, ann->sepisode.name(), ED);
		auto pages = ann->page_span( SF->vpagesize());
		SF->set_cur_vpage( pages.a, true);
	}
}




void
bMainCloseThatSF_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	if ( ED.close_this_SF_now == nullptr ) {
		fprintf( stderr, "Hey, keep your fingers off this button!\n");
		return;
	}

	delete ED.close_this_SF_now;
	ED.close_this_SF_now = nullptr;
}




// -------- colours
void
bColourX_color_set_cb( GtkColorButton *widget,
		       gpointer        userdata)
{
	auto& mc = *(SManagedColor*)userdata;
	mc.acquire();
}

} // extern "C"

// eof
