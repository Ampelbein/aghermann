// ;-*-C++-*-
/*
 *       File name:  tools/edfed.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-20
 *
 *         Purpose:  EDF header viewer
 *
 *         License:  GPL
 */


#include "../libagh/edf.hh"
#include <iostream>

int
main( int argc, char **argv)
{
	int	c;
	while ( (c = getopt( argc, argv, "h")) != -1 )
		switch ( c ) {
		case 'h':
			printf( "Usage: %s file.edf\n", argv[0]);
			return 0;
		}

	const char *fname;
	if ( optind < argc )
		fname = argv[optind];
	else {
		printf( "Usage: %s file.edf\n", argv[0]);
		return 1;
	}

	try {
		auto F = agh::CEDFFile (fname, 30);
		F.no_save_extra_files = true;

		cout << F.details();
	} catch (invalid_argument ex) {
		cerr << ex.what() << endl;
	}


	return 0;
}


// EOF
