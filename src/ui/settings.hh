// ;-*-C++-*- *  Time-stamp: "2011-04-17 23:09:12 hmmr"
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

namespace aghui {

using namespace std;
using namespace agh;

extern list<SChannel>::iterator
	_AghHi,
	_AghTi;
extern list<string>::iterator
	_AghGi,
	_AghDi,
	_AghEi;


inline const char*
AghH() { return (_AghHi != AghHH.end()) ? _AghHi->c_str() : NULL; }
inline const char*
AghT() { return (_AghTi != AghTT.end()) ? _AghTi->c_str() : NULL; }
inline const char*
AghG() { return (_AghGi != AghGG.end()) ? _AghGi->c_str() : NULL; }
inline const char*
AghD() { return (_AghDi != AghDD.end()) ? _AghDi->c_str() : NULL; }
inline const char*
AghE() { return (_AghEi != AghEE.end()) ? _AghEi->c_str() : NULL; }

inline int
AghTi()
{
	int i = 0;
	for ( auto Ti = AghTT.begin(); Ti != AghTT.end(); ++Ti, ++i )
		if ( Ti == _AghTi )
			return i;
	return -1;
}
inline int
AghDi()
{
	int i = 0;
	for ( auto Di = AghDD.begin(); Di != AghDD.end(); ++Di, ++i )
		if ( Di == _AghDi )
			return i;
	return -1;
}


// #define AghD (AghDD ? (AghDi < AghDs && AghDi >= 0) ? AghDD[AghDi] : "no session" : "invalid session")
// #define AghH (AghHH ? (AghHi < AghHs && AghHi >= 0) ? AghHH[AghHi] : "no channel" : "invalid channel")
// #define AghT (AghTT ? (AghTi < AghTs && AghTi >= 0) ? AghTT[AghTi] : "no channel" : "invalid channel")
// #define AghE (AghEE ? (AghEi < AghEs && AghEi >= 0) ? AghEE[AghEi] : "no episode" : "invalid episode")
// #define AghG (AghGG ? (AghGi < AghGs && AghGi >= 0) ? AghGG[AghGi] : "no group"   : "invalid group")

extern const CSubject
	*AghJ;



extern const char* const fft_window_types_s[(size_t)TFFTWinType::_total];

extern const char* const scoring_pagesize_values_s[9];
extern const char* const fft_pagesize_values_s[5];


extern char*	LastExpdesignDir;
extern int	LastExpdesignDirNo;



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

}

#endif

// EOF
