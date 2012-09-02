// ;-*-C++-*-
/*
 *       File name:  model/forward-decls.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-26
 *
 *         Purpose:  forward declarations of various modelling classes
 *
 *         License:  GPL
 */


#ifndef _AGH_MODEL_FORWARD_DECLS_H
#define _AGH_MODEL_FORWARD_DECLS_H

namespace agh {

	namespace ach {

		struct SSCourseParamSet;
		class CSCourse;
		struct SControlParamSet;
		class CModelRun;

		class STunableSet;
		class STunableSetFull;
	}


	namespace beersma {

		struct SClassicFit;
		struct SClassicFitCtl;

		struct SUltradianCycle;
		struct SUltradianCycleCtl;
	}

} // namespace agh


#endif

// eof
