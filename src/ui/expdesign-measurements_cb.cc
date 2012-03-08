// ;-*-C++-*-
/*
 *       File name:  ui/expdesign-measurements_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI measurements view widget callbacks
 *
 *         License:  GPL
 */

#include "misc.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"

using namespace aghui;

extern "C" {

void
iExpChange_activate_cb( GtkMenuItem *item, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_window_get_position( ED.wMainWindow, &ED.geometry.x, &ED.geometry.y);
	gtk_window_get_size( ED.wMainWindow, &ED.geometry.w, &ED.geometry.h);

	gtk_widget_show( (GtkWidget*)ED.wExpDesignChooser);
	gtk_widget_hide( (GtkWidget*)ED.wMainWindow);
	// if ( gtk_widget_get_visible( (GtkWidget*)wScoringFacility) )
	// 	gtk_widget_hide( (GtkWidget*)wScoringFacility);
	// better make sure bExpChange is greyed out on opening any child windows
}


void
iExpRefresh_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.do_rescan_tree( false);
}




void
iExpAnnotations_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( gtk_dialog_run( ED.wGlobalAnnotations) == -1 )
		;
}


void
iExpQuit_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.shutdown();
}


void
iMontageResetAll_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	snprintf_buf( "find '%s' -name '.*.montage' -delete",
		      ED.ED->session_dir());
	if ( system( __buf__) )
		;
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






// other main toolbar controls

void
eMsmtProfileAutoscale_toggled_cb( GtkToggleButton* b, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( (ED.autoscale = (bool)gtk_toggle_button_get_active(b)) ) {
		ED.calculate_ppuv2();
		gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
	}
}


void
eMsmtProfileType_changed_cb( GtkComboBox* b, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	switch ( gtk_combo_box_get_active( b) ) {
	case 0:
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParams1, TRUE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParams2, FALSE);
		ED.display_profile_type = sigfile::TProfileType::psd;
	    break;
	case 1:
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParams1, FALSE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParams2, TRUE);
		ED.display_profile_type = sigfile::TProfileType::ucont;
	    break;
	}

	agh::SSCourseParamSet params {
		ED.display_profile_type,
		ED.operating_range_from, ED.operating_range_upto,
		0., 0, false, false
	};
	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cscourse )
				J.cscourse->create_timeline( params);
	// always recalculate
	ED.calculate_ppuv2();

	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}


void
eMsmtPSDFreqFrom_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.operating_range_from = gtk_spin_button_get_value( spinbutton);
	ED.operating_range_upto = ED.operating_range_from + gtk_spin_button_get_value( ED.eMsmtPSDFreqWidth);

	agh::SSCourseParamSet params {
		ED.display_profile_type,
		ED.operating_range_from, ED.operating_range_upto,
		0., 0, false, false
	};
	params._freq_from = ED.operating_range_from;
	params._freq_upto = ED.operating_range_upto;
	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cscourse )
				J.cscourse->create_timeline( params);
	if ( ED.autoscale )
		ED.calculate_ppuv2();
	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}

void
eMsmtPSDFreqWidth_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.operating_range_upto = ED.operating_range_from + gtk_spin_button_get_value( spinbutton);

	agh::SSCourseParamSet params {
		ED.display_profile_type,
		ED.operating_range_from, ED.operating_range_upto,
		0., 0, false, false
	};
	params._freq_from = ED.operating_range_from;
	params._freq_upto = ED.operating_range_upto;
	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cscourse )
				J.cscourse->create_timeline( params);
	if ( ED.autoscale )
		ED.calculate_ppuv2();
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
		ED.calculate_ppuv2();
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
		ED.calculate_ppuv2();
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
	if ( ann == NULL )
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
		found->set_cur_vpage( pages.first);
	} else {
		ED.using_subject = ED.subject_presentation_by_csubject( ann->csubject);
		auto SF = new aghui::SScoringFacility( ann->csubject, ann->session, ann->sepisode.name(), ED);
		auto pages = ann->page_span( SF->vpagesize());
		SF->set_cur_vpage( pages.first);
	}
}





// individual channel callbacks

gboolean
daSubjectTimeline_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
{
	auto& J = *(SExpDesignUI::SSubjectPresentation*)userdata;
	J.draw_timeline( cr);
	return TRUE;
}


gboolean
daSubjectTimeline_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
{
	auto& J = *(SExpDesignUI::SSubjectPresentation*)userdata;
	auto e_before = J.using_episode;
	if ( J.get_episode_from_timeline_click( event->x), J.using_episode != e_before )
		gtk_widget_queue_draw( wid);
	return TRUE;
}
gboolean
daSubjectTimeline_leave_notify_event_cb( GtkWidget *wid, GdkEventCrossing *event, gpointer userdata)
{
	if ( event->mode != GDK_CROSSING_NORMAL )
		return TRUE;
	auto& J = *(SExpDesignUI::SSubjectPresentation*)userdata;
	J.is_focused = false;
	J.using_episode = nullptr;
	gtk_widget_queue_draw( wid);
	return TRUE;
}
gboolean
daSubjectTimeline_enter_notify_event_cb( GtkWidget *wid, GdkEventCrossing *event, gpointer userdata)
{
	auto& J = *(SExpDesignUI::SSubjectPresentation*)userdata;
	J.is_focused = true;
	J.get_episode_from_timeline_click( event->x);
	gtk_widget_queue_draw( wid);
	return TRUE;
}



