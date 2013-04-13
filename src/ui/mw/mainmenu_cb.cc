/*
 *       File name:  ui/mw/mainmenu_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-11-12
 *
 *         Purpose:  SExpDesignUI widget callbacks (main menu items)
 *
 *         License:  GPL
 */

#include "ui/misc.hh"
#include "mw.hh"
#include "mw_cb.hh"
#include "ui/sm/sm.hh"
#include "ui/sf/sf.hh"

using namespace aghui;

extern "C" {

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
iExpSubjectSortAny_toggled_cb( GtkCheckMenuItem* mi, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( ED.suppress_redraw )
		return;

	// only set ON
	if ( gtk_check_menu_item_get_active( mi) == FALSE )
		return;

	if      ( mi == (GtkCheckMenuItem*)ED.iExpSubjectSortName )
		ED.sort_by = SExpDesignUI::TSubjectSortBy::name;
	else if ( mi == (GtkCheckMenuItem*)ED.iExpSubjectSortAge )
		ED.sort_by = SExpDesignUI::TSubjectSortBy::age;
	else if ( mi == (GtkCheckMenuItem*)ED.iExpSubjectSortAdmissionDate )
		ED.sort_by = SExpDesignUI::TSubjectSortBy::admission_date;
	else if ( mi == (GtkCheckMenuItem*)ED.iExpSubjectSortAvgPower )
		ED.sort_by = SExpDesignUI::TSubjectSortBy::avg_profile_power;

	ED.populate_1();
}


void
iExpSubjectSortAscending_toggled_cb( GtkCheckMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( ED.suppress_redraw )
		return;

	ED.sort_ascending = !ED.sort_ascending;
	ED.populate_1();
}

void
iExpSubjectSortSegregate_toggled_cb( GtkCheckMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( ED.suppress_redraw )
		return;

	ED.sort_segregate = !ED.sort_segregate;
	ED.populate_1();
}






void
iExpAnnotations_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.suppress_redraw = true;
	gtk_toggle_button_set_active( (GtkToggleButton*)ED.eGlobalAnnotationsShowPhasicEvents,  ED.only_plain_global_annotations);
	gtk_toggle_button_set_active( (GtkToggleButton*)ED.eGlobalAnnotationsShowPhasicEvents, !ED.only_plain_global_annotations);
	ED.suppress_redraw = false;
	gtk_dialog_run( ED.wGlobalAnnotations);
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
		gtk_widget_show( (GtkWidget*)found->wSF);
		found->set_cur_vpage( pages.a, true);
	} else {
		ED.using_subject = ED.subject_presentation_by_csubject( ann->csubject);
		auto SF = new aghui::SScoringFacility( ann->csubject, ann->session, ann->sepisode.name(), ED);
		auto pages = ann->page_span( SF->vpagesize());
		SF->set_cur_vpage( pages.a, true);
	}
}



void
eGlobalAnnotationsShowPhasicEvents_toggled_cb( GtkToggleButton* b, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( ED.suppress_redraw )
		return;
	ED.only_plain_global_annotations = not gtk_toggle_button_get_active( b);
	ED.populate_mGlobalAnnotations();
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
			"Detect ultradian cycle %s/%s/%s",
			ED.ED->group_of(J), J.id.c_str(), E.name());
		ED.sb_main_progress_indicator( __buf__, n, i, TGtkRefreshMode::gtk);
		gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
	};

	ED.ED->for_all_episodes( F, reporter, filter);

	ED.sb_clear();
}






