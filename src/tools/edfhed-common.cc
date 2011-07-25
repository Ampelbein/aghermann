// ;-*-C++-*- *  Time-stamp: "2011-07-24 20:12:35 hmmr"
/*
 *       File name:  tools/edfed-gtk.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-20
 *
 *         Purpose:  EDF header editor utility (using gtk)
 *
 *         License:  GPL
 */


#include <cstring>
#include "edfhed.hh"

string
strtrim( const string& r0)
{
	string r (r0);
	auto rsize = r.size();
	if ( rsize == 0 )
		return r;
	while (r[rsize-1] == ' ')
		--rsize;
	r.resize( rsize);
	return r;
}

string
strpad( const string& r0, size_t to)
{
	string r (to, ' ');
	memcpy( (void*)r.data(), (const void*)r0.data(), r0.size());
	return r;
}


// eof
