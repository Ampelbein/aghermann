/*
 *       File name:  ui/sm/sm.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-15
 *
 *         Purpose:  session manager
 *
 *         License:  GPL
 */


#include <cassert>
#include <ftw.h>
#include <libconfig.h++>

#include "common/fs.hh"
#include "expdesign/forward-decls.hh"  // for edf_file_counter
#include "ui/misc.hh"
#include "ui/mw/mw.hh"
#include "sm.hh"
#include "sm_cb.hh"

using namespace std;


void
aghui::SSession::
get_session_stats()
{
	agh::fs::__n_edf_files = 0;
	string path = agh::str::tilda2homedir(c_str());
	nftw( path.c_str(), agh::fs::edf_file_counter, 20, 0);
	n_recordings = agh::fs::__n_edf_files;

	{
		struct stat stat0;
		int stst = stat( (path + "/.aghermann.conf").c_str(), &stat0);
		if ( stst != -1 )
			last_visited = stat0.st_mtime;
	}

}


aghui::SSessionChooser::
SSessionChooser (const char* explicit_session)
      : filename (string (getenv("HOME")) + "/.config/aghermann/sessionrc"),
	ed (nullptr)
{
	if ( construct_widgets() )
		throw runtime_error ("SSessionChooser::SSessionChooser(): failed to construct widgets");

	read_sessionrc();

	bool have_explicit_dir = (explicit_session && strlen(explicit_session) > 0);

	try {
		if ( have_explicit_dir ) {
			char* canonicalized = canonicalize_file_name( explicit_session);
			ed = new aghui::SExpDesignUI(
				this, canonicalized);
			set_unique_app_window( ed->wMainWindow);
			free( canonicalized);

		} else if ( last_dir_no == -1 ) {
			gtk_widget_show( (GtkWidget*)wSessionChooser);
			//set_unique_app_window( (GtkWindow*)wSessionChooser);

		} else {
			ed = new aghui::SExpDesignUI(
				this, get_dir());
			set_unique_app_window( ed->wMainWindow);
		}
	} catch (runtime_error ex) {
		aghui::pop_ok_message( nullptr, "Huh", "%s", ex.what());

		string new_experiment_dir = string (getenv("HOME")) + "/NewExperiment";
		if ( agh::fs::mkdir_with_parents( new_experiment_dir.c_str()) ) {
			aghui::pop_ok_message( nullptr, "Failed to create a new directory in your $HOME.",
					       " There's nothing we can do about that.");
		}
		ed = new aghui::SExpDesignUI( this, new_experiment_dir);
		// if HOME is non-writable, then don't catch: it's too seriously broken
	}
}


aghui::SSessionChooser::
~SSessionChooser()
{
	destruct_widgets();
	if ( ed )
		delete ed;
	write_sessionrc();
}





int
aghui::SSessionChooser::
open_selected_session()
{
	assert (ed == nullptr);
	string selected = get_selected_dir();
	if ( selected.size() == 0 )
		return 1; // double check

	try {
		ed = new aghui::SExpDesignUI(
			this, selected);
		set_unique_app_window( ed->wMainWindow);

		return 0;

	} catch (invalid_argument ex) {
		ed = nullptr;
		//set_unique_app_window( (GtkWindow*)wSessionChooser);

		pop_ok_message( nullptr,
				ex.what(),
				"Please choose another directory");
		return -1;
	}
}


void
aghui::SSessionChooser::
close_current_session()
{
	assert (ed);
	delete ed;
	ed = nullptr;
	//set_unique_app_window( unique_app, (GtkWindow*)wSessionChooser);
}





void
aghui::SSessionChooser::
destruct_widgets()
{
      // // destroy toplevels
      // 	gtk_widget_destroy( (GtkWidget*)wSessionChooser);
      // // and models, etc
      // 	g_object_unref( (GObject*)mSessionChooserList);
	// don't care
}





void
aghui::SSessionChooser::
buf_on_status_bar( const bool)
{
	// gtk_statusbar_pop( sbSessionChooserStatusBar, sbChooserContextIdGeneral);
	// gtk_statusbar_push( sbSessionChooserStatusBar, sbChooserContextIdGeneral, __buf__);
	/// segfaults no matter what. Cannot call gtk_events_pending() from a handler.
	// if ( ensure ) {
	//  	g_signal_handler_block( bSessionChooserSelect, bSessionChooserSelect_clicked_cb_handler);
	//   	aghui::gtk_flush();
	//  	g_signal_handler_unblock( bSessionChooserSelect, bSessionChooserSelect_clicked_cb_handler);
	// }
}

void
aghui::SSessionChooser::
sb_progress_indicator( const char* current, const size_t n, const size_t i)
{
	snprintf_buf( "(%zu of %zu) %s", i, n, current);
	buf_on_status_bar( true);
}



