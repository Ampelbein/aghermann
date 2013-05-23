/*
 *       File name:  aghermann/ui/sf/d/artifacts-simple.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-24
 *
 *         Purpose:  scoring facility Artifacts Simple (flat signal detection) dialog
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_SF_ARTIFACTS_SIMPLE_H
#define _AGH_UI_SF_ARTIFACTS_SIMPLE_H

#include <gtk/gtk.h>

#include "aghermann/ui/ui++.hh"
#include "aghermann/ui/sf/sf.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace aghui {

struct SArtifactsSimpleDialogWidgets {

	SArtifactsSimpleDialogWidgets ();
       ~SArtifactsSimpleDialogWidgets ();

	GtkBuilder *builder;

	GtkDialog
		*wSFADS;
	GtkSpinButton
		*eSFADSMinFlatRegionSize,
		*eSFADSPad;
};

struct SScoringFacility::SArtifactsSimpleDialog
  : public SArtifactsSimpleDialogWidgets {

	DELETE_DEFAULT_METHODS (SArtifactsSimpleDialog);

	SArtifactsSimpleDialog (SScoringFacility&);

	double	min_size,
		pad;

	SUIVarCollection
		W_V;

	SScoringFacility&
		_p;
};

} // namespace aghui

#endif // _AGH_UI_SF_FILTERS_H

// Local Variables:
// indent-tabs-mode: 8
// End:
