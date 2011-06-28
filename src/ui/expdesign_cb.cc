// ;-*-C++-*- *  Time-stamp: "2011-06-29 02:47:24 hmmr"
/*
 *       File name:  ui/expdesign_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI widget callbcks
 *
 *         License:  GPL
 */

#include "expdesign.hh"

using namespace aghui;

extern "C" {

	void
	eMsmtSession_changed_cb( GtkComboBox *widget, gpointer user_data)
	{
		auto oldval = _AghDi;
		_AghDi = find( AghDD.begin(), AghDD.end(),
			       gtk_combo_box_get_active_id( eMsmtSession));

		if ( oldval != _AghDi )
			msmt::populate();
	}

	void
	eMsmtChannel_changed_cb( GtkComboBox *widget, gpointer user_data)
	{
		auto oldval = _AghTi;
		_AghTi = find( AghTT.begin(), AghTT.end(),
			       gtk_combo_box_get_active_id( eMsmtChannel));
		if ( /* _AghTi != AghTT.end() && */ oldval != _AghTi )
			msmt::populate();
	}



	void
	eMsmtPSDFreqFrom_value_changed_cb( GtkSpinButton *spinbutton, gpointer user_data)
	{
		OperatingRangeFrom = gtk_spin_button_get_value( eMsmtPSDFreqFrom);
		OperatingRangeUpto = OperatingRangeFrom + gtk_spin_button_get_value( eMsmtPSDFreqWidth);
		msmt::populate();
	}

	void
	eMsmtPSDFreqWidth_value_changed_cb( GtkSpinButton *spinbutton, gpointer user_data)
	{
		OperatingRangeUpto = OperatingRangeFrom + gtk_spin_button_get_value( eMsmtPSDFreqWidth);
		msmt::populate();
	}

	gboolean
	daSubjectTimeline_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
	{
		((const msmt::SSubjectPresentation*)userdata) -> draw_timeline( cr);
		return TRUE;
	}


	gboolean
	daSubjectTimeline_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
	{
		SSubjectPresentation& J = *(SSubjectPresentation*)userdata;
		if ( J.get_episode_from_timeline_click( event->x) )
			gtk_widget_queue_draw( wid);
		return TRUE;
	}
	gboolean
	daSubjectTimeline_leave_notify_event_cb( GtkWidget *wid, GdkEventCrossing *event, gpointer userdata)
	{
		SSubjectPresentation& J = *(SSubjectPresentation*)userdata;
		J.is_focused = false;
		J.episode_focused = J.csubject.measurements[*_AghDi].episodes.end();
		gtk_widget_queue_draw( wid);
		return TRUE;
	}
	gboolean
	daSubjectTimeline_enter_notify_event_cb( GtkWidget *wid, GdkEventCrossing *event, gpointer userdata)
	{
		SSubjectPresentation& J = *(SSubjectPresentation*)userdata;
		J.is_focused = true;
		if ( J.get_episode_from_timeline_click( event->x) )
			;
		gtk_widget_queue_draw( wid);
		return TRUE;
	}



	gboolean
	daSubjectTimeline_button_press_event_cb( GtkWidget *widget, GdkEventButton *event, gpointer userdata)
	{
		using namespace msmt;
		SSubjectPresentation& J = *(SSubjectPresentation*)userdata;

		if ( J.get_episode_from_timeline_click( event->x) ) {
			// should some episodes be missing, we make sure the correct one gets identified by number
			_AghEi = find( AghEE.begin(), AghEE.end(), J.episode_focused->name());
		} else
			_AghEi = AghEE.end();
//		AghJ = _j;

		switch ( event->button ) {
		case 1:
			if ( _AghEi != AghEE.end() ) {
				new sf::SScoringFacility( J.csubject, *_AghDi, *_AghEi);
				// will be destroyed by its ui callbacks it has registered
			}
		    break;
		case 2:
		case 3:
			if ( event->state & GDK_MOD1_MASK ) {
				snprintf_buf( "%s/%s/%s/%s/%s.svg",
					      AghCC->session_dir(), AghCC->group_of( J.csubject), J.csubject.name(),
					      AghD(), AghT());
				string tmp (__buf__);
				J.draw_timeline( __buf__);
				snprintf_buf( "Wrote \"%s\"", tmp.c_str());
				gtk_statusbar_pop( sbMainStatusBar, sb::sbContextIdGeneral);
				gtk_statusbar_push( sbMainStatusBar, sb::sbContextIdGeneral,
						    __buf__);
			} else if ( AghE() ) {
				const agh::CEDFFile& F = J.cscourse->mm_list().front()->source();
				gtk_text_buffer_set_text( textbuf2, F.details().c_str(), -1);
				snprintf_buf( "%s header", F.filename());
				gtk_window_set_title( (GtkWindow*)wEDFFileDetails,
						      __buf__);
				gtk_widget_show_all( (GtkWidget*)(wEDFFileDetails));
			}
		    break;
		}

		return TRUE;
	}


	gboolean
	daSubjectTimeline_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer ignored)
	{
		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			if ( event->state & GDK_CONTROL_MASK ) {
				PPuV2 /= 1.3;
				gtk_widget_queue_draw( (GtkWidget*)(cMeasurements));
				return TRUE;
			}
			break;
		case GDK_SCROLL_UP:
			if ( event->state & GDK_CONTROL_MASK ) {
				PPuV2 *= 1.3;
				gtk_widget_queue_draw( (GtkWidget*)(cMeasurements));
				return TRUE;
			}
			break;
		default:
			break;
		}

		return FALSE;
	}



      // -------- colours
	void
	bColourPowerMT_color_set_cb( GtkColorButton *widget,
				     gpointer        user_data)
	{
		CwB[TColour::power_mt].acquire();
	}

	void
	bColourTicksMT_color_set_cb( GtkColorButton *widget,
				     gpointer        user_data)
	{
		CwB[TColour::ticks_mt].acquire();
	}

	void
	bColourLabelsMT_color_set_cb( GtkColorButton *widget,
				      gpointer        user_data)
	{
		CwB[TColour::labels_mt].acquire();
	}
}

