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
	printf("ica_components: %dx%d\n", ica_components.rows(), ica_components.cols());

	set_cursor_busy( false, (GtkWidget*)wScoringFacility);

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

	// discard some ICs
	auto r = ica_good_ones.size();
	while ( r-- )
		if ( not ica_good_ones[r] )
			ica_components.set_submatrix( r, r, 0, ica_components.cols()-1,
						      0.);
	// reconstitute
	remixed = ximmat * ica_components;

	return 0;
}

// eof
