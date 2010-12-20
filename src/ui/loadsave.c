// ;-*-C-*- *  Time-stamp: "2010-12-18 14:52:14 hmmr"
/*
 *       File name:  ui/loadsave.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-04-28
 *
 *         Purpose:  load/save ui-related vars
 *
 *         License:  GPL
 */

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include <glib.h>

#include "../libagh/iface.h"
#include "misc.h"
#include "settings.h"
#include "ui.h"



#define AGH_CONF_FILE ".aghermann.conf"

gint
agh_ui_settings_load()
{
	GKeyFile *kf = g_key_file_new();

	GError *kf_error = NULL;

//	gint	intval;
	gdouble	dblval;
	gchar	*chrval;
	GString *ext_msg = g_string_sized_new( 120);

	if ( !g_key_file_load_from_file( kf, AGH_CONF_FILE, G_KEY_FILE_KEEP_COMMENTS, &kf_error) ) {
		fprintf( stderr, "agh_ui_settings_load(): failed (%s)\n", kf_error->message);
		return -1;
	}

	dblval = g_key_file_get_double( kf, "Common", "OperatingRangeFrom", NULL);
	if ( dblval <= 0 ) {
		g_string_append_printf( ext_msg, "OperatingRangeFrom must be >0.\n");
		AghOperatingRangeFrom = 2.;
	} else
		AghOperatingRangeFrom = dblval;
	dblval = g_key_file_get_double( kf, "Common", "OperatingRangeUpto", NULL);
	if ( dblval <= AghOperatingRangeFrom ) {
		g_string_append_printf( ext_msg, "OperatingRangeUpto must be > OperatingRangeFrom.\n");
		AghOperatingRangeUpto = 3.;
	} else
		AghOperatingRangeUpto = dblval;

	AghDi = g_key_file_get_integer( kf, "Common", "CurrentSessionNo", NULL);
	AghTi = g_key_file_get_integer( kf, "Common", "CurrentChannelNo", NULL);
	// defer assignment of AghX until AghXs are collected from agh_enumerate_*

	dblval = g_key_file_get_double( kf, "Scoring Facility", "PixelsPeruV2", NULL);
	if ( dblval <= 0 ) {
		g_string_append_printf( ext_msg, "Bad value for PixelsPeruV2.\n");
		AghPPuV2 = 1e-5;
	} else
		AghPPuV2 = dblval;

	if ( g_key_file_has_group( kf, "Batch Run") ) {
		AghSimRunbatchIncludeAllChannels	= g_key_file_get_boolean( kf, "Batch Run", "IncludeAllChannels",  NULL);
		AghSimRunbatchIncludeAllSessions	= g_key_file_get_boolean( kf, "Batch Run", "IncludeAllSessions",  NULL);
		AghSimRunbatchIterateRanges		= g_key_file_get_boolean( kf, "Batch Run", "IterateRanges",  NULL);
//		AghSimRunbatchRedo_Option		= g_key_file_get_integer( kf, "Batch Run", "RedoOption",  NULL);
	}

	GdkColor clr;
	guint16	alpha;
#define DO_COLOR(W,B) \
	chrval = g_key_file_get_string( kf, "Colours", W, NULL); \
	if ( chrval && sscanf( chrval, "%x,%x,%x,%x", \
			       (unsigned int*)&clr.red, (unsigned int*)&clr.green, (unsigned int*)&clr.blue, \
			       (unsigned int*)&alpha) == 4 ) {		\
		gtk_color_button_set_color( GTK_COLOR_BUTTON (B), &clr); \
		/* gtk_color_button_set_alpha( GTK_COLOR_BUTTON (B), alpha); */	\
		free( chrval); \
	} \
	g_signal_emit_by_name( B, "color-set");

	DO_COLOR("NONE",	bColourNONE);
	DO_COLOR("NREM1",	bColourNREM1);
	DO_COLOR("NREM2",	bColourNREM2);
	DO_COLOR("NREM3",	bColourNREM3);
	DO_COLOR("NREM4",	bColourNREM4);
	DO_COLOR("REM",		bColourREM);
	DO_COLOR("Wake",	bColourWake);
	DO_COLOR("PowerSF",   	bColourPowerSF);
	DO_COLOR("EMG",   	bColourEMG);
	DO_COLOR("Hypnogram",	bColourHypnogram);
	DO_COLOR("Artifacts",	bColourArtifacts);
	DO_COLOR("TicksSF",	bColourTicksSF);
	DO_COLOR("LabelsSF",	bColourLabelsSF);
	DO_COLOR("BandDelta",	bColourBandDelta);
	DO_COLOR("BandTheta",	bColourBandTheta);
	DO_COLOR("BandAlpha",	bColourBandAlpha);
	DO_COLOR("BandBeta",	bColourBandBeta);
	DO_COLOR("BandGamma",	bColourBandGamma);
	DO_COLOR("Cursor",	bColourCursor);

	DO_COLOR("TicksMT",	bColourTicksMT);
	DO_COLOR("LabelsMT",	bColourLabelsMT);
	DO_COLOR("PowerMT",   	bColourPowerMT);
#undef DO_COLOR

	for ( gushort i = 0; i < AGH_BAND__TOTAL; ++i ) {
		chrval = g_key_file_get_string( kf, "Bands", AghFreqBandsNames[i], NULL);
		gfloat a, b;
		if ( chrval && sscanf( chrval, "%g,%g", &a, &b) == 2 ) {
			gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[i][0]), a);
			gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[i][1]), b);
			free( chrval);
		}
		g_signal_emit_by_name( GTK_SPIN_BUTTON (eBand[i][0]), "value-changed");
		g_signal_emit_by_name( GTK_SPIN_BUTTON (eBand[i][1]), "value-changed");
	}

	g_string_free( ext_msg, TRUE);
	g_key_file_free( kf);
	if ( kf_error)
		g_error_free( kf_error);

	return 0;
}






