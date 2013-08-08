/*
 *       File name:  aghermann/ui/sm/sm.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-13
 *
 *         Purpose:  session manager
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_SESSION_CHOOSER_H
#define _AGH_UI_SESSION_CHOOSER_H

#include <string>
#include <list>

#include <gtk/gtk.h>

#include "common/lang.hh"
#include "aghermann/ui/forward-decls.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


namespace agh {
namespace ui {

struct SSession : public string {
	size_t	n_groups,
		n_subjects,
		n_recordings;
	time_t	last_visited;
	// possibly something else
	SSession (string dir_)
	      : string (dir_),
		n_groups (-1), n_subjects (-1), n_recordings (-1),
		last_visited (0)
		{
			//get_session_stats();
		}
	void get_session_stats();
};


struct SSessionChooser {
	DELETE_DEFAULT_METHODS (SSessionChooser);

	SSessionChooser (const char*);
       ~SSessionChooser ();

	string	title;
	string	filename;
	int	last_dir_no;
	list<SSession>
		sessions;

	agh::ui::SExpDesignUI* ed;

	string get_selected_dir(); // and assign last_dir_no
	void read_sessionrc();
	void write_sessionrc() const;
	string get_dir( int) const;
	string get_dir() const
		{
			return get_dir( last_dir_no);
		}
	int open_selected_session();
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
    // private:
	void conditionally_enable_buttons();
	void _sync_list_to_model();
	void _sync_model_to_list();
};


}
} // namespace agh::ui

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
