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
		(field.selection_start < current_pattern->context_pad)
		? pattern::SPattern<TFloat>::context_pad - field.selection_start
		: pattern::SPattern<TFloat>::context_pad,
		context_after  = (field.selection_end + pattern::SPattern<TFloat>::context_pad > field.n_samples())
		? field.n_samples() - field.selection_end
		: pattern::SPattern<TFloat>::context_pad,
		full_sample = run + context_before + context_after;
	pattern::SPattern<TFloat> tim {
		"(unnamed)", "", pattern::TOrigin::transient, false,
		{field.signal_filtered[ slice (field.selection_start - context_before, full_sample, 1) ]},
		field.samplerate(),
		context_before, context_after,
		Pp2, criteria};
	// transient is always the last
	((not patterns.empty() and patterns.back().origin == pattern::TOrigin::transient)
	 ? patterns.back()
	 : (patterns.push_back( pattern::SPattern<TFloat> ()), patterns.back())
		) = tim;
	current_pattern = prev(patterns.end());

	field_channel = &field;

	thing_display_scale = field.signal_display_scale;

	set_thing_da_width( full_sample / field.spp());

	preselect_channel( field.name);

	setup_controls_for_find();

	gtk_widget_queue_draw( (GtkWidget*)_p.daSFFDThing);
}




const char*
	origin_markers[5] = {
	"~", "[S]", "[E]", "[U]", "<S>",
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
	patterns.clear();

	patterns.splice(
		patterns.end(), pattern::load_patterns_from_location<TFloat>(
			make_system_patterns_location(),
			pattern::TOrigin::system));
	patterns.splice(
		patterns.end(), pattern::load_patterns_from_location<TFloat>(
			make_user_patterns_location(),
			pattern::TOrigin::user));
	patterns.splice(
		patterns.end(), pattern::load_patterns_from_location<TFloat>(
			make_experiment_patterns_location( *_p._p.ED),
			pattern::TOrigin::experiment));
	patterns.splice(
		patterns.end(), pattern::load_patterns_from_location<TFloat>(
			make_subject_patterns_location( *_p._p.ED, _p.csubject()),
			pattern::TOrigin::subject));

	current_pattern = patterns.end();
}


void
aghui::SScoringFacility::SFindDialog::
populate_combo()
{
	g_signal_handler_block( _p.eSFFDPatternList, _p.eSFFDPatternList_changed_cb_handler_id);
	gtk_list_store_clear( _p.mSFFDPatterns);

	if ( not patterns.empty() ) {
		GtkTreeIter iter, current_pattern_iter;
		for ( auto I = patterns.begin(); I != patterns.end(); ++I ) {
			snprintf_buf( "%s %s", origin_markers[I->origin], I->name.c_str());
			gtk_list_store_append( _p.mSFFDPatterns, &iter);
			gtk_list_store_set( _p.mSFFDPatterns, &iter,
					    0, __buf__,
					    -1);
			if ( I == current_pattern )
				current_pattern_iter = iter;
		}

		gtk_combo_box_set_active_iter( _p.eSFFDPatternList, &current_pattern_iter);
	} else
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
discard_current_pattern()
{
	if ( current_pattern == patterns.end() )
		return;

	auto todelete = current_pattern;
	current_pattern = next(current_pattern);
	pattern::delete_pattern( *todelete);
	patterns.erase( todelete);
}



// eof
