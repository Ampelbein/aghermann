// ;-*-C++-*- *  Time-stamp: "2011-05-08 01:57:06 hmmr"
/*
 *       File name:  main.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-03
 *
 *         Purpose:  Function main
 *
 *         License:  GPL
 */




#include <cstdlib>
#include <unistd.h>
#include <gtk/gtk.h>
#include "ui/misc.hh"
#include "ui/ui.hh"
#include "ui/settings.hh"
#include "libagh/primaries.hh"



int
main( int argc, char **argv)
{
	printf( PACKAGE_STRING " compiled " __DATE__ " " __TIME__ " " BUILT_BY "\n");

	char *wd = getcwd( NULL, 0);

	int	c;
	while ( (c = getopt( argc, argv, "h")) != -1 )
		switch ( c ) {
		case 'h':
			printf( "Usage: %s [exp_root_dir]\n", argv[0]);
			return 0;
		}

	g_thread_init( NULL);
	gtk_init( &argc, &argv);

	if ( aghui::construct_once() ) {
		aghui::pop_ok_message( NULL, "UI failed to initialise (start " PACKAGE_NAME " in a terminal to see why)\n");
		return 2;
	}

	if ( optind < argc ) {
		aghui::settings::LastExpdesignDir = argv[optind];
		gtk_widget_set_sensitive( GTK_WIDGET (aghui::bExpChange), FALSE);
	} else
		aghui::sb::histfile_read();

	gtk_widget_show_all( GTK_WIDGET (aghui::wMainWindow));
	aghui::set_cursor_busy( true, GTK_WIDGET (aghui::wMainWindow));
	while ( gtk_events_pending() )
	 	gtk_main_iteration();

	try {
		AghCC = new agh::CExpDesign (aghui::settings::LastExpdesignDir, aghui::progress_indicator);

		if ( !AghCC ) {
			fprintf( stderr, "agh_expdesign_init(): AghCC is NULL\n");
			return 1;
		}
	} catch (invalid_argument ex) {
		fprintf( stderr, "agh_expdesign_init(\"%s\"): %s\n",
			 aghui::settings::LastExpdesignDir, AghCC->error_log());
		aghui::pop_ok_message( GTK_WINDOW (aghui::wMainWindow), AghCC->error_log());
		return 1;
	}

	if ( strlen( AghCC->error_log()) > 0 ) {
		gtk_text_buffer_set_text( gtk_text_view_get_buffer( GTK_TEXT_VIEW (aghui::lScanLog)),
					  AghCC->error_log(), -1);
		gtk_widget_show_all( GTK_WIDGET (aghui::wScanLog));
	}

	aghui::populate( true);
	aghui::set_cursor_busy( false, GTK_WIDGET (aghui::wMainWindow));
	gtk_main();
	aghui::depopulate( true);

	delete AghCC;

	// don't update hist_file if expdir was from command line
	if ( !(optind < argc) )
		aghui::sb::histfile_write();

	if ( chdir(wd) )
		;
	free( wd);

	return 0;
}

// EOF
