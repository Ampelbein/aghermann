/*
 *       File name:  common/subject_id.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-04-13
 *
 *         Purpose:  subject_id shared between agh::CSubject and libsigfile::CSource
 *
 *         License:  GPL
 */

#include <ctime>

#include <string>
#include <cstring>
#include "string.hh"
#include "subject_id.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;
using agh::SSubjectId;

SSubjectId::TGender
SSubjectId::
char_to_gender( char x)
{
	switch ( x ) {
	case 'M':
	case 'm':
		return TGender::male;
	case 'F':
	case 'f':
		return TGender::female;
	default:
		return TGender::unknown;
	}
}


char
__attribute__ ((const))
SSubjectId::
gender_sign( TGender g)
{
	switch ( g ) {
	case TGender::male:
		return 'M';
	case TGender::female:
		return 'F';
	default:
		return 'X';
	}
}



namespace {

int str_to_english_month( const string& s)
{
	if ( strcasecmp( s.c_str(), "jan") == 0 )
		return 0;
	if ( strcasecmp( s.c_str(), "feb") == 0 )
		return 1;
	if ( strcasecmp( s.c_str(), "mar") == 0 )
		return 2;
	if ( strcasecmp( s.c_str(), "apr") == 0 )
		return 3;
	if ( strcasecmp( s.c_str(), "may") == 0 )
		return 4;
	if ( strcasecmp( s.c_str(), "jun") == 0 )
		return 5;
	if ( strcasecmp( s.c_str(), "jul") == 0 )
		return 6;
	if ( strcasecmp( s.c_str(), "aug") == 0 )
		return 7;
	if ( strcasecmp( s.c_str(), "sep") == 0 )
		return 8;
	if ( strcasecmp( s.c_str(), "oct") == 0 )
		return 9;
	if ( strcasecmp( s.c_str(), "nov") == 0 )
		return 10;
	if ( strcasecmp( s.c_str(), "dec") == 0 )
		return 11;
	else
		return -1;
}
}


time_t
SSubjectId::
str_to_dob( const string& s)
{
	struct tm t;
	memset( &t, '\0', sizeof (t));

	// strptime( s, "%d-", &t); // will suck in non-US locales, so
	auto ff = agh::str::tokens(s, "-");
	if ( ff.size() != 3 )
		return (time_t)0;
	auto f = ff.begin();
	try {
		t.tm_mday = stoi( *f++);
		t.tm_mon  = str_to_english_month(*f++);
		t.tm_year = 1900 + stoi(*f);
		return mktime( &t);
	} catch (...) {
		return (time_t)0;
	}
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
