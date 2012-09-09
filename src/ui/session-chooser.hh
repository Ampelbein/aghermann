// ;-*-C++-*-
/*
 *       File name:  ui/session-chooser.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-13
 *
 *         Purpose:  session chooser
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_SESSION_CHOOSER_H
#define _AGH_UI_SESSION_CHOOSER_H

#include <string>
#include <list>
#include <gtk/gtk.h>
#include "../common/misc.hh"
#include "forward-decls.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;



namespace aghui {

struct SSession : public string {
	size_t	n_groups,
		n_subjects;
	// possibly something else
	SSession (string dir_)
	      : string (dir_),
		n_groups (-1), n_subjects (-1)
		{
			//get_session_stats();
		}
	void get_session_stats();
};


struct SSessionChooser {
	DELETE_DEFAULT_METHODS (SSessionChooser);

	SSessionChooser (const char*, GtkWindow** single_main_window_);
       ~SSessionChooser ();

	string	title;
	string	hist_filename;
	int	last_dir_no;
	list<SSession>
		sessions;
	GtkWindow
		**single_main_window; // for use by libunique

	aghui::SExpDesignUI* ed;

	string get_selected_dir(); // and assign last_dir_no
	void read_histfile();
	void write_histfile() const;
	string get_dir( int) const;
	string get_dir() const
		{
			return get_dir( last_dir_no);
		}
	void open_selected_session();
	void close_current_session();

	void buf_on_status_bar( bool ensure = true);
	void sb_progress_indicator( const char*, size_t n, size_t i);
	guint	sbChooserContextIdGeneral;

	// widgets
	int construct_widgets();
	void destruct_widgets();

	GtkListStore
		*mSessionChooserList;
	GtkDialog
		*wSessionChooser;
	GtkTreeView
		*tvSessionChooserList;
	GtkButton
		*bSessionChooserOpen,
		*bSessionChooserClose,
		*bSessionChooserCreateNew,
		*bSessionChooserRemove,
		*bSessionChooserQuit;
	GtkStatusbar
		*sbSessionChooserStatusBar;
};


} // namespace aghui

#endif // _AGH_UI_SESSION_CHOOSER_H

// eof
