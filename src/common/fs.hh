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

#ifndef _AGH_COMMON_FS_H
#define _AGH_COMMON_FS_H

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>
#include <cstring>
#include <cassert>
#include <string>
#include "string.hh"
#include "lang.hh"

using namespace std;

namespace agh {
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
	return agh::str::tokens( _filename, "/");
}

template<class T>
string
dirname( const T& _filename)
{
	auto ee = agh::str::tokens( _filename, "/");
	ee.pop_back();
	return agh::str::join( ee, "/");
}





template<class T>
bool
exists_and_is_writable( const T& _dir)
{
	string dir (_dir);
	struct stat attr;
	return stat( dir.c_str(), &attr) == 0 &&
		S_ISDIR (attr.st_mode) &&
		(attr.st_mode & S_IWUSR) &&
		(attr.st_uid == getuid());
}


template<class T>
int
mkdir_with_parents( const T& _dir)
{
	string dir (_dir);
	DEF_UNIQUE_CHARP(_);
	assert (asprintf( &_, "mkdir -p '%s'", dir.c_str()));
	return system( _);
}






// this is another global
int edf_file_counter( const char *fname, const struct stat*, int flag, struct FTW *ftw);
extern size_t __n_edf_files;




} // namespace fs
} // namespace agh

#endif // _AGH_COMMON_FS_H

// eof
