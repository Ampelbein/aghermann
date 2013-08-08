/*
 *       File name:  common/fs.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-02-11
 *
 *         Purpose:  generic utilities for handling paths and files
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

enum class TMakeFnameOption { normal, hidden };
string
make_fname_base( const string& fname_, const string& suffices, TMakeFnameOption);

inline list<string>
path_elements( const string& _filename)
{
	return agh::str::tokens( _filename, "/");
}

inline string
dirname( const string& _filename)
{
	string pre = (_filename[0] == '/') ? "/" : "";
	auto ee = agh::str::tokens( _filename, "/");
	ee.pop_back();
	return pre + agh::str::join( ee, "/");
}





inline bool
exists_and_is_writable( const string& dir)
{
	struct stat attr;
	return stat( dir.c_str(), &attr) == 0 &&
		S_ISDIR (attr.st_mode) &&
		(attr.st_mode & S_IWUSR) &&
		(attr.st_uid == getuid());
}


inline int
mkdir_with_parents( const string& dir)
{
	return system(
		agh::str::sasprintf(
			"mkdir -p '%s'",
			dir.c_str())
		.c_str());
}






// this is another global
int supported_sigfile_counter( const char *fname, const struct stat*, int flag, struct FTW *ftw);
extern size_t total_supported_sigfiles;




} // namespace fs
} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
