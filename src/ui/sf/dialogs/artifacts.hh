/*
 *       File name:  ui/sf/dialogs/artifacts.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-24
 *
 *         Purpose:  scoring facility Artifacts
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_SF_ARTIFACTS_H
#define _AGH_UI_SF_ARTIFACTS_H

#include <gtk/gtk.h>
#include "patterns/patterns.hh"
#include "ui/forward-decls.hh"
#include "ui/sf/sf.hh"

namespace aghui {

struct SArtifactsDialogWidgets;
struct SArtifactsDialog
    : public SArtifactsDialog {
	DELETE_DEFAULT_METHODS (SArtifactDetectionDialog);

	SArtifactsDialog (SScoringFacility&);
       ~SArtifactsDialog ();

	metrics::mc::SArtifactDetectionPP
		P;
	sigfile::SArtifacts
		artifacts_backup;
	bool	orig_signal_visible_backup;
	list<pair<SChannel*, bool>>
		channels_visible_backup;
	bool	suppress_preview_handler;
	SUIVarCollection
		W_V;

	SScoringFacility&
		_p;
};

struct SArtifactsDialogWidgets {
	DELETE_DEFAULT_METHODS (SArtifactsDialogWidgets);

	SArtifactsDialogWidgets (SScoringFacility&);
       ~SArtifactsDialogWidgets ();

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

} // namespace aghui

