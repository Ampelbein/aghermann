// ;-*-C++-*-
/*
 *       File name:  ui/session-chooser_cb.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-09
 *
 *         Purpose:  declarations of session chooser callbacks
 *
 *         License:  GPL
 */

#ifndef _AGHUI_SESSION_CHOOSER_CB_H
#define _AGHUI_SESSION_CHOOSER_CB_H

#include <gtk/gtk.h>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

extern "C" {

void wSessionChooser_show_cb( GtkWidget*, gpointer);
void wSessionChooser_destroy_cb( GtkWidget*, gpointer);

void tvSessionChooserList_changed_cb( GtkTreeSelection*, gpointer);
void tvSessionChooserList_row_activated_cb( GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);

void bSessionChooserOpen_clicked_cb( GtkButton*, gpointer);
void bSessionChooserClose_clicked_cb( GtkButton*, gpointer); // hidden
void bSessionChooserQuit_clicked_cb( GtkButton*, gpointer);
void bSessionChooserCreateNew_clicked_cb( GtkButton*, gpointer);
void bSessionChooserRemove_clicked_cb( GtkButton*, gpointer);

} // extern "C"


#endif // _AGH_UI_SESSION_CHOOSER_CB_H

// eof
