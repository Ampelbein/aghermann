// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-ica.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-10-25
 *
 *         Purpose:  scoring facility (ICA methods)
 *
 *         License:  GPL
 */



#include <itpp/base/algebra/inv.h>
#include "../libica/ica.hh"
#include "misc.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;


int
aghui::SScoringFacility::setup_ica()
{
	if ( ica )
		delete ica;

	vector<TICASetupFun> src;
	size_t checking_sr = 0;
	for ( auto H = channels.begin(); H != channels.end(); ++H ) {
		size_t this_sr = H->crecording.F().samplerate(H->h());
		if ( checking_sr and this_sr != checking_sr ) {
			pop_ok_message( wScoringFacility,
					"Cannot perform ICA on channels with different sample rates.");
			return 1;
		} else
			checking_sr = this_sr;

		src.emplace_back(
			bind (&agh::CEDFFile::get_signal_filtered<int, double>, &H->crecording.F(), H->h()));
	}
	ica = new ica::CFastICA (src, checking_sr * pagesize() * total_pages());

      // initialize
	// has no independent default
	gtk_spin_button_set_value( eSFICANofICs, channels.size());
	g_signal_emit_by_name( eSFICANonlinearity,	"changed");
	g_signal_emit_by_name( eSFICAApproach,		"changed");
	g_signal_emit_by_name( eSFICAFineTune,		"toggled");
	g_signal_emit_by_name( eSFICAStabilizationMode,	"toggled");
	g_signal_emit_by_name( eSFICAa1,		"value-changed");
	g_signal_emit_by_name( eSFICAa2,		"value-changed");
	g_signal_emit_by_name( eSFICAmu,		"value-changed");
	g_signal_emit_by_name( eSFICAepsilon,		"value-changed");
	g_signal_emit_by_name( eSFICASampleSizePercent,	"value-changed");
	g_signal_emit_by_name( eSFICAMaxIterations,	"value-changed");

	// populate mSFICAPage
	gtk_container_foreach( (GtkContainer*)mSFICAPage, (GtkCallback)gtk_widget_destroy, NULL);
	GSList *group = NULL;
	for_each( channels.begin(), channels.end(),
		  [&] ( SChannel& H)
		  {
			  auto item = (GtkWidget*)gtk_radio_menu_item_new_with_label( group, H.name);
			  group = gtk_radio_menu_item_get_group( (GtkRadioMenuItem*)item);
			  g_object_set( (GObject*)item,
					"visible", TRUE,
					NULL);
			  g_signal_connect( (GObject*)item,
					    "activate", (GCallback)iSFICAPageMapIC_activate_cb,
					    this);
			  gtk_container_add( (GtkContainer*)mSFICAPage, item);
		  });
	GtkWidget *another;
	// add separator and a "(clean)" item
	gtk_container_add( (GtkContainer*)mSFICAPage,
			   another = gtk_separator_menu_item_new());
	g_object_set( (GObject*)another, "visible", TRUE, NULL);

	gtk_container_add( (GtkContainer*)mSFICAPage,
			   another = gtk_radio_menu_item_new_with_label( group, "(clean)"));
	g_object_set( (GObject*)another, "visible", TRUE, NULL);
	g_signal_connect( (GObject*)another,
			  "activate", (GCallback)iSFICAPageMapIC_activate_cb,
			  this);

	suppress_redraw = true;
	gtk_toggle_button_set_active( bScoringFacICAPreview, FALSE);
	suppress_redraw = false;

	return 0;
}


int
aghui::SScoringFacility::run_ica()
{
	if ( ica == NULL )
		return 1;

	set_cursor_busy( true, (GtkWidget*)wScoringFacility);
	gtk_statusbar_push( sbSF, _p.sbContextIdGeneral, "Separating...");
	while ( gtk_events_pending () )
		gtk_main_iteration();

	ica->obj() . separate();

	ica_components = ica->obj() . get_independent_components();
	int n_ics = ica_components.rows();

	ica_map = vector<int> (n_ics, -1);

	set_cursor_busy( false, (GtkWidget*)wScoringFacility);

	gtk_statusbar_pop( sbSF, _p.sbContextIdGeneral);
	return 0;
}


