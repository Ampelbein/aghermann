/*
 *       File name:  aghermann/ui/sm/sm_cb.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-09
 *
 *         Purpose:  declarations of session chooser callbacks
 *
 *         License:  GPL
 */

#ifndef AGH_AGHEMANN_UI_SM_SM_CB_H_
#define AGH_AGHEMANN_UI_SM_SM_CB_H_

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


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
