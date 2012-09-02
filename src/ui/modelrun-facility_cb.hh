// ;-*-C++-*-
/*
 *       File name:  ui/modelrun-facility_cb.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-25
 *
 *         Purpose:  modelrun facility callback functions
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_MODELRUN_FACILITY_CB_H
#define _AGH_UI_MODELRUN_FACILITY_CB_H

#include <gtk/gtk.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


extern "C" {

gboolean daMFProfile_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
gboolean daMFProfile_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daMFProfile_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daMFProfile_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
void eMFSmooth_value_changed_cb( GtkScaleButton*, gdouble, gpointer);
void eMFHighlightNREM_toggled_cb( GtkCheckButton*, gpointer);
void eMFHighlightREM_toggled_cb( GtkCheckButton*, gpointer);
void eMFHighlightWake_toggled_cb( GtkCheckButton*, gpointer);

void eMFClassicFit_toggled_cb( GtkCheckButton*, gpointer);

void bMFRun_clicked_cb( GtkButton*, gpointer);
void bMFReset_clicked_cb( GtkButton*, gpointer);
void bMFAccept_clicked_cb( GtkToolButton*, gpointer);

void eMFDB1_toggled_cb( GtkCheckButton*, gpointer);
void eMFDB2_toggled_cb( GtkCheckButton*, gpointer);
void eMFAZ1_toggled_cb( GtkCheckButton*, gpointer);
void eMFAZ2_toggled_cb( GtkCheckButton*, gpointer);
void eMFVx_value_changed_cb( GtkSpinButton*, gpointer);

gboolean wModelrunFacility_delete_event_cb( GtkWidget*, GdkEvent*, gpointer);

} // extern "C"

#endif // _AGH_UI_MODELRUN_FACILITY_CB_H

// eof
