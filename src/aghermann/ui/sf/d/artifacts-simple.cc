/*
 *       File name:  aghermann/ui/sf/d/artifacts-simple.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-25
 *
 *         Purpose:  scoring facility: simple artifact detection dialog
 *
 *         License:  GPL
 */

#include "artifacts-simple.hh"

using namespace std;

aghui::SScoringFacility::SArtifactsSimpleDialog&
aghui::SScoringFacility::
artifacts_simple_d()
{
	if ( not _artifacts_simple_d )
		_artifacts_simple_d = new SArtifactsSimpleDialog(*this);
	return *_artifacts_simple_d;
}


aghui::SScoringFacility::SArtifactsSimpleDialog::
SArtifactsSimpleDialog (aghui::SScoringFacility& p_)
      : min_size (0.5),
	pad (),
	_p (p_)
{
	W_V.reg( eSFADSMinFlatRegionSize,	&min_size);
	W_V.reg( eSFADSPad,			&pad);
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
