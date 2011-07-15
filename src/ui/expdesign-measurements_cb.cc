// ;-*-C++-*- *  Time-stamp: "2011-07-15 17:38:51 hmmr"
/*
 *       File name:  ui/expdesign-measurements_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI measurements view widget callbcks
 *
 *         License:  GPL
 */

#include "misc.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"

using namespace aghui;

extern "C" {

	void
	bScanTree_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		ED.do_rescan_tree( false);
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
	}



	void
	eMsmtPSDFreqFrom_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		ED.operating_range_from = gtk_spin_button_get_value( spinbutton);
		ED.operating_range_upto = ED.operating_range_from + gtk_spin_button_get_value( ED.eMsmtPSDFreqWidth);
		ED.populate_1();
	}

	void
	eMsmtPSDFreqWidth_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		ED.operating_range_upto = ED.operating_range_from + gtk_spin_button_get_value( spinbutton);
		ED.populate_1();
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
		if ( J.get_episode_from_timeline_click( event->x) )
			gtk_widget_queue_draw( wid);
		return TRUE;
	}
	gboolean
	daSubjectTimeline_leave_notify_event_cb( GtkWidget *wid, GdkEventCrossing *event, gpointer userdata)
	{
		auto& J = *(SExpDesignUI::SSubjectPresentation*)userdata;
		J.is_focused = false;
		J.using_episode = J.sepisodesequence().end();
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
	daSubjectTimeline_button_press_event_cb( GtkWidget *widget, GdkEventButton *event, gpointer userdata)
	{
		auto& J = *(SExpDesignUI::SSubjectPresentation*)userdata;
		auto& ED = J._p._p;

		if ( J.get_episode_from_timeline_click( event->x) ) {
			// should some episodes be missing, we make sure the correct one gets identified by number
			ED._AghEi = find( ED.AghEE.begin(), ED.AghEE.end(), J.using_episode->name());
		} else
			ED._AghEi = ED.AghEE.end();
//		AghJ = _j;

		switch ( event->button ) {
		case 1:
			if ( J.is_episode_focused() ) {
				new SScoringFacility( J.csubject, *ED._AghDi, *ED._AghEi, ED);
				// will be destroyed via its ui callbacks it has registered
			}
		    break;
		case 2:
		case 3:
			ED.using_subject = &J;
			bool episode_ops = J.is_episode_focused();
			gtk_widget_set_visible( (GtkWidget*)ED.iSubjectTimelineScore, episode_ops);
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




      // menus
	void
	iiSubjectTimeline_show_cb( GtkWidget *widget, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		auto J = ED.using_subject;
		gtk_widget_set_sensitive( (GtkWidget*)ED.iSubjectTimelineScore,
					  J->is_episode_focused());
	}


	void
	iSubjectTimelineScore_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		auto J = ED.using_subject;
		new SScoringFacility( J->csubject, *ED._AghDi, *ED._AghEi, ED);
	}

	void
	iSubjectTimelineSubjectInfo_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		FAFA;
	}


	void
	iSubjectTimelineEDFInfo_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		auto J = ED.using_subject;

		const auto& F = J->cscourse->mm_list().front()->source();
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
		J->draw_timeline( __buf__);

		snprintf_buf( "Wrote \"%s\"", tmp.c_str());
		ED.buf_on_status_bar();
	}


	void
	iSubjectTimelineBrowse_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		auto J = ED.using_subject;

		snprintf_buf( "%s '%s/%s/%s/%s' &",
			      ED.browse_command.c_str(), ED.ED->session_dir(), ED.ED->group_of( J->csubject), J->csubject.name(), ED.AghD());
		// if ( fork() == 0 )
		// 	if ( execlp( cmd, __buf__) )
		// 		;
		if ( system( __buf__) )
			;
	}


}

// eof

