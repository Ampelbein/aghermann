// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-filter.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-30
 *
 *         Purpose:  scoring facility Butterworth filter dialog
 *
 *         License:  GPL
 */


#include <sys/stat.h>

#include "misc.hh"
#include "scoring-facility.hh"

using namespace std;
using namespace aghui;



int
aghui::SScoringFacility::SFiltersDialog::construct_widgets()
{
	GtkCellRenderer *renderer;

      // ------- wFilter
	if ( !(AGH_GBGETOBJ3 (_p.builder, GtkDialog,		wFilters)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkLabel,		lFilterCaption)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,	eFilterLowPassCutoff)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,	eFilterHighPassCutoff)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,	eFilterLowPassOrder)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,	eFilterHighPassOrder)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkComboBox,		eFilterNotchFilter)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkListStore,		mFilterNotchFilter)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkButton,		bFilterOK)) )
		return -1;

	gtk_combo_box_set_model( eFilterNotchFilter,
				 (GtkTreeModel*)mFilterNotchFilter);
	gtk_combo_box_set_id_column( eFilterNotchFilter, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eFilterNotchFilter, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eFilterNotchFilter, renderer,
					"text", 0,
					NULL);

	g_signal_connect_after( (GObject*)eFilterHighPassCutoff, "value-changed",
				(GCallback)eFilterHighPassCutoff_value_changed_cb,
				this);
	g_signal_connect_after( (GObject*)eFilterLowPassCutoff, "value-changed",
				(GCallback)eFilterLowPassCutoff_value_changed_cb,
				this);
	return 0;
}




// bool
// aghui::SScoringFacility::SChannel::validate_filters()
// {
// 	if ( low_pass.cutoff >= 0. && low_pass.order < 6 &&
// 	     high_pass.cutoff >= 0. && high_pass.order < 6
// 	     && ((low_pass.cutoff > 0. && high_pass.cutoff > 0. && high_pass.cutoff < low_pass.cutoff)
// 		 || high_pass.cutoff == 0. || low_pass.cutoff == 0.) )
// 		return true;
// 	low_pass.cutoff = high_pass.cutoff = 0;
// 	low_pass.order = high_pass.order = 1;
// 	return false;
// }




extern "C" {

void
iSFPageFilter_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD = SF.filters_dialog;
	gtk_spin_button_set_value( FD.eFilterLowPassCutoff,
				   SF.using_channel->filters.low_pass_cutoff);
	gtk_spin_button_set_value( FD.eFilterLowPassOrder,
				   SF.using_channel->filters.low_pass_order);
	gtk_spin_button_set_value( FD.eFilterHighPassCutoff,
				   SF.using_channel->filters.high_pass_cutoff);
	gtk_spin_button_set_value( FD.eFilterHighPassOrder,
				   SF.using_channel->filters.high_pass_order);
	gtk_combo_box_set_active( FD.eFilterNotchFilter,
				  (int)SF.using_channel->filters.notch_filter);

	snprintf_buf( "<big>Filters for channel <b>%s</b></big>", SF.using_channel->name);
	gtk_label_set_markup( FD.lFilterCaption,
			      __buf__);

	if ( gtk_dialog_run( FD.wFilters) == GTK_RESPONSE_OK ) {
		SF.using_channel -> filters.low_pass_cutoff
			= roundf( gtk_spin_button_get_value( FD.eFilterLowPassCutoff)*10) / 10;
		SF.using_channel -> filters.low_pass_order
			= roundf( gtk_spin_button_get_value( FD.eFilterLowPassOrder)*10) / 10;
		SF.using_channel -> filters.high_pass_cutoff
			= roundf( gtk_spin_button_get_value( FD.eFilterHighPassCutoff)*10) / 10;
		SF.using_channel -> filters.high_pass_order
			= roundf( gtk_spin_button_get_value( FD.eFilterHighPassOrder)*10) / 10;
		SF.using_channel -> filters.notch_filter =
			(sigfile::SFilterPack::TNotchFilter)gtk_combo_box_get_active( FD.eFilterNotchFilter);

		SF.using_channel->get_signal_filtered();

		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
	}
}



void
eFilterHighPassCutoff_value_changed_cb( GtkSpinButton *spinbutton,
					gpointer       userdata)
{
	auto& FD = *(SScoringFacility::SFiltersDialog*)userdata;
	double other_freq = gtk_spin_button_get_value( FD.eFilterLowPassCutoff);
	gtk_widget_set_sensitive( (GtkWidget*)FD.bFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) < other_freq);
}

void
eFilterLowPassCutoff_value_changed_cb( GtkSpinButton *spinbutton,
				       gpointer       userdata)
{
	auto& FD = *(SScoringFacility::SFiltersDialog*)userdata;
	gdouble other_freq = gtk_spin_button_get_value( FD.eFilterHighPassCutoff);
	gtk_widget_set_sensitive( (GtkWidget*)FD.bFilterOK,
				  fdim( other_freq, 0.) < 1e-5 || gtk_spin_button_get_value( spinbutton) > other_freq);
}


} // extern "C"


// eof
