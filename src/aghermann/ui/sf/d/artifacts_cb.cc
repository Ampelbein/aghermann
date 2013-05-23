/*
 *       File name:  aghermann/ui/sf/d/artifacts_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-05
 *
 *         Purpose:  scoring facility: artifact detection dialog callbacks
 *
 *         License:  GPL
 */

#include "aghermann/ui/misc.hh"

#include "artifacts.hh"

using namespace std;
using namespace aghui;



void
eSFADProfiles_changed_cb(
	GtkComboBox* w,
	const gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;
	auto& SF = AD._p;

	if ( gtk_combo_box_get_active( w) != -1 ) {
		AD.P = SF._p.global_artifact_detection_profiles[
			gtk_combo_box_get_active_id(w)];
		AD.W_V.up();
		gtk_widget_set_sensitive( (GtkWidget*)AD.bSFADProfileDelete, TRUE);
	} else
		gtk_widget_set_sensitive( (GtkWidget*)AD.bSFADProfileDelete, FALSE);
}

void
bSFADProfileSave_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;
	auto& SF = AD._p;

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( AD.wSFADSaveProfileName) ) {
		AD.W_V.down();
		SF._p.global_artifact_detection_profiles[
			gtk_entry_get_text( AD.eSFADSaveProfileNameName)] = AD.P;

		SF._p.populate_mGlobalADProfiles();
		AD.populate_mSFADProfiles(); // stupid

		int now_active = SF._p.global_artifact_detection_profiles.size()-1;
		gtk_combo_box_set_active( AD.eSFADProfiles, now_active);
		gtk_combo_box_set_active( SF._p.eGlobalADProfiles, now_active);
	}
}

void
bSFADProfileDelete_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;
	auto& SF = AD._p;

	const gchar *deleting_id = gtk_combo_box_get_active_id( AD.eSFADProfiles);
	int deleting = gtk_combo_box_get_active( AD.eSFADProfiles);
	SF._p.global_artifact_detection_profiles.erase( deleting_id);

	SF._p.populate_mGlobalADProfiles();
	AD.populate_mSFADProfiles(); // stupid

	if ( SF._p.global_artifact_detection_profiles.size() > 0 &&
	     deleting > (int)SF._p.global_artifact_detection_profiles.size()-1 )
		gtk_combo_box_set_active( AD.eSFADProfiles, deleting-1);

	g_signal_emit_by_name( AD.eSFADProfiles, "changed");
}


void
eSFADEstimateE_toggled_cb(
	GtkToggleButton *b,
	const gpointer userdata)
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
eSFADUseThisRange_toggled_cb(
	GtkToggleButton *b,
	const gpointer userdata)
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
bSFADApply_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;
	auto& SF = AD._p;

	gtk_widget_hide( (GtkWidget*)AD.wSFAD);

	for ( auto& H : AD.channels_visible_backup )
		H.first->hidden = H.second;
	AD.channels_visible_backup.clear();
	AD.artifacts_backup.clear_all();

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
}

void
bSFADCancel_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;
	auto& SF = AD._p;

	gtk_widget_hide( (GtkWidget*)AD.wSFAD);

	if ( gtk_toggle_button_get_active(AD.bSFADPreview) ) {
		SF.using_channel -> artifacts = AD.artifacts_backup;
		SF.using_channel -> get_signal_filtered();

		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
	}

	for ( auto& H : AD.channels_visible_backup )
		H.first->hidden = H.second;
	AD.channels_visible_backup.clear();
	AD.artifacts_backup.clear_all();
}

void
bSFADPreview_toggled_cb(
	GtkToggleButton *b,
	const gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;
	auto& SF = AD._p;

	if ( AD.suppress_preview_handler )
		return;

	if ( gtk_toggle_button_get_active(b) ) {
		SBusyBlock bb (AD.wSFAD);

		AD.orig_signal_visible_backup = SF.using_channel->draw_original_signal;
		AD.artifacts_backup = SF.using_channel->artifacts;

		SF.using_channel -> detect_artifacts( (AD.W_V.down(), AD.P));
		SF.using_channel -> draw_original_signal = true;
		gtk_widget_set_sensitive( (GtkWidget*)AD.bSFADApply, TRUE);

		AD.channels_visible_backup.clear();
		if ( gtk_toggle_button_get_active( (GtkToggleButton*)AD.eSFADSingleChannelPreview) )
			for ( auto& H : SF.channels ) {
				AD.channels_visible_backup.emplace_back(
					&H, H.hidden);
				if ( &H != SF.using_channel )
					H.hidden = true;
			}

	} else {
		SF.using_channel->artifacts = AD.artifacts_backup;
		for ( auto& H : AD.channels_visible_backup )
			H.first->hidden = H.second;
		SF.using_channel->draw_original_signal = AD.orig_signal_visible_backup;
		gtk_widget_set_sensitive( (GtkWidget*)AD.bSFADApply, FALSE);
	}

	SF.using_channel -> get_signal_filtered();

	snprintf_buf( "%4.2f%% marked", SF.using_channel->calculate_dirty_percent() * 100);
	gtk_label_set_markup( AD.lSFADDirtyPercent, __buf__);

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
}



void
wSFAD_show_cb(
	GtkWidget*,
	const gpointer userdata)
{
	auto& AD = *(SScoringFacility::SArtifactsDialog*)userdata;
	auto& SF = AD._p;

	AD.W_V.up();
	AD.populate_mSFADProfiles();
	g_signal_emit_by_name( AD.eSFADProfiles, "changed");

	g_signal_emit_by_name( AD.eSFADEstimateE, "toggled");
	g_signal_emit_by_name( AD.eSFADEstimateE, "toggled");
	g_signal_emit_by_name( AD.eSFADUseThisRange, "toggled");
	g_signal_emit_by_name( AD.eSFADUseThisRange, "toggled");

	gtk_widget_set_sensitive( (GtkWidget*)AD.bSFADApply, FALSE);
	AD.suppress_preview_handler = true;
	gtk_toggle_button_set_active( AD.bSFADPreview, FALSE);
	AD.suppress_preview_handler = false;

	snprintf_buf( "Artifact detection in channel %s", SF.using_channel->name());
	gtk_label_set_text( AD.lSFADInfo, __buf__);
	snprintf_buf( "%4.2f%% marked", SF.using_channel->calculate_dirty_percent() * 100);
	gtk_label_set_text( AD.lSFADDirtyPercent, __buf__);
}


gboolean
wSFAD_delete_event_cb(
	GtkWidget*,
	GdkEvent*,
	const gpointer userdata)
{
	bSFADCancel_clicked_cb( NULL, userdata);
	return TRUE;
}

void
wSFAD_close_cb(
	GtkWidget*,
	const gpointer userdata)
{
	bSFADCancel_clicked_cb( NULL, userdata);
}

// Local Variables:
// indent-tabs-mode: 8
// End:
