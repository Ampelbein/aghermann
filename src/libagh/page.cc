// ;-*-C++-*- *  Time-stamp: "2011-03-30 02:20:47 hmmr"
/*
 *       File name:  libagh/primaries.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  page scoring marks
 *
 * License: GPL
 *
 */

#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <fstream>
#include "page.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif

namespace agh {

using namespace std;





THypnogramError
CHypnogram::save( const char* fname) const
{
	ofstream of (fname, ios_base::trunc);
	if ( not of.good() )
		return THypnogramError::nofile;

	of << _pagesize << endl;
	for ( size_t p = 0; p < _pages.size(); ++p )
		of << nth_page(p).NREM << '\t' << nth_page(p).REM << '\t' << nth_page(p).Wake << endl;

	return THypnogramError::ok;
}


THypnogramError
CHypnogram::load( const char* fname)
{
	ifstream f (fname);
	if ( not f.good() )
		return THypnogramError::nofile;

	SPage	P;

	size_t saved_pagesize;
	f >> saved_pagesize;
	if ( not f.good() )
		return THypnogramError::baddata;

	if ( saved_pagesize != _pagesize ) {
		fprintf( stderr, "CHypnogram::load(\"%s\"): read pagesize (%zu) different from that specified at construct (%zu)\n",
			 fname, saved_pagesize, _pagesize);
		_pagesize = saved_pagesize;
		return THypnogramError::wrongpagesize;
	}

	while ( not (f >> P.NREM >> P.REM >> P.Wake).eof() )
		_pages.emplace_back( P);

	return THypnogramError::ok;
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
	while ( !feof (f) && p < length() ) {
		SPage	P = { 0., 0., 0. };
		if ( getline( &token, &readbytes, f) == -1 )
			goto out;
		if ( *token == '#' )
			continue;
		if ( !strcasecmp( token, "Wake") ||
		     (strchr( custom_score_codes[(size_t)TScore::wake], (int)*token) != NULL) )
			P.Wake = 1.;
		else
		if ( !strcasecmp( token, "NREM1") ||
		     (strchr( custom_score_codes[(size_t)TScore::nrem1], (int)*token) != NULL) )
			P.NREM = .25;
		else
		if ( !strcasecmp( token, "NREM2") ||
		     (strchr( custom_score_codes[(size_t)TScore::nrem2], (int)*token) != NULL) )
			P.NREM = .5;
		else
		if ( !strcasecmp( token, "NREM3") ||
		     (strchr( custom_score_codes[(size_t)TScore::nrem3], (int)*token) != NULL) )
			P.NREM = .75;
		else
		if ( !strcasecmp( token, "NREM4") ||
		     (strchr( custom_score_codes[(size_t)TScore::nrem4], (int)*token) != NULL) )
			P.NREM = 1.;
		else
		if ( !strcasecmp( token, "REM") ||
		     (strchr( custom_score_codes[(size_t)TScore::rem], (int)*token) != NULL) )
			P.REM = 1.;
		else
		if ( !strcasecmp( token, "MVT") ||
		     (strchr( custom_score_codes[(size_t)TScore::mvt], (int)*token) != NULL) )
			;
		else
		if ( !strcasecmp( token, "unscored") ||
		     (strchr( custom_score_codes[(size_t)TScore::none], (int)*token) != NULL) )
			;
		else {
			;
		}

		nth_page(p++) = P;
	}
out:
	free( (void*)token);
	fclose( f);
	_pages.resize( p+1);

	return 0;
}

}

// EOF
