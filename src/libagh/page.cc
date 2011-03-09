// ;-*-C++-*- *  Time-stamp: "2011-03-07 14:54:13 hmmr"

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
	' ', '1', '2', '3', '4', 'R', 'W', 'M',
};



int
CHypnogram::load_canonical( const char *fname, const char* custom_score_codes[8])
{
	FILE *f = fopen( fname, "r");
	if ( !f )
		return -1;

	size_t	p = 0;
	size_t readbytes = 80;
	char *token = (char*)malloc( readbytes);
	while ( !feof (f) ) {
		SPage	P = { 0., 0., 0. };
		if ( getline( &token, &readbytes, f) == -1 )
			goto out;
		if ( *token == '#' )
			continue;
		if ( !strcasecmp( token, "Wake") ||
		     (strchr( custom_score_codes[AGH_SCORE_WAKE], (int)*token) != NULL) )
			P.Wake = 1.;
		else
		if ( !strcasecmp( token, "NREM1") ||
		     (strchr( custom_score_codes[AGH_SCORE_NREM1], (int)*token) != NULL) )
			P.NREM = .25;
		else
		if ( !strcasecmp( token, "NREM2") ||
		     (strchr( custom_score_codes[AGH_SCORE_NREM2], (int)*token) != NULL) )
			P.NREM = .5;
		else
		if ( !strcasecmp( token, "NREM3") ||
		     (strchr( custom_score_codes[AGH_SCORE_NREM3], (int)*token) != NULL) )
			P.NREM = .75;
		else
		if ( !strcasecmp( token, "NREM4") ||
		     (strchr( custom_score_codes[AGH_SCORE_NREM4], (int)*token) != NULL) )
			P.NREM = 1.;
		else
		if ( !strcasecmp( token, "REM") ||
		     (strchr( custom_score_codes[AGH_SCORE_REM], (int)*token) != NULL) )
			P.REM = 1.;
		else
		if ( !strcasecmp( token, "MVT") ||
		     (strchr( custom_score_codes[AGH_SCORE_MVT], (int)*token) != NULL) )
			;
		else
		if ( !strcasecmp( token, "unscored") ||
		     (strchr( custom_score_codes[AGH_SCORE_NONE], (int)*token) != NULL) )
			;
		else {
			;
		}

		if ( p >= length() )
			_pages.resize( p+20);

		nth_page(p++) = P;
	}
out:
	free( (void*)token);
	fclose( f);
	_pages.resize( p+1);

	return 0;
}



// EOF
