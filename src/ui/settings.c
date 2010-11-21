// ;-*-C-*- *  Time-stamp: "2010-11-21 03:17:45 hmmr"
/*
 *       File name:  settings.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  FFT and annealing control settings
 *
 *         License:  GPL
 */


#include <math.h>
#include <glade/glade.h>
#include "../core/iface.h"
#include "misc.h"
#include "ui.h"


static GtkWidget
	*eCtlParamAnnlNTries,
	*eCtlParamAnnlItersFixedT,
	*eCtlParamAnnlStepSize,
	*eCtlParamAnnlBoltzmannk,
	*eCtlParamAnnlTInitial,
	*eCtlParamAnnlDampingMu,
	*eCtlParamAnnlTMinMantissa,
	*eCtlParamAnnlTMinExponent,
	*eCtlParamDBAmendment1,
	*eCtlParamDBAmendment2,
	*eCtlParamAZAmendment,
	*eCtlParamScoreMVTAsWake,
	*eCtlParamScoreUnscoredAsWake,
//	*eCtlParamScoreMVTAsPrevScore,
//	*eCtlParamScoreUnscoredAsPrevScore,

	*eFFTParamsBinSize,
	*eFFTParamsWindowType,
	*eFFTParamsPageSize,
//	*eArtifGlitchMag,
	*eArtifSmoothOver,
	*eArtifWindowType;





// ------------------ construct

gint
agh_ui_construct_Settings( GladeXML *xml)
{
	GtkCellRenderer *renderer;

     // ------------- eCtrlParam*
	if ( !(eCtlParamAnnlNTries		= glade_xml_get_widget( xml, "eCtlParamAnnlNTries")) ||
	     !(eCtlParamAnnlItersFixedT		= glade_xml_get_widget( xml, "eCtlParamAnnlItersFixedT")) ||
	     !(eCtlParamAnnlStepSize		= glade_xml_get_widget( xml, "eCtlParamAnnlStepSize")) ||
	     !(eCtlParamAnnlBoltzmannk		= glade_xml_get_widget( xml, "eCtlParamAnnlBoltzmannk")) ||
	     !(eCtlParamAnnlTInitial		= glade_xml_get_widget( xml, "eCtlParamAnnlTInitial")) ||
	     !(eCtlParamAnnlDampingMu		= glade_xml_get_widget( xml, "eCtlParamAnnlDampingMu")) ||
	     !(eCtlParamAnnlTMinMantissa	= glade_xml_get_widget( xml, "eCtlParamAnnlTMinMantissa")) ||
	     !(eCtlParamAnnlTMinExponent	= glade_xml_get_widget( xml, "eCtlParamAnnlTMinExponent")) ||
	     !(eCtlParamDBAmendment1		= glade_xml_get_widget( xml, "eCtlParamDBAmendment1")) ||
	     !(eCtlParamDBAmendment2		= glade_xml_get_widget( xml, "eCtlParamDBAmendment2")) ||
	     !(eCtlParamAZAmendment		= glade_xml_get_widget( xml, "eCtlParamAZAmendment")) ||
	     !(eCtlParamScoreMVTAsWake			= glade_xml_get_widget( xml, "eCtlParamScoreMVTAsWake")) ||
	     !(eCtlParamScoreUnscoredAsWake		= glade_xml_get_widget( xml, "eCtlParamScoreUnscoredAsWake"))
		)
		return -1;

     // ------------- fFFTParams
	if ( !(eFFTParamsBinSize    = glade_xml_get_widget( xml, "eFFTParamsBinSize")) ||
	     !(eFFTParamsPageSize   = glade_xml_get_widget( xml, "eFFTParamsPageSize")) ||
	     !(eFFTParamsWindowType = glade_xml_get_widget( xml, "eFFTParamsWindowType")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eFFTParamsPageSize),
				 GTK_TREE_MODEL (agh_mFFTParamsPageSize));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT (eFFTParamsPageSize), renderer, FALSE);
	gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT (eFFTParamsPageSize), renderer,
					"text", 0,
					NULL);

	gtk_combo_box_set_model( GTK_COMBO_BOX (eFFTParamsWindowType),
				 GTK_TREE_MODEL (agh_mFFTParamsWindowType));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eFFTParamsWindowType), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eFFTParamsWindowType), renderer,
					"text", 0,
					NULL);

     // ------------- fArtifacts & misc
	if ( !(eArtifSmoothOver      = glade_xml_get_widget( xml, "eArtifSmoothOver")) ||
//	     !(eArtifGlitchMag	     = glade_xml_get_widget( xml, "eArtifGlitchMag"))  ||
	     !(eArtifWindowType	     = glade_xml_get_widget( xml, "eArtifWindowType")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eArtifWindowType),
				 GTK_TREE_MODEL (agh_mAfDampingWindowType));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eArtifWindowType), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eArtifWindowType), renderer,
					"text", 0,
					NULL);

	return 0;
}









