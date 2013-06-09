/*
 *       File name:  common/string.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-02-11
 *
 *         Purpose:  strings ops
 *
 *         License:  GPL
 */

#ifndef _AGH_COMMON_STRING_H
#define _AGH_COMMON_STRING_H

#include <string>
#include <list>
#include <sstream>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {
namespace str {

string trim( const string& r0);
string pad( const string& r0, size_t to);

string sasprintf( const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));



template <typename C>
string
join( const C& l, const char* sep)
{
	if ( l.empty() )
		return "";
	ostringstream recv;
	auto I = l.begin();
	for ( ; next(I) != l.end(); ++I )
		recv << *I << sep;
	recv << *I;
	return recv.str();
}

list<string> tokens( const string& s_, const char* sep);
inline
list<string> tokens( const string& s_, char c)
{
	char sep[2] = {c, '\0'};
	return tokens( s_, sep);
}

list<string> tokens_trimmed( const string& s_, const char* sep);
inline
list<string> tokens_trimmed( const string& s_, char c)
{
	char sep[2] = {c, '\0'};
	return tokens_trimmed( s_, sep);
}



void decompose_double( double value, double *mantissa, int *exponent);



string& homedir2tilda( string& inplace);
string  homedir2tilda( const string& v);
string& tilda2homedir( string& inplace);
string  tilda2homedir( const string& v);

string dhms( double seconds, int decimal_digits = 0) __attribute__ ((pure));
string dhms_colon( double seconds, int decimal_digits = 0) __attribute__ ((pure));


// unicode/wcs

wstring to_wstring( const string&, const char* charset = "UTF-8");
string from_wstring( const wstring&, const char* charset = "UTF-8");


}
}

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
