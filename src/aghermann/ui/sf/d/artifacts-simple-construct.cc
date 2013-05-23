/*
 *       File name:  aghermann/ui/sf/d/artifacts-simple-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-10-25
 *
 *         Purpose:  scoring facility Artifacts Simple (flat signal detection) construct
 *
 *         License:  GPL
 */

#include <stdexcept>

#include "aghermann/ui/ui.hh"

#include "artifacts-simple.hh"

using namespace std;


aghui::SArtifactsSimpleDialogWidgets::
SArtifactsSimpleDialogWidgets ()
{
	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_resource( builder, "/org/gtk/aghermann/sf-artifacts-simple.glade", NULL) )
		throw runtime_error( "Failed to load SF::artifacts-simple glade resource");
	gtk_builder_connect_signals( builder, NULL);

	if ( !AGH_GBGETOBJ (GtkDialog,		wSFADS) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFADSMinFlatRegionSize) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFADSPad) )
		throw runtime_error ("Failed to construct SF widgets (8)");
}


aghui::SArtifactsSimpleDialogWidgets::
~SArtifactsSimpleDialogWidgets ()
{
	gtk_widget_destroy( (GtkWidget*)wSFADS);
	g_object_unref( (GObject*)builder);
}


// Local Variables:
// indent-tabs-mode: 8
// End:
