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


#include "scoring-facility.hh"


using namespace std;
using namespace aghui;


extern "C" {


// ---------- page value_changed


void
eSFPageSize_changed_cb( GtkComboBox *widget, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	gint item = gtk_combo_box_get_active( (GtkComboBox*)widget);
	SF.set_pagesize( item); // -1 is fine here
}

void
eSFCurrentPage_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( !SF.suppress_set_vpage_from_cb )
		SF.set_cur_vpage( gtk_spin_button_get_value( SF.eSFCurrentPage) - 1);
}



// -------------- various buttons


void bScoreNREM1_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::nrem1)); }
void bScoreNREM2_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::nrem2)); }
void bScoreNREM3_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::nrem3)); }
void bScoreNREM4_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::nrem4)); }
void bScoreREM_clicked_cb  ( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::rem));   }
void bScoreWake_clicked_cb ( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::wake));  }
void bScoreClear_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_back   ( sigfile::SPage::score_code(sigfile::SPage::TScore::none));  }





void
bSFForward_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	auto current = SF.cur_vpage();
	if ( current < SF.total_vpages() )
		SF.set_cur_vpage( ++current);
}

void
bSFBack_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	auto current = SF.cur_vpage();
	if ( current >= 1 )
		SF.set_cur_vpage( --current);
}




void
bScoreGotoPrevUnscored_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	if ( SF.cur_page() == 0 )
		return;
	size_t p = SF.cur_page() - 1;
	while ( SF.hypnogram[p] != sigfile::SPage::score_code(sigfile::SPage::TScore::none) )
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
	while ( SF.hypnogram[p] != sigfile::SPage::score_code(sigfile::SPage::TScore::none) )
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
	auto &SF = *(SScoringFacility*)userdata;
	if ( SF.cur_page() == 0 )
		return;
	size_t p = SF.cur_page() - 1;
	bool p_has_af;
	while ( !(p_has_af = SF.page_has_artifacts( p)) )
		if ( p != (size_t)-1 )
			--p;
		else
			break;
	SF.set_cur_vpage( SF.p2ap(p));
}

void
bScoreGotoNextArtifact_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	if ( SF.cur_page() == SF.total_pages()-1 )
		return;
	size_t p = SF.cur_page() + 1;
	bool p_has_af;
	while ( !(p_has_af = SF.page_has_artifacts( p)) )
		if ( p < SF.total_pages() )
			++p;
		else
			break;
	SF.set_cur_vpage( SF.p2ap(p));
}





void
bSFDrawCrosshair_toggled_cb( GtkToggleButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.draw_crosshair = !SF.draw_crosshair;
	SF.queue_redraw_all();
}





void
bSFShowFindDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( gtk_toggle_button_get_active( togglebutton) ) {
		gtk_widget_show_all( (GtkWidget*)SF.find_dialog.wPattern);
	} else
		gtk_widget_hide( (GtkWidget*)SF.find_dialog.wPattern);
}



void
bSFShowPhaseDiffDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( gtk_toggle_button_get_active( togglebutton) ) {
		gtk_widget_show_all( (GtkWidget*)SF.phasediff_dialog.wPhaseDiff);
	} else
		gtk_widget_hide( (GtkWidget*)SF.phasediff_dialog.wPhaseDiff);
}





void
bSFAccept_clicked_cb( GtkToolButton *button, gpointer userdata)
{
	auto SFp = (SScoringFacility*)userdata;

	SFp->_p.using_subject->create_cscourse();
	gtk_widget_queue_draw( (GtkWidget*)SFp->_p.using_subject->da);

	delete SFp;
	// the resulting destruction of all widgets owned by SF will cause
	// this warning: Gtk-CRITICAL **: gtk_widget_destroy: assertion `GTK_IS_WIDGET (widget)' failed
	// when I'm bored, perhaps I'll sit down and find a way to shut down scoring facility more cleanly
}


void
iSFAcceptAndTakeNext_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto SFp = (SScoringFacility*)userdata;

	auto& ED = SFp->_p; // keep same parent

	ED.using_subject->create_cscourse();
	gtk_widget_queue_draw( (GtkWidget*)ED.using_subject->da);

	set_cursor_busy( true, (GtkWidget*)SFp->wScoringFacility);
	const char
		*j = SFp->channels.front().crecording.subject(),
		*d = SFp->channels.front().crecording.session(),
		*e = SFp->channels.front().crecording.episode();
	agh::CSubject& J = ED.ED->subject_by_x(j);

	// guaranteed to have next(E)

	auto SFp2 = new SScoringFacility(
		J, d,
		next( J.measurements[d].episode_iter_by_name(e)) -> name(),
		ED);

	delete SFp;

	set_cursor_busy( false, (GtkWidget*)SFp2->wScoringFacility);
}



// ------- cleanup

gboolean
wScoringFacility_delete_event_cb( GtkWidget*, GdkEvent*, gpointer userdata)
{
	auto SFp = (SScoringFacility*)userdata;

	SFp->_p.using_subject->create_cscourse();
	gtk_widget_queue_draw( (GtkWidget*)SFp->_p.using_subject->da);

	// not sure resurrection will succeed, tho
	delete SFp;

	return TRUE; // to stop other handlers from being invoked for the event
}

} // extern "C"

// eof
