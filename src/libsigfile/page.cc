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

using namespace std;


const char sigfile::SPage::score_codes[] = {
	' ', '1', '2', '3', '4', 'R', 'W',
};


const char* const sigfile::SPage::score_names[TScore::TScore_total] = {
	"blank",
	"NREM1", "NREM2", "NREM3", "NREM4",
	"REM",
	"Wake"
};


float
sigfile::CHypnogram::
percent_scored( float *nrem_p, float *rem_p, float *wake_p) const
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
sigfile::CHypnogram::
save( const char* fname) const
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
sigfile::CHypnogram::
load( const char* fname)
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
sigfile::CHypnogram::
save_canonical( const char *fname) const
{
	FILE *f = fopen( fname, "w");
	if ( !f )
		return -1;

	for ( size_t p = 0; p < pages(); ++p ) {
		float	N = (*this)[p].NREM,
			R = (*this)[p].REM,
			W = (*this)[p].Wake;
		fprintf( f, "%s\n",
			 N > .7 ?"NREM4"
			 : N > .4 ?"NREM3"
			 : R > .5 ?"REM"
			 : W > .5 ?"Wake"
			 : N > .2 ?"NREM2"
			 : N > .01 ?"NREM1"
			 : "unscored");
	}
	fclose( f);

	return 0;
}









int
sigfile::CHypnogram::
load_canonical( const char *fname,
		const TCustomScoreCodes& custom_score_codes)
{
	ifstream f (fname);
	if ( !f.good() )
		return -1;

	size_t	p = 0;
	string token;
	while ( p < _pages.size() ) {
		if ( f.eof() )
			return 2; // short
		SPage	P = { 0., 0., 0. };
		getline( f, token);
		int c = (int)token[0];
		if ( c == '#' )
			continue;
		if ( !strcasecmp( token.c_str(), "Wake") ||
		     (strchr( custom_score_codes[SPage::TScore::wake].c_str(), c) != NULL) )
			P.Wake = 1.;
		else if ( !strcasecmp( token.c_str(), "NREM1") ||
			  (strchr( custom_score_codes[SPage::TScore::nrem1].c_str(), c) != NULL) )
			P.NREM = .25;
		else if ( !strcasecmp( token.c_str(), "NREM2") ||
			  (strchr( custom_score_codes[SPage::TScore::nrem2].c_str(), c) != NULL) )
			P.NREM = .5;
		else if ( !strcasecmp( token.c_str(), "NREM3") ||
			  (strchr( custom_score_codes[SPage::TScore::nrem3].c_str(), c) != NULL) )
			P.NREM = .75;
		else if ( !strcasecmp( token.c_str(), "NREM4") ||
			  (strchr( custom_score_codes[SPage::TScore::nrem4].c_str(), c) != NULL) )
			P.NREM = 1.;
		else if ( !strcasecmp( token.c_str(), "REM") ||
			  (strchr( custom_score_codes[SPage::TScore::rem].c_str(), c) != NULL) )
			P.REM = 1.;
		else if ( !strcasecmp( token.c_str(), "unscored") ||
			  (strchr( custom_score_codes[SPage::TScore::none].c_str(), c) != NULL) )
			;
		else {
			;
		}

		(*this)[p++] = P;
	}

	return f.eof() ? 0 : 1;
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

