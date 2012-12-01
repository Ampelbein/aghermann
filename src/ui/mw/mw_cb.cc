// ;-*-C++-*-
/*
 *       File name:  ui/mw/mw_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI widget callbacks
 *
 *         License:  GPL
 */

#include "ui/misc.hh"
#include "ui/sf/sf.hh"
#include "mw.hh"
#include "mw_cb.hh"

using namespace aghui;

extern "C" {

gboolean
wMainWindow_configure_event_cb( GtkWidget*, GdkEventConfigure *event, gpointer userdata)
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
wMainWindow_delete_event_cb( GtkWidget*, GdkEvent*, gpointer userdata)
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


// tab switch
void
tTaskSelector_switch_page_cb( GtkNotebook*, gpointer, guint page_num, gpointer userdata)
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
eMsmtProfileAutoscale_toggled_cb( GtkToggleButton* b, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( (ED.autoscale = (bool)gtk_toggle_button_get_active(b)) ) {
		ED.calculate_profile_scale();
		gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
	}
}

void
eMsmtProfileSmooth_value_changed_cb( GtkScaleButton* b, gdouble v, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.smooth_profile = v;
	snprintf_buf( "Smooth: %zu", ED.smooth_profile);
	gtk_button_set_label( (GtkButton*)b, __buf__);
	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}


void
eMsmtProfileType_changed_cb( GtkComboBox* b, gpointer userdata)
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
		gtk_adjustment_set_step_increment( ED.jMsmtProfileParamsMCF0,
						   ED.ED->mc_params.freq_inc); // matches the default in metrics/mc.cc
		gtk_adjustment_set_upper( ED.jMsmtProfileParamsMCF0,
					  ED.ED->mc_params.compute_n_bins(ED.pagesize()) *
					  ED.ED->mc_params.freq_inc);
		ED.display_profile_type = metrics::TType::mc;
	    break;
	}

//	aghui::SBusyBlock bb (ED.wMainWindow);
	auto params = ED.make_active_profile_paramset();
	// don't let it throw on insufficiently scored recordings
	params.req_percent_scored	= 0.;
	params.swa_laden_pages_before_SWA_0 = 0u;

	for ( auto &G : ED.groups )
		for ( auto &J : G )
			if ( J.cprofile )
				J.cprofile->create_timeline( params);

	if ( ED.profile_scale_psd == 0. || ED.profile_scale_mc == 0. ||  // don't know which
		ED.autoscale )
		ED.calculate_profile_scale();

	ED.adjust_op_freq_spinbuttons();

	gtk_widget_queue_draw( (GtkWidget*)ED.cMeasurements);
}



inline namespace {
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

}; // inline namespace

void
eMsmtProfileParamsPSDFreqFrom_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
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
eMsmtProfileParamsPSDFreqWidth_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.active_profile_psd_freq_upto =
		ED.active_profile_psd_freq_from + gtk_spin_button_get_value( spinbutton);
	if ( ED.suppress_redraw )
		return;

	mike_dewhirst_is_not_real(ED);
}




void
eMsmtProfileParamsSWUF0_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.active_profile_swu_f0 = gtk_spin_button_get_value( spinbutton);

	mike_dewhirst_is_not_real(ED);
}




void
eMsmtProfileParamsMCF0_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.active_profile_mc_f0 = gtk_spin_button_get_value( spinbutton);

	mike_dewhirst_is_not_real(ED);
}







// session and channel selection

void
eMsmtSession_changed_cb( GtkComboBox *combobox, gpointer userdata)
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
eMsmtChannel_changed_cb( GtkComboBox *combobox, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	auto oldval = ED._AghTi;
	ED._AghTi = find( ED.AghTT.begin(), ED.AghTT.end(),
		       gtk_combo_box_get_active_id( combobox));
	if ( /* _AghTi != AghTT.end() && */ oldval != ED._AghTi )
		ED.populate_1();
	if ( ED.autoscale )
		ED.calculate_profile_scale();
}





void
bMainCloseThatSF_clicked_cb( GtkButton*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	if ( ED.close_this_SF_now == nullptr ) {
		fprintf( stderr, "Hey, keep your fingers off this button!\n");
		return;
	}

	delete ED.close_this_SF_now;
	ED.close_this_SF_now = nullptr;
}




// -------- colours
void
bColourX_color_set_cb( GtkColorButton *widget,
		       gpointer        userdata)
{
	auto& mc = *(SManagedColor*)userdata;
	mc.acquire();
}

} // extern "C"

// eof
