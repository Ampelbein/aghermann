/*
 *       File name:  aghermann/patterns/patterns.cc
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



namespace {
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

	FILE *fd = fopen( fname, "r");
	if ( fd ) {
		size_t	full_sample;
		double t1, t2, t3, t4;
		if ( fscanf( fd,
			     "%lg  %u %lg %lg  %lg %lg %u"
			     " %lg %lg %lg %lg"
			     " %zu %zu %zu %zu\n"
			     "--DATA--\n",
			     &P.Pp.env_scope,
			     &P.Pp.bwf_order, &P.Pp.bwf_ffrom, &P.Pp.bwf_fupto,
			     &P.Pp.dzcdf_step, &P.Pp.dzcdf_sigma, &P.Pp.dzcdf_smooth,
			     &t1, &t2, &t3, &t4,
			     &P.samplerate, &P.context_before, &P.context_after,
			     &full_sample) == 15 ) {

			get<0>(P.criteria) = t1;
			get<1>(P.criteria) = t2;
			get<2>(P.criteria) = t3;
			get<3>(P.criteria) = t4;

			if ( P.samplerate == 0 || P.samplerate > 4096 ||
			     full_sample == 0 || full_sample > P.samplerate * 10 ||
			     P.context_before > P.samplerate * 2 ||
			     P.context_after > P.samplerate * 2 ||
			     not P.Pp.sane() ) {
				string msg = agh::str::sasprintf(
					"load_pattern(\"%s\"): bogus data in header; removing file",
					fname);
				fprintf( stderr, "%s\n", msg.c_str());
				P.thing.resize( 0);
				fclose( fd);
				unlink( fname);
				throw invalid_argument (msg);
			}

			P.thing.resize( full_sample);
			for ( size_t i = 0; i < full_sample; ++i ) {
				double d;
				if ( fscanf( fd, "%la", &d) != 1 ) {
					string msg = agh::str::sasprintf(
						"load_pattern(\"%s\"): short read at sample %zu; removing file",
						fname, i);
					fprintf( stderr, "%s\n", msg.c_str());
					P.thing.resize( 0);
					fclose( fd);
					unlink( fname);
					throw invalid_argument (msg);
				} else
					P.thing[i] = d;
			}

		} else {
			P.thing.resize( 0);
			string msg = agh::str::sasprintf( "load_pattern(\"%s\"): bad header, so removing file", fname);
			fprintf( stderr, "%s\n", msg.c_str());
			P.thing.resize( 0);
			fclose( fd);
			unlink( fname);
			throw invalid_argument (msg);
		}

		fclose( fd);

	} else {
		string msg = agh::str::sasprintf( "Failed to open pattern %s", fname);
		fprintf( stderr, "%s\n", msg.c_str());
		throw invalid_argument (msg);
	}

	printf( "loaded pattern in %s\n", fname);
	P.saved = true;
	P.name = agh::str::tokens( fname, "/").back();
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
	printf( "saving pattern in %s\n", fname);

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
					load_pattern<TFloat>( (loc + '/' + eps[i]->d_name).c_str()));
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

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
