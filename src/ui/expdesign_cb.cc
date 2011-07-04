// ;-*-C++-*- *  Time-stamp: "2011-07-05 02:16:50 hmmr"
/*
 *       File name:  ui/expdesign_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI widget callbacks
 *
 *         License:  GPL
 */

#include "misc.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"


using namespace aghui;

extern "C" {

	void
	bExpChange_clicked_cb( GtkButton *button, gpointer userdata)
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



	gboolean
	wMainWindow_destroy_event_cb( GtkWidget *wid, gpointer userdata)
	{
		auto EDp = (SExpDesignUI*)userdata;

		// check if any facilities are open, and prompt
		FAFA;

		gtk_window_get_position( EDp->wMainWindow, &EDp->geometry.x, &EDp->geometry.y);
		gtk_window_get_size( EDp->wMainWindow, &EDp->geometry.w, &EDp->geometry.h);

		delete EDp;

		gtk_main_quit();

		return FALSE; // whatever
	}

	gboolean
	wMainWindow_delete_event_cb( GtkWidget *wid, gpointer userdata)
	{
		return wMainWindow_destroy_event_cb( wid, userdata);
	}


      // tab switch
	void
	tTaskSelector_switch_page_cb( GtkNotebook     *notebook,
				      gpointer	       unused,
				      guint            page_num,
				      gpointer         userdata)
	{
		auto& ED = *(SExpDesignUI*)userdata;
		if ( page_num == 1 ) {
			ED.populate( false);
			snprintf_buf( "Session: <b>%s</b>", ED.AghD());
			gtk_label_set_markup( ED.lSimulationsSession, __buf__);
			snprintf_buf( "Channel: <b>%s</b>", ED.AghT());
			gtk_label_set_markup( ED.lSimulationsChannel, __buf__);
			gtk_widget_set_sensitive( (GtkWidget*)ED.bExpChange, FALSE);
		} else if ( page_num == 0 ) {
			ED.ED->remove_untried_modruns();
			ED.populate( false);
			gtk_widget_set_sensitive( (GtkWidget*)ED.bExpChange, TRUE);
		}
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
		;
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


      // -------- colours
	void
	bColourX_color_set_cb( GtkColorButton *widget,
			       gpointer        userdata)
	{
		auto& mc = *(SManagedColor*)userdata;
		mc.acquire();
	}
}

// eof