gint
agh_ui_settings_save()
{
	GKeyFile *kf = g_key_file_new();

	g_key_file_set_integer( kf, "Common", "CurrentSessionNo", AghDi);
	g_key_file_set_integer( kf, "Common", "CurrentChannelNo", AghTi);
	g_key_file_set_double(  kf, "Common", "OperatingRangeFrom", AghOperatingRangeFrom);
	g_key_file_set_double(  kf, "Common", "OperatingRangeUpto", AghOperatingRangeUpto);

	g_key_file_set_double(  kf, "Scoring Facility", "PixelsPeruV2", AghPPuV2);

	g_key_file_set_boolean( kf, "Batch Run", "IncludeAllChannels",	AghSimRunbatchIncludeAllChannels);
	g_key_file_set_boolean( kf, "Batch Run", "IncludeAllSessions",	AghSimRunbatchIncludeAllSessions);
	g_key_file_set_boolean( kf, "Batch Run", "IterateRanges",	AghSimRunbatchIterateRanges);
//	g_key_file_set_integer( kf, "Batch Run", "RedoOption",		aghsimrunbatchredo_option);

#define DO_COLOR(W,B) \
	snprintf_buf( "%#x,%#x,%#x,%#x", B.red, B.green, B.blue, 0);	\
	g_key_file_set_string( kf, "Colours", W, __buf__);

	DO_COLOR("TicksMT",	__fg0__[cTICKS_MT]);
	DO_COLOR("LabelsMT",	__fg0__[cLABELS_MT]);
	DO_COLOR("PowerMT",   	__fg0__[cPOWER_MT]);

	DO_COLOR("NONE",	__bg1__[cSIGNAL_SCORE_NONE]);
	DO_COLOR("NREM1",	__bg1__[cSIGNAL_SCORE_NREM1]);
	DO_COLOR("NREM2",	__bg1__[cSIGNAL_SCORE_NREM2]);
	DO_COLOR("NREM3",	__bg1__[cSIGNAL_SCORE_NREM3]);
	DO_COLOR("NREM4",	__bg1__[cSIGNAL_SCORE_NREM4]);
	DO_COLOR("REM",		__bg1__[cSIGNAL_SCORE_REM]);
	DO_COLOR("Wake",	__bg1__[cSIGNAL_SCORE_WAKE]);
	DO_COLOR("PowerSF",   	__fg1__[cPOWER_SF]);
	DO_COLOR("EMG",   	__fg1__[cEMG]);
	DO_COLOR("Hypnogram",	__bg1__[cHYPNOGRAM]);
	DO_COLOR("Artifacts",	__fg1__[cARTIFACT]);
	DO_COLOR("TicksSF",	__fg1__[cTICKS_SF]);
	DO_COLOR("LabelsSF",	__fg1__[cLABELS_SF]);
	DO_COLOR("BandDelta",	__fg1__[cBAND_DELTA]);
	DO_COLOR("BandTheta",   __fg1__[cBAND_THETA]);
	DO_COLOR("BandAlpha",	__fg1__[cBAND_ALPHA]);
	DO_COLOR("BandBeta",	__fg1__[cBAND_BETA]);
	DO_COLOR("BandGamma",	__fg1__[cBAND_GAMMA]);
#undef DO_COLOR

	for ( gushort i = 0; i < AGH_BAND__TOTAL; ++i ) {
		snprintf_buf( "%g,%g", AghFreqBands[i][0], AghFreqBands[i][1]);
		g_key_file_set_string( kf, "Bands", AghFreqBandsNames[i], __buf__);
	}

	gchar *towrite = g_key_file_to_data( kf, NULL, NULL);
	g_file_set_contents( AGH_CONF_FILE, towrite, -1, NULL);
	g_free( towrite);

	g_key_file_free( kf);

	return 0;
}








// EOF
