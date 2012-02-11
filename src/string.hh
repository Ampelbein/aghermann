// ;-*-C++-*-
/*
 *       File name:  string.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-02-11
 *
 *         Purpose:  strings ops
 *
 *         License:  GPL
 */

#ifndef _AGH_STRING_H
#define _AGH_STRING_H

#include <cstring>
#include <string>
#include <memory>
#include <list>
#include <sstream>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


inline string
strtrim( const string& r0)
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

inline string
strpad( const string& r0, size_t to)
{
	string r (to, ' ');
	memcpy( (void*)r.data(), (const void*)r0.data(), r0.size());
	return r;
}



template <class C>
string
string_join( const C& l, const char* sep)
{
	if ( l.size() == 0 )
		return "";
	ostringstream recv;
	size_t i = 0;
	auto I = l.begin();
	for ( ; i < l.size()-1; ++i, ++I )
		recv << *I << sep;
	recv << *I;
	return recv.str();
}


inline list<string>
string_tokens( const string& s_, const char* sep)
{
	string s {s_};
	list<string> acc;
	char *p = strtok( &s[0], sep);
	while ( p ) {
		acc.emplace_back( strtrim(p));
		p = strtok( NULL, sep);
	}
	return acc;
}


#endif

// eof
