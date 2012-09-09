// ;-*-C++-*-
/*
 *       File name:  ui/expdesign_cb.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-25
 *
 *         Purpose:  declarations of expdesign callbacks
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_EXPDESIGN_CB_H
#define _AGH_UI_EXPDESIGN_CB_H

#include <gtk/gtk.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

extern "C" {

gboolean wMainWindow_delete_event_cb( GtkWidget*, GdkEvent*, gpointer);
gboolean wMainWindow_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);

void iExpRefresh_activate_cb( GtkMenuItem*, gpointer);
void iExpPurgeComputed_activate_cb( GtkMenuItem*, gpointer);
void iExpAnnotations_activate_cb( GtkMenuItem*, gpointer);
void iExpClose_activate_cb( GtkMenuItem*, gpointer);
void iExpQuit_activate_cb( GtkMenuItem*, gpointer);
void iMontageResetAll_activate_cb( GtkMenuItem*, gpointer);
void iMontageNotchNone_activate_cb( GtkMenuItem*, gpointer);
void iMontageNotch50Hz_activate_cb( GtkMenuItem*, gpointer);
void iMontageNotch60Hz_activate_cb( GtkMenuItem*, gpointer);
void iHelpAbout_activate_cb( GtkMenuItem*, gpointer);
void iHelpUsage_activate_cb( GtkMenuItem*, gpointer);

void bDownload_clicked_cb( GtkButton*, gpointer);

void bScanTree_clicked_cb( GtkButton*, gpointer);
void eMsmtProfileAutoscale_toggled_cb( GtkToggleButton*, gpointer);
void eMsmtProfileSmooth_value_changed_cb( GtkScaleButton*, gdouble, gpointer);
void eMsmtProfileType_changed_cb( GtkComboBox*, gpointer);
void eMsmtSession_changed_cb( GtkComboBox*, gpointer);
void eMsmtChannel_changed_cb( GtkComboBox*, gpointer);
void eMsmtOpFreqFrom_value_changed_cb( GtkSpinButton*, gpointer);
void eMsmtOpFreqWidth_value_changed_cb( GtkSpinButton*, gpointer);
void eMsmtMCF0_value_changed_cb( GtkSpinButton*, gpointer);

void tvGlobalAnnotations_row_activated_cb( GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);

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
void common_drag_data_received_cb( GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, gpointer);
gboolean common_drag_drop_cb( GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);

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
void eCtlParamAZAmendment2_toggled_cb( GtkToggleButton*, gpointer);

} // extern "C"


#endif

// eof
