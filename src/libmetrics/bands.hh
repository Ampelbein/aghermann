/*
 *       File name:  libmetrics/bands.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2013-05-24
 *
 *         Purpose:  Bands enum
 *                   (actual range values are in agh::CExpDesign::freq_bands)
 *
 *         License:  GPL
 */

#ifndef AGH_LIBMETRICS_BANDS_H
#define AGH_LIBMETRICS_BANDS_H

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace metrics {

enum TBand {
	delta,
	theta,
	alpha,
	beta,
	gamma,
	TBand_total,
};

} // namespace metrics

#endif // AGH_LIBMETRICS_BANDS_H

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
