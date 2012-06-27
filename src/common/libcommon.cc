// ;-*-C++-*-
/*
 *       File name:  common/libcommon.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-23
 *
 *         Purpose:  sundry bits too big for inlining
 *
 *         License:  GPL
 */


#include <unistd.h>
#include <cstring>
#include <string>
#include <list>

#include "string.hh"
#include "misc.hh"


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;



string
agh::str::trim( const string& r0)
{
	string r (r0);
	auto rsize = r.size();
	if ( rsize == 0 )
		return r;
	while ( rsize > 0 && r[rsize-1] == ' ' )
		--rsize;
	r.resize( rsize);
	r.erase( 0, r.find_first_not_of(" \t"));
	return r;
}

string
agh::str::pad( const string& r0, size_t to)
{
	string r (to, ' ');
	memcpy( (void*)r.data(), (const void*)r0.data(), r0.size());
	return r;
}




list<string>
agh::str::tokens( const string& s_, const char* sep)
{
	string s {s_};
	list<string> acc;
	char *p = strtok( &s[0], sep);
	while ( p ) {
		acc.emplace_back( trim(p));
		p = strtok( NULL, sep);
	}
	return acc;
}





string&
agh::str::homedir2tilda( string& inplace)
{
	const char *home = getenv("HOME");
	if ( home )
		if ( inplace.compare( 0, strlen(home), home) == 0 )
			inplace.replace( 0, strlen(home), "~");
	return inplace;
}

string
agh::str::homedir2tilda( const string& v)
{
	string inplace (v);
	const char *home = getenv("HOME");
	if ( home )
		if ( inplace.compare( 0, strlen(home), home) == 0 )
			inplace.replace( 0, strlen(home), "~");
	return inplace;
}

string&
agh::str::tilda2homedir( string& inplace)
{
	const char *home = getenv("HOME");
	if ( home ) {
		size_t at;
		while ( (at = inplace.find( '~')) < inplace.size() )
			inplace.replace( at, 1, home);
	}
	return inplace;
}

string
agh::str::tilda2homedir( const string& v)
{
	string inplace (v);
	const char *home = getenv("HOME");
	if ( home ) {
		size_t at;
		while ( (at = inplace.find( '~')) < inplace.size() )
			inplace.replace( at, 1, home);
	}
	return inplace;
}





double
__attribute__ ((pure))
agh::sensible_scale_reduction_factor( double display_scale,
				      double constraint_max, double constraint_min)
{
	double f = 1.;
	bool	last_was_two = false;
	while ( display_scale * f > constraint_max ) {
		f /= last_was_two ? 5. : 2.;
		last_was_two = !last_was_two;
	}
	while ( display_scale * f < constraint_min ) {
		f *= last_was_two ? 5. : 2.;
		last_was_two = !last_was_two;
	}
	return f;
}



// eof
