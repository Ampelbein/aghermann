// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-patterns-enumerate.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-16
 *
 *         Purpose:  scoring facility patterns (enumerating & io)
 *
 *         License:  GPL
 */

#include <tuple>
#include "ui/misc.hh"
#include "sf.hh"

using namespace std;


void
aghui::SScoringFacility::SFindDialog::
import_from_selection( SScoringFacility::SChannel& field)
{
	// double check, possibly redundant after due check in callback
	size_t	run = field.selection_end - field.selection_start;
	if ( run == 0 )
		return;

	size_t	context_before = // agh::alg::ensure_within(
		(field.selection_start < Q->context_pad)
		? Q->context_pad - field.selection_start
		: Q->context_pad,
		context_after  = (field.selection_end + Q->context_pad > field.n_samples())
		? field.n_samples() - field.selection_end
		: Q->context_pad,
		full_sample = run + context_before + context_after;
	pattern::SPattern<TFloat> tim {
		"(unnamed)", pattern::TOrigin::transient, false,
		{field.signal_filtered[ slice (field.selection_start - context_before, full_sample, 1) ]},
		field.samplerate(),
		context_before, context_after,
		Pp2,
			criteria};
	// transient is always the last
	((patterns.back().origin == pattern::TOrigin::transient)
	 ? patterns.back()
	 : (patterns.push_back( pattern::SPattern<TFloat> ()), patterns.back())
		) = tim;

	field_channel = &field;

	thing_display_scale = field.signal_display_scale;

	set_thing_da_width( full_sample / field.spp());

	preselect_channel( field.name);
	preselect_entry( NULL, 0);
	setup_controls_for_find();

	gtk_widget_queue_draw( (GtkWidget*)_p.daSFFDThing);
}




const char*
	origin_markers[5] = {
	"[S]", "[U]", "[E]", "", "~",
};

string
make_system_patterns_location()
{
	DEF_UNIQUE_CHARP (buf);
	ASPRINTF( &buf, "%s/patterns", PACKAGE_DATADIR);
	string ret (buf);
	return ret;
}

string
make_user_patterns_location()
{
	DEF_UNIQUE_CHARP (buf);
	ASPRINTF( &buf, "%s/.local/share/aghermann/patterns", getenv("HOME"));
	string ret (buf);
	return ret;
}

string
make_experiment_patterns_location( const agh::CExpDesign& ED)
{
	DEF_UNIQUE_CHARP (buf);
	ASPRINTF( &buf, "%s/.patterns", ED.session_dir().c_str());
	string ret (buf);
	return ret;
}

string
make_subject_patterns_location(const agh::CExpDesign& ED, const agh::CSubject& J)
{
	DEF_UNIQUE_CHARP (buf);
	ASPRINTF( &buf, "%s/.patterns", ED.subject_dir( J).c_str());
	string ret (buf);
	return ret;
}



void
aghui::SScoringFacility::SFindDialog::
load_patterns()
{
	list<pattern::SPattern<TFloat>>
		collected;
	collected.splice(
		collected.end(), pattern::load_patterns_from_location<TFloat>(
			make_system_patterns_location(),
			pattern::TOrigin::system));
	collected.splice(
		collected.end(), pattern::load_patterns_from_location<TFloat>(
			make_user_patterns_location(),
			pattern::TOrigin::user));
	collected.splice(
		collected.end(), pattern::load_patterns_from_location<TFloat>(
			make_experiment_patterns_location( *_p._p.ED),
			pattern::TOrigin::experiment));
	collected.splice(
		collected.end(), pattern::load_patterns_from_location<TFloat>(
			make_subject_patterns_location( *_p._p.ED, _p.csubject()),
			pattern::TOrigin::subject));
}


void
aghui::SScoringFacility::SFindDialog::
enumerate_patterns_to_combo()
{
	g_signal_handler_block( _p.eSFFDPatternList, _p.eSFFDPatternList_changed_cb_handler_id);
	gtk_list_store_clear( _p.mSFFDPatterns);

	GtkTreeIter iter;
	for ( auto& P : patterns )
		if ( P.origin != pattern::TOrigin::discard ) {
			snprintf_buf( "%s %s", origin_markers[P.origin], P.name.c_str());
			gtk_list_store_append( _p.mSFFDPatterns, &iter);
			gtk_list_store_set( _p.mSFFDPatterns, &iter,
					    0, __buf__,
					    -1);
		}

	gtk_combo_box_set_active_iter( _p.eSFFDPatternList, NULL);
	g_signal_handler_unblock( _p.eSFFDPatternList, _p.eSFFDPatternList_changed_cb_handler_id);
}



void
aghui::SScoringFacility::SFindDialog::
save_patterns()
{
	for ( auto& P : patterns )
		if ( not P.saved ) {
			switch ( P.origin ) {
			case pattern::TOrigin::transient: // never save these two
			case pattern::TOrigin::system:
			    break;
			case pattern::TOrigin::user:
				pattern::save_pattern( P, (make_user_patterns_location() + '/' + P.name).c_str());
			    break;
			case pattern::TOrigin::experiment:
				pattern::save_pattern( P, (make_experiment_patterns_location(*_p._p.ED) + '/' + P.name).c_str());
			    break;
			case pattern::TOrigin::subject:
				pattern::save_pattern( P, (make_subject_patterns_location(*_p._p.ED, _p.csubject()) + '/' + P.name).c_str());
			    break;
			}
			P.saved = true;
		}
}


void
aghui::SScoringFacility::SFindDialog::
discard_pattern()
{
	Q->origin = pattern::TOrigin::discard;
	enumerate_patterns_to_combo();
}







void
aghui::SScoringFacility::SFindDialog::
preselect_entry( const char *label, bool do_globally)
{
	if ( label == NULL ) {
		gtk_combo_box_set_active_iter( _p.eSFFDPatternList, NULL);
		return;
	}

	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( (GtkTreeModel*)_p.mSFFDPatterns, &iter);
	while ( valid ) {
		char *entry;
		gtk_tree_model_get( (GtkTreeModel*)_p.mSFFDPatterns, &iter,
				    0, &entry,
				    -1);
		if ( (!do_globally && strcmp( entry, label) == 0) ||
		     (do_globally && (strlen( entry) > strlen( globally_marker) && strcmp( entry+strlen(globally_marker), label) == 0)) ) {
			gtk_combo_box_set_active_iter( _p.eSFFDPatternList, &iter);
			free( entry);
			return;
		}
		free( entry);
		valid = gtk_tree_model_iter_next( (GtkTreeModel*)_p.mSFFDPatterns, &iter);
	}
}


// eof
