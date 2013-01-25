/*
 *       File name:  ui/sf/d/phasediff-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-25
 *
 *         Purpose:  scoring facility Patterns widget construction
 *
 *         License:  GPL
 */

#include <stdexcept>
#include "phasediff.hh"

using namespace std;

aghui::SPhasediffDialogWidgets::
SPhasediffDialogWidgets (SScoringFacility& SF)
{
	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_resource( builder, "/org/gtk/aghermann/sf-patterns.glade", NULL) )
		throw runtime_error( "Failed to load SF::patterns glade resource");
	gtk_builder_connect_signals( builder, NULL);

	if ( !(AGH_GBGETOBJ (GtkDialog,		wSFPD)) ||
	     !(AGH_GBGETOBJ (GtkDrawingArea,	daSFPD)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eSFPDChannelA)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eSFPDChannelB)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFPDFreqFrom)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFPDBandwidth)) ||
	     !(AGH_GBGETOBJ (GtkScaleButton,	eSFPDSmooth)) )
		throw runtime_error ("Failed to construct SF widgets (11)");

	gtk_combo_box_set_model_properly(
		eSFPDChannelA, SF._p.mEEGChannels);
	eSFPDChannelA_changed_cb_handler_id =
		G_CONNECT_1 (eSFPDChannelA, changed);

	gtk_combo_box_set_model_properly( eSFPDChannelB, SF._p.mEEGChannels);
	eSFPDChannelB_changed_cb_handler_id =
		G_CONNECT_1 (eSFPDChannelB, changed);

	G_CONNECT_1 (daSFPD, draw);
	G_CONNECT_2 (daSFPD, scroll, event);
	G_CONNECT_1 (eSFPDChannelA, changed);
	G_CONNECT_1 (eSFPDChannelB, changed);
	G_CONNECT_2 (eSFPDFreqFrom, value, changed);
	G_CONNECT_2 (eSFPDBandwidth, value, changed);
	G_CONNECT_2 (eSFPDSmooth, value, changed);
	G_CONNECT_1 (wSFPD, show);
	G_CONNECT_1 (wSFPD, hide);
}


aghui::SPhasediffDialogWidgets::
~SPhasediffDialogWidgets ()
{
	gtk_widget_destroy( (GtkWidget*)wSFPD);
	g_object_unref( (GObject*)builder);
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
