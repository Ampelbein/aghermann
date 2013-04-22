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




namespace {
extern "C" {

void
bDownload_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& ED = *(aghui::SExpDesignUI*)userdata;
	ED.try_download();
}

void
download_process_child_exited_cb(
	VteTerminal *terminal,
	const gpointer userdata)
{
	auto& ED = *(aghui::SExpDesignUI*)userdata;
	ED.set_wMainWindow_interactive( true, true);
	int exit_status = vte_terminal_get_child_exit_status( terminal);
	if ( exit_status != 0 )
		aghui::pop_ok_message(
			ED.wMainWindow,
			"Download failed",
			"Exit status %d. Try again next time.", exit_status);
	ED.dl_pid = -1;
	ED.ED->scan_tree( bind (&aghui::SExpDesignUI::sb_main_progress_indicator, &ED,
				placeholders::_1, placeholders::_2, placeholders::_3,
				aghui::TGtkRefreshMode::gdk));
	ED.populate( false);
}

} // extern "C"
} // namespace




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
		"• Have your EDF sources named <i>Episode</i>.edf, and placed in the corresponding <i>Session</i> directory.\n"
		"\n"
		"Once set up, either:\n"
		"• select <b>Experiment→Close</b> and then select in the Session Manager the top directory of the newly created experiment tree, or\n"
		"• select <b>Experiment→Rescan Tree</b> if this is the tree you have just populated.\n"
		"\n"
		"Alternatively, <b>Drag-and-Drop</b> any EDF sources onto this window and identify and place them individually.\n"
		"\n"
		"Or, if you have none yet, here is a <a href=\"http://johnhommer.com/academic/code/aghermann/Experiment.tar.bz2\">set of EEG data</a>, for a primer;"
		" push the button below to download it into the current directory:";
	GtkLabel *blurb_label = (GtkLabel*)gtk_label_new( "");
	gtk_label_set_markup( blurb_label, blurb);

	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)blurb_label,
			    TRUE, TRUE, 0);
	GtkWidget *bDownload = gtk_button_new_with_label("  Get sample dataset  ");
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
	auto tTerm = (VteTerminal*)vte_terminal_new();
	g_signal_connect( tTerm, "child-exited",
			  (GCallback)download_process_child_exited_cb,
			   this);
	gtk_box_pack_start( (GtkBox*)cMeasurements,
			    (GtkWidget*)tTerm,
			    TRUE, FALSE, 0);
	set_wMainWindow_interactive( false, true);
	// punch a hole for VteTerminal for any user ^C
	gtk_widget_set_sensitive( (GtkWidget*)cMeasurements, TRUE);
	gtk_widget_show_all( (GtkWidget*)cMeasurements);
	gtk_widget_grab_focus( (GtkWidget*)tTerm);
	GError *Error = NULL;
	const char *argv[] = {
		"/bin/sh", // vte_get_user_shell(),
		"-c",
		"source " PACKAGE_DATADIR "/" PACKAGE "/experiment-dl.sh",
		NULL
	};
	vte_terminal_fork_command_full(
		tTerm,
		VTE_PTY_DEFAULT,
		ED->session_dir().c_str(),
		const_cast<char**> (argv),
		NULL, // char **envv,
		(GSpawnFlags)G_SPAWN_DO_NOT_REAP_CHILD, // GSpawnFlags spawn_flags,
		NULL, // GSpawnChildSetupFunc child_setup,
		NULL, // gpointer child_setup_data,
		&dl_pid,
		&Error); // GError **error);
	if ( Error ) {
		aghui::pop_ok_message(
			wMainWindow,
			"Error",
			"%s\n", Error->message);
		return 1;
	} else {
		vte_terminal_watch_child( tTerm, dl_pid);
		return 0;
	}
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
