// ;-*-C++-*- *  Time-stamp: "2011-05-29 15:08:25 hmmr"
/*
 *       File name:  ui/simulations.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-29
 *
 *         Purpose:  simulation results view
 *
 *         License:  GPL
 */


#ifndef _AGH_SIMULATIONS_H
#define _AGH_SIMULATIONS_H


//#include "misc.hh"
//#include "ui.hh"
//#include "settings.hh"
//#include "measurements.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {

namespace simview {
	int	construct_once();
	void	populate();
	void	cleanup();

#define AGH_TV_SIMULATIONS_VISIBILITY_SWITCH_COL 14
#define AGH_TV_SIMULATIONS_MODREF_COL 15
	extern GtkTreeStore
		*mSimulations;
} // namespace simview

} // namespace aghui

#endif

// eof

