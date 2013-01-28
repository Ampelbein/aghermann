// ;-*-C++-*-
/*
 *       File name:  ui/mw/mw-measurements_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI measurements view widget callbacks
 *
 *         License:  GPL
 */

#include "ui/misc.hh"
#include "ui/sf/sf.hh"
#include "mw.hh"

using namespace aghui;

extern "C" {

void
cGroupExpander_activate_cb( GtkExpander *w, gpointer userdata)
{
	auto& G = *(SExpDesignUI::SGroupPresentation*)userdata;
	G._p.group_unvisibility[G.name()] = gtk_expander_get_expanded(w);
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
		gtk_widget_set_visible( (GtkWidget*)ED.iSubjectTimelineDetectUltradianCycle, episode_ops);
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

	if ( event->state & GDK_CONTROL_MASK ) {
		ED.modify_profile_scales( event->direction);
		gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
		return TRUE;
	} else
		return FALSE;
}




// context cMeasurements menus
void
iiSubjectTimeline_show_cb( GtkWidget *widget, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto J = ED.using_subject;
	gtk_widget_set_sensitive(
		(GtkWidget*)ED.iSubjectTimelineScore,
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
iSubjectTimelineDetectUltradianCycle_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	agh::CSubject::SEpisode *Ep;
	if ( ED.using_subject && (Ep = ED.using_subject->using_episode) ) {
		auto& R = Ep->recordings.at(ED.AghH());
		aghui::SBusyBlock bb (ED.wMainWindow);
		ED.do_detect_ultradian_cycle( R);
	}
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
		      ED.ED->session_dir().c_str(), ED.ED->group_of( J->csubject), J->csubject.short_name.c_str(),
		      ED.AghD(), ED.AghT());
	string tmp (__buf__);
	J->is_focused = true;
	J->draw_timeline( __buf__);

	snprintf_buf( "Wrote \"%s\"", agh::str::homedir2tilda(tmp).c_str());
	ED.sb_message( __buf__);
}


void
iSubjectTimelineBrowse_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto J = ED.using_subject;

	snprintf_buf( "%s '%s/%s/%s/%s' &",
		      ED.browse_command.c_str(), ED.ED->session_dir().c_str(), ED.ED->group_of( J->csubject), J->csubject.short_name.c_str(), ED.AghD());
	if ( system( __buf__) ) {}
}

void
iSubjectTimelineResetMontage_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto J = ED.using_subject;

	if ( not J->is_episode_focused() )
		snprintf_buf( "find '%s/%s/%s/%s' -name '.*.montage' -delete",
			      ED.ED->session_dir().c_str(), ED.ED->group_of( J->csubject), J->csubject.short_name.c_str(), ED.AghD());
	else
		snprintf_buf( "rm -f '%s/%s/%s/%s/.%s.montage'",
			      ED.ED->session_dir().c_str(), ED.ED->group_of( J->csubject), J->csubject.short_name.c_str(), ED.AghD(), ED.AghE());

	if ( system( __buf__) )
		pop_ok_message( ED.wMainWindow, "Command '%s' returned a non-zero status. This is weird.", __buf__);
}


}

// eof

