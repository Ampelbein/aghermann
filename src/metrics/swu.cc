// ;-*-C++-*-
/*
 *       File name:  metrics/swu.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-11-10
 *
 *         Purpose:  CBinnedSWU methods
 *
 *         License:  GPL
 */


#include <cassert>
#include <unistd.h>

#include "common/lang.hh"
#include "common/fs.hh"
#include "sigproc/sigproc.hh"
#include "libsigfile/source.hh"
#include "swu.hh"

using namespace std;



void
metrics::swu::SPPack::
check() const
{
	if ( pagesize != 4  && pagesize != 20 &&
	     pagesize != 30 && pagesize != 60 )
		throw invalid_argument ("Invalid pagesize");

	if ( binsize != .1 && binsize != .25 && binsize != .5 )
		throw invalid_argument ("Invalid binsize");
}

void
metrics::swu::SPPack::
reset()
{
	pagesize = 30;
	binsize = .25;
}







metrics::swu::CProfile::
CProfile (const sigfile::CSource& F, int sig_no,
	  const SPPack &params)
	: CProfile_base (F, sig_no,
			 params.pagesize,
			 params.compute_n_bins(F.samplerate(sig_no))),
	  SPPack (params)
{
}



string
metrics::swu::CProfile::
fname_base() const
{
	DEF_UNIQUE_CHARP (_);
	assert (asprintf( &_,
			  "%s-%s-%zu-%zu",
			  _using_F.filename(), _using_F.channel_by_id(_using_sig_no),
			  SPPack::pagesize, //freq_trunc,
			  _signature) > 1);
	string ret {_};
	return ret;
}





int
metrics::swu::CProfile::
compute( const SPPack& req_params,
	 bool force)
{
      // check if we have it already
	auto req_signature = _using_F.dirty_signature( _using_sig_no);
	if ( have_data()
	     && not force
	     && (*this) == req_params
	     && _signature == req_signature )
		return 0;

	size_t	sr = samplerate();
	size_t	spp = sr * _pagesize;
	TFloat	freq_max = (TFloat)(spp+1)/2 / sr;
	_data.resize( pages() * _bins);
	printf( "CBinnedSWU::compute( %s, %s): %g sec (%zu pp @%zu + %zu sec last incomplete page); bins/size/freq_max = %zu/%g/%g",
		_using_F.filename(), _using_F.channel_by_id(_using_sig_no),
		_using_F.recording_time(),
		pages(), _pagesize, (size_t)_using_F.recording_time() - (pages() * _pagesize),
		_bins, binsize, freq_max);

	DEF_UNIQUE_CHARP (old_mirror_fname);
	DEF_UNIQUE_CHARP (new_mirror_fname);

	// insert a .
	string basename_dot = agh::fs::make_fname_base (_using_F.filename(), "", true);

	assert (asprintf( &old_mirror_fname,
			  "%s-%s-%zu-%g-%zu.psd",
			  basename_dot.c_str(),
			  _using_F.channel_by_id(_using_sig_no), _pagesize, binsize,
			  _signature)
		> 1);

      // update signature
	*(SPPack*)this = req_params;
	_signature = req_signature;
	assert (asprintf( &new_mirror_fname,
			  "%s-%s-%zu-%g-%zu.psd",
			  basename_dot.c_str(),
			  _using_F.channel_by_id(_using_sig_no), _pagesize, binsize,
			  _signature)
		> 1);

	bool got_it = (_mirror_back( new_mirror_fname) == 0);

      // remove previously saved power
	if ( strcmp( old_mirror_fname, new_mirror_fname) )
		if ( unlink( old_mirror_fname) ) {}

	if ( got_it and not force ) {
		printf( " (cached)\n");
		_status |= TFlags::computed;
		return 0;
	}
	printf( "\n");

      // 0. get signal sample; always use double not TFloat
      // so that saved power is usable irrespective of what TFloat is today
	valarray<double> S = to_vad( _using_F.get_signal_filtered( _using_sig_no));

      // 1. dampen samples marked as artifacts
	// already done in get_signal_filtered()

      // 2. zero-mean and detrend
	// zero-mean already done in CEDFFile::get_signal_filtered()

      // 4. obtain power spectrum
	// prepare
	for ( size_t p = 0; p < pages(); ++p ) {
		nmth_bin(p, 0) =
			sin(p * M_PI);
	}

	if ( _mirror_enable( new_mirror_fname) ) {}

	_status |= TFlags::computed;
	return 0;
}









int
metrics::swu::CProfile::
export_tsv( const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return -1;

	size_t bin, p;
	float bum = 0.;

	auto sttm = _using_F.start_time();
	char *asctime_ = asctime( localtime( &sttm));
	fprintf( f, "## Subject: %s;  Session: %s, Episode: %s recorded %.*s;  Channel: %s\n"
		 "## Total spectral power course (%zu %zu-sec pages) up to %g Hz in bins of %g Hz\n"
		 "#Page\t",
		 _using_F.subject(), _using_F.session(), _using_F.episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F.channel_by_id(_using_sig_no),
		 pages(), _pagesize, _bins*binsize, binsize);

	for ( bin = 0; bin < _bins; ++bin, bum += binsize )
		fprintf( f, "%g%c", bum, bin+1 == _bins ? '\n' : '\t');

	for ( p = 0; p < pages(); ++p ) {
		fprintf( f, "%zu", p);
		for ( bin = 0; bin < _bins; ++bin )
			fprintf( f, "\t%g", nmth_bin( p, bin));
		fprintf( f, "\n");
	}

	fclose( f);
	return 0;
}




int
metrics::swu::CProfile::
export_tsv( float from, float upto,
	    const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return -1;

	auto sttm = _using_F.start_time();
	char *asctime_ = asctime( localtime( &sttm));
	fprintf( f, "PSD profile of\n"
		 "## Subject: %s;  Session: %s, Episode: %s recorded %.*s;  Channel: %s\n"
		 "## Course (%zu %zu-sec pages) in range %g-%g Hz\n",
		 _using_F.subject(), _using_F.session(), _using_F.episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F.channel_by_id(_using_sig_no),
		 pages(), _pagesize, from, upto);

	valarray<TFloat> crs = course<TFloat>( from, upto);
	for ( size_t p = 0; p < pages(); ++p )
		fprintf( f, "%zu\t%g\n", p, crs[p]);

	fclose( f);
	return 0;
}



// eof
