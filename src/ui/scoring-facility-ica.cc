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
			bind (&agh::CEDFFile::get_signal_filtered<int, TFloat>, &H->crecording.F(), H->h()));
	}
	ica = new ica::CFastICA<TFloat> (src, checking_sr * pagesize() * total_pages());

      // has no independent default
	gtk_spin_button_set_value( eSFICANofICs, channels.size());

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

	gtk_statusbar_pop( sbSF, _p.sbContextIdGeneral);

	ica_components = ica->obj() . get_independent_components();
	int n_ics = ica_components.rows();

	ica_good_ones.clear();
	ica_good_ones = vector<bool> (n_ics, true);

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
	auto mixmat = ica->obj() . get_mixing_matrix();
	itpp::Mat<TFloat> ximmat;
	itpp::inv( mixmat, ximmat);

	printf( "ximmat %dx%d\n", ximmat.cols(), ximmat.rows());
	// discard some ICs
	auto r = ica_good_ones.size();
	while ( r-- )
		if ( not ica_good_ones[r] ) {
			printf( "-- %d\n", r);
			ica_components.set_submatrix( r, r+1, 0, ica_components.cols()-1,
						      0.);
		}
	// reconstitute
	itpp::Mat<TFloat> remixed = ica_components * ximmat;
	printf( "remixed %dx%d\n", remixed.rows(), remixed.cols());
	r = 0;
	for_each( channels.begin(), channels.end(),
		  [&] ( SChannel& H)
		  {
			  FAFA;
			  H.signal_reconstituted = itpp::to_va( remixed, r++);
		  });

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
