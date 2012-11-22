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


void
metrics::mc::SPPack::
check() const
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
	  const SPPack &params)
      : metrics::CProfile (F, sig_no,
			   params.pagesize, // acting 'pagesize' for metrics::CProfile
			   params.compute_n_bins(F.samplerate(sig_no))),
	Pp (params)
	// *_filter's initialized at compute time
{
	Pp.check(); // throw if not ok
}




string
metrics::mc::CProfile::
fname_base() const
{
	DEF_UNIQUE_CHARP (_);
	assert (asprintf( &_,
			  "%s.%s-%zu"
			  ":%zu-%g_%g" "_%g" "_%g_%g",
			  _using_F.filename(), _using_F.channel_by_id(_using_sig_no),
			  _using_F.dirty_signature( _using_sig_no),
			  Pp.pagesize,
			  Pp.scope,
			  Pp.iir_backpolate,
			  Pp.mc_gain,
			  Pp.f0fc, Pp.bandwidth)
		> 1);
	string ret {_};
	return ret;
}

string
metrics::mc::CProfile::
mirror_fname() const
{
	DEF_UNIQUE_CHARP (_);
	string basename_dot = agh::fs::make_fname_base (_using_F.filename(), "", true);
	assert (asprintf( &_,
			  "%s-%s-%zu"
			  ":%zu-%g_%g" "_%g" "_%g_%g"
			  ".mc",
			  basename_dot.c_str(), _using_F.channel_by_id(_using_sig_no),
			  _using_F.dirty_signature( _using_sig_no),
			  Pp.pagesize,
			  Pp.scope,
			  Pp.iir_backpolate,
			  Pp.mc_gain,
			  Pp.f0fc, Pp.bandwidth)
		> 1);
	string ret {_};
	return ret;
}

int
metrics::mc::CProfile::
go_compute()
{
	_data.resize( pages() * _bins);

	auto S = _using_F.get_signal_filtered( _using_sig_no);
	for ( size_t b = 0; b < bins(); ++b ) {
		auto suss = do_sssu_reduction(
			S, samplerate(),
			Pp.scope,
			Pp.mc_gain, Pp.iir_backpolate,
			Pp.freq_from + b * Pp.bandwidth,
			Pp.freq_from + b * Pp.bandwidth + Pp.f0fc,
			Pp.bandwidth);
		for ( size_t p = 0; p < pages(); ++p )
			nmth_bin(p, b) = (suss.first[p] - suss.second[p]); // make it positive
	}

	return 0;
}






metrics::mc::CProfile::TSSSU
metrics::mc::CProfile::
do_sssu_reduction( const valarray<TFloat>& S,
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
		pages = S.size() / integrate_samples;
	valarray<TFloat>
		due_filtered = due_filter.apply( S, false),
		se_filtered  =  se_filter.apply( S, false);

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
		 pages(), Pp.pagesize, Pp.freq_from, Pp.freq_from + Pp.bandwidth * bins(), Pp.bandwidth);

	for ( bin = 0; bin < _bins; ++bin, bum += Pp.bandwidth )
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
		 pages(), Pp.pagesize, Pp.freq_from, Pp.freq_from + (bin+1) * Pp.bandwidth);

	for ( size_t p = 0; p < pages(); ++p )
		fprintf( f, "%zu\t%g\n", p, nmth_bin(p, bin));

	fclose( f);
	return 0;
}


// eof
