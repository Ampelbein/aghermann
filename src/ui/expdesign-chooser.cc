// ;-*-C++-*-
/*
 *       File name:  ui/expdesign-selector.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-15
 *
 *         Purpose:  experiment design selector
 *
 *         License:  GPL
 */


#include <unistd.h>
#include "misc.hh"
#include "expdesign.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
using namespace aghui;



string
aghui::SExpDesignUI::chooser_get_selected_dir()
{
	auto selection = gtk_tree_view_get_selection( tvExpDesignChooserList);
	GtkTreeModel *model;
	GList *paths = gtk_tree_selection_get_selected_rows( selection, &model);
	GtkTreePath *path = (GtkTreePath*) g_list_nth_data( paths, 0);
	g_list_free( paths);

	chooser.last_dir_no = gtk_tree_path_get_indices( path)[0];
	GtkTreeIter iter;
	gtk_tree_model_get_iter( model, &iter, path);

	gchar *entry;
	//unique_ptr<void,void(*)(void*)> u(entry, g_free);
	gtk_tree_model_get( model, &iter, 0, &entry, -1);
	string ret {entry};
	tilda2homedir(ret);
	g_free(entry);
	gtk_tree_path_free( path);
	printf( "entry: %s\n", ret.c_str());
	return ret;
}


string
aghui::SExpDesignUI::chooser_get_dir( int idx)
{
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first( (GtkTreeModel*)mExpDesignChooserList, &iter);

	int i = 0;
	while ( valid ) {
		gchar *entry;
		//unique_ptr<void,void(*)(void*)> u(entry, free);
		gtk_tree_model_get( (GtkTreeModel*)mExpDesignChooserList, &iter,
				    0, &entry,
				    -1);
		if ( i++ == idx ) {
			string r {entry};
			tilda2homedir(r);
			g_free(entry);
			return r;
		}
		g_free( entry);
		valid = gtk_tree_model_iter_next( (GtkTreeModel*)mExpDesignChooserList, &iter);
	}
	return {""};
}


void
aghui::SExpDesignUI::chooser_read_histfile()
{
	using boost::property_tree::ptree;
	ptree pt;

	GtkTreeIter iter;
	try {
		read_xml( chooser.hist_filename, pt);
		chooser.last_dir_no = pt.get<int>( "Sessions.Last");
		string entries_ = pt.get<string>( "Sessions.List");

		list<string> entries {string_tokens( &entries_[0], ";")};
		gtk_list_store_clear( mExpDesignChooserList);
		for ( auto &E : entries ) {
			homedir2tilda(E);
			gtk_list_store_append( mExpDesignChooserList, &iter);
			gtk_list_store_set( mExpDesignChooserList, &iter,
					    0, E.c_str(),
					    -1);
		}
		if ( chooser.last_dir_no >= entries.size() )
			chooser.last_dir_no = 0;

	} catch (...) {
		char *cwd = getcwd( NULL, 0);
		string e {cwd};
		homedir2tilda(e);

		gtk_list_store_clear( mExpDesignChooserList);
		gtk_list_store_append( mExpDesignChooserList, &iter);
		gtk_list_store_set( mExpDesignChooserList, &iter,
				    0, e.c_str(),
				    -1);
		chooser.last_dir_no = 0;

		free( cwd);
	}
}





void
aghui::SExpDesignUI::chooser_write_histfile()
{
	GtkTreeIter iter;
	bool some_items_left =
		gtk_tree_model_get_iter_first( (GtkTreeModel*)mExpDesignChooserList, &iter);
	if ( !some_items_left )
		chooser.last_dir_no = -1;

	char	*entry;
	string	agg;
	while ( some_items_left ) {
		gtk_tree_model_get( (GtkTreeModel*)mExpDesignChooserList, &iter,  // at least one entry exists,
				    0, &entry,                             // added in read_histfile()
				    -1);
		agg += (string(entry) + ";");
		g_free( entry);
		some_items_left = gtk_tree_model_iter_next( (GtkTreeModel*)mExpDesignChooserList, &iter);
	}

	using boost::property_tree::ptree;
	ptree pt;
	pt.put( "Sessions.List", agg);
	pt.put( "Sessions.Last", chooser.last_dir_no);

	gchar *dirname = g_path_get_dirname( chooser.hist_filename.c_str());
	g_mkdir_with_parents( dirname, 0755);
	g_free( dirname);

	write_xml( chooser.hist_filename, pt);
}


// eof
