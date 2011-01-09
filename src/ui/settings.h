// ;-*-C++-*- *  Time-stamp: "2011-01-09 01:56:56 hmmr"
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
