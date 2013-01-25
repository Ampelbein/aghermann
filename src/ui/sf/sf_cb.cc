// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  scoring facility widget callbacks
 *
 *         License:  GPL
 */

#include "ui/mw/mw.hh"
#include "sf.hh"
#include "widgets.hh"
#include "d/patterns.hh"
#include "d/phasediff.hh"

using namespace std;
using namespace aghui;

extern "C" {


// gboolean
// wScoringFacility_configure_event_cb( GtkWidget *, GdkEventConfigure *event, gpointer userdata)
// {
// 	auto &SF = *(SScoringFacility*)userdata;
// 	if ( SF.suppress_redraw )
// 		return FALSE;

// 	if ( event->type == GDK_CONFIGURE )
// 		SF.geometry = {
// 			event -> x,
// 			event -> y,
// 			event -> width,
// 			event -> height
// 		};
// 	return FALSE; // whatever
// }


// ---------- page value_changed


void
eSFPageSize_changed_cb( GtkComboBox *combobox, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	SF.set_vpagesize_item( gtk_combo_box_get_active( combobox), false);
	SF.queue_redraw_all();
}

void
eSFCurrentPage_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.set_cur_vpage( gtk_spin_button_get_value( spinbutton) - 1, false);
	SF.queue_redraw_all();
}



// -------------- various buttons


void bSFScoreNREM1_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::nrem1)); }
void bSFScoreNREM2_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::nrem2)); }
void bSFScoreNREM3_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::nrem3)); }
void bSFScoreNREM4_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::nrem4)); }
void bSFScoreREM_clicked_cb  ( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::rem));   }
void bSFScoreWake_clicked_cb ( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_forward( sigfile::SPage::score_code(sigfile::SPage::TScore::wake));  }
void bSFScoreClear_clicked_cb( GtkButton *_, gpointer u)  { ((SScoringFacility*)u)->do_score_back   ( sigfile::SPage::score_code(sigfile::SPage::TScore::none));  }





void
bSFForward_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	auto current = SF.cur_vpage();
	if ( current < SF.total_vpages() - 1 )
		SF.set_cur_vpage( current+1);
	SF.sb_clear();
}

void
bSFBack_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	auto current = SF.cur_vpage();
	if ( current > 0 )
		SF.set_cur_vpage( current-1);
	SF.sb_clear();
}


void
eSFCurrentPos_clicked_cb( GtkButton*,
			  gpointer   userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.show_cur_pos_time_relative = !SF.show_cur_pos_time_relative;
	SF.draw_current_pos( NAN);
}


void
bSFGotoPrevUnscored_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	if ( SF.cur_page() == 0 )
		return;
	size_t p = SF.cur_page();
	while ( --p != (size_t)-1 )
		if ( SF.hypnogram[p] == sigfile::SPage::score_code(sigfile::SPage::TScore::none) ) {
			SF.sb_clear();
			SF.set_cur_vpage( SF.p2ap(p));
			return;
		}
	SF.sb_message( "No more unscored pages before this");
}

void
bSFGotoNextUnscored_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.cur_page() == SF.total_pages()-1 )
		return;
	size_t p = SF.cur_page();
	while ( ++p < SF.total_pages() )
		if ( SF.hypnogram[p] == sigfile::SPage::score_code(sigfile::SPage::TScore::none) ) {
			SF.sb_clear();
			SF.set_cur_vpage( SF.p2ap(p));
			return;
		}
	SF.sb_message( "No more unscored pages after this");
}




void
bSFGotoPrevArtifact_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	if ( SF.cur_vpage() == 0 )
		return;
	size_t p = SF.cur_vpage();
	while ( --p != (size_t)-1 )
		if ( SF.page_has_artifacts( p, false)) {
			SF.sb_clear();
			SF.set_cur_vpage( p);
			return;
		}
	SF.sb_message( "No more dirty pages before this");
}

void
bSFGotoNextArtifact_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto &SF = *(SScoringFacility*)userdata;
	if ( SF.cur_vpage() == SF.total_vpages()-1 )
		return;
	size_t p = SF.cur_vpage();
	while ( ++p < SF.total_vpages() )
		if ( SF.page_has_artifacts( p)) {
			SF.sb_clear();
			SF.set_cur_vpage( p);
			return;
		}
	SF.sb_message( "No more dirty pages after this");
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
		SF.patterns_d().setup_controls_for_find();
		gtk_widget_show( (GtkWidget*)SF.patterns_d().wSFFD);
	} else
		gtk_widget_hide( (GtkWidget*)SF.patterns_d().wSFFD);
}



void
bSFShowPhaseDiffDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( gtk_toggle_button_get_active( togglebutton) ) {
		gtk_widget_show( (GtkWidget*)SF.phasediff_d().wSFPD);
	} else
		gtk_widget_hide( (GtkWidget*)SF.phasediff_d().wSFPD);
}





void
bSFAccept_clicked_cb( GtkToolButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF._p.close_this_SF_now = &SF;
	g_signal_emit_by_name( SF._p.bMainCloseThatSF, "clicked");
}


void
iSFAcceptAndTakeNext_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& ED = SF._p; // keep same parent

	ED.using_subject->create_cprofile();
	gtk_widget_queue_draw( (GtkWidget*)ED.using_subject->da);

	SBusyBlock bb (SF.wSF);
	// guaranteed to have next(E)

	auto& J = SF.csubject();
	new SScoringFacility(
		J, SF.session().c_str(),
		next( J.measurements[SF.session()].episode_iter_by_name(SF.sepisode().name())) -> name(),
		ED);

	SF._p.close_this_SF_now = &SF;
	g_signal_emit_by_name( SF._p.bMainCloseThatSF, "clicked");
}



// ------- cleanup

gboolean
wSF_delete_event_cb( GtkWidget*, GdkEvent*, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF._p.close_this_SF_now = &SF;
	g_signal_emit_by_name( SF._p.bMainCloseThatSF, "clicked");

	return TRUE; // to stop other handlers from being invoked for the event
}

} // extern "C"

// eof
