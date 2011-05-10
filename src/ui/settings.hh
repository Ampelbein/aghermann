// ;-*-C++-*- *  Time-stamp: "2011-05-11 00:58:21 hmmr"
/*
 *       File name:  ui/settings.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-12-18
 *
 *         Purpose:  global decls of non-libagh bits not directly related to ui
 *
 *         License:  GPL
 */


#ifndef _AGH_SETTINGS_H
#define _AGH_SETTINGS_H

#include <array>
#include "../libagh/enums.hh"
#include "../libagh/primaries.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {
namespace settings {



extern char*
	LastExpdesignDir;
extern int
	LastExpdesignDirNo;



extern float
	OperatingRangeFrom,
	OperatingRangeUpto;

extern float
	PPuV2;


extern array<string, (size_t)agh::TScore::_total>
	ExtScoreCodes;


extern float
	FreqBands[(size_t)agh::TBand::_total][2];
extern const char // not quite a settings item, this
	*FreqBandNames[(size_t)agh::TBand::_total];


extern bool
	SimRunbatchIncludeAllChannels,
	SimRunbatchIncludeAllSessions,
	SimRunbatchIterateRanges;

extern unsigned int
	SFDAPageHeight,
	SFDASpectrumWidth,
	SFDAPowerProfileHeight,
	SFDAEMGProfileHeight;

} // namespace settings
} // namespace aghui

#endif

// EOF