void
iExpGloballyDetectArtifacts_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	if ( ED.global_artifact_detection_profiles.size() < 1 ) {
		pop_ok_message( ED.wMainWindow,
				"Create some profiles first",
				"You can do it by opening a recording in Scoring Facility and"
				" tweaking default parameters in Artifact Detect dialog."
				" After saving them as a profile, it will appear here.");
		return;
	}

	gtk_label_set_markup(
		ED.lGlobalADHint,
		(ED.global_artifact_detection_profiles.size() < 2)
		? "<small>You can create a custom profile in Scoring Facility,\n"
		  "after tuning parameters on a real recording.</small>"
		: ""); // good boy


	auto response = gtk_dialog_run( ED.wGlobalArtifactDetection);
	if ( response == GTK_RESPONSE_CANCEL ||
	     response == GTK_RESPONSE_DELETE_EVENT )
		return; // just to save on indents in those lambdas below

	auto& P = ED.global_artifact_detection_profiles[
		gtk_combo_box_get_active_id(ED.eGlobalADProfiles)];
	bool keep_existing = gtk_toggle_button_get_active( (GtkToggleButton*)ED.eGlobalADKeepExisting);

	SBusyBlock bb (ED.wMainWindow);

	using namespace agh;
	CExpDesign::TRecordingOpFun op;
	CExpDesign::TRecordingFilterFun filter;
	CExpDesign::TRecordingReportFun reporter =
		[&]( const CJGroup&, const CSubject& J, const string& D, const CSubject::SEpisode& E, const CRecording& R,
		     size_t i, size_t total)
		{
			snprintf_buf(
				"Detect artifacts in %s/%s/%s/%s:%s",
				ED.ED->group_of(J), J.id.c_str(), D.c_str(), E.name(), R.F().channel_by_id(R.h()));
			ED.sb_main_progress_indicator( __buf__, total, i, TGtkRefreshMode::gtk);
		};
	switch ( response ) {
	case GTK_RESPONSE_OK:
		op =
		[&]( CRecording& R)
		{
			auto	sr = R.F().samplerate(R.h());
			auto&	af = R.F().artifacts(R.h());

			auto	signal_original = R.F().get_signal_original(R.h());

			if ( not keep_existing )
				af.clear_all();
			auto	marked = metrics::mc::detect_artifacts( signal_original, sr, P);
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
	case 1: // "Clear All"
		op =
		[&]( CRecording& R)
		{
			auto& F = R.F();
			for ( auto& H : F.channel_list() ) {
				auto&	af = F.artifacts(H.c_str());
				af.clear_all();
			}
		};
		filter =
		[&]( CRecording&)
		{
			return true; // clear in all channels (mark in EEG channels only)
		};
	    break;
	default:
		throw runtime_error ("Fix AD dialog response?");
	}

	forward_list<aghui::SBusyBlock*> bbl;
	for ( auto& SFp : ED.open_scoring_facilities )
		bbl.push_front( new aghui::SBusyBlock (SFp->wSF));

	ED.ED -> for_all_recordings( op, reporter, filter);
	ED.sb_clear();

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
		forward_list<aghui::SBusyBlock*> bbl;
		for ( auto& SFp : ED.open_scoring_facilities )
			bbl.push_front( new aghui::SBusyBlock (SFp->wSF));
		W_V.down();
		for ( auto &G : ED.ED->groups )
			for ( auto &J : G.second )
				for ( auto &D : J.measurements )
					for ( auto &E : D.second.episodes )
						for ( auto &F : E.sources )
							for ( auto &H : F().channel_list() ) {
								auto& ff = F().filters(H.c_str());
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
		pop_ok_message( ED.wMainWindow, "How strange!", "Command '%s' returned a non-zero status. This is weird.", __buf__);
}



void
iHelpAbout_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	// auto w = gtk_widget_get_window( (GtkWidget*)ED.lAboutVersion);
	// gdk_window_set_composited( w, TRUE);
	// gdk_window_set_opacity( w, .7);
	gtk_widget_show_all( (GtkWidget*)ED.wAbout);
}

void
iHelpUsage_activate_cb( GtkMenuItem*, gpointer)
{
	gtk_show_uri( NULL,
		      "http://johnhommer.com/academic/code/aghermann/usage/",
		      GDK_CURRENT_TIME, NULL);
}


namespace {

void
before_ED_close( SExpDesignUI& ED)
{
	gtk_window_get_position( ED.wMainWindow, &ED.geometry.x, &ED.geometry.y);
	gtk_window_get_size( ED.wMainWindow, &ED.geometry.w, &ED.geometry.h);
}

} // namespace

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



} // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
