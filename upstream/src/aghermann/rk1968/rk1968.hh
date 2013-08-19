/*
 *       File name:  aghermann/rk1968/rk1968.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-05-16
 *
 *         Purpose:  assisted score
 *
 *         License:  GPL
 */

#ifndef _AGH_RK1968_H
#define _AGH_RK1968_H

#include "aghermann/expdesign/forward-decls.hh"

namespace agh {
namespace rk1968 {

struct SScoreAssistantPPack {
	double	nrem3_delta_theta_ratio;

	SScoreAssistantPPack (const SScoreAssistantPPack&) = default;
	SScoreAssistantPPack ()
	      : nrem3_delta_theta_ratio (1.5)
		{}
};

class CScoreAssistant
  : public SScoreAssistantPPack {

    public:
	int score( agh::SEpisode&);
};


} // namespace rk1968
} // namespace agh

#endif // _AGH_RK1968_H

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
