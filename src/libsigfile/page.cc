// ;-*-C++-*-
/*
 *       File name:  libsigfile/page.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  page and hypnogram classes
 *
 * License: GPL
 *
 */

#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <algorithm>
#include "page.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif

using namespace std;


const char sigfile::SPage::score_codes[] = {
	' ', '1', '2', '3', '4', 'R', 'W', 'M',
};


const char* const sigfile::SPage::score_names[(size_t)TScore::_total] = {
	"blank",
	"NREM1", "NREM2", "NREM3", "NREM4",
	"REM",
	"Wake",
	"MVT"
};


float
sigfile::CHypnogram::percent_scored( float *nrem_p, float *rem_p, float *wake_p) const
{
	if ( nrem_p )
		*nrem_p = (float)count_if( _pages.begin(), _pages.end(),
					   mem_fun_ref (&SPage::is_nrem)) / _pages.size() * 100;
	if ( rem_p )
		*rem_p = (float)count_if( _pages.begin(), _pages.end(),
					  mem_fun_ref (&SPage::is_rem)) / _pages.size() * 100;
	if ( wake_p )
		*wake_p = (float)count_if( _pages.begin(), _pages.end(),
					   mem_fun_ref (&SPage::is_wake)) / _pages.size() * 100;

	return (float)count_if( _pages.begin(), _pages.end(),
				mem_fun_ref (&SPage::is_scored)) / _pages.size() * 100;
}



sigfile::CHypnogram::TError
sigfile::CHypnogram::save( const char* fname) const
{
	ofstream of (fname, ios_base::trunc);
	if ( not of.good() )
		return CHypnogram::TError::nofile;

	of << _pagesize << endl;
	for ( size_t p = 0; p < _pages.size(); ++p )
		of << (*this)[p].NREM << '\t' << (*this)[p].REM << '\t' << (*this)[p].Wake << endl;

	return CHypnogram::TError::ok;
}


sigfile::CHypnogram::TError
sigfile::CHypnogram::load( const char* fname)
{
	ifstream f (fname);
	if ( not f.good() )
		return CHypnogram::TError::nofile;

	SPage	P;

	size_t saved_pagesize;
	f >> saved_pagesize;
	if ( not f.good() )
		return CHypnogram::TError::baddata;

	if ( saved_pagesize != _pagesize ) {
		fprintf( stderr, "CHypnogram::load(\"%s\"): read pagesize (%zu) different from that specified at construct (%zu)\n",
			 fname, saved_pagesize, _pagesize);
		_pagesize = saved_pagesize;
		return CHypnogram::TError::wrongpagesize;
	}

	while ( not (f >> P.NREM >> P.REM >> P.Wake).eof() )
		_pages.emplace_back( P);

	return CHypnogram::TError::ok;
}






int
sigfile::CHypnogram::save_canonical( const char *fname) const
{
	if ( !length() )
		return 0;

	FILE *f = fopen( fname, "w");
	if ( !f )
		return -1;

	for ( size_t p = 0; p < length(); ++p ) {
		float	N = (*this)[p].NREM,
			R = (*this)[p].REM,
			W = (*this)[p].Wake;
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









int
sigfile::CHypnogram::load_canonical( const char *fname,
				     const TCustomScoreCodes& custom_score_codes)
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
		     (strchr( custom_score_codes[(size_t)SPage::TScore::wake].c_str(), (int)*token) != NULL) )
			P.Wake = 1.;
		else
		if ( !strcasecmp( token, "NREM1") ||
		     (strchr( custom_score_codes[(size_t)SPage::TScore::nrem1].c_str(), (int)*token) != NULL) )
			P.NREM = .25;
		else
		if ( !strcasecmp( token, "NREM2") ||
		     (strchr( custom_score_codes[(size_t)SPage::TScore::nrem2].c_str(), (int)*token) != NULL) )
			P.NREM = .5;
		else
		if ( !strcasecmp( token, "NREM3") ||
		     (strchr( custom_score_codes[(size_t)SPage::TScore::nrem3].c_str(), (int)*token) != NULL) )
			P.NREM = .75;
		else
		if ( !strcasecmp( token, "NREM4") ||
		     (strchr( custom_score_codes[(size_t)SPage::TScore::nrem4].c_str(), (int)*token) != NULL) )
			P.NREM = 1.;
		else
		if ( !strcasecmp( token, "REM") ||
		     (strchr( custom_score_codes[(size_t)SPage::TScore::rem].c_str(), (int)*token) != NULL) )
			P.REM = 1.;
		else
		if ( !strcasecmp( token, "MVT") ||
		     (strchr( custom_score_codes[(size_t)SPage::TScore::mvt].c_str(), (int)*token) != NULL) )
			;
		else
		if ( !strcasecmp( token, "unscored") ||
		     (strchr( custom_score_codes[(size_t)SPage::TScore::none].c_str(), (int)*token) != NULL) )
			;
		else {
			;
		}

		(*this)[p++] = P;
	}
out:
	free( (void*)token);
	fclose( f);
	_pages.resize( p+1);

	return 0;
}


// EOF
