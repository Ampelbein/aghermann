// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-ica_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-10-30
 *
 *         Purpose:  scoring facility widget callbacks (ICA controls)
 *
 *         License:  GPL
 */


#include "scoring-facility.hh"


using namespace std;
using namespace aghui;


extern "C" {


void
bScoringFacRunICA_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.mode = aghui::SScoringFacility::TMode::showing_ics;
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacScoringModeContainer, FALSE);
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacICAModeContainer, TRUE);

	gtk_widget_set_sensitive( (GtkWidget*)SF.bScoringFacICATry, TRUE);
	gtk_widget_set_sensitive( (GtkWidget*)SF.bScoringFacICAPreview, FALSE);

	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFAccept, FALSE);
	SF.set_tooltip( aghui::SScoringFacility::TTipIdx::ica_mode);

	SF.setup_ica();

	SF.queue_redraw_all();
}



void
eSFICANonlinearity_changed_cb( GtkComboBox* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	static int vv[] = {
		FICA_NONLIN_POW3,
		FICA_NONLIN_TANH,
		FICA_NONLIN_GAUSS,
		FICA_NONLIN_SKEW
	};
	int select = gtk_combo_box_get_active( w);
	SF.ica->obj().set_non_linearity( vv[select]);

	gtk_widget_set_sensitive( (GtkWidget*)SF.eSFICAa1, vv[select] == FICA_NONLIN_TANH);
	gtk_widget_set_sensitive( (GtkWidget*)SF.eSFICAa2, vv[select] == FICA_NONLIN_GAUSS);
}

void
eSFICAApproach_changed_cb( GtkComboBox* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	static int vv[] = {
		FICA_APPROACH_SYMM,
		FICA_APPROACH_DEFL,
	};
	SF.ica->obj().set_approach( vv[gtk_combo_box_get_active( w)]);
}

void
eSFICAFineTune_toggled_cb( GtkCheckButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_fine_tune( (bool)gtk_toggle_button_get_active( (GtkToggleButton*)w));
}

void
eSFICAStabilizationMode_toggled_cb( GtkCheckButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_stabilization( (bool)gtk_toggle_button_get_active( (GtkToggleButton*)w));
}

void
eSFICAa1_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_a1( gtk_spin_button_get_value( w));
}

void
eSFICAa2_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_a2( gtk_spin_button_get_value( w));
}

void
eSFICAmu_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_mu( gtk_spin_button_get_value( w));
}

void
eSFICAepsilon_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_epsilon( gtk_spin_button_get_value( w));
}

void
eSFICASampleSizePercent_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_sample_size( gtk_spin_button_get_value( w)/100);
}

void
eSFICANofICs_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	int n = (int)roundf( gtk_spin_button_get_value( w));
	SF.ica->obj().set_nrof_independent_components( n);

	gtk_adjustment_set_upper( SF.jSFICAEigVecFirst, n);
	gtk_adjustment_set_upper( SF.jSFICAEigVecLast, n);
}

void
eSFICAEigVecFirst_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	int n = (int)roundf( gtk_spin_button_get_value( w));
	SF.ica->obj().set_first_eig( n);
	gtk_adjustment_set_lower( SF.jSFICAEigVecLast, n);
}

void
eSFICAEigVecLast_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	int n = (int)roundf( gtk_spin_button_get_value( w));
	SF.ica->obj().set_last_eig( n);
	gtk_adjustment_set_upper( SF.jSFICAEigVecFirst, n);
}

void
eSFICAMaxIterations_value_changed_cb( GtkSpinButton* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	SF.ica->obj().set_max_num_iterations( (int)roundf( gtk_spin_button_get_value( w)));
}

void
eSFICARemixMode_changed_cb( GtkComboBox* w, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	static aghui::SScoringFacility::TICARemixMode vv[] = {
		aghui::SScoringFacility::TICARemixMode::punch,
		aghui::SScoringFacility::TICARemixMode::map,
	};
	SF.remix_mode = vv[gtk_combo_box_get_active( w)];
	SF.ica_map = vector<int> (SF.ica_components.rows(), -1);

	SF.queue_redraw_all();
}




void
bScoringFacICATry_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.mode = aghui::SScoringFacility::TMode::separating; // strictly to have draw_montage display a banner
	SF.queue_redraw_all();

	SF.run_ica();

	SF.mode = aghui::SScoringFacility::TMode::showing_ics;
	gtk_widget_set_sensitive( (GtkWidget*)SF.bScoringFacICAPreview, TRUE);

	SF.queue_redraw_all();
}

void
bScoringFacICAPreview_toggled_cb( GtkToggleButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;

	SF.remix_ics();

	if ( gtk_toggle_button_get_active(button) ) {
		SF.mode = aghui::SScoringFacility::TMode::showing_remixed;
		gtk_widget_set_sensitive( (GtkWidget*)SF.bScoringFacICATry, FALSE);
	} else {
		SF.mode = aghui::SScoringFacility::TMode::showing_ics;
		gtk_widget_set_sensitive( (GtkWidget*)SF.bScoringFacICATry, TRUE);
	}

	SF.queue_redraw_all();
}

void
bScoringFacICAApply_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.apply_remix(
		gtk_toggle_button_get_active( (GtkToggleButton*)SF.eSFICAApplyToEEGChannelsOnly));

	SF.mode = aghui::SScoringFacility::TMode::scoring;
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacScoringModeContainer, TRUE);
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacICAModeContainer, FALSE);
	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFAccept, TRUE);
	SF.set_tooltip( aghui::SScoringFacility::TTipIdx::scoring_mode);

	SF.queue_redraw_all();
}

void
bScoringFacICACancel_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.mode = aghui::SScoringFacility::TMode::scoring;
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacScoringModeContainer, TRUE);
	gtk_widget_set_visible( (GtkWidget*)SF.cScoringFacICAModeContainer, FALSE);
	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFAccept, TRUE);
	SF.set_tooltip( aghui::SScoringFacility::TTipIdx::scoring_mode);

	SF.queue_redraw_all();
}




} // extern "C"

// eof
