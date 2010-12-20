// ;-*-C++-*- *  Time-stamp: "2010-12-18 16:47:40 hmmr"
/*
 *       File name:  ui/settings.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-12-18
 *
 *         Purpose:  global decls of bits not directly related to ui
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

extern float
	AghDZCDFStep,
	AghDZCDFSigma,
	AghDZCDFWindow;
extern size_t
	AghDZCDFSmooth;

extern size_t
	AghExtremaScope;

extern gfloat
	AghFreqBands[AGH_BAND__TOTAL][2];
extern const gchar
	*AghFreqBandsNames[AGH_BAND__TOTAL];

extern gboolean
	AghSimRunbatchIncludeAllChannels,
	AghSimRunbatchIncludeAllSessions,
	AghSimRunbatchIterateRanges;


G_END_DECLS

#endif

// EOF
