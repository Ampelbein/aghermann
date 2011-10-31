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

const char
	*aghui::SScoringFacility::ica_unmapped_menu_item_label = "(not mapped)";

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
			bind (&agh::CEDFFile::get_signal_original<int, double>, &H->crecording.F(), H->h()));
	}
	ica = new ica::CFastICA (src, checking_sr * pagesize() * total_pages());

      // initialize
	// has no independent default
	gtk_spin_button_set_value( eSFICANofICs, channels.size());
	gtk_adjustment_set_upper( jSFICANofICs, channels.size());
	gtk_spin_button_set_value( eSFICAEigVecFirst, 1);
	gtk_spin_button_set_value( eSFICAEigVecLast, channels.size());

	g_signal_emit_by_name( eSFICAApproach,		"changed");
	g_signal_emit_by_name( eSFICANonlinearity,	"changed");
	g_signal_emit_by_name( eSFICAFineTune,		"toggled");
	g_signal_emit_by_name( eSFICAStabilizationMode,	"toggled");
	g_signal_emit_by_name( eSFICAa1,		"value-changed");
	g_signal_emit_by_name( eSFICAa2,		"value-changed");
	g_signal_emit_by_name( eSFICAmu,		"value-changed");
	g_signal_emit_by_name( eSFICAepsilon,		"value-changed");
	g_signal_emit_by_name( eSFICASampleSizePercent,	"value-changed");
	g_signal_emit_by_name( eSFICAMaxIterations,	"value-changed");
	g_signal_emit_by_name( eSFICARemixMode,		"changed");

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
			   another = gtk_radio_menu_item_new_with_label( group, ica_unmapped_menu_item_label));
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

	ica_components = itpp::mat (0, 0); // free up couple of hundred megs
	ica->obj() . separate();
	ica_components = ica->obj() . get_independent_components();

	ica_map = vector<int> (ica_components.rows(), -1);

	set_cursor_busy( false, (GtkWidget*)wScoringFacility);
	return 0;
}


int
aghui::SScoringFacility::remix_ics()
{
	if ( ica == NULL )
		return 1;

	set_cursor_busy( true, (GtkWidget*)wScoringFacility);

	switch ( remix_mode ) {
	case TICARemixMode::map:
	{
		size_t r = 0;
		for ( r = 0; r < ica_map.size(); ++r ) {
			int map_to = ica_map[r];
			if ( map_to != -1 )
				channel_by_idx(map_to).signal_reconstituted =
					itpp::to_va<TFloat, double>(
						ica_components, r);
		}
	}
	break;
	case TICARemixMode::punch:
	{
		// get unmixing matrix
		itpp::mat
			mixmat = ica->obj() . get_separating_matrix(),
			ximmat;
		itpp::inv( mixmat, ximmat);

		// reconstitute projections of good ICs
		size_t r = 0;
		for ( r = 0; r < ica_map.size(); ++r )
			if ( ica_map[r] != -1 ) {
				// discard non-clen ICs (couldn't do del_row as matrix multiplication will segfault)
				itpp::vec row (ica_components.cols());
				row = 0.;
				ica_components.set_row(r, row);
			}
		itpp::mat reconst = ximmat * ica_components;
		r = 0;
		for_each( channels.begin(), channels.end(),
			  [&] ( SChannel& H)
			  {
				  H.signal_reconstituted.resize( H.signal_filtered.size());
				  H.signal_reconstituted = itpp::to_va<TFloat, double>( reconst, r++);
			  });
		// don't forget
		ica_map = vector<int> (ica_components.rows(), -1);
	}
	break;
	}

	set_cursor_busy( false, (GtkWidget*)wScoringFacility);
	return 0;
}




int
aghui::SScoringFacility::apply_remix( bool eeg_channels_only)
{
	if ( ica == NULL )
		return 1;

	delete ica;
	ica = NULL;

	// move the original edf file aside
	;
	// put signal
	for_each( channels.begin(), channels.end(),
		  [&] ( SChannel& H)
		  {
			  if ( eeg_channels_only and strcmp( H.type, "EEG") != 0 )
				  return;
			  if ( H.signal_reconstituted.size() > 0 )
				  H.crecording.F().put_signal(
					  H.h(),
					  H.signal_reconstituted);
			  H.signal_reconstituted = valarray<TFloat> (0);
			  H.get_signal_original();
			  H.get_signal_filtered();
			  if ( strcmp( H.type, "EEG") == 0 ) {
				  H.get_power( true);
				  H.get_power_in_bands( true);
			  }
		  });

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
