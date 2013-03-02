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




namespace {

#define EVENT_X 30

size_t position_for_channel = -1;
void channel_menu_position( GtkMenu *menu,
			    gint *x,
			    gint *y,
			    gboolean *push_in,
			    gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	int mwx, mwy, rwx, rwy;
	gtk_window_get_position( SF.wSF, &mwx, &mwy);
	gdk_window_get_position( gtk_widget_get_window( (GtkWidget*)SF.daSFMontage), &rwx, &rwy);
	if ( position_for_channel < SF.channels.size() ) {
		*x = mwx + rwx + EVENT_X;
		*y = mwy + rwx + SF[position_for_channel].zeroy-20;
	} else
		*x = *y = 0;
}

}

gboolean
wSF_key_press_event_cb( GtkWidget *wid, GdkEventKey *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	if ( event->type == GDK_KEY_RELEASE or
	     !(event->state & GDK_MOD1_MASK) )
		return FALSE;

#define KEKE(N) \
	position_for_channel = N-1;					\
	if ( position_for_channel < SF.channels.size() ) {		\
		SF.using_channel = &SF[position_for_channel];		\
		SF.using_channel->update_channel_menu_items( EVENT_X);	\
		gtk_menu_popup( SF.iiSFPage, NULL, NULL, channel_menu_position, userdata, 3, event->time); \
	} else								\
		gdk_beep();						\
	return TRUE;

	switch ( event->keyval ) {
	case GDK_KEY_1: KEKE(1);
	case GDK_KEY_2: KEKE(2);
	case GDK_KEY_3: KEKE(3);
	case GDK_KEY_4: KEKE(4);
	case GDK_KEY_5: KEKE(5);
	case GDK_KEY_6: KEKE(6);
	case GDK_KEY_7: KEKE(7);
	case GDK_KEY_8: KEKE(8);
	case GDK_KEY_9: KEKE(9);
	case GDK_KEY_0: KEKE(10);
	}
#undef KEKE
	return FALSE;
}


gboolean
wSF_delete_event_cb( GtkWidget*, GdkEvent*, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF._p.close_this_SF_now = &SF;
	g_signal_emit_by_name( SF._p.bMainCloseThatSF, "clicked");

	return TRUE; // to stop other handlers from being invoked for the event
}

} // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
