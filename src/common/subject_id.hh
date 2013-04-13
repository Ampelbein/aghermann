/*
 *       File name:  common/subject_id.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-04-13
 *
 *         Purpose:  subject_id shared between agh::CSubject and libsigfile::CSource
 *
 *         License:  GPL
 */

#ifndef _AGH_SUBJECT_ID
#define _AGH_SUBJECT_ID

#include <ctime>

#include <string>
#include <cstring>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {


// follow http://www.edfplus.info/specs/edfplus.html#datarecords, section 2.1.3.3
struct SSubjectId {
	string	id,
		name;
	time_t	dob;
	enum class TGender : char {
		unknown = 'X', male = 'M', female = 'F'
	};
	TGender	gender;

	SSubjectId ( const string& id_ = "", const string& name_ = "",
		     time_t dob_ = (time_t)0,
		     TGender gender_ = TGender::unknown)
	      : id (id_),
		name (name_),
		dob (dob_),
		gender (gender_)
		{}
	SSubjectId (const SSubjectId& rv)
 		{
			id = rv.id;
			name = rv.name;
			dob = rv.dob;
			gender = rv.gender;
		}

	SSubjectId (SSubjectId&& rv)
 		{
			id.swap( rv.id);
			name.swap( rv.name);
			dob = rv.dob;
			gender = rv.gender;
		}

	char gender_sign() const
		{
			return gender_sign(gender);
		}

	static char gender_sign( TGender);
	static TGender char_to_gender( char);
	static time_t str_to_dob( const string&);
};

} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
