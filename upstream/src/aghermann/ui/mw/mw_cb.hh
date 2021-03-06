/*
 *       File name:  aghermann/ui/mw/mw_cb.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-25
 *
 *         Purpose:  declarations of ed callbacks
 *
 *         License:  GPL
 */

#ifndef AGH_AGHERMANN_UI_MW_MW_CB_H_
#define AGH_AGHERMANN_UI_MW_MW_CB_H_

#include <gtk/gtk.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

extern "C" {

gboolean wMainWindow_delete_event_cb( GtkWidget*, GdkEvent*, gpointer);
gboolean wMainWindow_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
gboolean wMainWindow_key_press_event_cb( GtkWidget*, GdkEventKey*, gpointer);

void iExpRefresh_activate_cb( GtkMenuItem*, gpointer);
void iExpPurgeComputed_activate_cb( GtkMenuItem*, gpointer);
void iExpAnnotations_activate_cb( GtkMenuItem*, gpointer);
void iExpSubjectSortAny_toggled_cb( GtkCheckMenuItem*, gpointer);
void iExpSubjectSortAscending_toggled_cb( GtkCheckMenuItem*, gpointer);
void iExpSubjectSortSegregate_toggled_cb( GtkCheckMenuItem*, gpointer);
void iExpBasicSADetectUltradianCycles_activate_cb( GtkMenuItem*, gpointer);
void iExpGloballyDetectArtifacts_activate_cb( GtkMenuItem*, gpointer);
void iExpGloballySetFilters_activate_cb( GtkMenuItem*, gpointer);
void iExpClose_activate_cb( GtkMenuItem*, gpointer);
void iExpQuit_activate_cb( GtkMenuItem*, gpointer);
// void iMontageSetDefaults_activate_cb( GtkMenuItem*, gpointer);
void iHelpAbout_activate_cb( GtkMenuItem*, gpointer);
void iHelpUsage_activate_cb( GtkMenuItem*, gpointer);

void eGlobalADProfiles_changed_cb( GtkComboBox*, gpointer);
void bGlobalMontageResetAll_clicked_cb( GtkButton*, gpointer);

void bDownload_clicked_cb( GtkButton*, gpointer);

void bScanTree_clicked_cb( GtkButton*, gpointer);
void eMsmtProfileAutoscale_toggled_cb( GtkToggleButton*, gpointer);
void eMsmtProfileSmooth_value_changed_cb( GtkScaleButton*, gdouble, gpointer);
void eMsmtProfileType_changed_cb( GtkComboBox*, gpointer);
void eMsmtSession_changed_cb( GtkComboBox*, gpointer);
void eMsmtChannel_changed_cb( GtkComboBox*, gpointer);
void eMsmtProfileParamsPSDFreqFrom_value_changed_cb( GtkSpinButton*, gpointer);
void eMsmtProfileParamsPSDFreqWidth_value_changed_cb( GtkSpinButton*, gpointer);
void eMsmtProfileParamsSWUF0_value_changed_cb( GtkSpinButton*, gpointer);
void eMsmtProfileParamsMCF0_value_changed_cb( GtkSpinButton*, gpointer);

void tvGlobalAnnotations_row_activated_cb( GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);
void eGlobalAnnotationsShowPhasicEvents_toggled_cb( GtkToggleButton*, gpointer);

void cGroupExpander_activate_cb( GtkExpander*, gpointer);
void iiSubjectTimeline_show_cb( GtkWidget*, gpointer);
void iSubjectTimelineScore_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineDetectUltradianCycle_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineSubjectInfo_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineEDFInfo_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineSaveAsSVG_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineBrowse_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineResetMontage_activate_cb( GtkMenuItem*, gpointer);

gboolean daSubjectTimeline_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSubjectTimeline_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
gboolean daSubjectTimeline_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSubjectTimeline_enter_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
gboolean daSubjectTimeline_leave_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
gboolean daSubjectTimeline_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);

void tvSimulations_row_activated_cb( GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);
void iSimulationsRunBatch_activate_cb( GtkMenuItem*, gpointer);
void iSimulationsRunClearAll_activate_cb( GtkMenuItem*, gpointer);
void iSimulationsReportGenerate_activate_cb( GtkMenuItem*, gpointer);

gboolean check_gtk_entry_nonempty_cb( GtkEditable*, gpointer);

void cMeasurements_drag_data_received_cb( GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, gpointer);
gboolean cMeasurements_drag_drop_cb( GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);

void tTaskSelector_switch_page_cb( GtkNotebook*, gpointer, guint, gpointer);
void tDesign_switch_page_cb( GtkNotebook*, gpointer, guint, gpointer);
void tSimulations_switch_page_cb( GtkNotebook*, gpointer, guint, gpointer);
void bSimParamRevertTunables_clicked_cb( GtkButton*, gpointer);

void bColourX_color_set_cb( GtkColorButton*, gpointer);

void wExpDesignChooser_show_cb( GtkWidget*, gpointer);
void wExpDesignChooser_hide_cb( GtkWidget*, gpointer);
void wExpDesignChooserChooser_hide_cb( GtkWidget*, gpointer);
void tvExpDesignChooserList_changed_cb( GtkTreeSelection*, gpointer);
void tvExpDesignChooserList_row_activated_cb( GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);
void bExpDesignChooserSelect_clicked_cb( GtkButton*, gpointer);
void bExpDesignChooserQuit_clicked_cb( GtkButton*, gpointer);
void bExpDesignChooserCreateNew_clicked_cb( GtkButton*, gpointer);
void bExpDesignChooserRemove_clicked_cb( GtkButton*, gpointer);

void eCtlParamDBAmendment1_toggled_cb( GtkToggleButton*, gpointer);
void eCtlParamDBAmendment2_toggled_cb( GtkToggleButton*, gpointer);
void eCtlParamAZAmendment1_toggled_cb( GtkToggleButton*, gpointer);

void bMainCloseThatSF_clicked_cb( GtkButton*, gpointer);

} // extern "C"


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:

