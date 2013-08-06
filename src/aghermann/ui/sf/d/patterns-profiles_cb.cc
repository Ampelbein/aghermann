/*
 *       File name:  aghermann/ui/sf/d/patterns-profiles_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-03
 *
 *         Purpose:  scoring facility patterns
 *
 *         License:  GPL
 */

#include <sys/time.h>

#include "aghermann/ui/misc.hh"

#include "patterns.hh"


using namespace std;

using namespace agh::ui;


extern "C" {

void
eSFFDPatternList_changed_cb(
	GtkComboBox *combo,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	if ( FD.current_pattern != FD.patterns.end() ) {
		FD.current_pattern->Pp = FD.Pp2;
		FD.current_pattern->criteria = FD.criteria;
	}

	gint ci = gtk_combo_box_get_active( combo);
	if ( ci != -1 ) {
		FD.current_pattern = FD.pattern_by_idx(ci);
		FD.Pp2 = FD.current_pattern->Pp;
		FD.criteria = FD.current_pattern->criteria;
		FD.atomic_up();
		FD.thing_display_scale = FD.field_channel->signal_display_scale;
	} else
		gtk_label_set_text( FD.lSFFDParametersBrief, "");

	FD.setup_controls_for_find();
	FD.set_profile_manage_buttons_visibility();

	gtk_widget_queue_draw( (GtkWidget*)FD.daSFFDThing);
}



void
bSFFDProfileSave_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	g_signal_emit_by_name( FD.eSFFDPatternSaveName, "changed");

	if ( gtk_dialog_run( FD.wSFFDPatternSave) == GTK_RESPONSE_OK ) {
		pattern::SPattern<TFloat> P (*FD.current_pattern);
		P.name = gtk_entry_get_text( FD.eSFFDPatternSaveName);
		P.origin = gtk_toggle_button_get_active( FD.eSFFDPatternSaveOriginSubject)
			? pattern::TOrigin::subject
			: gtk_toggle_button_get_active( FD.eSFFDPatternSaveOriginExperiment)
			? pattern::TOrigin::experiment
			: pattern::TOrigin::user;
		P.saved = false;

		if ( FD.current_pattern->origin == pattern::TOrigin::transient ) // replace unnamed
			FD.patterns.back() = P;
		else {
			auto found = find( FD.patterns.begin(), FD.patterns.end(), P);
			if ( found == FD.patterns.end() )
				FD.patterns.insert( FD.current_pattern, move(P));
			else
				*(FD.current_pattern = found) = P;
		}

		FD.populate_combo();
		FD.set_profile_manage_buttons_visibility();
	}
}


namespace {
void
hildebranden( const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	gtk_widget_set_sensitive(
		(GtkWidget*)FD.bSFFDPatternSaveOK,
		gtk_entry_get_text_length( FD.eSFFDPatternSaveName) > 0);

	auto this_name = gtk_entry_get_text( FD.eSFFDPatternSaveName);
	auto this_origin = gtk_toggle_button_get_active( FD.eSFFDPatternSaveOriginSubject)
			? pattern::TOrigin::subject
			: gtk_toggle_button_get_active( FD.eSFFDPatternSaveOriginExperiment)
			? pattern::TOrigin::experiment
			: pattern::TOrigin::user;

	bool overwriting =
		find_if( FD.patterns.begin(), FD.patterns.end(),
			 [&] ( const pattern::SPattern<TFloat>& P) -> bool
			 { return P.name == this_name && P.origin == this_origin; })
		!= FD.patterns.end();
	gtk_button_set_label(
		FD.bSFFDPatternSaveOK,
		overwriting ? "Overwrite" : "Save");
}
}

void eSFFDPatternSaveName_changed_cb(
	GtkEditable*,
	const gpointer userdata)
{
	hildebranden(userdata);
}

void
eSFFD_any_pattern_origin_toggled_cb(
	GtkRadioButton*,
	const gpointer userdata)
{
	hildebranden(userdata);
}


void
bSFFDProfileDiscard_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	gint ci = gtk_combo_box_get_active( FD.eSFFDPatternList);

	assert ( FD.current_pattern != FD.patterns.end() );
	assert ( FD.current_pattern->origin != pattern::TOrigin::transient );
	assert ( ci != -1 );
	assert ( ci < (int)FD.patterns.size() );

	FD.discard_current_pattern();

	if ( not FD.patterns.empty() ) {
		FD.Pp2 = FD.current_pattern->Pp;
		FD.criteria = FD.current_pattern->criteria;

		FD.atomic_up();
	}

	FD.populate_combo();
	FD.set_profile_manage_buttons_visibility();
	FD.setup_controls_for_find();

	gtk_widget_queue_draw( (GtkWidget*)FD.daSFFDThing);
}


void
bSFFDProfileRevert_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	assert ( FD.current_pattern != FD.patterns.end() );
	assert ( FD.current_pattern->origin != pattern::TOrigin::transient );

	FD.Pp2 = FD.current_pattern->Pp;
	FD.criteria = FD.current_pattern->criteria;

	FD.atomic_up();

	FD.set_profile_manage_buttons_visibility();
}

} // extern "C"



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
