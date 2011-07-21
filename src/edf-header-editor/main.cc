// ;-*-C++-*- *  Time-stamp: "2011-07-20 01:44:45 hmmr"
/*
 *       File name:  edf-heder-editor/main.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-20
 *
 *         Purpose:  EDF header editor utility
 *
 *         License:  GPL
 */




#include <gtk/gtk.h>
#include "libagh/edf.hh"



int
main( int argc, char **argv)
{
	int	c;
	while ( (c = getopt( argc, argv, "h")) != -1 )
		switch ( c ) {
		case 'h':
			printf( "Usage: %s [exp_root_dir]\n", argv[0]);
			return 0;
		}

	g_thread_init( NULL);
	gtk_init( &argc, &argv);

	if ( aghui::prepare_for_expdesign() ) {
		aghui::pop_ok_message( NULL, "UI failed to initialise (try running " PACKAGE_NAME " in a terminal to see why)\n");
		return 2;
	}

	auto ed = new aghui::SExpDesignUI( (optind < argc) ? argv[optind] : "");
	gtk_main();
	delete ed;

//	if ( chdir(wd) )
//		;
//	free( wd);

	return 0;
}

// EOF
