// ;-*-C++-*- *  Time-stamp: "2010-11-20 21:43:47 hmmr"
/*
 *       File name:  ui-loadsave.c
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

#include "../core/iface.h"
#include "ui.h"



#define AGH_CONF_FILE ".aghermann.conf"

gint
agh_ui_settings_load()
{
	GKeyFile *kf = g_key_file_new();

	GError *kf_error = NULL;

//	gint	intval;
	gdouble	dblval;
	GString *ext_msg = g_string_sized_new( 120);

	if ( !g_key_file_load_from_file( kf, AGH_CONF_FILE, G_KEY_FILE_KEEP_COMMENTS, &kf_error) ) {
		fprintf( stderr, "agh_ui_settings_load(): failed (%s)\n", kf_error->message);
		return -1;
	}

	dblval = g_key_file_get_double( kf, "View", "PixelsPeruV2", NULL);
	if ( dblval < 0 ) {
		g_string_append_printf( ext_msg, "Bad value for PixelsPeruV2.\n");
		AghPPuV2 = 1e-5;
	} else
		AghPPuV2 = dblval;

	dblval = g_key_file_get_double( kf, "Design", "OperatingRangeFrom", NULL);
	if ( dblval <= 0 ) {
		g_string_append_printf( ext_msg, "OperatingRangeFrom must be >0.\n");
		AghOperatingRangeFrom = 2.;
	} else
		AghOperatingRangeFrom = dblval;
	dblval = g_key_file_get_double( kf, "Design", "OperatingRangeUpto", NULL);
	if ( dblval <= AghOperatingRangeFrom ) {
		g_string_append_printf( ext_msg, "OperatingRangeUpto must be > OperatingRangeFrom.\n");
		AghOperatingRangeUpto = 3.;
	} else
		AghOperatingRangeUpto = dblval;


	AghDi = g_key_file_get_integer( kf, "Design", "CurrentSessionNo", NULL);
	AghTi = g_key_file_get_integer( kf, "Design", "CurrentChannelNo", NULL);
	// defer assignment of AghX until AghXs are collected from agh_enumerate_*

	if ( g_key_file_has_group( kf, "Batch Run") ) {
		AghSimRunbatchIncludeAllChannels	= g_key_file_get_boolean( kf, "Batch Run", "IncludeAllChannels",  NULL);
		AghSimRunbatchIncludeAllSessions	= g_key_file_get_boolean( kf, "Batch Run", "IncludeAllSessions",  NULL);
		AghSimRunbatchIterateRanges		= g_key_file_get_boolean( kf, "Batch Run", "IterateRanges",  NULL);
//		AghSimRunbatchRedo_Option		= g_key_file_get_integer( kf, "Batch Run", "RedoOption",  NULL);
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

	g_key_file_set_integer( kf, "Design", "CurrentSessionNo", AghDi);
	g_key_file_set_integer( kf, "Design", "CurrentChannelNo", AghTi);
	g_key_file_set_double(  kf, "Design", "OperatingRangeFrom", AghOperatingRangeFrom);
	g_key_file_set_double(  kf, "Design", "OperatingRangeUpto", AghOperatingRangeUpto);


	g_key_file_set_boolean( kf, "Batch Run", "IncludeAllChannels",	AghSimRunbatchIncludeAllChannels);
	g_key_file_set_boolean( kf, "Batch Run", "IncludeAllSessions",	AghSimRunbatchIncludeAllSessions);
	g_key_file_set_boolean( kf, "Batch Run", "IterateRanges",	AghSimRunbatchIterateRanges);
//	g_key_file_set_integer( kf, "Batch Run", "RedoOption",		aghsimrunbatchredo_option);

	g_key_file_set_double(  kf, "View", "PixelsPeruV2", AghPPuV2);

	gchar *towrite = g_key_file_to_data( kf, NULL, NULL);
	g_file_set_contents( AGH_CONF_FILE, towrite, -1, NULL);
	g_free( towrite);

	g_key_file_free( kf);

	return 0;
}








// EOF
