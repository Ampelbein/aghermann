// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-patterns_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-03
 *
 *         Purpose:  scoring facility patterns
 *
 *         License:  GPL
 */

#include <sys/time.h>

#include "ui/misc.hh"
#include "sf.hh"

using namespace std;

using namespace aghui;

extern "C" {

gboolean
daSFFDThing_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	FD.draw_thing( cr);

	return TRUE;
}

gboolean
daSFFDField_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	FD.draw_field( cr);

	return TRUE;
}




gboolean
daSFFDThing_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		if ( event->state & GDK_SHIFT_MASK )
			FD.set_thing_da_width( FD.da_thing_wd + 10);
		else
			FD.thing_display_scale *= 1.05;
	    break;
	case GDK_SCROLL_DOWN:
		if ( event->state & GDK_SHIFT_MASK ) {
			if ( FD.da_thing_wd > 20 )
				FD.set_thing_da_width( FD.da_thing_wd - 10);
		} else
			FD.thing_display_scale /= 1.05;
	    break;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}

gboolean
daSFFDField_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		if ( event->state & GDK_SHIFT_MASK )
			FD.set_field_da_width( FD.da_field_wd + 10);
		else
			FD.field_display_scale *= 1.05;
	    break;
	case GDK_SCROLL_DOWN:
		if ( event->state & GDK_SHIFT_MASK ) {
			if ( FD.da_field_wd > 20 )
				FD.set_field_da_width( FD.da_field_wd - 10);
		} else
			FD.field_display_scale /= 1.05;
	    break;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}

gboolean
daSFFDField_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	switch ( event->button ) {
	case 1:
		SF.set_cur_vpage( ((double)FD.occurrences[FD.highlighted_occurrence] / FD.diff_line.size()) * SF.total_vpages());
	    break;
	case 3:
		gtk_menu_popup( SF.iiSFFDField,
				NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}
	gtk_widget_queue_draw( wid);

	return TRUE;
}


gboolean
daSFFDField_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	FD.highlighted_occurrence = FD.nearest_occurrence( event->x);

	gtk_widget_queue_draw( wid);

	return TRUE;
}



void
bSFFDSearch_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	aghui::SBusyBlock bb (SF.wSFFD);

	FD.setup_controls_for_wait();
	gtk_flush();
	FD.search();

	FD.set_field_da_width( SF.total_pages() * 3);

	FD.field_display_scale =
		(FD.field_channel->type == sigfile::SChannel::TType::eeg)
		? agh::alg::calibrate_display_scale(
			FD.field_channel->which_profile(FD.field_profile_type),
			SF.total_pages(),
			FD.da_field_ht)
		: agh::alg::calibrate_display_scale(
			FD.field_channel->signal_filtered,
			SF.total_pages(),
			FD.da_field_ht);

	FD.save_annotations();

	FD.setup_controls_for_tune();
	snprintf_buf( "A: <b>%g</b>  "
		      "B: <b>%g</b>/<b>%g</b>/<b>%d</b>  "
		      "C: <b>%g</b>/<b>%g</b>/<b>%d</b>",
		      FD.Pp2.env_scope,
		      FD.Pp2.bwf_ffrom, FD.Pp2.bwf_fupto, FD.Pp2.bwf_order,
		      FD.Pp2.dzcdf_step, FD.Pp2.dzcdf_sigma, FD.Pp2.dzcdf_smooth);
	gtk_label_set_markup( SF.lSFFDParametersBrief, __buf__);
}

void
bSFFDAgain_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	FD.restore_annotations();
	FD.occurrences.clear();

	FD.setup_controls_for_find();
}


void
eSFFD_any_pattern_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;
	if ( FD.suppress_w_v )
		return;

	FD.W_V.down();
	FD.setup_controls_for_find();

	FD.set_profile_manage_buttons_visibility();

	gtk_widget_queue_draw( (GtkWidget*)FD._p.daSFFDThing);
}

inline namespace {
inline double
timeval_elapsed( const struct timeval &x, const struct timeval &y)
{
	return y.tv_sec - x.tv_sec
		+ 1e-6 * (y.tv_usec - x.tv_usec);
}
}

void
eSFFD_any_criteria_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;
	if ( FD.suppress_w_v )
		return;

	static struct timeval last_criteria_change = {0, 0};
	struct timeval currently;
	gettimeofday( &currently, NULL);
	if ( timeval_elapsed( last_criteria_change, currently) > .5 ) {
		gettimeofday( &last_criteria_change, NULL);

		FD.W_V.down();
		FD.find_occurrences();

		snprintf_buf( "%zu match%s\n",
			      FD.occurrences.size(), (FD.occurrences.size() == 1) ? "" : "es");
		gtk_label_set_markup( FD._p.lSFFDFoundInfo, __buf__);

		FD.set_profile_manage_buttons_visibility();

		gtk_widget_queue_draw( (GtkWidget*)FD._p.daSFFDField);
	}
}






