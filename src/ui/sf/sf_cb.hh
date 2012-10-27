// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf_cb.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-25
 *
 *         Purpose:  forward declarations of SF callbacks
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_SCORING_FACILITY_CB_H
#define _AGH_UI_SCORING_FACILITY_CB_H

#include <gtk/gtk.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

extern "C" {

//gboolean wScoringFacility_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
gboolean daSFMontage_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);

gboolean daSFMontage_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFMontage_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFMontage_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFMontage_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
gboolean daSFMontage_leave_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
gboolean daSFMontage_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

void eSFPageSize_changed_cb( GtkComboBox*, gpointer);
void eSFCurrentPage_value_changed_cb( GtkSpinButton*, gpointer);

void bScoreClear_clicked_cb( GtkButton*, gpointer);
void bScoreNREM1_clicked_cb( GtkButton*, gpointer);
void bScoreNREM2_clicked_cb( GtkButton*, gpointer);
void bScoreNREM3_clicked_cb( GtkButton*, gpointer);
void bScoreNREM4_clicked_cb( GtkButton*, gpointer);
void bScoreREM_clicked_cb  ( GtkButton*, gpointer);
void bScoreWake_clicked_cb ( GtkButton*, gpointer);

void eSFCurrentPos_clicked_cb( GtkButton*, gpointer);
void bSFForward_clicked_cb( GtkButton*, gpointer);
void bSFBack_clicked_cb( GtkButton*, gpointer);
void bScoreGotoPrevUnscored_clicked_cb( GtkButton*, gpointer);
void bScoreGotoNextUnscored_clicked_cb( GtkButton*, gpointer);
void bScoreGotoPrevArtifact_clicked_cb( GtkButton*, gpointer);
void bScoreGotoNextArtifact_clicked_cb( GtkButton*, gpointer);
void bSFDrawCrosshair_toggled_cb( GtkToggleButton*, gpointer);
void bSFShowFindDialog_toggled_cb( GtkToggleButton*, gpointer);
void bSFShowPhaseDiffDialog_toggled_cb( GtkToggleButton*, gpointer);
void bSFRunICA_clicked_cb( GtkButton*, gpointer);
//void bSFResetMontage_clicked_cb( GtkButton*, gpointer);


void eSFICARemixMode_changed_cb( GtkComboBox*, gpointer);
void eSFICANonlinearity_changed_cb( GtkComboBox*, gpointer);
void eSFICAApproach_changed_cb( GtkComboBox*, gpointer);
void eSFICAFineTune_toggled_cb( GtkCheckButton*, gpointer);
void eSFICAStabilizationMode_toggled_cb( GtkCheckButton*, gpointer);
void eSFICAa1_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAa2_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAmu_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAepsilon_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICASampleSizePercent_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICANofICs_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAEigVecFirst_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAEigVecLast_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAMaxIterations_value_changed_cb( GtkSpinButton*, gpointer);
void bSFICATry_clicked_cb( GtkButton*, gpointer);
void bSFICAPreview_toggled_cb( GtkToggleButton*, gpointer);
void bSFICAShowMatrix_toggled_cb( GtkToggleButton*, gpointer);
void wSFICAMatrix_hide_cb( GtkWidget*, gpointer);
void bSFICAApply_clicked_cb( GtkButton*, gpointer);
void bSFICACancel_clicked_cb( GtkButton*, gpointer);


void bSFAccept_clicked_cb( GtkToolButton*, gpointer);
void iSFAcceptAndTakeNext_activate_cb( GtkMenuItem*, gpointer);

void iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageUseResample_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawZeroline_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDetectArtifacts_activate_cb( GtkMenuItem*, gpointer);
void iSFPageClearArtifacts_activate_cb( GtkMenuItem*, gpointer);
void iSFPageFilter_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSaveChannelAsSVG_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSaveMontageAsSVG_activate_cb( GtkMenuItem*, gpointer);
void iSFPageExportSignal_activate_cb( GtkMenuItem*, gpointer);
void iSFPageUseThisScale_activate_cb( GtkMenuItem*, gpointer);
void iSFPageClearArtifacts_activate_cb( GtkMenuItem*, gpointer);
void iSFPageHide_activate_cb( GtkMenuItem*, gpointer);
void iSFPageHidden_select_cb( GtkMenuItem*, gpointer);
void iSFPageHidden_deselect_cb( GtkMenuItem*, gpointer);
void iSFPageShowHidden_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSpaceEvenly_activate_cb( GtkMenuItem*, gpointer);
void iSFPageLocateSelection_activate_cb( GtkMenuItem*, gpointer);
void iSFPageDrawPSDProfile_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawPSDSpectrum_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawMCProfile_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawEMGProfile_toggled_cb( GtkCheckMenuItem*, gpointer);

