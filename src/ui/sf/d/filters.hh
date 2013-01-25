/*
 *       File name:  ui/sf/d/filters.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-24
 *
 *         Purpose:  scoring facility Filters dialog
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_SF_FILTERS_H
#define _AGH_UI_SF_FILTERS_H

#include <gtk/gtk.h>

#include "ui/ui++.hh"
#include "ui/sf/sf.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace aghui {

struct SFiltersDialogWidgets;

struct SScoringFacility::SFiltersDialog
  : public SFiltersDialogWidgets {

	DELETE_DEFAULT_METHODS (SFiltersDialog);

	SFiltersDialog (SScoringFacility&);
       ~SFiltersDialog ();

	sigfile::SFilterPack&
		P;

	SUIVarCollection
		W_V;

	SScoringFacility&
		_p;
};


struct SFiltersDialogWidgets {

	SFiltersDialogWidgets ();
       ~SFiltersDialogWidgets ();

	GtkBuilder *builder;

	GtkDialog
		*wSFFilters;
	GtkLabel
		*lSFFilterCaption;
	GtkSpinButton
		*eSFFilterLowPassCutoff, *eSFFilterHighPassCutoff,
		*eSFFilterLowPassOrder, *eSFFilterHighPassOrder;
	GtkComboBox
		*eSFFilterNotchFilter;
	GtkListStore
		*mSFFilterNotchFilter;
	GtkButton
		*bSFFilterOK;
};

extern "C" {
void eSFFilterHighPassCutoff_value_changed_cb( GtkSpinButton*, gpointer);
void eSFFilterLowPassCutoff_value_changed_cb( GtkSpinButton*, gpointer);
}

// Local Variables:
// indent-tabs-mode: 8
// End:
