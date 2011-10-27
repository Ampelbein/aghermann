// ;-*-C++-*-
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


void bScoreNREM1_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::nrem1)); }
void bScoreNREM2_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::nrem2)); }
void bScoreNREM3_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::nrem3)); }
void bScoreNREM4_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::nrem4)); }
void bScoreREM_clicked_cb  ( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::rem));   }
void bScoreWake_clicked_cb ( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( agh::SPage::score_code(agh::SPage::TScore::wake));  }
void bScoreClear_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_back   ( agh::SPage::score_code(agh::SPage::TScore::none));  }





void
bScoringFacForward_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto current = SF.cur_vpage();
	if ( current < SF.total_vpages() )
		SF.set_cur_vpage( ++current);
}

void
bScoringFacBack_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto current = SF.cur_vpage();
	if ( current >= 1 )
		SF.set_cur_vpage( --current);
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





void
eSFICANonlinearity_changed_cb( GtkComboBox* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	static int vv[] = {
		FICA_NONLIN_POW3,
		FICA_NONLIN_TANH,
		FICA_NONLIN_GAUSS,
		FICA_NONLIN_SKEW
	};
	SF.ica->obj().set_non_linearity( vv[gtk_combo_box_get_active( w)]);
}

void
eSFICAApproach_changed_cb( GtkComboBox* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	static int vv[] = {
		FICA_APPROACH_DEFL,
		FICA_APPROACH_SYMM
	};
	SF.ica->obj().set_approach( vv[gtk_combo_box_get_active( w)]);
}

void
eSFICAFineTune_toggled_cb( GtkCheckButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_fine_tune( (bool)gtk_toggle_button_get_active( (GtkToggleButton*)w));
}

void
eSFICAStabilizationMode_toggled_cb( GtkCheckButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_stabilization( (bool)gtk_toggle_button_get_active( (GtkToggleButton*)w));
}

void
eSFICAa1_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_a1( gtk_spin_button_get_value( w));
}

void
eSFICAa2_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_a2( gtk_spin_button_get_value( w));
}

void
eSFICAmu_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_mu( gtk_spin_button_get_value( w));
}

void
eSFICAepsilon_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_epsilon( gtk_spin_button_get_value( w));
}

void
eSFICASampleSizePercent_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_sample_size( gtk_spin_button_get_value( w)/100);
}

void
eSFICANofICs_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_nrof_independent_components( (int)roundf( gtk_spin_button_get_value( w)));
}

void
eSFICAMaxIterations_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_max_num_iterations( (int)roundf( gtk_spin_button_get_value( w)));
}


void
bScoringFacRunICA_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.mode = aghui::SScoringFacility::TMode::showing_ics;
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacScoringModeContainer, FALSE);
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacICAModeContainer, TRUE);

	gtk_widget_set_sensitive( (GtkWidget*)SF.bScoringFacICAPreview, FALSE);

	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFAccept, FALSE);
	SF.set_tooltip( aghui::SScoringFacility::TTipIdx::ica_mode);

	SF.setup_ica();

	SF.queue_redraw_all();
}




void
bScoringFacICATry_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.run_ica();

	gtk_widget_set_sensitive( (GtkWidget*)SF.bScoringFacICAPreview, TRUE);

	SF.queue_redraw_all();
}

void
bScoringFacICAPreview_toggled_cb( GtkToggleButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;

	SF.remix_ics();

	if ( gtk_toggle_button_get_active(button) ) {
		SF.mode = aghui::SScoringFacility::TMode::showing_remixed;
	} else {
		SF.mode = aghui::SScoringFacility::TMode::showing_ics;
	}

	SF.queue_redraw_all();
}

void
bScoringFacICAApply_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.mode = aghui::SScoringFacility::TMode::scoring;
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacScoringModeContainer, TRUE);
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacICAModeContainer, FALSE);
	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFAccept, TRUE);
	SF.set_tooltip( aghui::SScoringFacility::TTipIdx::scoring_mode);
	
	// move the original edf file aside
	
	// put signal
	
	SF.queue_redraw_all();
}











void
bSFAccept_clicked_cb( GtkToolButton *button, gpointer userdata)
{
	auto SF = (SScoringFacility*)userdata;

	gtk_widget_queue_draw( (GtkWidget*)SF->_p.cMeasurements);

	delete SF;
	// the resulting destruction of all widgets owned by SF will cause
	// this warning: Gtk-CRITICAL **: gtk_widget_destroy: assertion `GTK_IS_WIDGET (widget)' failed
	// when I'm bored, perhaps I'll sit down and find a way to shut down scoring facility more cleanly
}


void
iSFAcceptAndTakeNext_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto SFp = (SScoringFacility*)userdata;
	auto& ED = SFp->_p; // keep same parent

	set_cursor_busy( true, (GtkWidget*)SFp->wScoringFacility);
	const char
		*j = SFp->channels.front().crecording.subject(),
		*d = SFp->channels.front().crecording.session(),
		*e = SFp->channels.front().crecording.episode();
	agh::CSubject& J = ED.ED->subject_by_x(j);

	// guaranteed to have next(E)

	delete SFp;

	SFp = new SScoringFacility( J, d,
				    next( J.measurements[d].episode_iter_by_name(e)) -> name(),
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
