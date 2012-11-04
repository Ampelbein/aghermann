// ;-*-C++-*-
/*
 *       File name:  ui/mw/mw-splash.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-11-05
 *
 *         Purpose:  SExpDesignUI::try_download and supporting bits
 *
 *         License:  GPL
 */

#include <functional>
#include <stdexcept>

#include <vte/vte.h>

#include "ui/misc.hh"
#include "mw.hh"

using namespace std;




inline namespace {
extern "C" {

void
bDownload_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& ED = *(aghui::SExpDesignUI*)userdata;
	ED.try_download();
}

void
download_process_child_exited_cb( VteTerminal*, gpointer userdata)
{
	auto& ED = *(aghui::SExpDesignUI*)userdata;
	if ( ED.dl_watch_busyblock ) {
		delete ED.dl_watch_busyblock;
		ED.dl_watch_busyblock = nullptr;
		ED.ED->scan_tree( bind (&aghui::SExpDesignUI::sb_main_progress_indicator, &ED,
					placeholders::_1, placeholders::_2, placeholders::_3));
		ED.populate( false);
	} else
		FAFA;
		//throw runtime_error ("Who's here?");
}

} // extern "C"
} // inline namespace




void
aghui::SExpDesignUI::
show_empty_experiment_blurb()
{
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	const char *blurb =
		"<b><big>Empty experiment\n</big></b>\n"
		"When you have your recordings ready as a set of .edf files,\n"
		"• Create your experiment tree as follows: <i>Experiment/Group/Subject/Session</i>;\n"
		"• Have your EDF sources named <i>Episode</i>.edf, and placed in the corresponding <i>Session</i> directory, or\n"
		"• Drag-and-Drop any EDF sources onto this window and identify and place them individually.\n\n"
		"Once set up, either:\n"
		"• select <b>Experiment→Change</b> and select the top directory of the (newly created) experiment tree, or\n"
		"• select <b>Experiment→Rescan Tree</b> if this is the tree you have just populated.\n"
		"\n"
		"Or, If you have none yet, here is a <a href=\"http://johnhommer.com/academic/code/aghermann/Experiment.tar.bz2\">set of EEG data</a>, for a primer;"
		" push the button below to download it into the current directory:";
	GtkLabel *blurb_label = (GtkLabel*)gtk_label_new( "");
	gtk_label_set_markup( blurb_label, blurb);

	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)blurb_label,
			    TRUE, TRUE, 0);
	GtkWidget *bDownload = gtk_button_new_with_label("  Download  ");
	g_object_set( (GObject*)bDownload,
		      "expand", FALSE,
		      "halign", GTK_ALIGN_CENTER,
		      NULL);
	g_signal_connect( bDownload, "clicked",
			  (GCallback)bDownload_clicked_cb,
			  this);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    bDownload,
			    FALSE, FALSE, 0);

	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)gtk_image_new_from_file(
					    PACKAGE_DATADIR "/" PACKAGE "/idle-bg.svg"),
			    TRUE, FALSE, 0);

	gtk_widget_show_all( (GtkWidget*)cMeasurements);
}




int
aghui::SExpDesignUI::
try_download()
{
	gtk_container_foreach( (GtkContainer*)cMeasurements,
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	const char
		*url = "http://johnhommer.com/academic/code/aghermann/Experiment.tar.bz2",
		*archive_file = "Experiment.tar.bz2";
	snprintf_buf( " wget -c \"%s\" && "
		      " tar xjf \"%s\" && "
		      " rm -f \"%s\" && "
		      " echo \"Sample data set downloaded and unpacked\" && "
		      " read -p \"Press <Enter> to close this window...\"",
		      url, archive_file, archive_file);

	auto tTerm = (VteTerminal*)vte_terminal_new();
	// (dl_watch_busyblock = new aghui::SBusyBlock (wMainWindow),
	dl_watch_busyblock = nullptr;
	g_signal_connect( tTerm, "child-exited",
			  (GCallback)download_process_child_exited_cb,
			   this);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)tTerm,
			    TRUE, FALSE, 0);
	// punch a hole for VteTerminal for any user ^C
	gtk_widget_set_sensitive( (GtkWidget*)tTerm, TRUE);
	gtk_widget_show_all( (GtkWidget*)cMeasurements);
	GPid pid;
	GError *Error = NULL;
	char *argv[] = {
		vte_get_user_shell(),
		"-c",
		__buf__,
		NULL
	};
	vte_terminal_fork_command_full(
		tTerm,
		VTE_PTY_DEFAULT,
		ED->session_dir().c_str(),
		argv,
		NULL, // char **envv,
		(GSpawnFlags)G_SPAWN_DO_NOT_REAP_CHILD, // GSpawnFlags spawn_flags,
		NULL, // GSpawnChildSetupFunc child_setup,
		NULL, // gpointer child_setup_data,
		&pid,
		&Error); // GError **error);
	if ( Error ) {
		aghui::pop_ok_message(
			wMainWindow,
			"Failed to download dataset",
			"%s\n", Error->message);
		return 1;
	} else {
		vte_terminal_watch_child( tTerm, pid);
		return 0;
	}
}


// eof
