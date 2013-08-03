/*
 *       File name:  aghermann/ui/sf/mainmenu_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-08-03
 *
 *         Purpose:  scoring facility main menu callbacks
 *
 *         License:  GPL
 */

#include "sf.hh"
#include "d/patterns.hh"
#include "d/phasediff.hh"

using namespace std;
using namespace aghui;

extern "C" {


void
iSFMontageMenu_activate_cb(
	const GtkMenuItem *menuitem,
	gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.update_main_menu_items();
}



void
iSFMontageDrawOriginalSignal_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;

	bool indeed = (bool)gtk_check_menu_item_get_active( checkmenuitem);

	for ( auto& H : SF.channels ) {
		H.draw_original_signal = indeed;
		if ( not H.draw_original_signal and not H.draw_filtered_signal )
			H.draw_filtered_signal = true;
	}

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);

	SF.update_main_menu_items();
}


void
iSFMontageDrawProcessedSignal_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;

	bool indeed = (bool)gtk_check_menu_item_get_active( checkmenuitem);

	for ( auto& H : SF.channels ) {
		H.draw_filtered_signal = indeed;
		if ( not H.draw_original_signal and not H.draw_filtered_signal )
			H.draw_original_signal = true;
	}

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);

	SF.update_main_menu_items();
}


void
iSFMontageDrawFast_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;

	bool indeed = (bool)gtk_check_menu_item_get_active( checkmenuitem);

	for ( auto& H : SF.channels )
		H.resample_signal = indeed;

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);

	SF.update_main_menu_items();
}


void
iSFMontageDrawZeroLine_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;

	bool indeed = (bool)gtk_check_menu_item_get_active( checkmenuitem);

	for ( auto& H : SF.channels )
		H.draw_zeroline = indeed;

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);

	SF.update_main_menu_items();
}



void
iSFMontagePatterns_activate_cb(
	const GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.patterns_d().setup_controls_for_find();
	gtk_widget_show( (GtkWidget*)SF.patterns_d().wSFFD);
}



void
iSFMontagePhaseDiff_activate_cb(
	const GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	gtk_widget_show( (GtkWidget*)SF.phasediff_d().wSFPD);
}


void
iSFMontageICA_activate_cb(
	const GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.setup_ica() == 0 ) {
		SF.mode = SScoringFacility::TMode::showing_ics;
		gtk_widget_set_visible( (GtkWidget*)SF.cSFScoringModeContainer, FALSE);
		gtk_widget_set_visible( (GtkWidget*)SF.cSFICAModeContainer, TRUE);

		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICATry, TRUE);
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAShowMatrix, FALSE);
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAPreview, FALSE);
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAApply, FALSE);

		gtk_widget_set_sensitive( (GtkWidget*)SF.iSFMontageClose, FALSE);
		gtk_widget_set_sensitive( (GtkWidget*)SF.iSFMontageCloseAndNext, FALSE);
		SF.set_tooltip( SScoringFacility::TTipIdx::ica_mode);
		SF.queue_redraw_all();
	} else
		gdk_window_beep( gtk_widget_get_window( (GtkWidget*)SF.wSF));
}







void
iSFMontageScoreAssist_activate_cb(
	const GtkMenuItem*,
	gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	
}

void
iSFMontageScoreImport_activate_cb(
	const GtkMenuItem*,
	gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.do_dialog_import_hypnogram();
}

void
iSFMontageScoreExport_activate_cb(
	const GtkMenuItem*,
	const gpointer userdata)
{
	const auto& SF = *(SScoringFacility*)userdata;

	SF.do_dialog_export_hypnogram();
}

void
iSFMontageScoreClear_activate_cb(
	const GtkMenuItem*,
	gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.do_clear_hypnogram();
}





void
iSFMontageClose_activate_cb(
	const GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF._p.close_this_SF_now = &SF;
	g_signal_emit_by_name( SF._p.bMainCloseThatSF, "clicked");
}


void
iSFMontageCloseAndNext_activate_cb(
	const GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& ED = SF._p; // keep same parent

	ED.using_subject->create_cprofile();
	gtk_widget_queue_draw( (GtkWidget*)ED.using_subject->da);

	SBusyBlock bb (SF.wSF);
	// guaranteed to have next(E)

	SF._p.close_this_SF_now = &SF;
	g_signal_emit_by_name( SF._p.bMainCloseThatSF, "clicked");

	auto& J = SF.csubject();
	new SScoringFacility(
		J, SF.session().c_str(),
		next( J.measurements[SF.session()].episode_iter_by_name(SF.sepisode().name())) -> name(),
		ED); // ED records SScoringFacility::this alright
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


} // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
