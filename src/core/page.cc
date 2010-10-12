// ;-*-C++-*- *  Time-stamp: "2010-10-04 03:08:55 hmmr"

/*
 * Author: Andrei Zavada (johnhommer@gmail.com)
 *
 * License: GPL
 *
 * Initial version: 2010-04-28
 */

#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include "page.hh"

using namespace std;





int
CHypnogram::save( const char* fname) const
{
	if ( !length() )
		return 0;

	FILE *f = fopen( fname, "w");
	if ( !f )
		return -1;

	fprintf( f, "%zu\n", _pagesize);
	for ( size_t p = 0; p < _pages.size(); ++p )
		fprintf( f, "%g\t%g\t%g\n", nth_page(p).NREM, nth_page(p).REM, nth_page(p).Wake);

	fclose( f);

	return 0;
}


int
CHypnogram::load( const char* fname)
{
	int retval = 0;
	FILE *f = fopen( fname, "r");
	if ( !f )
		return AGH_HYP_NOFILE;

	SPage	P;

	size_t saved_pagesize;
	if ( fscanf( f, "%zu\n", &saved_pagesize) < 1 ) {
		retval = AGH_HYP_BADDATA;
		goto out;
	}
	if ( saved_pagesize != _pagesize ) {
		fprintf( stderr, "CHypnogram::load(\"%s\"): read pagesize (%zu) different from that specified at construct (%zu)\n",
			 fname, saved_pagesize, _pagesize);
		_pagesize = saved_pagesize;
		retval = AGH_HYP_WRONGPAGESIZE;
		goto out;
	}

	while ( fscanf( f, "%g %g %g\n", &P.NREM, &P.REM, &P.Wake) == 3 )
		_pages.emplace_back( P);
out:
	fclose( f);

	return retval;
}






int
CHypnogram::save_canonical( const char *fname) const
{
	if ( !length() )
		return 0;

	FILE *f = fopen( fname, "w");
	if ( !f )
		return -1;

	for ( size_t p = 0; p < length(); ++p ) {
		float	N = nth_page(p).NREM,
			R = nth_page(p).REM,
			W = nth_page(p).Wake;
		fprintf( f, "%s\n",
			 N > .7 ?"NREM4" :N > .4 ?"NREM3"
			 :R > .5 ?"REM"
			 :W > .5 ?"Wake"
			 :N > .2 ?"NREM2"
			 :N > .01 ?"NREM1"
			 :"unscored");
	}
	fclose( f);

	return 0;
}






const char AghScoreCodes[] = {
	' ', '1', '2', '3', '4', 'W', 'R', 'M',
};



int
CHypnogram::load_canonical( const char *fname)
{
	FILE *f = fopen( fname, "r");
	if ( !f )
		return -1;

	size_t	p = 0;
	char token[19];
	while ( !feof (f) ) {
		SPage	P = { 0., 0., 0. };
		if ( fscanf( f, "%18s", token) < 1 )
			return -1;
		if ( !strcasecmp( token, "Wake") ||
		     !strcasecmp( token, "W") ||
		     (strlen( token) == 1 && *token == AghScoreCodes[AGH_SCORE_WAKE]) )
			P.Wake = 1.;
		else
		if ( !strcasecmp( token, "NREM1") ||
		     !strcasecmp( token, "N1") ||
		     (strlen( token) == 1 && *token == AghScoreCodes[AGH_SCORE_NREM1]) )
			P.NREM = .25;
		else
		if ( !strcasecmp( token, "NREM2") ||
		     !strcasecmp( token, "N2") ||
		     (strlen( token) == 1 && *token == AghScoreCodes[AGH_SCORE_NREM2]) )
			P.NREM = .5;
		else
		if ( !strcasecmp( token, "NREM3") ||
		     !strcasecmp( token, "N3") ||
		     (strlen( token) == 1 && *token == AghScoreCodes[AGH_SCORE_NREM3]) )
			P.NREM = .75;
		else
		if ( !strcasecmp( token, "NREM4") ||
		     !strcasecmp( token, "N4") ||
		     (strlen( token) == 1 && *token == AghScoreCodes[AGH_SCORE_NREM4]) )
			P.NREM = 1.;
		else
		if ( !strcasecmp( token, "REM") ||
		     !strcasecmp( token, "R") ||
		     (strlen( token) == 1 && *token == AghScoreCodes[AGH_SCORE_REM]) )
			P.REM = 1.;
		else
		if ( !strcasecmp( token, "MVT") ||
		     !strcasecmp( token, "M") ||
		     (strlen( token) == 1 && *token == AghScoreCodes[AGH_SCORE_MVT]) )
			;
		else
		if ( !strcasecmp( token, "-") ||
		     !strcasecmp( token, "9") ||
		     !strcasecmp( token, "unscored") ||
		     (strlen( token) == 1 && *token == AghScoreCodes[AGH_SCORE_NONE]) )
			;
		else {
			continue;
		}

		if ( p >= length() )
			_pages.resize( p+20);

		nth_page(p++) = P;
	}
	fclose( f);
	_pages.resize( p+1);

	return 0;
}



// EOF
