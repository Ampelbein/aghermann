// ;-*-C++-*- *
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




#include <gtk/gtk.h>
#include "ui/misc.hh"
#include "ui/expdesign.hh"


int
main( int argc, char **argv)
{
	agh::printversion();

//	char *wd = getcwd( NULL, 0);

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
