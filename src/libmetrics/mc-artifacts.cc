/*
 *       File name:  libmetrics/mc-artifacts.cc
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
#include "common/alg.hh"
#include "libsigproc/sigproc.hh"
#include "mc.hh"
#include "mc-artifacts.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

template vector<size_t> metrics::mc::detect_artifacts( const valarray<TFloat>&, size_t, const SArtifactDetectionPP&);

namespace metrics {
namespace mc {

template <>
double
estimate_E( const valarray<double>& sssu_diff,
	    size_t sssu_hist_size,
	    double dmin, double dmax)
{
	gsl_histogram *hist = gsl_histogram_alloc( sssu_hist_size);
	gsl_histogram_set_ranges_uniform( hist, dmin, dmax);

	for ( size_t i = 0; i < sssu_diff.size(); ++i )
		gsl_histogram_increment( hist, sssu_diff[i]);

	return dmin + (gsl_histogram_max_bin( hist) + .5)
		* ((dmax-dmin) / sssu_hist_size);
}

template <>
double
estimate_E( const valarray<float>& S,
	    size_t bins,
	    double dmin, double dmax)
{
	return estimate_E( agh::alg::to_vad(S), bins, dmin, dmax);
}

} // namespace mc
} // namespace metrics

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
