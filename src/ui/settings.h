// ;-*-C++-*- *  Time-stamp: "2011-03-07 16:37:33 hmmr"
/*
 *       File name:  ui/settings.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-12-18
 *
 *         Purpose:  global decls of non-libagh bits not directly related to ui
 *
 *         License:  GPL
 */


#ifndef _AGH_SETTINGS_H
#define _AGH_SETTINGS_H

#include "../common.h"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

G_BEGIN_DECLS


extern int
	AghHi,
	AghTi,
	AghGi,
	AghDi,
	AghEi;
#define AghD (AghDD ? (AghDi < AghDs && AghDi >= 0) ? AghDD[AghDi] : "no session" : "invalid session")
#define AghH (AghHH ? (AghHi < AghHs && AghHi >= 0) ? AghHH[AghHi] : "no channel" : "invalid channel")
#define AghT (AghTT ? (AghTi < AghTs && AghTi >= 0) ? AghTT[AghTi] : "no channel" : "invalid channel")
#define AghE (AghEE ? (AghEi < AghEs && AghEi >= 0) ? AghEE[AghEi] : "no episode" : "invalid episode")
#define AghG (AghGG ? (AghGi < AghGs && AghGi >= 0) ? AghGG[AghGi] : "no group"   : "invalid group")

extern const struct SSubject
	*AghJ;

const gchar* const agh_scoring_pagesize_values_s[9];
const gchar* const agh_fft_pagesize_values_s[5];
const gchar* const agh_fft_window_types_s[9];


extern char*	AghLastExpdesignDir;
extern int	AghLastExpdesignDirNo;



extern gfloat
	AghOperatingRangeFrom,
	AghOperatingRangeUpto;

extern gfloat
	AghPPuV2;


extern gboolean
	AghUseSigAnOnNonEEGChannels;

extern float
	AghBWFCutoff;
extern unsigned
	AghBWFOrder;

extern float
	AghDZCDFStep,
	AghDZCDFSigma;
extern size_t
	AghDZCDFSmooth;

extern size_t
	AghEnvTightness;

extern const char
	*AghExtScoreCodes[AGH_SCORE__TOTAL];
extern const char
	*AghExtScoreCodes_defaults[AGH_SCORE__TOTAL];
extern const char
	*AghScoreNames[AGH_SCORE__TOTAL];

extern gfloat
	AghFreqBands[AGH_BAND__TOTAL][2];
extern const gchar
	*AghFreqBandsNames[AGH_BAND__TOTAL];

extern gboolean
	AghSimRunbatchIncludeAllChannels,
	AghSimRunbatchIncludeAllSessions,
	AghSimRunbatchIterateRanges;

extern guint
	AghSFDAPageHeight,
	AghSFDASpectrumWidth,
	AghSFDAPowerProfileHeight,
	AghSFDAEMGProfileHeight;

G_END_DECLS

#endif

// EOF
