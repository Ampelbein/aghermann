/*
 *       File name:  common/libcommon.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-23
 *
 *         Purpose:  sundry bits too big for inlining
 *
 *         License:  GPL
 */


#include <cstdio>
#include <cstring>
#include <string>
#include <list>

#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <wchar.h>
#include <iconv.h>

#include "string.hh"
#include "alg.hh"
#include "fs.hh"


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;



string
agh::str::
trim( const string& r0)
{
	string r (r0);
	auto rsize = r.size();
	if ( rsize == 0 )
		return r;
	while ( rsize > 0 && r[rsize-1] == ' ' )
		--rsize;
	r.resize( rsize);
	r.erase( 0, r.find_first_not_of(" \t"));
	return move(r);
}

string
agh::str::
pad( const string& r0, size_t to)
{
	string r (to, ' ');
	memcpy( (void*)r.data(), (const void*)r0.data(), min( to, r0.size()));
	return move(r);
}




string
agh::str::
sasprintf( const char* fmt, ...)
{
	char *_;
	va_list ap;
	va_start (ap, fmt);
	if (vasprintf( &_, fmt, ap) <= 0)
		abort();
	va_end (ap);

	string ret {_};
	free( (void*)_);
	return move(ret);
}



list<string>
agh::str::
tokens_trimmed( const string& s_, const char* sep)
{
	string s {s_};
	list<string> acc;
	char   *pp,
	       *p = strtok_r( &s[0], sep, &pp);
	while ( p ) {
		acc.emplace_back( trim(p));
		p = strtok_r( NULL, sep, &pp);
	}
	return move(acc);
}

list<string>
agh::str::
tokens( const string& s_, const char* sep)
{
	string s {s_};
	list<string> acc;
	char   *pp,
	       *p = strtok_r( &s[0], sep, &pp);
	while ( p ) {
		acc.emplace_back( p);
		p = strtok_r( NULL, sep, &pp);
	}
	return move(acc);
}




void
agh::str::
decompose_double( double value, double *mantissa, int *exponent)
{
	char buf[32];
	snprintf( buf, 31, "%e", value);
	*strchr( buf, 'e') = '|';
	sscanf( buf, "%lf|%d", mantissa, exponent);
}




string&
agh::str::
homedir2tilda( string& inplace)
{
	const char *home = getenv("HOME");
	if ( home )
		if ( inplace.compare( 0, strlen(home), home) == 0 )
			inplace.replace( 0, strlen(home), "~");
	return inplace;
}

string
agh::str::
homedir2tilda( const string& v)
{
	string inplace (v);
	const char *home = getenv("HOME");
	if ( home )
		if ( inplace.compare( 0, strlen(home), home) == 0 )
			inplace.replace( 0, strlen(home), "~");
	return inplace;
}

string&
agh::str::
tilda2homedir( string& inplace)
{
	const char *home = getenv("HOME");
	if ( home ) {
		size_t at;
		while ( (at = inplace.find( '~')) < inplace.size() )
			inplace.replace( at, 1, home);
	}
	return inplace;
}

string
agh::str::
tilda2homedir( const string& v)
{
	string inplace (v);
	const char *home = getenv("HOME");
	if ( home ) {
		size_t at;
		while ( (at = inplace.find( '~')) < inplace.size() )
			inplace.replace( at, 1, home);
	}
	return inplace;
}



string
agh::str::
dhms( double seconds, int dd)
{
	bool	positive = seconds >= 0.;
	if ( not positive )
		seconds = -seconds;

	int	s = (int)seconds % 60,
		m = (int)seconds/60 % 60,
		h = (int)seconds/60/60 % (60*60),
		d = (int)seconds/60/60/24 % (60*60*24);
	double	f = seconds - floor(seconds);

	using agh::str::sasprintf;
	string	f_ = ( dd == 0 )
		? ""
		: sasprintf( ".%0*d", dd, (int)(f*pow(10, dd)));
	return ( d > 0 )
		? sasprintf( "%dd %dh %dm %d%ss", d, h, m, s, f_.c_str())
		: ( h > 0 )
		? sasprintf( "%dh %dm %d%ss", h, m, s, f_.c_str())
		: ( m > 0 )
		? sasprintf( "%dm %d%ss", m, s, f_.c_str())
		: sasprintf( "%d%ss", s, f_.c_str());
}

