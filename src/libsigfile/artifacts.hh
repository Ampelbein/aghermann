// ;-*-C++-*-
/*
 *       File name:  libsigfile/artifacts.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-10-21
 *
 *         Purpose:  artifacts, mostly MC-based
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_ARTIFACTS_H
#define _SIGFILE_ARTIFACTS_H

#include <vector>
#include <valarray>
#include "forward-decls.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigfile {

struct SArtifactDetectionPP {
	double	scope,
		upper_thr, lower_thr,
		f0, fc, bandwidth,
		mc_gain, iir_backpolate;
	double	E, dmin, dmax;
	size_t	sssu_hist_size,
		smooth_side;
	bool	estimate_E,
		use_range;
	SArtifactDetectionPP ()
		: scope (4.),
		  upper_thr (9.), lower_thr (-9.),
		  f0 (1.), fc (1.8), bandwidth (1.5),
		  mc_gain (10.), iir_backpolate (.5),
		  E (4.), dmin (-10), dmax (20),
		  sssu_hist_size (100), smooth_side (0),
		  estimate_E (true), use_range (false)
		{}
};

// artifacts (having sssu_diff outside thresholds * E), see paper pp 1190-1)
vector<size_t> // don't estimate, use pi*B*x^2 (E) as provided
detect_artifacts( const valarray<TFloat>&, size_t sr,
		  const SArtifactDetectionPP&);
TFloat
estimate_E( const valarray<TFloat>&,
	    size_t bins,
	    TFloat dmin, TFloat dmax);

inline TFloat
estimate_E( const valarray<TFloat>& sssu_diff,
	    size_t sssu_hist_size)
{
	return estimate_E( sssu_diff, sssu_hist_size,
			   sssu_diff.min(), sssu_diff.max());
}


} // namespace sigfile


#endif // _SIGFILE_ARTIFACTS_H

// eof
