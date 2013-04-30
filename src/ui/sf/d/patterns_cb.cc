/*
 *       File name:  ui/sf/d/patterns_cb.cc
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
#include "patterns.hh"

using namespace std;
using namespace aghui;

extern "C" {

gboolean
daSFFDThing_draw_cb(
	GtkWidget*,
	cairo_t *cr,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	FD.draw_thing( cr);

	return TRUE;
}

gboolean
daSFFDField_draw_cb(
	GtkWidget*,
	cairo_t *cr,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	FD.draw_field( cr);

	return TRUE;
}




gboolean
daSFFDThing_button_press_event_cb(
	GtkWidget *wid,
	GdkEventButton *event,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	switch ( event->button ) {
	case 2:
		FD.thing_display_scale = FD.field_channel->signal_display_scale;
	    break;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}



gboolean
daSFFDThing_scroll_event_cb(
	GtkWidget *wid,
	GdkEventScroll *event,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		if ( event->state & GDK_SHIFT_MASK )
			FD.set_thing_da_width( FD.da_thing_wd + 10);
		else
			FD.thing_display_scale *= FD._p._p.scroll_factor;
	    break;
	case GDK_SCROLL_DOWN:
		if ( event->state & GDK_SHIFT_MASK ) {
			if ( FD.da_thing_wd > 20 )
				FD.set_thing_da_width( FD.da_thing_wd - 10);
		} else
			FD.thing_display_scale /= FD._p._p.scroll_factor;
	    break;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}



gboolean
daSFFDField_button_press_event_cb(
	GtkWidget *wid,
	GdkEventButton *event,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;
	auto& SF = FD._p;

	switch ( event->button ) {
	case 1:
		if ( event->y > FD.da_field_ht/2 ) {
			if ( FD.highlighted_occurrence != (size_t)-1 )
				SF.set_cur_vpage(
					((double)FD.occurrences[FD.highlighted_occurrence] / FD.diff_line.size()) * SF.total_vpages());
		} else
			SF.set_cur_vpage(
				((double)event->x/FD.da_field_wd * SF.total_vpages()));
		gtk_widget_queue_draw( wid);
	    break;
	case 3:
		FD.update_field_check_menu_items();
		gtk_menu_popup(
			FD.iiSFFDField,
			NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}

	return TRUE;
}

void
iSFFDFieldDrawMatchIndex_toggled_cb(
	GtkCheckMenuItem* mitem,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;
	if ( FD.suppress_redraw )
		return;
	FD.draw_match_index = gtk_check_menu_item_get_active( mitem);
	gtk_widget_queue_draw( (GtkWidget*)FD.daSFFDField);
}


void
iSFFD_any_field_profile_type_toggled_cb(
	GtkRadioMenuItem* ritem,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;
	if ( FD.suppress_redraw )
		return;
	if ( not gtk_check_menu_item_get_active( (GtkCheckMenuItem*)ritem) )
		return; // let the item being turned on handle

	if ( ritem == FD.iSFFDFieldProfileTypeRaw )
		FD.field_profile_type = metrics::TType::raw;
	else if ( ritem == FD.iSFFDFieldProfileTypePSD )
		FD.field_profile_type = metrics::TType::psd;
	else if ( ritem == FD.iSFFDFieldProfileTypeMC )
		FD.field_profile_type = metrics::TType::mc;
	else if ( ritem == FD.iSFFDFieldProfileTypeSWU )
		FD.field_profile_type = metrics::TType::swu;

	// autoscale
	auto profile_with_corrected_type =
		FD.field_channel->which_profile( FD.field_profile_type);
	auto& profile = get<1>(profile_with_corrected_type);
	FD.field_display_scale =
		agh::alg::calibrate_display_scale(
			profile,
			profile.size(),
			FD.da_field_ht/2);

	gtk_widget_queue_draw( (GtkWidget*)FD.daSFFDField);
}



gboolean
daSFFDField_scroll_event_cb(
	GtkWidget *wid,
	GdkEventScroll *event,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		if ( event->state & GDK_SHIFT_MASK )
			FD.set_field_da_width( FD.da_field_wd + 10);
		else
			FD.field_display_scale *= FD._p._p.scroll_factor;
	    break;
	case GDK_SCROLL_DOWN:
		if ( event->state & GDK_SHIFT_MASK ) {
			if ( FD.da_field_wd > 20 )
				FD.set_field_da_width( FD.da_field_wd - 10);
		} else
			FD.field_display_scale /= FD._p._p.scroll_factor;
	    break;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}


gboolean
daSFFDField_motion_notify_event_cb(
	GtkWidget *wid,
	GdkEventMotion *event,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	auto prev_ho = FD.highlighted_occurrence;
	if ( prev_ho != (FD.highlighted_occurrence = FD.nearest_occurrence( event->x)) )
	     gtk_widget_queue_draw( wid);

	return TRUE;
}






void
bSFFDSearch_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;
	auto& SF = FD._p;

	SBusyBlock bb (FD.wSFFD);

	FD.setup_controls_for_wait();
	gtk_flush();

	FD.search();
	FD.save_annotations();

	FD.set_field_da_width( SF.total_pages() * SF.pagesize() / 3600 * SF._p.tl_pph);

	FD.field_display_scale =
		agh::alg::calibrate_display_scale(
			get<1>(FD.field_channel->which_profile(FD.field_profile_type)),
			SF.total_pages(),
			FD.da_field_ht);

	FD.setup_controls_for_tune();
	snprintf_buf( "A: <b>%g</b>  "
		      "B: <b>%g</b>/<b>%g</b>/<b>%d</b>  "
		      "C: <b>%g</b>/<b>%g</b>/<b>%d</b>",
		      FD.Pp2.env_scope,
		      FD.Pp2.bwf_ffrom, FD.Pp2.bwf_fupto, FD.Pp2.bwf_order,
		      FD.Pp2.dzcdf_step, FD.Pp2.dzcdf_sigma, FD.Pp2.dzcdf_smooth);
	gtk_label_set_markup( FD.lSFFDParametersBrief, __buf__);

	gtk_widget_queue_draw( (GtkWidget*)FD.daSFFDField);

	g_signal_emit_by_name( FD.eSFFDParameterA, "value-changed");
}



void
bSFFDAgain_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	FD.restore_annotations();
	FD.occurrences.clear();

	FD.setup_controls_for_find();
}

void
iSFFDMarkPhasicEventSpindles_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	FD.restore_annotations();
	FD.occurrences_to_annotations( sigfile::SAnnotation::TType::phasic_event_spindle);
	FD.occurrences.clear();
	FD._p.queue_redraw_all();

	gtk_widget_hide( (GtkWidget*)FD.wSFFD);
}

void
iSFFDMarkPhasicEventKComplexes_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	FD.restore_annotations();
	FD.occurrences_to_annotations( sigfile::SAnnotation::TType::phasic_event_K_complex);
	FD.occurrences.clear();
	FD._p.queue_redraw_all();

	gtk_widget_hide( (GtkWidget*)FD.wSFFD);
}

void
iSFFDMarkPlain_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	// FD.restore_annotations();
	// FD.occurrences_to_annotations( sigfile::SAnnotation::TType::plain);
	FD.occurrences.clear();
	// FD._p.queue_redraw_all();

	gtk_widget_hide( (GtkWidget*)FD.wSFFD);
}









void
eSFFD_any_pattern_value_changed_cb(
	GtkSpinButton*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	if ( FD.suppress_w_v )
		return;

	FD.W_V.down();
	FD.setup_controls_for_find();

	FD.set_profile_manage_buttons_visibility();

	gtk_widget_queue_draw( (GtkWidget*)FD.daSFFDThing);
}

namespace {
inline double
timeval_elapsed( const struct timeval &x, const struct timeval &y)
{
	return y.tv_sec - x.tv_sec
		+ 1e-6 * (y.tv_usec - x.tv_usec);
}
}

void
eSFFD_any_criteria_value_changed_cb(
	GtkSpinButton* button,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	if ( FD.suppress_w_v )
		return;

	if      ( button == FD.eSFFDParameterA )
		FD.now_tweaking = 1;
	else if ( button == FD.eSFFDParameterB )
		FD.now_tweaking = 2;
	else if ( button == FD.eSFFDParameterC )
		FD.now_tweaking = 3;
	else if ( button == FD.eSFFDParameterD )
		FD.now_tweaking = 4;

	static struct timeval last_criteria_change = {0, 0};
	struct timeval currently;
	gettimeofday( &currently, NULL);
	if ( timeval_elapsed( last_criteria_change, currently) > .5 ) {
		gettimeofday( &last_criteria_change, NULL);

		FD.W_V.down();
		FD.find_occurrences();

		snprintf_buf(
			"%zu match%s in <b>%s</b>",
			FD.occurrences.size(), (FD.occurrences.size() == 1) ? "" : "es",
			FD.field_channel->name());
		gtk_label_set_markup( FD.lSFFDFoundInfo, __buf__);

		FD.set_profile_manage_buttons_visibility();

		gtk_widget_queue_draw( (GtkWidget*)FD.daSFFDField);
	}
}

gboolean
eSFFD_any_criteria_focus_in_event_cb(
	GtkWidget *button,
	GdkEvent*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;
	if      ( button == (GtkWidget*)FD.eSFFDParameterA )
		FD.now_tweaking = 1;
	else if ( button == (GtkWidget*)FD.eSFFDParameterB )
		FD.now_tweaking = 2;
	else if ( button == (GtkWidget*)FD.eSFFDParameterC )
		FD.now_tweaking = 3;
	else if ( button == (GtkWidget*)FD.eSFFDParameterD )
		FD.now_tweaking = 4;

	gtk_widget_queue_draw( (GtkWidget*)FD.daSFFDField);

	return FALSE;
}





void
eSFFDChannel_changed_cb(
	GtkComboBox *combo,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;
	auto& SF = FD._p;

	gint h = gtk_combo_box_get_active( combo);
	if ( h > 0 )
		FD.field_channel = SF.using_channel = &SF[h];
}


void
wSFFD_show_cb(
	GtkWidget*,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	FD.setup_controls_for_find();
	FD.populate_combo();
	FD.set_profile_manage_buttons_visibility();

	if ( not FD._p.using_channel ) // not invoked for a preselected signal via a menu
		FD._p.using_channel = &FD._p.channels.front();
	FD.field_channel = FD.field_channel_saved = FD._p.using_channel;

	FD.preselect_channel( FD._p.using_channel_idx());
}



void
wSFFD_hide_cb(
	GtkWidget*,
	gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;
	auto& SF = FD._p;

	if ( not FD.occurrences.empty() ) { // closing while dialog is in matching state
		if ( GTK_RESPONSE_YES !=
		     pop_question( SF.wSF, "Keep annotations?") )
			FD.restore_annotations();
		else {
			SF._p.populate_mGlobalAnnotations();
			SF.queue_redraw_all();
		}
	}

	gtk_toggle_button_set_active( (GtkToggleButton*)FD.field_channel->_p.bSFShowFindDialog, FALSE);
}



gboolean
wSFFD_configure_event_cb(
	GtkWidget*,
	GdkEventConfigure *event,
	const gpointer userdata)
{
	auto& FD = *(SScoringFacility::SPatternsDialog*)userdata;

	if ( event->type == GDK_CONFIGURE ) {
		int marijke = gtk_widget_get_allocated_width( (GtkWidget*)FD.swSFFDThing);
		FD.set_thing_da_width( marijke);
		FD.set_field_da_width( marijke);
	}
	return FALSE;
}

} // extern "C"



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
