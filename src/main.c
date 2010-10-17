// ;-*-C-*- *  Time-stamp: "2010-10-17 18:26:43 hmmr"
/*
 *       File name:  main.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-05-03
 *
 *         Purpose:  Function main
 *
 *         License:  GPL
 */




#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "ui/misc.h"
#include "ui/ui.h"
#include "core/iface.h"



int
main( int argc, char **argv)
{
	printf( PACKAGE_NAME " v. " PACKAGE_VERSION " compiled " __DATE__ " " __TIME__"\n");

	char *wd = getcwd( NULL, 0);

	g_thread_init( NULL);
	gtk_init( &argc, &argv);

	if ( agh_ui_construct() ) {
		pop_ok_message( NULL, "UI failed to initialise (start " PACKAGE_NAME " in a terminal to see why)\n");
		return 2;
	}


	agh_histfile_read();

	gtk_widget_show_all( wMainWindow);
	while ( gtk_events_pending () )
	 	gtk_main_iteration();

	if ( agh_expdesign_init( AghLastExpdesignDir, progress_indicator) ) {
		pop_ok_message( GTK_WINDOW (wMainWindow), agh_expdesign_messages());
		return 1;
	}
	if ( strlen( agh_expdesign_messages()) > 0 ) {
		gtk_text_buffer_set_text( gtk_text_view_get_buffer( GTK_TEXT_VIEW (lScanLog)),
					  agh_expdesign_messages(), -1);
		gtk_widget_show_all( wScanLog);
	}

	agh_ui_populate();
	gtk_main();
	agh_ui_depopulate();

	agh_expdesign_shutdown();

	agh_histfile_write();

	if ( chdir(wd) )
		;
	free( wd);

	return 0;
}

// EOF
