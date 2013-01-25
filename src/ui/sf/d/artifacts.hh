/*
 *       File name:  ui/sf/d/artifacts.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-24
 *
 *         Purpose:  scoring facility Artifacts dialog
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_SF_ARTIFACTS_H
#define _AGH_UI_SF_ARTIFACTS_H

#include <list>

#include <gtk/gtk.h>

#include "metrics/mc-artifacts.hh"
#include "libsigfile/source-base.hh"
#include "ui/ui++.hh"
#include "ui/sf/sf.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace aghui {

struct SArtifactsDialogWidgets {

	SArtifactsDialogWidgets ();
       ~SArtifactsDialogWidgets ();

	GtkBuilder *builder;

	GtkDialog
		*wSFAD;
	GtkListStore
		*mSFADProfiles;
	GtkComboBox
		*eSFADProfiles;
	gulong	eSFADProfiles_changed_cb_handler_id;
	GtkButton
		*bSFADProfileSave,
		*bSFADProfileDelete;
	GtkSpinButton
		*eSFADUpperThr,
		*eSFADLowerThr,
		*eSFADScope,
		*eSFADF0,
		*eSFADFc,
		*eSFADBandwidth,
		*eSFADMCGain,
		*eSFADBackpolate,
		*eSFADEValue,
		*eSFADHistRangeMin,
		*eSFADHistRangeMax,
		*eSFADHistBins,
		*eSFADSmoothSide;
	GtkCheckButton
		*eSFADEstimateE,
		*eSFADSingleChannelPreview;
	GtkRadioButton
		*eSFADUseThisRange,
		*eSFADUseComputedRange;
	GtkTable
		*cSFADWhenEstimateEOn,
		*cSFADWhenEstimateEOff;
	GtkLabel
		*lSFADInfo,
		*lSFADDirtyPercent;
	GtkToggleButton
		*bSFADPreview;
	GtkButton
		*bSFADApply,
		*bSFADCancel;
	GtkDialog
		*wSFADSaveProfileName;
	GtkEntry
		*eSFADSaveProfileNameName;
};

struct SScoringFacility::SArtifactsDialog
    : public SArtifactsDialogWidgets {

	DELETE_DEFAULT_METHODS (SArtifactsDialog);

	SArtifactsDialog (SScoringFacility&);

	metrics::mc::SArtifactDetectionPP
		P;
	sigfile::SArtifacts
		artifacts_backup;
	bool	orig_signal_visible_backup;
	list<pair<SScoringFacility::SChannel*, bool>>
		channels_visible_backup;
	bool	suppress_preview_handler;
	SUIVarCollection
		W_V;

	SScoringFacility&
		_p;

	void populate_mSFADProfiles();
};


} // namespace aghui


extern "C" {
gboolean wSFAD_delete_event_cb(GtkWidget*, GdkEvent*, gpointer);
void wSFAD_close_cb(GtkWidget*, gpointer);
void wSFAD_show_cb(GtkWidget*, gpointer);
void eSFADProfiles_changed_cb( GtkComboBox*, gpointer);
void bSFADProfileSave_clicked_cb( GtkButton*, gpointer);
void bSFADProfileDelete_clicked_cb( GtkButton*, gpointer);
void eSFADEstimateE_toggled_cb( GtkToggleButton*, gpointer);
void eSFADUseThisRange_toggled_cb( GtkToggleButton*, gpointer);
void bSFADPreview_toggled_cb( GtkToggleButton*, gpointer);
void bSFADApply_clicked_cb( GtkButton*, gpointer);
void bSFADCancel_clicked_cb( GtkButton*, gpointer);
}

#endif // _AGH_UI_SF_ARTIFACTS_H

// Local Variables:
// indent-tabs-mode: 8
// End:
