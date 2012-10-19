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
iExpClose_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_window_get_position( ED.wMainWindow, &ED.geometry.x, &ED.geometry.y);
	gtk_window_get_size( ED.wMainWindow, &ED.geometry.w, &ED.geometry.h);

	g_signal_emit_by_name( ED._p->bSessionChooserClose, "clicked");
	// gtk_widget_show( (GtkWidget*)ED.wExpDesignChooser);
	// gtk_widget_hide( (GtkWidget*)ED.wMainWindow);
	// gtk_widget_hide( (GtkWidget*)ED.wGlobalAnnotations);
	// gtk_widget_hide( (GtkWidget*)ED.wEDFFileDetails);
	// gtk_widget_hide( (GtkWidget*)ED.wScanLog);
	// // if ( gtk_widget_get_visible( (GtkWidget*)wScoringFacility) )
	// // 	gtk_widget_hide( (GtkWidget*)wScoringFacility);
	// // better make sure bExpChange is greyed out on opening any child windows
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

	function<bool( agh::CSubject::SEpisode&)> filter =
		[&ED]( agh::CSubject::SEpisode& E) -> bool
	{
		return E.recordings.find( ED.AghH()) != E.recordings.end();
	};
	function<void( agh::CSubject::SEpisode&)> F =
		[&ED]( agh::CSubject::SEpisode& E)
	{
		ED.do_detect_ultradian_cycle( E.recordings.at( ED.AghH()));
	};
	function<void( const agh::CJGroup&, const agh::CSubject&, const string&, const agh::CSubject::SEpisode&,
		       size_t, size_t)> reporter =
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

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( ED.wGlobalArtifactDetection) )
		FAFA;
}


void
iExpQuit_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	g_signal_emit_by_name( ED._p->bSessionChooserQuit, "clicked");
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
iMontageResetAll_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	snprintf_buf( "find '%s' -name '.*.montage' -delete",
		      ED.ED->session_dir().c_str());
	if ( system( __buf__) )
		pop_ok_message( ED.wMainWindow, "Command '%s' returned a non-zero status. This is weird.", __buf__);
}


inline namespace {
void
set_all_filters( agh::CExpDesign& ED, sigfile::SFilterPack::TNotchFilter value)
{
	for ( auto &G : ED.groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources )
						for ( auto &H : F.channel_list() )
							F.filters(H.c_str()).notch_filter = value;
	ED.sync();
}
} // namespace

void
iMontageNotchNone_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	set_all_filters( *ED.ED, sigfile::SFilterPack::TNotchFilter::none);
}

void
iMontageNotch50Hz_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	set_all_filters( *ED.ED, sigfile::SFilterPack::TNotchFilter::at50Hz);
}

void
iMontageNotch60Hz_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	set_all_filters( *ED.ED, sigfile::SFilterPack::TNotchFilter::at60Hz);
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