gboolean
fSimParamCtlParm_expose_event_cb()
{
	struct SConsumerCtlParams ctl_params;
	agh_ctlparams0_get( &ctl_params);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlNTries),	ctl_params.siman_params.n_tries);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlItersFixedT),	ctl_params.siman_params.iters_fixed_T);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlStepSize),	ctl_params.siman_params.step_size);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlBoltzmannk),	ctl_params.siman_params.k);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTInitial),	ctl_params.siman_params.t_initial);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlDampingMu),	ctl_params.siman_params.mu_t);
	char buffer[16];
	float mantissa;
	int exponent;
	snprintf( buffer, 15, "%le", ctl_params.siman_params.t_min);
	sscanf( buffer, "%fe%d", &mantissa, &exponent);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinMantissa),	mantissa);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinExponent),	exponent);

	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment1),	ctl_params.DBAmendment1);
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamDBAmendment2),	ctl_params.DBAmendment2);
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamAZAmendment),		ctl_params.AZAmendment);


	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamScoreMVTAsWake),
				      ctl_params.ScoreMVTAsWake);
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eCtlParamScoreUnscoredAsWake),
				      ctl_params.ScoreUnscoredAsWake);

	return FALSE;
}








#define DO_PRE \
	struct SConsumerCtlParams ctlparams; \
	agh_ctlparams0_get(&ctlparams);

#define DO_POST \
	agh_ctlparams0_put(&ctlparams);

void eCtlParamAnnlNTries_value_changed_cb( GtkSpinButton *e, gpointer u)	{ DO_PRE; ctlparams.siman_params.n_tries       = gtk_spin_button_get_value(e); DO_POST; }
void eCtlParamAnnlItersFixedT_value_changed_cb( GtkSpinButton *e, gpointer u)	{ DO_PRE; ctlparams.siman_params.iters_fixed_T = gtk_spin_button_get_value(e); DO_POST; }
void eCtlParamAnnlStepSize_value_changed_cb( GtkSpinButton *e, gpointer u)	{ DO_PRE; ctlparams.siman_params.step_size     = gtk_spin_button_get_value(e); DO_POST; }
void eCtlParamAnnlBoltzmannk_value_changed_cb( GtkSpinButton *e, gpointer u)	{ DO_PRE; ctlparams.siman_params.k             = gtk_spin_button_get_value(e); DO_POST; }
void eCtlParamAnnlTInitial_value_changed_cb( GtkSpinButton *e, gpointer u)	{ DO_PRE; ctlparams.siman_params.t_initial     = gtk_spin_button_get_value(e); DO_POST; }
void eCtlParamAnnlDampingMu_value_changed_cb( GtkSpinButton *e, gpointer u)	{ DO_PRE; ctlparams.siman_params.mu_t          = gtk_spin_button_get_value(e); DO_POST; }
void eCtlParamAnnlTMinMantissa_value_changed_cb( GtkSpinButton *e, gpointer u)	{ DO_PRE; ctlparams.siman_params.t_min = gtk_spin_button_get_value(e)
												  * pow(10, gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinExponent))); DO_POST; }
void eCtlParamAnnlTMinExponent_value_changed_cb( GtkSpinButton *e, gpointer u)	{ DO_PRE; ctlparams.siman_params.t_min = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eCtlParamAnnlTMinMantissa))
												  * pow(10, gtk_spin_button_get_value(e)); DO_POST; }
void eCtlParamScoreMVTAs_toggled_cb( GtkToggleButton *e, gpointer u)		{ DO_PRE; ctlparams.ScoreMVTAsWake      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (e)); DO_POST; }
void eCtlParamScoreUnscoredAs_toggled_cb( GtkToggleButton *e, gpointer u)	{ DO_PRE; ctlparams.ScoreUnscoredAsWake = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (e)); DO_POST; }

// #define UPDATE_HOOK
// 	agh_collect_simulations_from_tree(), agh_populate_mSimulations()

void eCtlParamDBAmendment1_toggled_cb( GtkToggleButton *e, gpointer u)	{ DO_PRE; ctlparams.DBAmendment1 = gtk_toggle_button_get_active(e); DO_POST; }
void eCtlParamDBAmendment2_toggled_cb( GtkToggleButton *e, gpointer u)	{ DO_PRE; ctlparams.DBAmendment2 = gtk_toggle_button_get_active(e); DO_POST; }
void eCtlParamAZAmendment_toggled_cb( GtkToggleButton *e, gpointer u)	{ DO_PRE; ctlparams.AZAmendment  = gtk_toggle_button_get_active(e); DO_POST; }



