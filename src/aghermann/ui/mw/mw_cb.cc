/*
 *       File name:  aghermann/ui/mw/mw_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI widget callbacks
 *
 *         License:  GPL
 */

#include <gdk/gdkkeysyms.h>

#include "aghermann/ui/misc.hh"
#include "aghermann/ui/sf/sf.hh"
#include "mw.hh"
#include "mw_cb.hh"

using namespace std;
using namespace aghui;

extern "C" {

gboolean
wMainWindow_configure_event_cb(
	GtkWidget*,
	GdkEventConfigure *event,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( event->type == GDK_CONFIGURE )
		ED.geometry = {
			event -> x,
			event -> y,
			event -> width,
			event -> height
		};
	return FALSE; // whatever
}

gboolean
wMainWindow_delete_event_cb(
	GtkWidget*,
	GdkEvent*,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( ED.nodestroy_by_cb )
		return TRUE;

	// if the user closes the main window while in a settings tab,
	// ensure we switch away from it and trigger any saving callbacks
	gtk_notebook_set_current_page( ED.tDesign, 0);
	gtk_notebook_set_current_page( ED.tSimulations, 0);

	iExpClose_activate_cb( NULL, userdata);

	return TRUE; // whatever
}


namespace {

inline void
cycle_combo( GtkComboBox* c, const int n, const int by)
{
	gtk_combo_box_set_active(
		c, (gtk_combo_box_get_active( c) + n + by) % n);
}
}

gboolean
wMainWindow_key_press_event_cb(
	GtkWidget*,
	GdkEventKey* event,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	int by = (event->state & GDK_SHIFT_MASK) ? -1 : 1;

	switch ( event->keyval ) {
	case GDK_KEY_F1:
		cycle_combo(
			ED.eMsmtProfileType, 3, // three profiles
			by);
		return TRUE;
	case GDK_KEY_F2:
		cycle_combo(
			ED.eMsmtSession, ED.AghDD.size(),
			by);
		return TRUE;
	case GDK_KEY_F3:
		cycle_combo(
			ED.eMsmtChannel, ED.AghTT.size(),
			by);
		return TRUE;
	}

	return FALSE;
}


// tab switch
void
tTaskSelector_switch_page_cb(
	GtkNotebook*,
	gpointer,
	const guint page_num,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( page_num == 1 ) {
		//ED.populate( false);
		snprintf_buf( "Session: <b>%s</b>", ED.AghD());
		gtk_label_set_markup( ED.lSimulationsSession, __buf__);
		snprintf_buf( "Channel: <b>%s</b>", ED.AghT());
		gtk_label_set_markup( ED.lSimulationsChannel, __buf__);
		snprintf_buf( "Metric: <b>%s</b>", metrics::name( ED.display_profile_type));
		gtk_label_set_markup( ED.lSimulationsProfile, __buf__);
		gtk_widget_set_sensitive( (GtkWidget*)ED.iExpClose, FALSE);
		ED.populate_2();
	} else if ( page_num == 0 ) {
		// ED.ED->remove_untried_modruns(); // done in populate_2
		// ED.populate( false);
		gtk_widget_set_sensitive( (GtkWidget*)ED.iExpClose, TRUE);
	}
}



// other main toolbar controls

void
eMsmtProfileAutoscale_toggled_cb(
	GtkToggleButton* b,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( (ED.autoscale = (bool)gtk_toggle_button_get_active(b)) ) {
		ED.calculate_profile_scale();
		gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
	}
}

void
eMsmtProfileSmooth_value_changed_cb(
	GtkScaleButton* b,
	const gdouble v,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.smooth_profile = v;
	snprintf_buf( "Smooth: %zu", ED.smooth_profile);
	gtk_button_set_label( (GtkButton*)b, __buf__);
	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}


void
eMsmtProfileType_changed_cb(
	GtkComboBox* b,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	switch ( gtk_combo_box_get_active( b) ) {
	case 0:
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParamsPSD, TRUE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParamsSWU, FALSE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParamsMC, FALSE);
		ED.display_profile_type = metrics::TType::psd;
	    break;
	case 1:
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParamsPSD, FALSE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParamsSWU, TRUE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParamsMC, FALSE);
		ED.display_profile_type = metrics::TType::swu;
		// set adjustment inc and upper like it's done for MC, below
	    break;
	case 2:
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParamsPSD, FALSE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParamsSWU, FALSE);
		gtk_widget_set_visible( (GtkWidget*)ED.cMsmtProfileParamsMC, TRUE);
		ED.display_profile_type = metrics::TType::mc;
	    break;
	}

	auto params = ED.make_active_profile_paramset();
	// don't let it throw on insufficiently scored recordings
	params.req_percent_scored	    = 0.;
	params.swa_laden_pages_before_SWA_0 = 0u;

	// collect profiles that need to be re-created
	SBusyBlock *bb = nullptr;
	vector<agh::CProfile*> redo_profiles;
	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cprofile and J.cprofile->need_compute( params) ) {
				if ( !bb )
					bb = new SBusyBlock (ED.wMainWindow);
				redo_profiles.push_back( J.cprofile);
			}

	size_t global_i = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(guided)
