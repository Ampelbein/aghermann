/*
 *       File name:  ui/ui++.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  ui++
 *
 *         License:  GPL
 */

#include <gtk/gtk.h>

#include <list>
#include <string>
#include "ui++.hh"

using namespace std;

namespace agh {
namespace ui {

template <> void
SUIVar_<GtkListStore, list<string>>::up() const
{
	gtk_list_store_clear( w);
	GtkTreeIter iter;
	for ( auto& s : *v ) {
		gtk_list_store_append( w, &iter);
		gtk_list_store_set(
			w, &iter,
			1, s.c_str(),
			-1);
	}
}
template <> void
SUIVar_<GtkListStore, list<string>>::down() const
{
	v->clear();
	GtkTreeIter
		iter;
	gchar	*entry;
	while ( gtk_tree_model_get_iter_first( (GtkTreeModel*)w, &iter) ) {
		gtk_tree_model_get(
			(GtkTreeModel*)w, &iter,
			1, &entry,
			-1);
		v->emplace_back( entry);
		g_free( entry);
	}
}


}} // namespace agh::ui


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:

