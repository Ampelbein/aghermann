/*
 *       File name:  aghermann/ui/sf/d/artifacts.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-24
 *
 *         Purpose:  scoring facility Artifacts dialog
 *
 *         License:  GPL
 */

#ifndef AGH_AGHERMANN_UI_SF_D_ARTIFACTS_H_
#define AGH_AGHERMANN_UI_SF_D_ARTIFACTS_H_

#include <list>

#include <gtk/gtk.h>

#include "libmetrics/mc-artifacts.hh"
#include "libsigfile/source-base.hh"
#include "aghermann/ui/ui++.hh"
#include "aghermann/ui/sf/sf.hh"

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

	SScoringFacility::SChannel
		*using_channel;
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

#endif // _AGH_UI_SF_D_ARTIFACTS_H

// Local Variables:
// indent-tabs-mode: 8
// tab-width: 8
// End:
