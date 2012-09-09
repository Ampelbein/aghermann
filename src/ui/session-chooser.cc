// ;-*-C++-*-
/*
 *       File name:  ui/session-chooser.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-15
 *
 *         Purpose:  experiment design selector
 *
 *         License:  GPL
 */


#include <cassert>
#include <libconfig.h++>

#include "../common/fs.hh"
#include "misc.hh"
#include "session-chooser.hh"
#include "session-chooser_cb.hh"
#include "expdesign.hh"

using namespace std;


void
aghui::SSession::
get_session_stats()
{
	FAFA;
	
}


aghui::SSessionChooser::
SSessionChooser (const char* explicit_session, GtkWindow** single_main_window_)
      : hist_filename (string (getenv("HOME")) + "/.config/aghermann/sessionrc"),
	single_main_window (single_main_window_),
	ed (nullptr)
{
	if ( construct_widgets() )
		throw runtime_error ("SSessionChooser::SSessionChooser(): failed to construct widgets");

	try {
		char *canonicalized = canonicalize_file_name( explicit_session);
		ed = new aghui::SExpDesignUI(
			this,
			(explicit_session && strlen(explicit_session) > 0 )
			? canonicalized
			: (read_histfile(), get_dir()));
		*single_main_window = ed->wMainWindow;

		free( canonicalized);
	} catch (runtime_error ex) {
		aghui::pop_ok_message( nullptr, "%s", ex.what());

		string new_experiment_dir = string (getenv("HOME")) + "/NewExperiment";
		if ( agh::fs::mkdir_with_parents( new_experiment_dir.c_str()) ) {
			aghui::pop_ok_message( nullptr, "Failed to create a new directory in your $HOME."
					       " There's nothing we can do about that.");
		}
		ed = new aghui::SExpDesignUI( this, new_experiment_dir);
		// if HOME is non-writable, then don't catch: it's too seriously broken
	}

}


aghui::SSessionChooser::
~SSessionChooser()
{
	if ( ed )
		delete ed;
	write_histfile();
}





void
aghui::SSessionChooser::
open_selected_session()
{
	assert (ed == nullptr);
	try {
		ed = new aghui::SExpDesignUI(
			this, get_selected_dir());
		*single_main_window = ed->wMainWindow;

	} catch (runtime_error ex) {
		ed = nullptr;
		*single_main_window = (GtkWindow*)wSessionChooser;

		pop_ok_message( nullptr,
				"%s\n\n"
				"Please choose another directory", ex.what());
	}
}


void
aghui::SSessionChooser::
close_current_session()
{
	assert (ed);
	delete ed;
	ed = nullptr;
	*single_main_window = (GtkWindow*)wSessionChooser;
}





void
aghui::SSessionChooser::
destruct_widgets()
{
      // destroy toplevels
	gtk_widget_destroy( (GtkWidget*)wSessionChooser);
      // and models, etc
	g_object_unref( (GObject*)mSessionChooserList);
}





void
aghui::SSessionChooser::
buf_on_status_bar( bool ensure)
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
sb_progress_indicator( const char* current, size_t n, size_t i)
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

	last_dir_no = gtk_tree_path_get_indices( path)[0];
	GtkTreeIter iter;
	gtk_tree_model_get_iter( model, &iter, path);

	gchar *entry;
	//unique_ptr<void,void(*)(void*)> u(entry, g_free);
	gtk_tree_model_get( model, &iter, 0, &entry, -1);
	string ret {entry};
	agh::str::tilda2homedir(ret);
	g_free(entry);
	gtk_tree_path_free( path);

	return ret;
}


string
aghui::SSessionChooser::
get_dir( int idx) const
{
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first( (GtkTreeModel*)mSessionChooserList, &iter);

	int i = 0;
	while ( valid ) {
		gchar *entry;
		//unique_ptr<void,void(*)(void*)> u(entry, g_free);
		gtk_tree_model_get( (GtkTreeModel*)mSessionChooserList, &iter,
				    0, &entry,
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
read_histfile()
{
	libconfig::Config conf;

	GtkTreeIter iter;
	try {
		conf.readFile( hist_filename.c_str());
		conf.lookupValue( "SessionLast", last_dir_no);
		string entries_;
		conf.lookupValue( "SessionList", entries_);

		list<string> entries {agh::str::tokens( &entries_[0], ";")};
		gtk_list_store_clear( mSessionChooserList);
		if ( entries.empty() )
			throw runtime_error ("add a cwd then");
		for ( auto &E : entries ) {
			sessions.emplace_back( E);
			agh::str::homedir2tilda(E);
			gtk_list_store_append( mSessionChooserList, &iter);
			gtk_list_store_set( mSessionChooserList, &iter,
					    0, E.c_str(),
					    -1);
		}

		agh::ensure_within( last_dir_no, 0, (int)entries.size());

	} catch (...) {
		// create new
		printf( "Creating new sessionrc\n");
		char *cwd = getcwd( NULL, 0);
		string e {cwd};
		agh::str::homedir2tilda(e);

		gtk_list_store_clear( mSessionChooserList);
		gtk_list_store_append( mSessionChooserList, &iter);
		gtk_list_store_set( mSessionChooserList, &iter,
				    0, e.c_str(),
				    -1);
		last_dir_no = 0;

		free( cwd);
	}
}





void
aghui::SSessionChooser::
write_histfile() const
{
	GtkTreeIter iter;
	bool some_items_left =
		gtk_tree_model_get_iter_first( (GtkTreeModel*)mSessionChooserList, &iter);

	char	*entry;
	string	agg;
	while ( some_items_left ) {
		gtk_tree_model_get( (GtkTreeModel*)mSessionChooserList, &iter,  // at least one entry exists,
				    0, &entry,                             // added in read_histfile()
				    -1);
		agg += (string(entry) + ";");
		g_free( entry);
		some_items_left = gtk_tree_model_iter_next( (GtkTreeModel*)mSessionChooserList, &iter);
	}

	try {
		libconfig::Config conf;
		conf.getRoot().add( "SessionList", libconfig::Setting::Type::TypeString) = agg;
		conf.getRoot().add( "SessionLast", libconfig::Setting::Type::TypeInt) = last_dir_no;

		gchar *dirname = g_path_get_dirname( hist_filename.c_str());
		g_mkdir_with_parents( dirname, 0755);
		g_free( dirname);

		conf.writeFile( hist_filename.c_str());
	} catch (...) {
		pop_ok_message( (GtkWindow*)wSessionChooser, "Couldn't write %s", hist_filename.c_str());
	}
}


// eof
