// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-ica_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-10-30
 *
 *         Purpose:  scoring facility widget callbacks (ICA controls)
 *
 *         License:  GPL
 */

#include "ui/misc.hh"
#include "sf.hh"
#include "sf-widgets.hh"

using namespace std;
using namespace aghui;

extern "C" {

void
bSFRunICA_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.setup_ica() == 0 ) {
		SF.mode = aghui::SScoringFacility::TMode::showing_ics;
		gtk_widget_set_visible( (GtkWidget*)SF.cSFScoringModeContainer, FALSE);
		gtk_widget_set_visible( (GtkWidget*)SF.cSFICAModeContainer, TRUE);

		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICATry, TRUE);
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAShowMatrix, FALSE);
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAPreview, FALSE);
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAApply, FALSE);

		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFAccept, FALSE);
		SF.set_tooltip( aghui::SScoringFacility::TTipIdx::ica_mode);
		SF.queue_redraw_all();
	}
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
		aghui::SScoringFacility::TICARemixMode::zero,
	};
	SF.remix_mode = vv[gtk_combo_box_get_active( w)];
	SF.ica_map.assign( SF.ica_map.size(), {{-1}});

	SF.queue_redraw_all();
}




void
bSFICATry_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.mode = aghui::SScoringFacility::TMode::separating; // strictly to have draw_montage display a banner

	SF.queue_redraw_all();
	aghui::SBusyBlock bb (SF.wSF);

	SF.run_ica();

	SF.mode = aghui::SScoringFacility::TMode::showing_ics;
	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAPreview, TRUE);
	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAShowMatrix, TRUE);

	SF.queue_redraw_all();
}

void
bSFICAPreview_toggled_cb( GtkToggleButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;

	if ( gtk_toggle_button_get_active( button) ) {
		SF.remix_ics();
		SF.mode = aghui::SScoringFacility::TMode::showing_remixed;
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICATry, FALSE);
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAApply, TRUE);
	} else {
		SF.restore_ics();
		SF.mode = aghui::SScoringFacility::TMode::showing_ics;
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICATry, TRUE);
		gtk_widget_set_sensitive( (GtkWidget*)SF.bSFICAApply, FALSE);
	}

	SF.queue_redraw_all();
}

inline namespace {
void
mat2text_buffer( GtkTextBuffer *buffer, const itpp::mat& mx)
{
	gtk_text_buffer_set_text( buffer, "", -1);
	for ( int r = 0; r < mx.rows(); ++r ) {
		for ( int c = 0; c < mx.cols(); ++c ) {
			snprintf_buf( "\t% #6.3f", mx(r, c));
			gtk_text_buffer_insert_at_cursor(
				buffer, __buf__, -1);
		}
		if ( r + 1 < mx.rows() )
			gtk_text_buffer_insert_at_cursor( buffer, "\n", -1);
	}
}

} // inline namespace
void
bSFICAShowMatrix_toggled_cb( GtkToggleButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	mat2text_buffer(
		gtk_text_view_get_buffer( SF.tSFICAMatrix),
		SF.ica->obj().get_separating_matrix());

	if ( gtk_toggle_button_get_active( button) )
		gtk_widget_show_all( (GtkWidget*)SF.wSFICAMatrix);
	else
		gtk_widget_hide( (GtkWidget*)SF.wSFICAMatrix);
}

void
wSFICAMatrix_hide_cb( GtkWidget *widget, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	gtk_toggle_button_set_active( SF.bSFICAShowMatrix, FALSE);
}


void
bSFICAApply_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.apply_remix( true);

	SF.mode = aghui::SScoringFacility::TMode::scoring;
	gtk_widget_set_visible( (GtkWidget*)SF.cSFScoringModeContainer, TRUE);
	gtk_widget_set_visible( (GtkWidget*)SF.cSFICAModeContainer, FALSE);
	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFAccept, TRUE);
	SF.set_tooltip( aghui::SScoringFacility::TTipIdx::scoring_mode);

	SF.queue_redraw_all();
}

void
bSFICACancel_clicked_cb( GtkButton *button, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	// cleanup
	delete SF.ica;
	SF.ica = NULL;
	SF.ica_components = itpp::mat (0, 0);

	SF.mode = aghui::SScoringFacility::TMode::scoring;
	gtk_widget_set_visible( (GtkWidget*)SF.cSFScoringModeContainer, TRUE);
	gtk_widget_set_visible( (GtkWidget*)SF.cSFICAModeContainer, FALSE);
	gtk_widget_set_sensitive( (GtkWidget*)SF.bSFAccept, TRUE);
	SF.set_tooltip( aghui::SScoringFacility::TTipIdx::scoring_mode);

	SF.queue_redraw_all();
}




} // extern "C"

// eof
