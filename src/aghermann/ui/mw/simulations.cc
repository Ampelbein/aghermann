/*
 *       File name:  aghermann/ui/mw/simulations.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  simulation results view
 *
 *         License:  GPL
 */

#include "aghermann/ui/misc.hh"
#include "mw.hh"

using namespace std;


void
aghui::SExpDesignUI::
populate_2()
{
	gtk_tree_store_clear( mSimulations);

      // clean up
	ED->remove_untried_modruns();
	GtkTreeIter iter_g, iter_j, iter_m, iter_h;

	for ( auto &G : ED->groups ) {

		gtk_tree_store_append( mSimulations, &iter_g, NULL);
		gtk_tree_store_set( mSimulations, &iter_g,
				    0, G.first.c_str(),
				    msimulations_visibility_switch_col, TRUE,
				    -1);

		for ( auto &J : G.second ) {
			auto& EE = J.measurements[*_AghDi];
			if ( not J.have_session(*_AghDi) or
			     EE.episodes.empty() ) // subject lacking one
				continue;

			gtk_tree_store_append( mSimulations, &iter_j, &iter_g);
			gtk_tree_store_set( mSimulations, &iter_j,
					    0, J.id.c_str(),
					    msimulations_visibility_switch_col, TRUE,
					    -1);

		      // collect previously obtained modruns
			for ( auto &MS : EE.modrun_sets ) {
				const agh::SProfileParamSet& P = MS.first;
				gtk_tree_store_append( mSimulations, &iter_m, &iter_j);
				gtk_tree_store_set( mSimulations, &iter_m,
						    0, P.display_name().c_str(),
						    msimulations_visibility_switch_col, TRUE,
						    -1);

				for ( auto &HS : MS.second ) {
					const string& H = HS.first;
					const agh::ach::CModelRun& M = HS.second;

					gtk_tree_store_append( mSimulations, &iter_h, &iter_m);
					gtk_tree_store_set( mSimulations, &iter_h,
							    0, H.c_str(),
							    msimulations_visibility_switch_col, TRUE,
							    -1);

					// status (put CF here)
					snprintf_buf( "CF = %g", M.cf);
					gtk_tree_store_set( mSimulations, &iter_h,
							    1, __buf__,
							    msimulations_modref_col, &M,
							    -1);

					// tunable columns
					for ( size_t t = 0; t < M.tx.size(); ++t ) {
						auto tg = min(t, (size_t)agh::ach::TTunable::_basic_tunables - 1);
						const auto& td = agh::ach::stock[tg];
						snprintf_buf( td.fmt,
							      M.tx[t] * td.display_scale_factor);
						gtk_tree_store_set( mSimulations, &iter_h,
								    2+t, __buf__, -1);
					}
				}
			}
		      // and a virgin offering
			auto P_new = make_active_profile_paramset();
			auto Mi = EE.modrun_sets.find( P_new);
			if ( Mi == EE.modrun_sets.end() ||
			     Mi->second.find( AghT()) == Mi->second.end() ) {

				gtk_tree_store_append( mSimulations, &iter_m, &iter_j);
				gtk_tree_store_set( mSimulations, &iter_m,
						    0, P_new.display_name().c_str(),
						    -1);
				gtk_tree_store_append( mSimulations, &iter_h, &iter_m);
				gtk_tree_store_set( mSimulations, &iter_h,
						    0, AghT(),
						    -1);

				agh::ach::CModelRun *virgin;
				int retval =
					ED->setup_modrun( J.id.c_str(), AghD(), AghT(),
							  P_new,
							  &virgin);
				if ( retval ) {
					gtk_tree_store_set( mSimulations, &iter_h,
							    1, agh::str::tokens_trimmed( agh::CProfile::explain_status( retval), ";").front().c_str(),
							    msimulations_modref_col, NULL,
							    -1);
				} else {
					gtk_tree_store_set( mSimulations, &iter_h,
							    1, "(untried — dbl-click to run)",
							    msimulations_modref_col, virgin,
							    -1);
				}
			}
		}
	}
	gtk_tree_view_expand_all( tvSimulations);
}


void
aghui::SExpDesignUI::
cleanup_2()
{
	ED->remove_untried_modruns();
	//populate( false);
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

