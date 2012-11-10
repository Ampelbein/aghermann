// ;-*-C++-*-
/*
 *       File name:  metrics/mc-artifacts.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-10-21
 *
 *         Purpose:  EEG microcontinuity based functions for artifact detection
 *
 *         License:  GPL
 */

#include <gsl/gsl_histogram.h>

#include "common/lang.hh"
#include "sigproc/sigproc.hh"
#include "mc.hh"
#include "mc-artifacts.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


vector<size_t>
metrics::mc::
detect_artifacts( const valarray<TFloat>& signal, size_t sr,
		  const SArtifactDetectionPP& P)
{
	auto	sssu
		= CProfile::do_sssu_reduction(
			signal,
			sr, P.scope,
			P.mc_gain, P.iir_backpolate,
			P.f0, P.fc, P.bandwidth);
	valarray<TFloat>
		sssu_diff = {sssu.first - sssu.second};

	sigproc::smooth( sssu_diff, P.smooth_side);

	double E;
	if ( P.estimate_E )
		E = P.use_range
			? estimate_E(
				sssu_diff,
				P.sssu_hist_size,
				P.dmin, P.dmax)
			: estimate_E(
				sssu_diff,
				P.sssu_hist_size);
	else
		E = P.E;

	vector<size_t>
		marked;
	for ( size_t p = 0; p < sssu_diff.size(); ++p )
		if ( sssu_diff[p] < E + E * P.lower_thr ||
		     sssu_diff[p] > E + E * P.upper_thr ) {
			marked.push_back(p);
		}

	return marked;
}




TFloat
metrics::mc::
estimate_E( const valarray<TFloat>& sssu_diff,
	    size_t sssu_hist_size,
	    TFloat dmin, TFloat dmax)
{
	gsl_histogram *hist = gsl_histogram_alloc( sssu_hist_size);
	gsl_histogram_set_ranges_uniform( hist, dmin, dmax);

	for ( size_t i = 0; i < sssu_diff.size(); ++i )
		gsl_histogram_increment( hist, sssu_diff[i]);

	return dmin + (gsl_histogram_max_bin( hist) + .5)
		* ((dmax-dmin) / sssu_hist_size);
}



// eof