guint AghFFTPageSizeValues[] = {
	15,	20,	30,	60,
	(guint)-1
};

guint AghFFTPageSizeCurrent = 2;

void
fFFTParams_map_cb()
{
	guint i = 0;
	while ( AghFFTPageSizeValues[i] != (guint)-1 && AghFFTPageSizeValues[i] < agh_fft_get_pagesize() )
		++i;
	gtk_combo_box_set_active( GTK_COMBO_BOX (eFFTParamsPageSize), AghFFTPageSizeCurrent = i);

	gtk_combo_box_set_active( GTK_COMBO_BOX (eFFTParamsWindowType), agh_fft_get_window_type());
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eFFTParamsBinSize), agh_fft_get_binsize());
}



void
eFFTParamsPageSize_changed_cb()
{
	agh_fft_set_pagesize( AghFFTPageSizeValues[ AghFFTPageSizeCurrent = gtk_combo_box_get_active( GTK_COMBO_BOX (eFFTParamsPageSize))] );
	AghDisplayPageSizeItem = 0;
	while ( AghDisplayPageSizeValues[AghDisplayPageSizeItem] != AghFFTPageSizeValues[AghFFTPageSizeCurrent] )
		if ( ++AghDisplayPageSizeItem > 10 )
			abort();
}

void
eFFTParamsWindowType_changed_cb()
{
	agh_fft_set_window_type( gtk_combo_box_get_active( GTK_COMBO_BOX (eFFTParamsWindowType)));
}


void
eFFTParamsBinSize_value_changed_cb()
{
	agh_fft_set_binsize( gtk_spin_button_get_value( GTK_SPIN_BUTTON (eFFTParamsBinSize)));
}






void
fArtifacts_map_cb()
{
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eArtifSmoothOver), agh_af_get_smoothover());
	gtk_combo_box_set_active( GTK_COMBO_BOX (eArtifWindowType), agh_af_get_window_type());
}

// void
// eArtifGlitchMag_value_changed_cb()
// {
// 	AghAfGlitchMag = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eArtifGlitchMag));
// }

void
eArtifSmoothOver_value_changed_cb()
{
	agh_af_set_smoothover( gtk_spin_button_get_value( GTK_SPIN_BUTTON (eArtifSmoothOver)));
}

void
eArtifWindowType_changed_cb()
{
	agh_af_set_window_type( gtk_combo_box_get_active( GTK_COMBO_BOX (eArtifWindowType)));
}





void
tDesign_switch_page_cb( GtkNotebook     *notebook,
			GtkNotebookPage *page,
			guint            page_num,
			gpointer         user_data)
{
	static size_t
		AghFFTPageSizeCurrent_saved,
		AghFFTWindowType_saved,
		AghAfDampingWindowType_saved;
	static float
		AghFFTBinSize_saved;

	if ( page_num == 0 ) {
		if ( AghFFTPageSizeCurrent_saved != AghFFTPageSizeCurrent ||
		     AghFFTWindowType_saved != agh_fft_get_window_type() ||
		     AghAfDampingWindowType_saved != agh_af_get_window_type() ||
		     AghFFTBinSize_saved != agh_fft_get_binsize() ) {
			set_cursor_busy( TRUE, wMainWindow);
			gtk_widget_set_sensitive( wMainWindow, FALSE);
			while ( gtk_events_pending() )
				gtk_main_iteration();
			agh_expdesign_scan_tree( progress_indicator);
			agh_ui_populate();

			set_cursor_busy( FALSE, wMainWindow);
			gtk_widget_set_sensitive( wMainWindow, TRUE);
			gtk_statusbar_push( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General,
					    "Scanning complete");
		}
	} else {
		AghFFTPageSizeCurrent_saved = AghFFTPageSizeCurrent;
		AghFFTWindowType_saved = agh_fft_get_window_type();
		AghAfDampingWindowType_saved = agh_af_get_window_type();
		AghFFTBinSize_saved = agh_fft_get_binsize();
	}
}





void
tTaskSelector_switch_page_cb( GtkNotebook     *notebook,
			      GtkNotebookPage *page,
			      guint            page_num,
			      gpointer         user_data)
{
	if ( page_num == 1 ) {
		agh_populate_mSimulations( TRUE);
	} else if ( page_num == 0 ) {
		agh_cleanup_mSimulations();
	}
}


// EOF
