// ;-*-C++-*- *  Time-stamp: "2011-07-03 16:20:29 hmmr"
/*
 *       File name:  ui/expdesign-simulations.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  simulation results view
 *
 *         License:  GPL
 */


#include "misc.hh"
#include "ui.hh"
#include "expdesign.hh"
#include "modelrun-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

const char* const
	aghui::SExpDesignUI::msimulations_column_names[] = {
	"Id", "Status",
	"rise rate",	 "rate const.",
	"fc(REM)",	 "fc(Wake)",
	"S\342\202\200", "S\342\210\236",
	"t\342\206\227", "t\342\206\230",
	"gain const.",
	"gc2", "gc3", "gc4",
};


inline namespace {

}




void
aghui::SExpDesignUI::populate_2()
{
	gtk_tree_store_clear( mSimulations);

      // clean up
	ED->remove_untried_modruns();

	GtkTreeIter iter_g, iter_j, iter_h, iter_q;

	for ( auto G = ED->groups_begin(); G != ED->groups_end(); ++G ) {

		gtk_tree_store_append( mSimulations, &iter_g, NULL);
		gtk_tree_store_set( mSimulations, &iter_g,
				    0, G->first.c_str(),
				    msimulations_visibility_switch_col, TRUE,
				    -1);

		for ( auto J = G->second.begin(); J != G->second.end(); ++J ) {
			if ( not J->have_session(*_AghDi) ) // subject lacking one
				continue;

			gtk_tree_store_append( mSimulations, &iter_j, &iter_g);
			gtk_tree_store_set( mSimulations, &iter_j,
					    0, J->name(),
					    msimulations_visibility_switch_col, TRUE,
					    -1);

		      // collect previously obtained modruns
			for ( auto RS = J->measurements[*_AghDi].modrun_sets.begin(); RS != J->measurements[*_AghDi].modrun_sets.end(); ++RS ) {
				const string& H = RS->first;

				gtk_tree_store_append( mSimulations, &iter_h, &iter_j);
				gtk_tree_store_set( mSimulations, &iter_h,
						    0, H.c_str(),
						    msimulations_visibility_switch_col, TRUE,
						    -1);

				for ( auto R = RS->second.begin(); R != RS->second.end(); ++R ) {
					float	from = R->first.first,
						upto = R->first.second;
					agh::CSimulation&
						M = R->second;

					snprintf_buf( "%g\342\200\223%g", from, upto);
					gtk_tree_store_append( mSimulations, &iter_q, &iter_h);
					gtk_tree_store_set( mSimulations, &iter_q,
							    0, __buf__,
							    msimulations_modref_col, (gpointer)&M,
							    msimulations_visibility_switch_col, TRUE,
							    -1);
					// status (put CF here)
					snprintf_buf( "CF = %g", M.snapshot());
					gtk_tree_store_set( mSimulations, &iter_q,
							    1, __buf__,
							    -1);

					// tunable columns
					for ( agh::TTunable_underlying_type t = 0; t < M.cur_tset.size(); ++t ) {
						const auto& td = agh::STunableSet::stock[t];
						snprintf_buf( td.fmt,
							      M.cur_tset[t] * td.display_scale_factor);
						gtk_tree_store_set( mSimulations, &iter_q,
								    2+t, __buf__,
								    -1);
					}

				}
			}
		      // and a virgin offering
			gtk_tree_store_append( mSimulations, &iter_h, &iter_j);
			gtk_tree_store_set( mSimulations, &iter_h,
					    0, AghT(),
					    -1);
			snprintf_buf( "%g\342\200\223%g *", operating_range_from, operating_range_upto);
			gtk_tree_store_append( mSimulations, &iter_q, &iter_h);
			gtk_tree_store_set( mSimulations, &iter_q,
					    0, __buf__,
					    -1);

			agh::CSimulation *virgin;
			int retval = ED->setup_modrun( J->name(), AghD(), AghT(),
						       operating_range_from, operating_range_upto,
						       virgin);
			if ( retval ) {
				gtk_tree_store_set( mSimulations, &iter_q,
						    1, agh::CSCourse::explain_status( retval).c_str(),
						    msimulations_modref_col, NULL,
						    -1);
			} else {
				gtk_tree_store_set( mSimulations, &iter_q,
						    1, "untried",
						    msimulations_modref_col, (gpointer)virgin,
						    -1);
			}
		}
	}
	gtk_tree_view_expand_all( tvSimulations);
}


void
aghui::SExpDesignUI::cleanup_2()
{
	ED->remove_untried_modruns();
	populate( false);
}





// EOF
