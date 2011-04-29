// ;-*-C++-*- *  Time-stamp: "2011-04-24 14:45:08 hmmr"
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


extern bool
	UseSigAnOnNonEEGChannels;

extern float
	BWFCutoff;
extern unsigned
	BWFOrder;

extern float
	DZCDFStep,
	DZCDFSigma;
extern size_t
	DZCDFSmooth;

extern size_t
	EnvTightness;

extern array<string, (size_t)TScore::_total> ExtScoreCodes;
extern array<string, (size_t)TScore::_total> ScoreNames;


extern float
	FreqBands[(size_t)TBand::_total][2];
extern const char
	*FreqBandsNames[(size_t)TBand::_total];


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