#endif
	for ( size_t i = 0; i < redo_profiles.size(); ++i ) {
#ifdef _OPENMP
#pragma omp critical
#endif
		{
			auto& P = *redo_profiles[i];
			ED.sb_main_progress_indicator(
				(string ("Compute ") + P.subject() + "/" + P.session() + "/" + P.channel()).c_str(),
				redo_profiles.size(), ++global_i,
				TGtkRefreshMode::gtk);
		}

		redo_profiles[i]->create_timeline( params);
	}
	ED.sb_clear();

	// do it for all the rest (those needing heavy recompute will be fetched from cache)
	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cprofile )
				J.cprofile->create_timeline( params);


	if ( ED.profile_scale_psd == 0. || ED.profile_scale_swu == 0. || ED.profile_scale_mc == 0. ||  // don't know which
		ED.autoscale )
		ED.calculate_profile_scale();

	ED.adjust_op_freq_spinbuttons();

	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);

	if ( bb )
		delete bb;
}



namespace {
void
mike_dewhirst_is_not_real( SExpDesignUI& ED)
{
	if ( ED.suppress_redraw )
		return;

	auto params = ED.make_active_profile_paramset();
	params.req_percent_scored	    = 0.;
	params.swa_laden_pages_before_SWA_0 = 0u;
	params.score_unscored_as_wake	    = false;

	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cprofile )
				J.cprofile->create_timeline( params);
	if ( ED.autoscale )
		ED.calculate_profile_scale();

	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}

}; // namespace

void
eMsmtProfileParamsPSDFreqFrom_value_changed_cb(
	GtkSpinButton *spinbutton,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.active_profile_psd_freq_from = gtk_spin_button_get_value( spinbutton);
	ED.active_profile_psd_freq_upto =
		ED.active_profile_psd_freq_from + gtk_spin_button_get_value( ED.eMsmtProfileParamsPSDFreqWidth);
	if ( ED.suppress_redraw )
		return;

	mike_dewhirst_is_not_real(ED);
}

void
eMsmtProfileParamsPSDFreqWidth_value_changed_cb(
	GtkSpinButton *spinbutton,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.active_profile_psd_freq_upto =
		ED.active_profile_psd_freq_from + gtk_spin_button_get_value( spinbutton);
	if ( ED.suppress_redraw )
		return;

	mike_dewhirst_is_not_real(ED);
}




void
eMsmtProfileParamsSWUF0_value_changed_cb(
	GtkSpinButton *spinbutton,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.active_profile_swu_f0 = gtk_spin_button_get_value( spinbutton);

	mike_dewhirst_is_not_real(ED);
}




void
eMsmtProfileParamsMCF0_value_changed_cb(
	GtkSpinButton *spinbutton,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.active_profile_mc_f0 = gtk_spin_button_get_value( spinbutton);

	mike_dewhirst_is_not_real(ED);
}







// session and channel selection

void
eMsmtSession_changed_cb(
	GtkComboBox *combobox,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto oldval = ED._AghDi;
	ED._AghDi = find( ED.AghDD.begin(), ED.AghDD.end(),
			  gtk_combo_box_get_active_id( combobox));
	if ( oldval != ED._AghDi )
		ED.populate_1();
	if ( ED.autoscale )
		ED.calculate_profile_scale();
}

void
eMsmtChannel_changed_cb(
	GtkComboBox *combobox,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto oldval = ED._AghTi;
	auto newval = gtk_combo_box_get_active_id( combobox);
	ED._AghTi = find( ED.AghTT.begin(), ED.AghTT.end(),
			  newval);
	ED._AghHi = find( ED.AghHH.begin(), ED.AghHH.end(),
			  newval);
	if ( /* _AghTi != AghTT.end() && */ oldval != ED._AghTi )
		ED.populate_1();
	if ( ED.autoscale )
		ED.calculate_profile_scale();
}





void
bMainCloseThatSF_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	if ( ED.close_this_SF_now == nullptr ) {
		fprintf( stderr, "Hey, keep your fingers off this button!\n");
		return;
	}

	delete ED.close_this_SF_now;
	ED.close_this_SF_now = nullptr;
}

} // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
