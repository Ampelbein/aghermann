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
#include "libsigfile/source-base.hh" // for CEDFFile::TStatus flags
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

int
str_to_english_month( const string& s)
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

const char*
english_month_to_str( int m)
{
	switch ( m ) {
	case  0: return "jan";
	case  1: return "feb";
	case  2: return "mar";
	case  3: return "apr";
	case  4: return "may";
	case  5: return "jun";
	case  6: return "jul";
	case  7: return "aug";
	case  8: return "sep";
	case  9: return "oct";
	case 10: return "nov";
	case 11: return "dec";
	default: return "---";
	}
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
		t.tm_year = stoi(*f);
		if ( t.tm_year > 100 )
			t.tm_year -= 1900;
		t.tm_isdst = -1;
		return mktime( &t);
	} catch (...) {
		return (time_t)0;
	}
}

string
SSubjectId::
dob_to_str( const time_t t_)
{
	struct tm t;
	gmtime_r( &t_, &t);
	return agh::str::sasprintf(
		"%02d-%s-%02d",
		t.tm_mday,
		english_month_to_str(t.tm_mon),
		t.tm_year % 100);
}



int
SSubjectId::
update_from( const SSubjectId& j)
{
	int mismatched_fields = 0;
	if ( id.empty() or id == "X" )
		id = j.id;
	else if ( id != j.id )
		++mismatched_fields;

	if ( name.empty() or name == "X" )
		name = j.name;
	else if ( name != j.name )
		++mismatched_fields;

	if ( gender == TGender::unknown )
		gender = j.gender;
	else if ( gender != j.gender )
		++mismatched_fields;

	if ( dob == (time_t)0 )
		dob = j.dob;
	else if ( dob != j.dob )
		++mismatched_fields;

	return mismatched_fields;
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
