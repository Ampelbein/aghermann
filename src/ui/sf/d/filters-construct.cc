/*
 *       File name:  ui/sf/d/filters-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-10-24
 *
 *         Purpose:  scoring facility Filters construct
 *
 *         License:  GPL
 */

#include <stdexcept>

#include "ui/ui.hh"
#include "filters.hh"

using namespace std;

aghui::SFiltersDialogWidgets::
SFiltersDialogWidgets ()
{
	builder = gtk_builder_new();
	if ( !gtk_builder_add_from_resource( builder, "/org/gtk/aghermann/sf-filters.glade", NULL) )
		throw runtime_error( "Failed to load SF::artifacts glade resource");
	gtk_builder_connect_signals( builder, NULL);

	if ( !AGH_GBGETOBJ (GtkDialog,		wSFFilters) ||
	     !AGH_GBGETOBJ (GtkLabel,		lSFFilterCaption) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFilterLowPassCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFilterLowPassOrder) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFilterHighPassCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFilterHighPassOrder) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eSFFilterNotchFilter) ||
	     !AGH_GBGETOBJ (GtkListStore,	mSFFilterNotchFilter) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFFilterOK) )
		throw runtime_error ("Failed to construct SF widgets (10)");

	gtk_combo_box_set_model_properly(
		eSFFilterNotchFilter, mSFFilterNotchFilter); // can't reuse _p.mNotchFilter

	G_CONNECT_2 (eSFFilterHighPassCutoff, value, changed);
	G_CONNECT_2 (eSFFilterLowPassCutoff, value, changed);
}

aghui::SFiltersDialogWidgets::
~SFiltersDialogWidgets ()
{
	// destroy toplevels
	gtk_widget_destroy( (GtkWidget*)wSFFilters);
	g_object_unref( (GObject*)builder);
}


// Local Variables:
// indent-tabs-mode: 8
// End:

