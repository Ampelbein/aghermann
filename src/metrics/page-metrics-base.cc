// ;-*-C++-*-
/*
 *       File name:  metrics/page-metrics-base.cc
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

metrics::CProfile_base::
CProfile_base (const sigfile::CSource& F, int sig_no,
		   size_t pagesize, size_t bins)
	: _status (0),
	  _bins (bins),
	  _pagesize (pagesize),
	  _signature (0),
	  _using_F (F),
	  _using_sig_no (sig_no)
{
	_data.resize( pages() * bins);
}

size_t
metrics::CProfile_base::
samplerate() const
{
	return _using_F.samplerate( _using_sig_no);
}

size_t
metrics::CProfile_base::
pages() const
{
	return _using_F.recording_time() / _pagesize;
}



list<agh::alg::SSpan<size_t>>
metrics::CProfile_base::
artifacts_in_samples() const
{
	return _using_F.artifacts( _using_sig_no)();
}


list<agh::alg::SSpan<float>>
metrics::CProfile_base::
artifacts_in_seconds() const
{
	list<agh::alg::SSpan<float>> ret;
	auto af_ = artifacts_in_samples();
	size_t sr = _using_F.samplerate(_using_sig_no);
	for ( auto &A : af_ )
		ret.emplace_back( A.a / (float)sr, A.z / (float)sr);
	return ret;
}




int
metrics::CProfile_base::
_mirror_enable( const char *fname)
{
	int fd, retval = 0;
	if ( (fd = open( fname, O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1 ||
	     write( fd, &_data[0], _data.size() * sizeof(double)) == -1 )
	     retval = -1;
	close( fd);
	return retval;
}


int
metrics::CProfile_base::
_mirror_back( const char *fname)
{
	int fd = -1;
	try {
		if ( (fd = open( fname, O_RDONLY)) == -1 )
			throw -1;
		if ( read( fd, &_data[0], _data.size() * sizeof(double))
		     != (ssize_t)(_data.size() * sizeof(double)) )
			throw -2;
		close(fd);
		return 0;
	} catch (int ex) {
		if ( fd != -1 ) {
			close( fd);
			if ( unlink( fname) ) {}
		}
		return ex;
	}
}






int
metrics::CProfile_base::
export_tsv( const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return -1;

	size_t bin, p;

	auto sttm = _using_F.start_time();
	char *asctime_ = asctime( localtime( &sttm));
	fprintf( f, "## Subject: %s;  Session: %s, Episode: %s recorded %.*s;  Channel: %s\n"
		 "#Page\t",
		 _using_F.subject(), _using_F.session(), _using_F.episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F.channel_by_id(_using_sig_no));

	for ( bin = 0; bin < _bins; ++bin )
		fprintf( f, "%zu%c", bin, bin+1 == _bins ? '\n' : '\t');

	for ( p = 0; p < pages(); ++p ) {
		fprintf( f, "%zu", p);
		for ( bin = 0; bin < _bins; ++bin )
			fprintf( f, "\t%g", nmth_bin( p, bin));
		fprintf( f, "\n");
	}

	fclose( f);
	return 0;
}



// eof
