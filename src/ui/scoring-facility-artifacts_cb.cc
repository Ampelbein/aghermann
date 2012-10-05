// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-artifacts_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-05
 *
 *         Purpose:  scoring facility: artifact detection dialog callbacks
 *
 *         License:  GPL
 */

#include "scoring-facility.hh"
#include "scoring-facility_cb.hh"


using namespace std;
using namespace aghui;



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

	gtk_widget_hide( (GtkWidget*)SF.wSFArtifactDetectionSetup);

	SF.artifacts_backup.clear_all();
}

void
bSFADCancel_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	gtk_widget_hide( (GtkWidget*)SF.wSFArtifactDetectionSetup);

	if ( gtk_toggle_button_get_active(SF.bSFADPreview) ) {
		SF.using_channel -> artifacts = SF.artifacts_backup;
		SF.using_channel -> get_signal_filtered();

		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
	}
	SF.artifacts_backup.clear_all();
}

void
bSFADPreview_toggled_cb( GtkToggleButton *b, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_preview_handler )
		return;

	if ( gtk_toggle_button_get_active(b) ) {
		aghui::SBusyBlock bb (SF.wSFArtifactDetectionSetup);
		SF.artifacts_backup = SF.using_channel->artifacts;
		SF.using_channel -> detect_artifacts(
			SF.get_mc_params_from_SFAD_widgets());
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFADApply, TRUE);
	} else {
		SF.using_channel->artifacts = SF.artifacts_backup;
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFADApply, FALSE);
	}

	SF.using_channel -> get_signal_filtered();
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
}

// eof