int
aghui::SScoringFacility::remix_ics()
{
	if ( ica == NULL )
		return 1;

	set_cursor_busy( true, (GtkWidget*)wScoringFacility);
	gtk_statusbar_push( sbSF, _p.sbContextIdGeneral, "Remixing...");
	while ( gtk_events_pending () )
		gtk_main_iteration();

	// get unmixing matrix
	itpp::mat
		mixmat = ica->obj() . get_separating_matrix(),
		ximmat;
	itpp::inv( mixmat, ximmat);

	// // reconstitute projection of discarded ICs
	// auto r = ica_marks.size();
	// while ( r-- )
	// 	if ( ica_marks[r] == TICMark::good )
	// 		ica_components.del_row(r);
	// //printf( "ximmat %dx%d tmp %dx%d\n", ximmat.rows(), ximmat.cols(), tmp.rows(), tmp.cols());
	// itpp::mat unmix = ximmat * ica_components;

	FAFA;
	size_t r = 0;
	itpp::mat remix = ica_components;
	// remix.set_size( ica_components.rows(), ica_components.cols());
	// for ( auto H = channels.begin(); H != channels.end(); ++H, ++r )
	// 	remix.set_row( r, itpp::Vec<double> (&H->crecording.F().get_signal_filtered<int, double>(H->h())[0],
	// 					     ica_components.cols()));
	// // remix is now a copy of the original bunch here

	for ( r = 0; r < ica_map.size(); ++r )
		if ( ica_map[r] != -1 ) {
			size_t q = (size_t)r;
			snprintf_buf( "Removing component %zu (%zu)...", r, q);
			gtk_statusbar_push( sbSF, _p.sbContextIdGeneral, __buf__);
			while ( gtk_events_pending () )
				gtk_main_iteration();

			itpp::mat
				this_component_col (ximmat.get_col(q)),
				this_ic (ica_components.get_row(r));
			remix -= (this_component_col * this_ic);

			gtk_statusbar_pop( sbSF, _p.sbContextIdGeneral);
		}
	r = 0;
	for_each( channels.begin(), channels.end(),
		  [&] ( SChannel& H)
		  {
			  // if ( (strcmp( H.type, "EMG") == 0 &&
			  // 	find( ica_marks.begin(), ica_marks.begin(), TICMark::emg_artifacts) != ica_marks.end() ) ||
			  //      (strcmp( H.type, "EOG") == 0 &&
			  // 	find( ica_marks.begin(), ica_marks.begin(), TICMark::eog_artifacts) != ica_marks.end() ) ||
			  //      (strcmp( H.type, "ECG") == 0 &&
			  // 	find( ica_marks.begin(), ica_marks.begin(), TICMark::ecg_artifacts) != ica_marks.end() ) )
			  // 	  H.signal_reconstituted.resize(0);
			  // else
			  H.signal_reconstituted = H.signal_filtered - itpp::to_va<TFloat, double>( remix, r++);
			  FAFA;
		  });
	// don't forget
	ica_map = vector<int> (ica_components.rows(), -1);

	set_cursor_busy( false, (GtkWidget*)wScoringFacility);
	gtk_statusbar_pop( sbSF, _p.sbContextIdGeneral);
	return 0;
}






int
aghui::SScoringFacility::ic_near( double y) const
{
	int nearest = INT_MAX, thisd;
	int nearest_h = 0;
	int gap = da_ht/ica_components.rows();
	int thisy = gap/2;
	for ( int h = 0; h < ica_components.rows(); ++h ) {
		thisd = y - thisy;
		if ( thisd < 0 ) {
			if ( -thisd < nearest )
				return h;
			else
				return nearest_h;
		}
		if ( thisd < nearest ) {
			nearest = thisd;
			nearest_h = h;
		}
		thisy += gap;
	}
	return nearest_h;
}


// eof
