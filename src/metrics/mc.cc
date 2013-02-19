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
	freq_from		=     0.5;
	freq_inc		=      .5;
	n_bins			=     5;
}






metrics::mc::CProfile::
CProfile (const sigfile::CTypedSource& F, int sig_no,
	  const SPPack &params)
      : metrics::CProfile (F, sig_no,
			   params.pagesize, // acting 'pagesize' for metrics::CProfile
			   params.compute_n_bins(F().samplerate(sig_no))),
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
	ASPRINTF( &_,
		  "%s.%s-%zu"
		  ":%lu-%g_%g" "_%g" "_%g_%g",
		  _using_F().filename(), _using_F().channel_by_id(_using_sig_no),
		  _using_F().dirty_signature( _using_sig_no),
		  Pp.pagesize,
		  Pp.scope,
		  Pp.iir_backpolate,
		  Pp.mc_gain,
		  Pp.f0fc, Pp.bandwidth);
	string ret {_};
	return ret;
}

string
metrics::mc::CProfile::
mirror_fname() const
{
	DEF_UNIQUE_CHARP (_);
	string basename_dot = agh::fs::make_fname_base (_using_F().filename(), "", true);
	ASPRINTF( &_,
		  "%s-%s-%zu"
		  ":%lu-%g_%g" "_%g" "_%g_%g" "_%g_%g@%zu"
		  ".mc",
		  basename_dot.c_str(), _using_F().channel_by_id(_using_sig_no),
		  _using_F().dirty_signature( _using_sig_no),
		  Pp.pagesize,
		  Pp.scope,
		  Pp.iir_backpolate,
		  Pp.mc_gain,
		  Pp.f0fc, Pp.bandwidth,
		  Pp.freq_from, Pp.freq_inc,
		  sizeof(TFloat));
	string ret {_};
	return ret;
}

int
metrics::mc::CProfile::
go_compute()
{
	_data.resize( pages() * _bins);
	auto S = _using_F().get_signal_filtered( _using_sig_no);
	for ( size_t b = 0; b < bins(); ++b ) {
		auto su_ss = metrics::mc::do_sssu_reduction(
			S, samplerate(),
			Pp.scope,
			Pp.mc_gain, Pp.iir_backpolate,
			Pp.freq_from + b * Pp.freq_inc,
			Pp.freq_from + b * Pp.freq_inc + Pp.f0fc,
			Pp.bandwidth);
		auto suss = su_ss.first - su_ss.second;  // make it positive
		// collapse into our pages
		for ( size_t p = 0; p < pages(); ++p ) {
			auto range = slice (p * Pp.scope, Pp.pagesize/Pp.scope, 1);
			nmth_bin(p, b) =
				agh::alg::value_within( suss[range].sum(), (TFloat)0., (TFloat)INFINITY);
		}
	}

	return 0;
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

	auto sttm = _using_F().start_time();
	char *asctime_ = asctime( localtime( &sttm));
	fprintf( f, "## Subject: %s;  Session: %s, Episode: %s recorded %.*s;  Channel: %s\n"
		 "## Total EEG Microcontinuity course (%zu %zu-sec pages) from %g up to %g Hz in bins of %g Hz\n"
		 "#Page\t",
		 _using_F().subject(), _using_F().session(), _using_F().episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F().channel_by_id(_using_sig_no),
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

	auto sttm = _using_F().start_time();
	char *asctime_ = asctime( localtime( &sttm));
	fprintf( f, "## Microcontinuity profile of\n"
		 "## Subject: %s;  Session: %s, Episode: %s recorded %.*s;  Channel: %s\n"
		 "## Course (%zu %zu-sec pages) in range %g-%g Hz\n",
		 _using_F().subject(), _using_F().session(), _using_F().episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F().channel_by_id(_using_sig_no),
		 pages(), Pp.pagesize, Pp.freq_from, Pp.freq_from + (bin+1) * Pp.bandwidth);

	for ( size_t p = 0; p < pages(); ++p )
		fprintf( f, "%zu\t%g\n", p, nmth_bin(p, bin));

	fclose( f);
	return 0;
}



template
pair<valarray<TFloat>, valarray<TFloat>>
metrics::mc::
do_sssu_reduction( const valarray<TFloat>&,
		   size_t, double, double, double,
		   double, double, double);

const size_t sssu_hist_size = 100;


// Local Variables:
// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
// indent-tabs-mode: 8
// End:
