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

#include "aghermann/expdesign/primaries.hh"

namespace agh {
namespace rk1968 {

struct SScoreAssistantPPack {
};

class CScoreAssistant {

    public:
	int score( agh::CSubject::SEpisode&);

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
