// ;-*-C++-*-
/*
 *       File name:  patterns/patterns.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-09
 *
 *         Purpose:  CPatternTool explicit pattern instantiations be
 *                   here, also loading patterns
 *
 *         License:  GPL
 */

#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include "common/fs.hh"
#include "patterns.hh"

using namespace std;

template pattern::CPatternTool<TFloat>::CPatternTool( const sigproc::SSignalRef<TFloat>&, size_t, size_t, const SPatternPPack<TFloat>&);
template int pattern::CPatternTool<TFloat>::do_search( const valarray<TFloat>&, const valarray<TFloat>&, const valarray<TFloat>&, const valarray<TFloat>&, size_t);
template int pattern::CPatternTool<TFloat>::do_search( const sigproc::SSignalRef<TFloat>&, size_t);
template int pattern::CPatternTool<TFloat>::do_search( const valarray<TFloat>&, size_t);



inline namespace {
int
scandir_filter( const struct dirent *e)
{
	return strcmp( e->d_name, ".") && strcmp( e->d_name, "..");
}
}


namespace pattern {


template <>
SPattern<TFloat>
load_pattern( const char* fname) throw(invalid_argument)
{
	SPattern<TFloat>
		P;

	DEF_UNIQUE_CHARP (buf);
	FILE *fd = fopen( fname, "r");
	if ( fd ) {
		size_t	full_sample;
		if ( fscanf( fd,
			     (sizeof(TFloat) == sizeof(float))
			     ? "%lg  %u %lg %lg  %lg %lg %u"
			     " %g %g %g %g"
			     " %zu %zu %zu %zu\n"
			     "--DATA--\n"
			     :
			     "%lg  %u %lg %lg  %lg %lg %u"
			     " %lg %lg %lg %lg"
			     " %zu %zu %zu %zu\n"
			     "--DATA--\n"
			     ,
			     &P.Pp.env_scope,
			     &P.Pp.bwf_order, &P.Pp.bwf_ffrom, &P.Pp.bwf_fupto,
			     &P.Pp.dzcdf_step, &P.Pp.dzcdf_sigma, &P.Pp.dzcdf_smooth,
			     &get<0>(P.criteria), &get<1>(P.criteria), &get<2>(P.criteria), &get<3>(P.criteria),
			     &P.samplerate, &P.context_before, &P.context_after,
			     &full_sample) == 14 ) {

			P.thing.resize( full_sample);
			for ( size_t i = 0; i < full_sample; ++i ) {
				double d;
				if ( fscanf( fd, "%la", &d) != 1 ) {
					ASPRINTF( &buf, "load_pattern(\"%s\"): short read at sample %zu; "
						  "removing file", fname, i);
					fprintf( stderr, "%s\n", buf);
					P.thing.resize( 0);
					fclose( fd);
					unlink( fname);
					throw invalid_argument (buf);
				} else
					P.thing[i] = d;
			}

		} else {
			P.thing.resize( 0);
			ASPRINTF( &buf, "load_pattern(\"%s\"): bad header, so removing file", fname);
			fprintf( stderr, "%s\n", buf);
			P.thing.resize( 0);
			fclose( fd);
			unlink( fname);
			throw invalid_argument (buf);
		}

		fclose( fd);

	} else {
		ASPRINTF( &buf, "Failed to open pattern %s", fname);
		fprintf( stderr, "%s\n", buf);
		throw invalid_argument (buf);
	}

	P.saved = true;
	P.path = fname;
	return P;
}


template <>
int
save_pattern( SPattern<TFloat>& P, const char* fname)
{
	if ( agh::fs::mkdir_with_parents( agh::fs::dirname(fname)) ) {
		fprintf( stderr, "save_pattern(\"%s\"): mkdir %s failed\n", fname, agh::fs::dirname(fname).c_str());
		return -1;
	}

	FILE *fd = fopen( fname, "w");
	try {
		if ( !fd )
			throw -2;

		if ( fprintf( fd,
			      "%g  %u %g %g  %g %g %u  %g %g %g %g\n"
			      "%zu  %zu %zu %zu\n"
			      "--DATA--\n",
			      P.Pp.env_scope,
			      P.Pp.bwf_order, P.Pp.bwf_ffrom, P.Pp.bwf_fupto,
			      P.Pp.dzcdf_step, P.Pp.dzcdf_sigma, P.Pp.dzcdf_smooth,
			      get<0>(P.criteria), get<1>(P.criteria), get<2>(P.criteria), get<3>(P.criteria),
			      P.samplerate, P.context_before, P.context_after,
			      P.thing.size()) < 1 ) {
			fprintf( stderr, "save_pattern(\"%s\"): write failed\n", fname);
			throw -3;
		}

		for ( size_t i = 0; i < P.thing.size(); ++i )
			if ( fprintf( fd, "%a\n", (double)P.thing[i]) < 1 ) {
				fprintf( stderr, "save_pattern(\"%s\"): write failed\n", fname);
				throw -3;
			}
		fclose( fd);

		return 0;

	} catch (int ret) {
		if ( fd )
			fclose( fd);
		return ret;
	}
}


template <>
int
delete_pattern( const SPattern<TFloat>& P)
{
	return unlink( P.path.c_str());
}


template <>
list<pattern::SPattern<TFloat>>
load_patterns_from_location<TFloat>( const string& loc, pattern::TOrigin origin)
{
	list<SPattern<TFloat>>
		ret;

	struct dirent **eps;
	int	total = scandir( loc.c_str(), &eps, scandir_filter, alphasort);

	if ( total != -1 ) {
		for ( int i = 0; i < total; ++i ) {
			try {
				ret.push_back(
					load_pattern<TFloat>( eps[i]->d_name));
				ret.back().origin = origin;
			} catch (invalid_argument& ex) {
				;
			}
			free( eps[i]);
		}
		free( (void*)eps);
	}

	return ret;
}


} // namespace pattern

// eof
