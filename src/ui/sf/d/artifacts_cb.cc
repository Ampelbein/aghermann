/*
 *       File name:  ui/sf/d/artifacts_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-05
 *
 *         Purpose:  scoring facility: artifact detection dialog callbacks
 *
 *         License:  GPL
 */

#include "ui/misc.hh"
#include "sf.hh"
#include "sf_cb.hh"

using namespace std;
using namespace aghui;



void
eSFADProfiles_changed_cb( GtkComboBox* w, gpointer userdata)
{
	auto& SD = *(SScoringFacility::SArtifactsDialog*)userdata;

	if ( gtk_combo_box_get_active( w) != -1 ) {
		AD.P = AD._p._p.global_artifact_detection_profiles[
			gtk_combo_box_get_active_id(w)];
		AD.W_V.up();
		gtk_widget_set_sensitive( (GtkWidget*)AD.bSFADProfileDelete, TRUE);
	} else
		gtk_widget_set_sensitive( (GtkWidget*)AD.bSFADProfileDelete, FALSE);
}

void
bSFADProfileSave_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( AD.wSFADSaveProfileName) ) {
		AD.W_V.down();
		AD._p._p.global_artifact_detection_profiles[
			gtk_entry_get_text( SF.eSFADSaveProfileNameName)] = AD.P;

		AD._p._p.populate_mGlobalADProfiles();
		AD.populate_mSFADProfiles(); // stupid

		int now_active = AD._p._p.global_artifact_detection_profiles.size()-1;
		gtk_combo_box_set_active( AD.eSFADProfiles, now_active);
		gtk_combo_box_set_active( AD._p._p.eGlobalADProfiles, now_active);
	}
}

void
bSFADProfileDelete_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;

	const gchar *deleting_id = gtk_combo_box_get_active_id( AD.eSFADProfiles);
	int deleting = gtk_combo_box_get_active( AD.eSFADProfiles);
	AD._p._p.global_artifact_detection_profiles.erase( deleting_id);

	AD._p._p.populate_mGlobalADProfiles();
	AD._p.populate_mSFADProfiles(); // stupid

	if ( AD._p._p.global_artifact_detection_profiles.size() > 0 &&
	     deleting > (int)AD._p._p.global_artifact_detection_profiles.size()-1 )
		gtk_combo_box_set_active( AD.eSFADProfiles, deleting-1);

	g_signal_emit_by_name( AD.eSFADProfiles, "changed");
}


void
eSFADEstimateE_toggled_cb( GtkToggleButton *b, gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;
	auto state = gtk_toggle_button_get_active( b);
	gtk_widget_set_visible(
		(GtkWidget*)AD.cSFADWhenEstimateEOn,
		state);
	gtk_widget_set_visible(
		(GtkWidget*)AD.cSFADWhenEstimateEOff,
		!state);
}

void
eSFADUseThisRange_toggled_cb( GtkToggleButton *b, gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;
	auto state = gtk_toggle_button_get_active( b);
	gtk_widget_set_sensitive(
		(GtkWidget*)AD.eSFADHistRangeMin,
		state);
	gtk_widget_set_sensitive(
		(GtkWidget*)AD.eSFADHistRangeMax,
		state);

	// if ( state ) {
	// 	snprintf_buf( "Estimated <i>E</i> = %4.2f",
	// 		      SF.using_channel -> estimate_E( P));
	// }
}



void
bSFADApply_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;

	gtk_widget_hide( (GtkWidget*)AD.wSFAD);

	for ( auto& H : AD.channels_visible_backup )
		H.first->hidden = H.second;
	AD.channels_visible_backup.clear();
	AD.artifacts_backup.clear_all();

	gtk_widget_queue_draw( (GtkWidget*)AD.daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)AD.daSFHypnogram);
}

void
bSFADCancel_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;

	gtk_widget_hide( (GtkWidget*)AD.wSFAD);

	if ( gtk_toggle_button_get_active(AD.bSFADPreview) ) {
		AD._p.using_channel -> artifacts = AD.artifacts_backup;
		AD._p.using_channel -> get_signal_filtered();

		gtk_widget_queue_draw( (GtkWidget*)AD.daSFMontage);
		gtk_widget_queue_draw( (GtkWidget*)AD.daSFHypnogram);
	}

	for ( auto& H : AD.channels_visible_backup )
		H.first->hidden = H.second;
	AD.channels_visible_backup.clear();
	AD.artifacts_backup.clear_all();
}

void
bSFADPreview_toggled_cb( GtkToggleButton *b, gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;

	if ( AD.suppress_preview_handler )
		return;

	if ( gtk_toggle_button_get_active(b) ) {
		aghui::SBusyBlock bb (AD.wSFAD);

		AD.orig_signal_visible_backup = AD._p.using_channel->draw_original_signal;
		AD.artifacts_backup = AD._p.using_channel->artifacts;

		AD._p.using_channel -> detect_artifacts( (AD.W_V.down(), AD.P));
		AD._p.using_channel -> draw_original_signal = true;
		gtk_widget_set_sensitive( (GtkWidget*)AD.bSFADApply, TRUE);

		AD.channels_visible_backup.clear();
		if ( gtk_toggle_button_get_active( (GtkToggleButton*)AD.eSFADSingleChannelPreview) )
			for ( auto& H : AD._p.channels ) {
				AD.channels_visible_backup.emplace_back(
					&H, H.hidden);
				if ( &H != SF.using_channel )
					H.hidden = true;
			}

	} else {
		AD._p.using_channel->artifacts = AD.artifacts_backup;
		for ( auto& H : AD.channels_visible_backup )
			H.first->hidden = H.second;
		AD._p.using_channel->draw_original_signal = AD.orig_signal_visible_backup;
		gtk_widget_set_sensitive( (GtkWidget*)AD.bSFADApply, FALSE);
	}

	AD._p.using_channel -> get_signal_filtered();

	snprintf_buf( "%4.2f%% marked", AD._p.using_channel->calculate_dirty_percent() * 100);
	gtk_label_set_markup( AD.lSFADDirtyPercent, __buf__);

	gtk_widget_queue_draw( (GtkWidget*)AD.daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)AD.daSFHypnogram);
}


gboolean
wSFAD_delete_event_cb( GtkWidget*, GdkEvent*, gpointer userdata)
{
	bSFADCancel_clicked_cb( NULL, userdata);
	return TRUE;
}

void
wSFAD_close_cb( GtkWidget*, gpointer userdata)
{
	bSFADCancel_clicked_cb( NULL, userdata);
}

// Local Variables:
// indent-tabs-mode: 8
// End:
