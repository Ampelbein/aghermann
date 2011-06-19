// ;-*-C++-*- *  Time-stamp: "2011-06-19 21:52:48 hmmr"
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

int	construct_once();
int	load();
int	save();
extern GtkListStore
	*mFFTParamsWindowType,
	*mFFTParamsPageSize,
	*mScoringPageSize,
	*mAfDampingWindowType;


extern string
	LastExpdesignDir;
extern int
	LastExpdesignDirNo;
extern unsigned short
	DisplayPageSizeItem,  // the one used to obtain FFTs
	FFTPageSizeItem;


extern float
	OperatingRangeFrom,
	OperatingRangeUpto;


extern array<string, (size_t)agh::TScore::_total>
	ExtScoreCodes;


extern float
	FreqBands[(size_t)agh::TBand::_total][2];
extern const char // not quite a settings item, this
	*const FreqBandNames[(size_t)agh::TBand::_total];

extern float SFNeighPagePeek;

extern bool
	SimRunbatchIncludeAllChannels,
	SimRunbatchIncludeAllSessions,
	SimRunbatchIterateRanges;

extern unsigned
	WidgetSize_SFPageHeight,
	WidgetSize_SFSpectrumWidth,
	WidgetSize_SFHypnogramHeight,
	WidgetSize_SFEMGProfileHeight;

} // namespace settings
} // namespace aghui

#endif

// EOF
