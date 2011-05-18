// ;-*-C++-*- *  Time-stamp: "2011-05-18 02:51:05 hmmr"
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

extern GtkSpinButton
	*eBand[(size_t)agh::TBand::_total][2];


namespace settings {

// these are user-exposed settings with gui controls
// some other settings are saved, too, but those are in their own namespaces

extern string
	LastExpdesignDir;
extern int
	LastExpdesignDirNo;



extern float
	OperatingRangeFrom,
	OperatingRangeUpto;


extern array<string, (size_t)agh::TScore::_total>
	ExtScoreCodes;


extern float
	FreqBands[(size_t)agh::TBand::_total][2];
extern const char // not quite a settings item, this
	*const FreqBandNames[(size_t)agh::TBand::_total];


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