void iSFPageSelectionDrawCourse_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageSelectionDrawEnvelope_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageSelectionDrawDzxdf_toggled_cb( GtkCheckMenuItem*, gpointer);


gboolean wSFArtifactDetection_delete_event_cb(GtkWidget*, GdkEvent*, gpointer);
void wSFArtifactDetection_close_cb(GtkWidget*, gpointer);
void eSFADProfiles_changed_cb( GtkComboBox*, gpointer);
void bSFADProfileSave_clicked_cb( GtkButton*, gpointer);
void bSFADProfileDelete_clicked_cb( GtkButton*, gpointer);
void eSFADEstimateE_toggled_cb( GtkToggleButton*, gpointer);
void eSFADUseThisRange_toggled_cb( GtkToggleButton*, gpointer);
void bSFADPreview_toggled_cb( GtkToggleButton*, gpointer);
void bSFADApply_clicked_cb( GtkButton*, gpointer);
void bSFADCancel_clicked_cb( GtkButton*, gpointer);

void iSFICAPageMapIC_activate_cb( GtkRadioMenuItem*, gpointer);

void iSFPageAnnotationDelete_activate_cb( GtkMenuItem*, gpointer);
void iSFPageAnnotationEdit_activate_cb( GtkMenuItem*, gpointer);

void iSFPageSelectionMarkArtifact_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSelectionClearArtifact_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSelectionFindPattern_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSelectionAnnotate_activate_cb( GtkMenuItem*, gpointer);

void iSFPowerExportRange_activate_cb( GtkMenuItem*, gpointer);
void iSFPowerExportAll_activate_cb( GtkMenuItem*, gpointer);
void iSFPowerSmooth_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPowerDrawBands_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPowerUseThisScale_activate_cb( GtkMenuItem*, gpointer);
void iSFPowerAutoscale_toggled_cb( GtkCheckMenuItem*, gpointer);

gboolean daSFHypnogram_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFHypnogram_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFHypnogram_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFHypnogram_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);

void iSFScoreAssist_activate_cb( GtkMenuItem*, gpointer);
void iSFScoreImport_activate_cb( GtkMenuItem*, gpointer);
void iSFScoreExport_activate_cb( GtkMenuItem*, gpointer);
void iSFScoreClear_activate_cb( GtkMenuItem*, gpointer);

void ePatternList_changed_cb( GtkComboBox*, gpointer);
void ePatternChannel_changed_cb( GtkComboBox*, gpointer);
gboolean daPatternSelection_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daPatternSelection_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
void bPatternFind_clicked_cb( GtkButton*, gpointer);
void bPatternSave_clicked_cb( GtkButton*, gpointer);
void bPatternDiscard_clicked_cb( GtkButton*, gpointer);
void ePattern_any_value_changed_cb( GtkSpinButton*, gpointer);
void wPattern_show_cb( GtkWidget*, gpointer);
void wPattern_hide_cb( GtkWidget*, gpointer);

void eFilterHighPassCutoff_value_changed_cb( GtkSpinButton*, gpointer);
void eFilterLowPassCutoff_value_changed_cb( GtkSpinButton*, gpointer);

void eSFPDChannelA_changed_cb( GtkComboBox*, gpointer);
void eSFPDChannelB_changed_cb( GtkComboBox*, gpointer);
gboolean daSFPD_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFPD_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
void eSFPDChannelA_changed_cb( GtkComboBox*, gpointer);
void eSFPDChannelB_changed_cb( GtkComboBox*, gpointer);
void eSFPDFreqFrom_value_changed_cb( GtkSpinButton*, gpointer);
void eSFPDBandwidth_value_changed_cb( GtkSpinButton*, gpointer);
void eSFPDSmooth_value_changed_cb( GtkScaleButton*, gdouble, gpointer);
void wSFPD_show_cb( GtkWidget*, gpointer);
void wSFPD_hide_cb( GtkWidget*, gpointer);

gboolean wScoringFacility_delete_event_cb( GtkWidget*, GdkEvent*, gpointer);

} // extern "C"

#endif // _AGH_UI_SCORING_FACILITY_CB_H

// eof