void
eSFFDPatternList_changed_cb( GtkComboBox *combo, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	if ( FD.current_pattern != FD.patterns.end() ) {
		FD.current_pattern->Pp = FD.Pp2;
		FD.current_pattern->criteria = FD.criteria;
	}

	gint ci = gtk_combo_box_get_active( combo);
	if ( ci == -1 )
		return;

	auto now_current = FD.pattern_by_idx(ci);
	FD.Pp2 = now_current->Pp;
	FD.criteria = now_current->criteria;

	FD.set_profile_manage_buttons_visibility();

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFFDThing);
}



void
bSFFDProfileSave_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	assert (FD.current_pattern->origin == pattern::TOrigin::transient );

	auto& P = *FD.current_pattern;
	if ( gtk_dialog_run( SF.wSFFDPatternSave) == GTK_RESPONSE_OK ) {
		P.name = gtk_entry_get_text( SF.eSFFDPatternSaveName);
		P.origin = gtk_toggle_button_get_active( SF.eSFFDPatternSaveOriginSubject)
			? pattern::TOrigin::subject
			: gtk_toggle_button_get_active( SF.eSFFDPatternSaveOriginExperiment)
			? pattern::TOrigin::experiment
			: pattern::TOrigin::user;
	}

	FD.populate_combo();
}


void
bSFFDProfileDiscard_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	gint ci = gtk_combo_box_get_active( SF.eSFFDPatternList);

	assert ( FD.current_pattern != FD.patterns.end() );
	assert ( FD.current_pattern->origin != pattern::TOrigin::transient );
	assert ( ci == -1 );
	assert ( ci < (int)FD.patterns.size() );

	FD.discard_current_pattern();

	FD.Pp2 = FD.current_pattern->Pp;
	FD.criteria = FD.current_pattern->criteria;

	FD.suppress_w_v = true;
	FD.W_V.up();
	FD.suppress_w_v = false;

	FD.populate_combo();
}


void
bSFFDProfileRevert_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	assert ( FD.current_pattern != FD.patterns.end() );
	assert ( FD.current_pattern->origin != pattern::TOrigin::transient );

	FD.Pp2 = FD.current_pattern->Pp;
	FD.criteria = FD.current_pattern->criteria;

	FD.suppress_w_v = true;
	FD.W_V.up();
	FD.suppress_w_v = false;
}





void
eSFFDChannel_changed_cb( GtkComboBox *combo, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	GtkTreeIter iter;
	if ( gtk_combo_box_get_active_iter( combo, &iter) == FALSE )
		return;

	gchar *label;
	gtk_tree_model_get( gtk_combo_box_get_model( combo), &iter,
			    0, &label,
			    -1);
	for ( auto &H : SF.channels ) {
		if ( strcmp( H.name, label) == 0 ) {
			FD.field_channel = SF.using_channel = &H;
			break;
		}
	}
	g_free( label);
}


void
wSFFD_show_cb( GtkWidget *widget, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	FD.setup_controls_for_find();
	FD.populate_combo();

	if ( FD._p.using_channel == nullptr ) // not invoked for a preselected signal via a menu
		FD._p.using_channel = &FD._p.channels.front();
	FD.field_channel = FD.field_channel_saved = FD._p.using_channel;

	FD.preselect_channel( FD.field_channel->name);
}

void
wSFFD_hide_cb( GtkWidget *widget, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.find_dialog;

	if ( not FD.occurrences.empty() )  // closing while dialog is in matching state
		if ( GTK_RESPONSE_YES !=
		     aghui::pop_question( SF.wSF, "Keep annotations?") )
			FD.restore_annotations();

	FD.occurrences.clear();

	gtk_toggle_button_set_active( (GtkToggleButton*)FD.field_channel->_p.bSFShowFindDialog, FALSE);
}



gboolean
wSFFD_configure_event_cb( GtkWidget *widget,
			  GdkEventConfigure *event,
			  gpointer userdata)
{
	 if ( event->type == GDK_CONFIGURE ) {
		 auto& SF = *(SScoringFacility*)userdata;
		 int marijke = gtk_widget_get_allocated_width( (GtkWidget*)SF.swSFFDThing);
		 SF.find_dialog.set_thing_da_width( marijke);
		 SF.find_dialog.set_field_da_width( marijke);
	 }
	 return FALSE;
}

} // extern "C"

// eof
