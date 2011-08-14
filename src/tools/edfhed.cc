// ;-*-C++-*-
/*
 *       File name:  tools/edfed-gtk.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-20
 *
 *         Purpose:  EDF header editor utility (using gtk)
 *
 *         License:  GPL
 */


#include "../libagh/edf.hh"


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

	auto F = agh::CEDFFile (fname, 30);
	;
//	if ( chdir(wd) )
//		;
//	free( wd);

	return 0;
}


// EOF
