/*
 *       File name:  libmetrics/mc.cc
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
#include "libsigfile/typed-source.hh"
#include "mc.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;



metrics::mc::CProfile::
CProfile (const sigfile::CTypedSource& F, const int sig_no,
	  const SPPack &params)
      : metrics::CProfile (F, sig_no,
			   params.pagesize, params.step,
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
	return agh::str::sasprintf(
		  "%s.%s-%lu"
		  ":%g+%g-%g_%g" "_%g" "_%g_%g",
		  _using_F().filename(), _using_F().channel_by_id(_using_sig_no).name(),
		  _using_F().dirty_signature( _using_sig_no),
		  Pp.pagesize, Pp.step,
		  Pp.scope, Pp.iir_backpolate,
		  Pp.mc_gain,
		  Pp.f0fc, Pp.bandwidth);
}

string
metrics::mc::CProfile::
mirror_fname() const
{
	return agh::str::sasprintf(
		  "%s-%s-%lu"
		  ":%g+%g-%g_%g" "_%g" "_%g_%g" "_%g_%g@%zu"
		  ".mc",
		  agh::fs::make_fname_base (_using_F().filename(), "", agh::fs::TMakeFnameOption::hidden).c_str(),
		  _using_F().channel_by_id(_using_sig_no).name(),
		  _using_F().dirty_signature( _using_sig_no),
		  Pp.pagesize, Pp.step,
		  Pp.scope, Pp.iir_backpolate,
		  Pp.mc_gain,
		  Pp.f0fc, Pp.bandwidth,
		  Pp.freq_from, Pp.freq_inc,
		  sizeof(TFloat));
}

int
metrics::mc::CProfile::
go_compute()
{
	_data.resize( steps() * _bins);
	auto S = _using_F().get_signal_filtered( _using_sig_no);
	for ( size_t b = 0; b < bins(); ++b ) {
		auto su_ss = metrics::mc::do_sssu_reduction(
			S, samplerate(),
			Pp.scope, Pp.step,
			Pp.mc_gain, Pp.iir_backpolate,
			Pp.freq_from + b * Pp.freq_inc,
			Pp.freq_from + b * Pp.freq_inc + Pp.f0fc,
			Pp.bandwidth);
		auto suss = su_ss.first - su_ss.second;  // make it positive

		for ( size_t p = 0; p < steps(); ++p )
			nmth_bin(p, b) =
				agh::alg::value_within( suss[p], (TFloat)0., (TFloat)INFINITY);
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
		 "## Total EEG Microcontinuity course (%zu %g-sec pages, step %g sec) from %g up to %g Hz in bins of %g Hz\n"
		 "#Page\t",
		 _using_F().subject().name.c_str(), _using_F().session(), _using_F().episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F().channel_by_id(_using_sig_no).name(),
		 steps(), Pp.pagesize, Pp.step, Pp.freq_from, Pp.freq_from + Pp.bandwidth * bins(), Pp.bandwidth);

	for ( bin = 0; bin < _bins; ++bin, bum += Pp.bandwidth )
		fprintf( f, "%g%c", bum, bin+1 == _bins ? '\n' : '\t');

	for ( p = 0; p < steps(); ++p ) {
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
		 "## Course (%zu %g-sec pages, step %g sec) in range %g-%g Hz\n",
		 _using_F().subject().name.c_str(), _using_F().session(), _using_F().episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F().channel_by_id(_using_sig_no).name(),
		 steps(), Pp.pagesize, Pp.step, Pp.freq_from, Pp.freq_from + (bin+1) * Pp.bandwidth);

	for ( size_t p = 0; p < steps(); ++p )
		fprintf( f, "%zu\t%g\n", p, nmth_bin(p, bin));

	fclose( f);
	return 0;
}



template
pair<valarray<TFloat>, valarray<TFloat>>
metrics::mc::
do_sssu_reduction( const valarray<TFloat>&,
		   size_t, double, double, double, double,
		   double, double, double);

const size_t sssu_hist_size = 100;


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
