// ;-*-C++-*- *  Time-stamp: "2011-07-03 20:10:53 hmmr"
/*
 *       File name:  ui/scoring-facility_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  scoring facility widget callbacks
 *
 *         License:  GPL
 */


// callbaaaackz!

#include "scoring-facility.hh"


using namespace std;
using namespace aghui;


extern "C" {


// ---------- page value_changed


	void
	eScoringFacPageSize_changed_cb( GtkComboBox *widget, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		gint item = gtk_combo_box_get_active( (GtkComboBox*)widget);
		SF->set_pagesize( item); // -1 is fine here
	}

	void
	eScoringFacCurrentPage_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		SF->set_cur_vpage( gtk_spin_button_get_value( SF->eScoringFacCurrentPage) - 1);
	}



// -------------- various buttons


	void bScoreClear_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::none)); }
	void bScoreNREM1_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::nrem1)); }
	void bScoreNREM2_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::nrem2)); }
	void bScoreNREM3_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::nrem3)); }
	void bScoreNREM4_clicked_cb( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::nrem4)); }
	void bScoreREM_clicked_cb  ( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::rem)); }
	void bScoreWake_clicked_cb ( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::wake)); }
	void bScoreMVT_clicked_cb  ( GtkButton *_, gpointer userdata)  { ((SScoringFacility*)userdata)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::mvt)); }





	void
	bScoringFacForward_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.set_cur_vpage( SF.cur_vpage() + 1);
	}

	void
	bScoringFacBack_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.set_cur_vpage( SF.cur_vpage() - 1);
	}




	void
	bScoreGotoPrevUnscored_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( SF.cur_page() == 0 )
			return;
		size_t p = SF.cur_page() - 1;
		while ( SF.hypnogram[p] != agh::SPage::score_code(agh::SPage::TScore::none) )
			if ( p != (size_t)-1 )
				--p;
			else
				break;
		// overflown values will be reset here:
		SF.set_cur_vpage( SF.p2ap(p));
	}

	void
	bScoreGotoNextUnscored_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( SF.cur_page() == SF.total_pages()-1 )
			return;
		size_t p = SF.cur_page() + 1;
		while ( SF.hypnogram[p] != agh::SPage::score_code(agh::SPage::TScore::none) )
			if ( p < SF.total_pages() )
				++p;
			else
				break;
		// out-of-range values will be reset here:
		SF.set_cur_vpage( SF.p2ap(p));
	}




	void
	bScoreGotoPrevArtifact_clicked_cb( GtkButton *button, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		if ( SF->cur_page() > 0 )
			return;
		size_t p = SF->cur_page() - 1;
		bool p_has_af;
		while ( !(p_has_af = SF->page_has_artifacts( p)) )
			if ( p != (size_t)-1 )
				--p;
			else
				break;
		SF->set_cur_vpage( SF->p2ap(p));
	}

	void
	bScoreGotoNextArtifact_clicked_cb( GtkButton *button, gpointer userdata)
	{
		SScoringFacility* SF = (SScoringFacility*)userdata;
		if ( SF->cur_page() == SF->total_pages()-1 )
			return;
		size_t p = SF->cur_page() + 1;
		bool p_has_af;
		while ( !(p_has_af = SF->page_has_artifacts( p)) )
			if ( p < SF->total_pages() )
				++p;
			else
				break;
		SF->set_cur_vpage( SF->p2ap(p));
	}




	void
	bScoringFacDrawPower_toggled_cb( GtkToggleButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.draw_power = (bool)gtk_toggle_button_get_active( button);
		for ( auto H = SF.channels.begin(); H != SF.channels.end(); ++H )
			// if ( H->have_power() )
				H->draw_power = SF.draw_power;
		SF.queue_redraw_all();
	}

	void
	bScoringFacDrawCrosshair_toggled_cb( GtkToggleButton *button, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		SF.draw_crosshair = !SF.draw_crosshair;
		SF.queue_redraw_all();
	}





	void
	bScoringFacShowFindDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( gtk_toggle_button_get_active( togglebutton) ) {
			gtk_widget_show_all( (GtkWidget*)SF.find_dialog.wPattern);
		} else
			gtk_widget_hide( (GtkWidget*)SF.find_dialog.wPattern);
	}



	void
	bScoringFacShowPhaseDiffDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer userdata)
	{
		auto& SF = *(SScoringFacility*)userdata;
		if ( gtk_toggle_button_get_active( togglebutton) ) {
			gtk_widget_show_all( (GtkWidget*)SF.phasediff_dialog.wPhaseDiff);
		} else
			gtk_widget_hide( (GtkWidget*)SF.phasediff_dialog.wPhaseDiff);
	}







// -- PageSelection


	void
	bSFAccept_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto SF = (SScoringFacility*)userdata;

		//gtk_widget_queue_draw( (GtkWidget*)aghui::cMeasurements);

		delete SF;
	}


	void
	iSFAcceptAndTakeNext_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
	{
		auto SFp = (SScoringFacility*)userdata;
		auto& ED = SFp->_p; // keep same parent

		set_cursor_busy( true, (GtkWidget*)SFp->wScoringFacility);
		const char
			*j = SFp->channels.front().recording.subject(),
			*d = SFp->channels.front().recording.session(),
			*e = SFp->channels.front().recording.episode();
		agh::CSubject& J = ED.ED->subject_by_x(j);
		auto& EE = J.measurements[d].episodes;
		// auto E = find( EE.begin(), EE.end(), e);
		// guaranteed to have next(E)

		delete SFp;

		SFp = new SScoringFacility( J, d,
					    next( find( EE.begin(), EE.end(), e)) -> name(),
					    ED);
		gtk_widget_show_all( (GtkWidget*)SFp->wScoringFacility);
		set_cursor_busy( false, (GtkWidget*)SFp->wScoringFacility);
	}



// ------- cleanup

	gboolean
	wScoringFacility_delete_event_cb( GtkWidget *widget,
					  GdkEvent  *event,
					  gpointer   userdata)
	{
		auto SFp = (SScoringFacility*)userdata;
		// not sure resurrection will succeed, tho
		gtk_widget_queue_draw( (GtkWidget*)SFp->_p.cMeasurements);

		delete SFp;

		return TRUE; // to stop other handlers from being invoked for the event
	}

} // extern "C"

// eof
