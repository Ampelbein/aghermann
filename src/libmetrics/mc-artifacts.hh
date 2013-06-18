/*
 *       File name:  libmetrics/mc-artifacts.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-10-21
 *
 *         Purpose:  artifacts, MC-based
 *
 *         License:  GPL
 */

#ifndef AGH_LIBMETRICS_MC_ARTIFACTS_H_
#define AGH_LIBMETRICS_MC_ARTIFACTS_H_

#include <vector>
#include <valarray>
#include "libsigproc/sigproc.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {
namespace mc {

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

template <typename T>
vector<size_t> // don't estimate, use pi*B*x^2 (E) as provided
detect_artifacts( const valarray<T>& signal, size_t sr,
		  const SArtifactDetectionPP& P);


template <typename T>
double
estimate_E( const valarray<T>&,
	    size_t bins,
	    double dmin, double dmax);

template <typename T>
inline double
estimate_E( const valarray<T>& sssu_diff,
	    size_t sssu_hist_size)
{
	return estimate_E( sssu_diff, sssu_hist_size,
			   sssu_diff.min(), sssu_diff.max());
}


#include "mc-artifacts.ii"


} // namespace mc
} // namespace metrics


#endif // AGH_LIBMETRICS_MC_ARTIFACTS_H_

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
