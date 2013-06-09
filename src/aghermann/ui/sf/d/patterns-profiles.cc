/*
 *       File name:  aghermann/ui/sf/d/patterns-profiles.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-16
 *
 *         Purpose:  scoring facility patterns (enumerating & io)
 *
 *         License:  GPL
 */

#include <tuple>

#include "aghermann/ui/misc.hh"

#include "patterns.hh"

using namespace std;


int
aghui::SScoringFacility::SPatternsDialog::
import_from_selection( SScoringFacility::SChannel& field)
{
	// double check, possibly redundant after due check in callback
	double	run_time = field.selection_end_time - field.selection_start_time;
	size_t	run = field.selection_end - field.selection_start;
	if ( run == 0 )
		return -1;
	if ( run_time > 60. ) {
		aghui::pop_ok_message( (GtkWindow*)wSFFD, "Selection greater than a minute", "This is surely the single occurrence, I tell you!");
		return -2;
	}
	if ( run_time > 10. and
	     GTK_RESPONSE_YES !=
	     aghui::pop_question( (GtkWindow*)wSFFD, "The selection is greater than 10 sec. Sure to proceed with search?") ) {
		return -3;
	}

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
	populate_combo();

	field_channel_saved = field_channel = &field;

	thing_display_scale = field.signal_display_scale;

	set_thing_da_width( full_sample / field.spp());

	preselect_channel( _p.channel_idx( &field));

	setup_controls_for_find();

	gtk_widget_queue_draw( (GtkWidget*)daSFFDThing);

	return 0;
}




const char*
	origin_markers[5] = {
	"~", "[S]", "[E]", "[U]", "<S>",
};

string
make_system_patterns_location()
{
	return agh::str::sasprintf( "%s/patterns", PACKAGE_DATADIR);
}

string
make_user_patterns_location()
{
	return agh::str::sasprintf( "%s/.local/share/aghermann/patterns", getenv("HOME"));
}

string
make_experiment_patterns_location( const agh::CExpDesign& ED)
{
	return agh::str::sasprintf( "%s/.patterns", ED.session_dir().c_str());
}

string
make_subject_patterns_location( const agh::CExpDesign& ED, const agh::CSubject& J)
{
	return agh::str::sasprintf( "%s/.patterns", ED.subject_dir( J).c_str());
}



void
aghui::SScoringFacility::SPatternsDialog::
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
aghui::SScoringFacility::SPatternsDialog::
populate_combo()
{
	g_signal_handler_block( eSFFDPatternList, eSFFDPatternList_changed_cb_handler_id);
	gtk_list_store_clear( mSFFDPatterns);

	if ( not patterns.empty() ) {
		GtkTreeIter iter, current_pattern_iter;
		for ( auto I = patterns.begin(); I != patterns.end(); ++I ) {
			gtk_list_store_append( mSFFDPatterns, &iter);
			gtk_list_store_set( mSFFDPatterns, &iter,
					    0, snprintf_buf( "%s %s", origin_markers[I->origin], I->name.c_str()),
					    -1);
			if ( I == current_pattern )
				current_pattern_iter = iter;
		}

		gtk_combo_box_set_active_iter( eSFFDPatternList, &current_pattern_iter);
	} else
		gtk_combo_box_set_active_iter( eSFFDPatternList, NULL);

	g_signal_handler_unblock( eSFFDPatternList, eSFFDPatternList_changed_cb_handler_id);
}



void
aghui::SScoringFacility::SPatternsDialog::
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
aghui::SScoringFacility::SPatternsDialog::
discard_current_pattern()
{
	if ( current_pattern == patterns.end() )
		return;

	auto todelete = current_pattern;
	current_pattern = next(current_pattern);
	pattern::delete_pattern( *todelete);
	patterns.erase( todelete);
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