gboolean
daSubjectTimeline_button_press_event_cb( GtkWidget*, GdkEventButton *event, gpointer userdata)
{
	auto& J = *(SExpDesignUI::SSubjectPresentation*)userdata;
	auto& ED = J._p._p;
	ED.using_subject = &J;

	if ( J.get_episode_from_timeline_click( event->x) ) {
		// should some episodes be missing, we make sure the correct one gets identified by number
		ED._AghEi = find( ED.AghEE.begin(), ED.AghEE.end(), J.using_episode->name());
	} else
		ED._AghEi = ED.AghEE.end();

	switch ( event->button ) {
	case 1:
		if ( J.is_episode_focused() ) {
			new SScoringFacility( J.csubject, *ED._AghDi, *ED._AghEi, ED);
			// will be destroyed via its ui callbacks it has registered
		}
	    break;
	case 2:
	case 3:
		bool episode_ops = J.is_episode_focused();
		gtk_widget_set_visible( (GtkWidget*)ED.iSubjectTimelineScore, episode_ops);
		gtk_widget_set_visible( (GtkWidget*)ED.iSubjectTimelineEDFInfo, episode_ops);
		gtk_menu_popup( ED.iiSubjectTimeline,
				NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}

	return TRUE;
}

gboolean
daSubjectTimeline_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	auto& J = *(SExpDesignUI::SSubjectPresentation*)userdata;
	auto& ED = J._p._p;

	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		if ( event->state & GDK_CONTROL_MASK ) {
			ED.ppuv2 /= 1.1;
			gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
			return TRUE;
		}
	    break;
	case GDK_SCROLL_UP:
		if ( event->state & GDK_CONTROL_MASK ) {
			ED.ppuv2 *= 1.1;
			gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
			return TRUE;
		}
	    break;
	default:
	    break;
	}

	return FALSE;
}




// context cMeasurements menus
void
iiSubjectTimeline_show_cb( GtkWidget *widget, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto J = ED.using_subject;
	gtk_widget_set_sensitive( (GtkWidget*)ED.iSubjectTimelineScore,
				  J->is_episode_focused());
}


void
iSubjectTimelineScore_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto J = ED.using_subject;
	new SScoringFacility( J->csubject, *ED._AghDi, *ED._AghEi, ED);
}

void
iSubjectTimelineSubjectInfo_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.update_subject_details_interactively( ED.using_subject->csubject);
	gtk_widget_queue_draw( (GtkWidget*)ED.using_subject->da);
}


void
iSubjectTimelineEDFInfo_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto J = ED.using_subject;

	const auto& F = J->using_episode->sources.front();
	gtk_text_buffer_set_text( ED.tEDFFileDetailsReport, F.details().c_str(), -1);
	snprintf_buf( "%s header", F.filename());
	gtk_window_set_title( (GtkWindow*)ED.wEDFFileDetails,
			      __buf__);
	gtk_widget_show_all( (GtkWidget*)ED.wEDFFileDetails);
}


void
iSubjectTimelineSaveAsSVG_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto J = ED.using_subject;

	snprintf_buf( "%s/%s/%s/%s/%s.svg",
		      ED.ED->session_dir(), ED.ED->group_of( J->csubject), J->csubject.name(),
		      ED.AghD(), ED.AghT());
	string tmp (__buf__);
	J->is_focused = true;
	J->draw_timeline( __buf__);

	snprintf_buf( "Wrote \"%s\"", homedir2tilda(tmp).c_str());
	ED.buf_on_status_bar();
}


void
iSubjectTimelineBrowse_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto J = ED.using_subject;

	snprintf_buf( "%s '%s/%s/%s/%s' &",
		      ED.browse_command.c_str(), ED.ED->session_dir(), ED.ED->group_of( J->csubject), J->csubject.name(), ED.AghD());
	if ( system( __buf__) )
		;
}

void
iSubjectTimelineResetMontage_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto J = ED.using_subject;

	if ( not J->is_episode_focused() )
		snprintf_buf( "find '%s/%s/%s/%s' -name '.*.montage' -delete",
			      ED.ED->session_dir(), ED.ED->group_of( J->csubject), J->csubject.name(), ED.AghD());
	else
		snprintf_buf( "rm -f '%s/%s/%s/%s/.%s.montage'",
			      ED.ED->session_dir(), ED.ED->group_of( J->csubject), J->csubject.name(), ED.AghD(), ED.AghE());

	if ( system( __buf__) )
		;
}


}

// eof

