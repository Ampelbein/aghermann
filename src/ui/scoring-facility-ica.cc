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


#include <unistd.h>
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

      // check for previous ICA runs
	{
		list<string> affected_sources;
		for ( auto &H : channels )
			affected_sources.push_back( H.crecording.F().filename());
		affected_sources.unique();
		for ( auto &fname : affected_sources ) {
			if ( access( (fname + ".orig").c_str(), F_OK) == -1 ) {
				; // good
			} else {
				if ( GTK_RESPONSE_NO ==
				     pop_question( wScoringFacility,
						   "It seems you have already run ICA on these channels\n"
						   "(a backup file <i>\"%s.orig\"</i> exists, and\n"
						   "will be overwritten if you proceed now)\n\n"
						   "Sure you want to do it again?", fname.c_str()) )
					return 1;
			}
		}
	}

	vector<TICASetupFun> src;
	size_t checking_sr = 0;
	for ( auto &H : channels ) {
		size_t this_sr = H.crecording.F().samplerate(H.h());
		if ( checking_sr and this_sr != checking_sr ) {
			pop_ok_message( wScoringFacility,
					"Cannot perform ICA on channels with different sample rates.");
			return 1;
		} else
			checking_sr = this_sr;

		src.emplace_back(
			bind (&agh::CEDFFile::get_signal_filtered<int, double>, &H.crecording.F(), H.h()));
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
	for ( auto &H : channels ) {
		auto item = (GtkWidget*)gtk_radio_menu_item_new_with_label( group, H.name);
		group = gtk_radio_menu_item_get_group( (GtkRadioMenuItem*)item);
		g_object_set( (GObject*)item,
			      "visible", TRUE,
			      NULL);
		g_signal_connect( (GObject*)item,
				  "activate", (GCallback)iSFICAPageMapIC_activate_cb,
				  this);
		gtk_container_add( (GtkContainer*)mSFICAPage, item);
	}
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
	gtk_toggle_button_set_active( bSFICAPreview, FALSE);
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

	ica_map.clear();
	ica_map.resize(ica_components.rows(), {-1});

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
			int map_to = ica_map[r].m;
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
		size_t r = ica_map.size();
		while ( r-- )
			if ( ica_map[r].m != -1 ) {
				ica_components.del_row(r);
				ximmat.del_col(r);
			}
		itpp::mat reconst = ximmat * ica_components;
		r = 0;
		for ( auto &H : channels ) {
			H.signal_reconstituted.resize( H.signal_filtered.size());
			H.signal_reconstituted = itpp::to_va<TFloat, double>( reconst, r++);
		}
	}
	break;
	case TICARemixMode::zero:
	{
		// get unmixing matrix
		itpp::mat
			mixmat = ica->obj() . get_separating_matrix(),
			ximmat;
		itpp::inv( mixmat, ximmat);

		// reconstitute projections of good ICs
		size_t r = 0;
		for ( r = 0; r < ica_map.size(); ++r )
			if ( ica_map[r].m != -1 ) {
				for ( int c = 0; c < ica_components.cols(); ++c )
					ica_components(r, c) = 0.;
			}
		itpp::mat reconst = ximmat * ica_components;
		r = 0;
		for ( auto &H : channels ) {
			H.signal_reconstituted.resize( H.signal_filtered.size());
			H.signal_reconstituted = itpp::to_va<TFloat, double>( reconst, r++);
		}
	}
	break;
	}

	set_cursor_busy( false, (GtkWidget*)wScoringFacility);
	return 0;
}

int
aghui::SScoringFacility::restore_ics()
{
	if ( ica == NULL )
		return 1;

	switch ( remix_mode ) {
	case TICARemixMode::map:
	    break;
	case TICARemixMode::zero:
	case TICARemixMode::punch:
		ica_components = ica->obj() . get_independent_components();
	    break;
	}

	return 0;
}




int
aghui::SScoringFacility::apply_remix( bool do_backup)
{
	if ( ica == NULL )
		return 1;

	delete ica;
	ica = NULL;

	// move the original edf file aside
	if ( do_backup ) {
		list<string> affected_sources;
		for ( auto &H : channels )
			affected_sources.push_back( H.crecording.F().filename());
		affected_sources.unique();
		for ( auto &fname : affected_sources ) {
			snprintf_buf( "cp -f '%s' '%s.orig'", fname.c_str(), fname.c_str());
			if ( system(__buf__) )
				;
		}
	}
	// put signal
	for ( auto &H : channels ) {
		if ( not H.apply_reconstituted )
			continue;
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
	}

	return 0;
}




int
__attribute__ ((pure))
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

int
__attribute__ ((pure))
aghui::SScoringFacility::ic_of( const SChannel* ch) const
{
	int h = 0;
	for ( auto &H : channels ) {
		if ( &H == ch )
			return h;
		++h;
	}
	throw out_of_range ("aghui::SScoringFacility::ic_of(): bad channel");
}


// eof
