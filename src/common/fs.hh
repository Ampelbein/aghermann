// ;-*-C++-*-
/*
 *       File name:  common/fs.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-02-11
 *
 *         Purpose:  generic path handling utility
 *
 *         License:  GPL
 */

#ifndef _AGH_FS_H
#define _AGH_FS_H

#include <string>
#include "string.hh"

using namespace std;


namespace fs {

template<class T>
string
make_fname_base( const T& _filename, const char *suffix, bool hidden)
{
	string	fname_ (_filename);
	auto	slen = strlen( suffix);
	if ( fname_.size() > slen && strcasecmp( &fname_[fname_.size()-slen], suffix) == 0 )
		fname_.erase( fname_.size()-slen, slen);
	if ( hidden ) {
		size_t slash_at = fname_.rfind('/');
		if ( slash_at < fname_.size() )
			fname_.insert( slash_at+1, ".");
	}
	return fname_;
}

template<class T>
list<string>
path_elements( const T& _filename)
{
	return string_tokens( _filename, "/");
}


} // namespace fs

#endif // _AGH_FS_H

// eof
