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

#include <cstring>
#include <string>
#include <list>
#include <sstream>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {
namespace str {

enum class TStrCmpCaseOption {
	sensitive, insensitive
};

string sasprintf( const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));

string trim( const string& r0);
string pad( const string& r0, size_t to);

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

list<string> tokens( const string&, const char* sep);

inline
list<string>
tokens( const string& s_, char c)
{
	char sep[2] = {c, '\0'};
	return move(tokens( s_, sep));
}

list<string> tokens_trimmed( const string& s_, const char* sep);

inline
list<string>
tokens_trimmed( const string& s_, char c)
{
	char sep[2] = {c, '\0'};
	return move(tokens_trimmed( s_, sep));
}



inline
bool
has_suffix( const string& s, const string& suffix,
	    TStrCmpCaseOption case_option = TStrCmpCaseOption::sensitive)
{
	return	suffix.size() <= s.size() and
		0 == (case_option == TStrCmpCaseOption::sensitive ? strcmp : strcasecmp)(
			&s[s.size()-suffix.size()], &suffix[0]);
}

void decompose_double( double value, double *mantissa, int *exponent);



string& homedir2tilda( string& inplace);
string  homedir2tilda( const string&);
string& tilda2homedir( string& inplace);
string  tilda2homedir( const string&);

string dhms( double seconds, int decimal_digits = 0) __attribute__ ((pure));
string dhms_colon( double seconds, int decimal_digits = 0) __attribute__ ((pure));


// unicode/wcs; uncomment on demand
// wstring to_wstring( const string&, const char* charset = "UTF-8");
// string from_wstring( const wstring&, const char* charset = "UTF-8");


}
}

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