string
agh::str::
dhms_colon( double seconds, int dd)
{
	bool	positive = seconds >= 0.;
	if ( not positive )
		seconds = -seconds;

	int	s = (int)seconds % 60,
		m = (int)seconds/60 % 60,
		h = (int)seconds/60/60 % (60*60),
		d = (int)seconds/60/60/24 % (60*60*24);
	double	f = seconds - floor(seconds);

	using agh::str::sasprintf;
	string	f_ = ( dd == 0 )
		? ""
		: sasprintf( ".%0*d", dd, (int)(f*pow(10, dd)));

	return sasprintf( "%dd %02d:%02d:%02d%ss", d, h, m, s, f_.c_str());
}




// uncomment on demand
// wstring
// agh::str::
// to_wstring( const string& in, const char* charset)
// {
//         wstring out;

//         size_t sufficient = ((in.size() + 1) * sizeof(wchar_t));

//         iconv_t cd = iconv_open( "WCHAR_T", charset);
//         if ( cd == (iconv_t) -1 )
//                 return out;

//         char    *inptr  = const_cast<char*> (&in[0]), // iconv doesn't touch input, or does it?
//                 *wrptr  = (char*)malloc( sufficient),
//                 *wrptr0 = wrptr;

//         size_t  insize = in.size(),
//                 avail  = sufficient;
//         size_t  nconv  = iconv( cd, &inptr, &insize, &wrptr, &avail);
//         if ( nconv != (size_t) -1 ) {
//                 if ( avail >= sizeof(wchar_t) ) {
//                         *((wchar_t*) wrptr) = L'\0';
//                         out.assign( (wchar_t*)wrptr0);
//                 }
//         }

//         free( (void*)wrptr0);
//         if ( iconv_close( cd) != 0 )
//                 perror ("iconv_close");

//         return out;
// }


// string
// agh::str::
// from_wstring( const wstring& in, const char* charset)
// {
//         string out;

//         size_t sufficient = (in.size() * 4 + 1);

//         iconv_t cd = iconv_open( charset, "WCHAR_T");
//         if ( cd == (iconv_t) -1 )
//                 return out;

//         char    *inptr = (char*) const_cast<wchar_t*> (&in[0]), // yes we can!
//                 *wrptr = (char*)malloc( sufficient),
//                 *wrptr0 = wrptr;

//         size_t  insize = in.size() * sizeof(wchar_t),
//                 avail  = sufficient;
//         size_t  nconv  = iconv( cd, &inptr, &insize, &wrptr, &avail);
//         if ( nconv != (size_t) -1 ) {
//                 if ( avail > 0 ) {
//                         *wrptr = '\0';
//                         out.assign( wrptr0);
//                 }
//         }

//         free( wrptr0);
//         iconv_close( cd);

//         return out;
// }




// fs

string
agh::fs::
make_fname_base( const string& fname_, const string& suffices, const TMakeFnameOption option)
{
	string	fname (fname_);
	for ( const auto& X : agh::str::tokens( suffices, ",; ") )
		if ( fname.size() > X.size() &&
		     strcasecmp( &fname[fname.size()-X.size()], X.c_str()) == 0 ) {
			fname.erase( fname.size()-X.size(), X.size());
			break;
		}

	if ( option == TMakeFnameOption::hidden ) {
		size_t slash_at = fname.rfind('/');
		if ( slash_at < fname.size() )
			fname.insert( slash_at+1, ".");
	}
	return move(fname);
}



// found to be of use elsewhere
size_t	agh::fs::total_supported_sigfiles;

int
agh::fs::
supported_sigfile_counter( const char *fname, const struct stat*, int flag, struct FTW *ftw)
{
	if ( flag == FTW_F && ftw->level == 4 ) {
		int fnlen = strlen(fname); // - ftw->base;
		if ( fnlen < 5 )
			return 0;
		if ( strcasecmp( &fname[fnlen-4], ".edf") == 0 ||
		     strcasecmp( &fname[fnlen-4], ".tsv") == 0 )
			++total_supported_sigfiles;
	}
	return 0;
}






double
__attribute__ ((pure))
agh::alg::
sensible_scale_reduction_factor( double display_scale,
				 double constraint_max, double constraint_min)
{
	double f = 1.;
	bool	last_was_two = false;
	while ( display_scale * f > constraint_max ) {
		f /= last_was_two ? 5. : 2.;
		last_was_two = !last_was_two;
	}
	while ( display_scale * f < constraint_min ) {
		f *= last_was_two ? 5. : 2.;
		last_was_two = !last_was_two;
	}
	return f;
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
