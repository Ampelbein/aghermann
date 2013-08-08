/*
 *       File name:  aghermann/model/forward-decls.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-26
 *
 *         Purpose:  forward declarations of various modelling classes
 *
 *         License:  GPL
 */


#ifndef AGH_AGHERMANN_MODEL_FORWARD_DECLS_H_
#define AGH_AGHERMANN_MODEL_FORWARD_DECLS_H_

namespace agh {

	namespace ach {

		struct SControlParamSet;
		class CModelRun;

		enum class TTRole;
		template <TTRole Of> struct STunableSet;
		struct STunableSetWithState;
	}


	namespace beersma {

		struct SClassicFit;
		struct SClassicFitCtl;

		struct SUltradianCycle;
		struct SUltradianCycleCtl;
	}

} // namespace agh


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
