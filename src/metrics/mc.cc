// ;-*-C++-*-
/*
 *       File name:  metrics/mc.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  CBinnedMC ("EEG microcontinuity")
 *
 *         License:  GPL
 */

#include "common/lang.hh"
#include "libsigfile/source.hh"
#include "mc.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


string
metrics::mc::CProfile::
fname_base() const
{
	DEF_UNIQUE_CHARP (_);
	assert (asprintf( &_,
			  "%s-%s-%zu-%zu",
			  _using_F.filename(), _using_F.channel_by_id(_using_sig_no),
			  _using_F.pagesize(),
			  _signature) > 1);
	string ret {_};
	return ret;
}


inline int
heaviside( TFloat x)
{
	return x > 0.;
}




void
metrics::mc::SPPack::
check( size_t) const
{
        if ( mc_gain < 1.0 )
		throw invalid_argument ("mc_gain must be >= 1.0");
	// if ( (int)(pagesize/scope) != (double)pagesize / (double)scope )
	// 	throw invalid_argument ("Page size not a multiple of MC scope");
}



void
metrics::mc::SPPack::
reset()
{
	scope			=     30 / 6.;  // 5 sec is close to 4 sec ('recommended')
	f0fc			=      .8;
	bandwidth		=     1.5;
	iir_backpolate		=     0.5;	// 0.0 < Backpolate < 1.0 on s: standard 0.5
	mc_gain			=    10.0;	// Gain (DigiRange/PhysiRange) of MicroContinuity
	smooth_side		=     0;
}






metrics::mc::CProfile::
CProfile (const sigfile::CSource& F, int sig_no,
	  const SPPack &params,
	  size_t pagesize)
      : CProfile_base (F, sig_no,
		       pagesize, // acting 'pagesize' for CPageMetrics_base
		       params.compute_n_bins(F.samplerate(sig_no))),
	SPPack (params)
	// *_filter's initialized at compute time
{
	SPPack::check( pagesize); // throw if not ok
}




int
metrics::mc::CProfile::
compute( const SPPack& req_params,
	 bool force)
{
	auto req_signature = _using_F.dirty_signature( _using_sig_no);
	if ( have_data()
	     && not force
	     && (*this) == req_params
	     && _signature == req_signature )
		return 0;

	_data.resize( pages() * _bins);

	DEF_UNIQUE_CHARP (old_mirror_fname);
	DEF_UNIQUE_CHARP (new_mirror_fname);

	// insert a .
	string basename_dot = agh::fs::make_fname_base (_using_F.filename(), "", true);

	assert (asprintf( &old_mirror_fname,
			  "%s-%s-%zu:"
			  "_%g_%g" "_%g" "_%g_%g"
			  ":%zu.mc",
			  basename_dot.c_str(),
			  _using_F.channel_by_id(_using_sig_no), _pagesize, scope,
			  iir_backpolate,
			  mc_gain,
			  f0fc, bandwidth,
			  _signature)
		> 1);

      // update signature
	*(SPPack*)this = req_params;
	_signature = req_signature;

	printf( "CBinnedMC::compute( %s, %s): %g sec (%zu pp @%zu + %zu sec last incomplete page), scope %g sec",
		_using_F.filename(), _using_F.channel_by_id(_using_sig_no),
		_using_F.recording_time(),
		pages(), _pagesize, (size_t)_using_F.recording_time() - (pages() * _pagesize), scope);


	assert (asprintf( &new_mirror_fname,
			  "%s-%s-%zu:"
			  "_%g_%g" "_%g" "_%g_%g"
			  ":%zu.mc",
			  basename_dot.c_str(),
			  _using_F.channel_by_id(_using_sig_no), _pagesize, scope,
			  iir_backpolate,
			  mc_gain,
			  f0fc, bandwidth,
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

	auto signal = _using_F.get_signal_filtered(_using_sig_no);
	for ( size_t b = 0; b < bins(); ++b ) {
		auto suss = do_sssu_reduction(
			signal, samplerate(), scope,
			mc_gain, iir_backpolate,
			freq_from + b * bandwidth,
			freq_from + b * bandwidth + f0fc,
			bandwidth);
		for ( size_t p = 0; p < pages(); ++p )
			nmth_bin(p, b) = (suss.first[p] - suss.second[p]); // make it positive
	}

	if ( _mirror_enable( new_mirror_fname) ) {}

	_status |= TFlags::computed;

	return 0;
}






metrics::mc::CProfile::TSSSU
metrics::mc::CProfile::
do_sssu_reduction( const valarray<TFloat>& signal,
		   size_t samplerate, double scope,
		   double mc_gain, double iir_backpolate,
		   double f0, double fc,
		   double bandwidth)
{
	sigproc::CFilterDUE
		due_filter (samplerate, sigproc::CFilterIIR::TFilterDirection::forward,
			    mc_gain, iir_backpolate,
			    fc);
	sigproc::CFilterSE
		se_filter (samplerate, sigproc::CFilterIIR::TFilterDirection::forward,
			   mc_gain, iir_backpolate,
			   f0, fc,
			   bandwidth);

	size_t	integrate_samples = scope * samplerate,
		pages = signal.size() / integrate_samples;
	valarray<TFloat>
		due_filtered = due_filter.apply( signal, false),
		se_filtered  =  se_filter.apply( signal, false);

	valarray<TFloat>
		ss (pages),
		su (pages);
	for ( size_t p = 0; p < pages; ++p ) {
		auto range = slice (p * integrate_samples, integrate_samples, 1);
		su[p] =
			(valarray<TFloat> {due_filtered[range]} * valarray<TFloat> {se_filtered[range]})
			.sum()
			/ integrate_samples;
		ss[p] =
			pow(valarray<TFloat> {se_filtered[range]}, (TFloat)2.)
			.sum() / samplerate
			/ integrate_samples;
	}

	return TSSSU {su, ss};
}












int
metrics::mc::CProfile::
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
		 "## Total EEG Microcontinuity course (%zu %zu-sec pages) from %g up to %g Hz in bins of %g Hz\n"
		 "#Page\t",
		 _using_F.subject(), _using_F.session(), _using_F.episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F.channel_by_id(_using_sig_no),
		 pages(), _pagesize, freq_from, freq_from + bandwidth * bins(), bandwidth);

	for ( bin = 0; bin < _bins; ++bin, bum += bandwidth )
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
metrics::mc::CProfile::
export_tsv( size_t bin,
	    const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return -1;

	auto sttm = _using_F.start_time();
	char *asctime_ = asctime( localtime( &sttm));
	fprintf( f, "## Microcontinuity profile of\n"
		 "## Subject: %s;  Session: %s, Episode: %s recorded %.*s;  Channel: %s\n"
		 "## Course (%zu %zu-sec pages) in range %g-%g Hz\n",
		 _using_F.subject(), _using_F.session(), _using_F.episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F.channel_by_id(_using_sig_no),
		 pages(), _pagesize, freq_from, freq_from + (bin+1) * bandwidth);

	for ( size_t p = 0; p < pages(); ++p )
		fprintf( f, "%zu\t%g\n", p, nmth_bin(p, bin));

	fclose( f);
	return 0;
}


// eof