string
aghui::SSessionChooser::
get_selected_dir()
{
	auto selection = gtk_tree_view_get_selection( tvSessionChooserList);
	GtkTreeModel *model;
	GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
	GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
	g_list_free( paths);

	// the only place last_dir_no becomes != -1
	last_dir_no = gtk_tree_path_get_indices( path)[0];

	GtkTreeIter iter;
	gtk_tree_model_get_iter( model, &iter, path);

	gchar *entry;
	//unique_ptr<void,void(*)(void*)> u(entry, g_free);
	gtk_tree_model_get( model, &iter, 2, &entry, -1);
	string ret {entry};
	agh::str::tilda2homedir(ret);
	g_free(entry);
	gtk_tree_path_free( path);

	return ret;
}


string
aghui::SSessionChooser::
get_dir( const int idx) const
{
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first( (GtkTreeModel*)mSessionChooserList, &iter);

	int i = 0;
	while ( valid ) {
		gchar *entry;
		//unique_ptr<void,void(*)(void*)> u(entry, g_free);
		gtk_tree_model_get( (GtkTreeModel*)mSessionChooserList, &iter,
				    2, &entry,
				    -1);
		string r {entry};
		g_free(entry);
		agh::str::tilda2homedir(r);

		if ( i++ == idx ) {
			if ( not agh::fs::exists_and_is_writable( r) ) {
				fprintf( stderr, "SSessionChooser::get_dir(%d): entry \"%s\" does not exist or is not a dir\n",
					 idx, r.c_str());
				gtk_list_store_remove( mSessionChooserList, &iter);
				return string("");
			} else
				return r;
		}
		valid = gtk_tree_model_iter_next( (GtkTreeModel*)mSessionChooserList, &iter);
	}
	return string("");
}


void
aghui::SSessionChooser::
read_sessionrc()
{
	sessions.clear();
	try {
		libconfig::Config conf;
		conf.readFile( filename.c_str());
		conf.lookupValue( "SessionLast", last_dir_no);
		string entries_;
		conf.lookupValue( "SessionList", entries_);

		list<string> entries {agh::str::tokens_trimmed( &entries_[0], ";")};
		if ( entries.empty() )
			throw runtime_error ("add a cwd then");
		for ( auto &E : entries ) {
			sessions.emplace_back( agh::str::homedir2tilda(E));
			sessions.back().get_session_stats();
		}

		agh::alg::ensure_within( last_dir_no, -1, (int)entries.size());

	} catch (...) {
		// create new
		printf( "Creating new sessionrc\n");
		char *cwd = getcwd( NULL, 0);
		string e {cwd};
		sessions.emplace_back( agh::str::homedir2tilda(e));
		last_dir_no = 0;

		free( cwd);
	}
	_sync_list_to_model();
}





void
aghui::SSessionChooser::
write_sessionrc() const
{
	try {
		libconfig::Config conf;
		conf.getRoot().add( "SessionList", libconfig::Setting::Type::TypeString)
			= agh::str::join( sessions, ";");
		conf.getRoot().add( "SessionLast", libconfig::Setting::Type::TypeInt)
			= last_dir_no;

		gchar *dirname = g_path_get_dirname( filename.c_str());
		g_mkdir_with_parents( dirname, 0755);
		g_free( dirname);

		conf.writeFile( filename.c_str());
	} catch (...) {
		pop_ok_message( (GtkWindow*)wSessionChooser, "Error saving your sessionrc file", "(%s)", filename.c_str());
	}
}



void
aghui::SSessionChooser::
conditionally_enable_buttons()
{
	auto selection = gtk_tree_view_get_selection( tvSessionChooserList);
	gboolean chris = gtk_tree_selection_count_selected_rows( selection) == 1;
	gtk_widget_set_sensitive( (GtkWidget*)bSessionChooserOpen, chris);
	gtk_widget_set_sensitive( (GtkWidget*)bSessionChooserRemove, chris);
}




void
aghui::SSessionChooser::
_sync_list_to_model()
{
	gtk_list_store_clear( mSessionChooserList);
	GtkTreeIter iter;
	for ( auto &E : sessions ) {
		gtk_list_store_append( mSessionChooserList, &iter);
		snprintf_buf( "%d", (int)E.n_recordings);
		gtk_list_store_set( mSessionChooserList, &iter,
				    2, E.c_str(),
				    1, __buf__,
				    -1);
		strftime( __buf__, AGH_BUF_SIZE-1, "%c", localtime(&E.last_visited));
		gtk_list_store_set( mSessionChooserList, &iter,
				    0, __buf__,
				    -1);
	}
}

void
aghui::SSessionChooser::
_sync_model_to_list()
{
	sessions.clear();
	GtkTreeIter
		iter;
	bool	some_items_left = gtk_tree_model_get_iter_first( (GtkTreeModel*)mSessionChooserList, &iter);
	gchar	*entry;
	while ( some_items_left ) {
		gtk_tree_model_get( (GtkTreeModel*)mSessionChooserList, &iter,  // at least one entry exists,
				    2, &entry,                             // added in read_histfile()
				    -1);
		sessions.emplace_back( entry);
		sessions.back().get_session_stats();
		g_free( entry);
		some_items_left = gtk_tree_model_iter_next( (GtkTreeModel*)mSessionChooserList, &iter);
	}
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
