/*
 *       File name:  aghermann/ui/sf/d/artifacts-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-10-24
 *
 *         Purpose:  scoring facility Artifacts construct
 *
 *         License:  GPL
 */

#include <stdexcept>

#include "aghermann/ui/ui.hh"

#include "artifacts.hh"

using namespace std;
using namespace agh::ui;

SArtifactsDialogWidgets::
SArtifactsDialogWidgets ()
{
	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_resource( builder, "/org/gtk/aghermann/sf-artifacts.glade", NULL) )
		throw runtime_error( "Failed to load SF::artifacts glade resource");
	gtk_builder_connect_signals( builder, NULL);

	if ( !(AGH_GBGETOBJ (GtkDialog,			wSFAD)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,		eSFADProfiles)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADProfileSave)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADProfileDelete)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADScope)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADUpperThr)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADLowerThr)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADF0)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADFc)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADBandwidth)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADMCGain)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADBackpolate)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADEValue)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADHistRangeMin)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADHistRangeMax)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADHistBins)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,		eSFADSmoothSide)) ||
	     !(AGH_GBGETOBJ (GtkCheckButton,		eSFADSingleChannelPreview)) ||
	     !(AGH_GBGETOBJ (GtkCheckButton,		eSFADEstimateE)) ||
	     !(AGH_GBGETOBJ (GtkRadioButton,		eSFADUseThisRange)) ||
	     !(AGH_GBGETOBJ (GtkRadioButton,		eSFADUseComputedRange)) ||
	     !(AGH_GBGETOBJ (GtkTable,			cSFADWhenEstimateEOn)) ||
	     !(AGH_GBGETOBJ (GtkTable,			cSFADWhenEstimateEOff)) ||
	     !(AGH_GBGETOBJ (GtkLabel,			lSFADInfo)) ||
	     !(AGH_GBGETOBJ (GtkLabel,			lSFADDirtyPercent)) ||
	     !(AGH_GBGETOBJ (GtkToggleButton,		bSFADPreview)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADApply)) ||
	     !(AGH_GBGETOBJ (GtkButton,			bSFADCancel)) ||
	     !(AGH_GBGETOBJ (GtkDialog,			wSFADSaveProfileName)) ||
	     !(AGH_GBGETOBJ (GtkEntry,			eSFADSaveProfileNameName)) )
		throw runtime_error ("Failed to construct SF widgets (7)");

	mSFADProfiles = gtk_list_store_new( 1, G_TYPE_STRING);
	// this GtkListStore is populated from the same source, but something
	// haunting GTK+ forbids reuse of _p.mGlobalArtifactDetectionProfiles
	gtk_combo_box_set_model_properly( eSFADProfiles, mSFADProfiles);

	G_CONNECT_1 (wSFAD, show);
	G_CONNECT_1 (wSFAD, close);
	G_CONNECT_2 (wSFAD, delete, event);
	eSFADProfiles_changed_cb_handler_id =
		G_CONNECT_1 (eSFADProfiles, changed);
	G_CONNECT_1 (bSFADProfileSave, clicked);
	G_CONNECT_1 (bSFADProfileDelete, clicked);
	G_CONNECT_1 (eSFADEstimateE, toggled);
	G_CONNECT_1 (eSFADUseThisRange, toggled);
	G_CONNECT_1 (bSFADPreview, toggled);
	G_CONNECT_1 (bSFADApply, clicked);
	G_CONNECT_1 (bSFADCancel, clicked);
}


SArtifactsDialogWidgets::
~SArtifactsDialogWidgets ()
{
	gtk_widget_destroy( (GtkWidget*)wSFAD);
	g_object_unref( (GObject*)builder);
}


// Local Variables:
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
