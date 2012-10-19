// ;-*-C++-*-
/*
 *       File name:  ui/sf-artifacts_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-05
 *
 *         Purpose:  scoring facility: artifact detection dialog callbacks
 *
 *         License:  GPL
 */

#include "sf.hh"
#include "sf_cb.hh"


using namespace std;
using namespace aghui;



void
eSFADProfiles_changed_cb( GtkComboBox* b, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& AD = SF.artifact_detection_dialog;

	AD.P = SF._p.global_artifact_detection_profiles[
		gtk_combo_box_get_active_id(b)];
	AD.W_V.up();
}

void
bSFADProfileSave_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& AD = SF.artifact_detection_dialog;

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( SF.wSFADSaveProfileName) ) {
		AD.W_V.down();
		SF._p.global_artifact_detection_profiles[
			gtk_entry_get_text( SF.eSFADSaveProfileNameName)] = AD.P;
		SF._p.populate_mGlobalADProfiles();
		SF.populate_mSFADProfiles(); // stupid
	}
}

void
bSFADProfileDelete_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& AD = SF.artifact_detection_dialog;

	
}


void
eSFADEstimateE_toggled_cb( GtkToggleButton *b, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto state = gtk_toggle_button_get_active( b);
	gtk_widget_set_visible(
		(GtkWidget*)SF.cSFADWhenEstimateEOn,
		state);
	gtk_widget_set_visible(
		(GtkWidget*)SF.cSFADWhenEstimateEOff,
		!state);
}

void
eSFADUseThisRange_toggled_cb( GtkToggleButton *b, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto state = gtk_toggle_button_get_active( b);
	gtk_widget_set_sensitive(
		(GtkWidget*)SF.eSFADHistRangeMin,
		state);
	gtk_widget_set_sensitive(
		(GtkWidget*)SF.eSFADHistRangeMax,
		state);

	// if ( state ) {
	// 	snprintf_buf( "Estimated <i>E</i> = %4.2f",
	// 		      SF.using_channel -> estimate_E( P));
	// }
}



void
bSFADApply_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& AD = SF.artifact_detection_dialog;

	gtk_widget_hide( (GtkWidget*)SF.wSFArtifactDetection);

	for ( auto& H : AD.channels_visible_backup )
		H.first->hidden = H.second;
	AD.channels_visible_backup.clear();

	AD.artifacts_backup.clear_all();
}

void
bSFADCancel_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& AD = SF.artifact_detection_dialog;

	gtk_widget_hide( (GtkWidget*)SF.wSFArtifactDetection);

	if ( gtk_toggle_button_get_active(SF.bSFADPreview) ) {
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
bSFADPreview_toggled_cb( GtkToggleButton *b, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& AD = SF.artifact_detection_dialog;

	if ( AD.suppress_preview_handler )
		return;

	if ( gtk_toggle_button_get_active(b) ) {
		aghui::SBusyBlock bb (SF.wSFArtifactDetection);
		AD.artifacts_backup = SF.using_channel->artifacts;
		SF.using_channel -> detect_artifacts( (AD.W_V.down(), AD.P));
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFADApply, TRUE);

		if ( gtk_toggle_button_get_active( (GtkToggleButton*)SF.eSFADSingleChannelPreview) ) {
			AD.channels_visible_backup.clear();
			for ( auto& H : SF.channels ) {
				AD.channels_visible_backup.emplace_back(
					&H, H.hidden);
				if ( &H != SF.using_channel )
					H.hidden = true;
			}
		}  else
			AD.channels_visible_backup.clear();

	} else {
		SF.using_channel->artifacts = AD.artifacts_backup;
		for ( auto& H : AD.channels_visible_backup )
			H.first->hidden = H.second;
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFADApply, FALSE);
	}

	SF.using_channel -> get_signal_filtered();

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
}

// eof
