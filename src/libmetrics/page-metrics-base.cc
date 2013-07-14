/*
 *       File name:  libmetrics/page-metrics-base.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  Base class for various per-page EEG metrics (PSD, MC)
 *
 *         License:  GPL
 */

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <list>
#include <numeric>
#include <valarray>

#include "libsigfile/source.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

metrics::CProfile::
CProfile (const sigfile::CTypedSource& F, int sig_no,
	  double pagesize, double step, size_t bins)
      : _status (0),
	_bins (bins),
	_signature_when_mirrored (0),
	_using_F (F),
	_using_sig_no (sig_no)
{
	Pp.pagesize = pagesize;
	Pp.step = step;
}

size_t
metrics::CProfile::
samplerate() const
{
	return _using_F().samplerate( _using_sig_no);
}

size_t
metrics::CProfile::
steps() const
{
	return (_using_F().recording_time() - Pp.pagesize) / Pp.step;
}


void
metrics::SPPack::
check() const
{
	for ( auto c : {4., 20., 30., 60.} )
		if ( pagesize == c )
			return;
#ifdef _OPENMP
#pragma omp critical
#endif
	throw invalid_argument (string ("Invalid pagesize: ") + to_string(pagesize));

	if ( step <= pagesize )
		return;
#ifdef _OPENMP
#pragma omp critical
#endif
	throw invalid_argument (string ("step > pagesize: ") + to_string(step) + " > "+ to_string(pagesize));
}

void
metrics::SPPack::
reset()
{
	pagesize = step = 30.;
}


list<agh::alg::SSpan<size_t>>
metrics::CProfile::
artifacts_in_samples() const
{
	size_t sr = _using_F().samplerate(_using_sig_no);
	list<agh::alg::SSpan<size_t>> Q;
	for ( auto& a : _using_F().artifacts( _using_sig_no)() )
		Q.emplace_back( a.a * sr, a.z * sr);
	return Q;
}


list<agh::alg::SSpan<double>>
metrics::CProfile::
artifacts_in_seconds() const
{
	return _using_F().artifacts( _using_sig_no)();
}



int
metrics::CProfile::
compute( const SPPack& req_params)
{
	auto req_signature = _using_F().dirty_signature( _using_sig_no);
	if ( have_data()
	     and req_signature == _signature_when_mirrored
	     and Pp.same_as(req_params) )
		return 0;

	auto old_mirror = mirror_fname();
	Pp.make_same( req_params);
	_signature_when_mirrored = req_signature;
	auto new_mirror = mirror_fname();

	bool got_it = (mirror_back( new_mirror) == 0);

	if ( old_mirror != new_mirror )
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic push
		unlink( old_mirror.c_str());
#pragma GCC diagnostic pop

	if ( got_it )
		return 0;

	// printf( "CProfile::compute( %s, %s): %g sec (%zu pp @%zu + %zu sec last incomplete page); bins/size/freq_max = %zu/%g/%g",
	// 	_using_F.filename(), _using_F.channel_by_id(_using_sig_no),
	// 	_using_F.recording_time(),
	// 	pages(), _pagesize, (size_t)_using_F.recording_time() - (pages() * _pagesize),
	// 	_bins, binsize, freq_max);

	auto retval = go_compute();

	mirror_enable( new_mirror) or true;

	return retval;
}

bool
metrics::CProfile::
need_compute( const SPPack& req_params)
{
	auto req_signature = _using_F().dirty_signature( _using_sig_no);
	if ( have_data()
	     and req_signature == _signature_when_mirrored
	     and Pp.same_as(req_params) )
		return false;

	auto old_mirror = mirror_fname();
	Pp.make_same( req_params);
	_signature_when_mirrored = req_signature;
	auto new_mirror = mirror_fname();

	bool got_it = (mirror_back( new_mirror) == 0);

	return not got_it;
}



int
metrics::CProfile::
mirror_enable( const string& fname)
{
	int fd, retval = 0;
	if ( (fd = open( fname.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1 ||
	     write( fd, &_data[0], _data.size() * sizeof(TFloat)) == -1 )
	     retval = -1;
	close( fd);
	return retval;
}


int
metrics::CProfile::
mirror_back( const string& fname)
{
	int fd = -1;
	try {
		if ( (fd = open( fname.c_str(), O_RDONLY)) == -1 )
			throw -1;
		_data.resize( steps() * _bins);
		if ( read( fd, &_data[0], _data.size() * sizeof(TFloat))
		     != (ssize_t)(_data.size() * sizeof(TFloat)) )
			throw -2;
		close(fd);
		return 0;
	} catch (int ex) {
		if ( fd != -1 ) {
			close( fd);
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic push
			unlink( fname.c_str());
#pragma GCC diagnostic pop
		}
		return ex;
	}
}






int
metrics::CProfile::
export_tsv( const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return -1;

	size_t bin, p;

	auto sttm = _using_F().start_time();
	char *asctime_ = asctime( localtime( &sttm));
	fprintf( f, "## Subject: %s;  Session: %s, Episode: %s recorded %.*s;  Channel: %s\n"
		 "#Page\t",
		 _using_F().subject().name.c_str(), _using_F().session(), _using_F().episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F().channel_by_id(_using_sig_no).name());

	for ( bin = 0; bin < _bins; ++bin )
		fprintf( f, "%zu%c", bin, bin+1 == _bins ? '\n' : '\t');

	for ( p = 0; p < steps(); ++p ) {
		fprintf( f, "%zu", p);
		for ( bin = 0; bin < _bins; ++bin )
			fprintf( f, "\t%g", nmth_bin( p, bin));
		fprintf( f, "\n");
	}

	fclose( f);
	return 0;
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
