// ;-*-C++-*-
/*
 *       File name:  ui/mw/mw-simulations.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  simulation results view
 *
 *         License:  GPL
 */

#include "ui/misc.hh"
#include "mw.hh"

using namespace std;


void
aghui::SExpDesignUI::
populate_2()
{
	gtk_tree_store_clear( mSimulations);

      // clean up
	ED->remove_untried_modruns();
	GtkTreeIter iter_g, iter_j, iter_h, iter_m, iter_q;

	for ( auto &G : ED->groups ) {

		gtk_tree_store_append( mSimulations, &iter_g, NULL);
		gtk_tree_store_set( mSimulations, &iter_g,
				    0, G.first.c_str(),
				    msimulations_visibility_switch_col, TRUE,
				    -1);

		for ( auto &J : G.second ) {
			if ( not J.have_session(*_AghDi) or
			     J.measurements[*_AghDi].episodes.empty() ) // subject lacking one
				continue;

			gtk_tree_store_append( mSimulations, &iter_j, &iter_g);
			gtk_tree_store_set( mSimulations, &iter_j,
					    0, J.name(),
					    msimulations_visibility_switch_col, TRUE,
					    -1);

		      // collect previously obtained modruns
			for ( auto &Q : J.measurements[*_AghDi].modrun_sets ) {
				auto MT = Q.first;

				gtk_tree_store_append( mSimulations, &iter_m, &iter_j);
				gtk_tree_store_set( mSimulations, &iter_m,
						    0, sigfile::metric_method(MT),
						    msimulations_visibility_switch_col, TRUE,
						    -1);

				for ( auto &RS : Q.second ) {
					const string& H = RS.first;

					gtk_tree_store_append( mSimulations, &iter_h, &iter_m);
					gtk_tree_store_set( mSimulations, &iter_h,
							    0, H.c_str(),
							    msimulations_visibility_switch_col, TRUE,
							    -1);

					for ( auto &R : RS.second ) {
						float	from = R.first.first,
							upto = R.first.second;
						auto&	M = R.second;

						snprintf_buf( "%g\342\200\223%g", from, upto);
						gtk_tree_store_append( mSimulations, &iter_q, &iter_h);
						gtk_tree_store_set( mSimulations, &iter_q,
								    0, __buf__,
								    msimulations_modref_col, (gpointer)&M,
								    msimulations_visibility_switch_col, TRUE,
								    -1);
						// status (put CF here)
						snprintf_buf( "CF = %g", M.cf);
						gtk_tree_store_set( mSimulations, &iter_q,
								    1, __buf__,
								    -1);

						// tunable columns
						for ( size_t t = 0; t < M.tx.size(); ++t ) {
							auto tg = min(t, (size_t)agh::ach::TTunable::_basic_tunables - 1);
							const auto& td = agh::ach::stock[tg];
							snprintf_buf( td.fmt,
								      M.tx[t] * td.display_scale_factor);
							gtk_tree_store_set( mSimulations, &iter_q,
									    2+t, __buf__, -1);
						}

					}
				}
			}
		      // and a virgin offering
			auto &lo = J.measurements[*_AghDi].modrun_sets[display_profile_type][AghT()];
			if ( lo.find( pair<float,float> ({operating_range_from, operating_range_upto})) == lo.end() ) {

				gtk_tree_store_append( mSimulations, &iter_m, &iter_j);
				gtk_tree_store_set( mSimulations, &iter_m,
						    0, sigfile::metric_method(display_profile_type),
						    -1);
				gtk_tree_store_append( mSimulations, &iter_h, &iter_m);
				gtk_tree_store_set( mSimulations, &iter_h,
						    0, AghT(),
						    -1);
				snprintf_buf( "%g\342\200\223%g *", operating_range_from, operating_range_upto);
				gtk_tree_store_append( mSimulations, &iter_q, &iter_h);
				gtk_tree_store_set( mSimulations, &iter_q,
						    0, __buf__,
						    -1);

				agh::ach::CModelRun *virgin;
				int retval =
					ED->setup_modrun( J.name(), AghD(), AghT(),
							  display_profile_type,
							  operating_range_from, operating_range_upto,
							  &virgin);
				if ( retval ) {
					gtk_tree_store_set( mSimulations, &iter_q,
							    1, agh::CSCourse::explain_status( retval).c_str(),
							    msimulations_modref_col, NULL,
							    -1);
				} else {
					gtk_tree_store_set( mSimulations, &iter_q,
							    1, "untried",
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





// EOF