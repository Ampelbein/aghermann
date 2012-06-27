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


#include "scoring-facility.hh"
#include "scoring-facility_cb.hh"

using namespace std;



int
aghui::SScoringFacility::SFiltersDialog::
construct_widgets()
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

	g_signal_connect( (GObject*)eFilterHighPassCutoff, "value-changed",
			  (GCallback)eFilterHighPassCutoff_value_changed_cb,
			  this);
	g_signal_connect( (GObject*)eFilterLowPassCutoff, "value-changed",
			  (GCallback)eFilterLowPassCutoff_value_changed_cb,
			  this);
	return 0;
}



aghui::SScoringFacility::SFiltersDialog::
~SFiltersDialog()
{
	gtk_widget_destroy( (GtkWidget*)wFilters);
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


// eof
