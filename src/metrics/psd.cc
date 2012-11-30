// ;-*-C++-*-
/*
 *       File name:  metrics/psd.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *                   Parts from PhysioToolKit (http://www.physionet.org/physiotools,
 *                   by George B. Moody (george@mit.edu))
 *
 * Initial version:  2010-04-28
 *
 *         Purpose:  CBinnedPower methods
 *
 *         License:  GPL
 */


#include <cassert>
#include <unistd.h>

#if defined(_OPENMP)
#include <omp.h>
#endif

#include <fftw3.h>

#include "common/lang.hh"
#include "common/fs.hh"
#include "sigproc/sigproc.hh"
#include "libsigfile/source.hh"
#include "psd.hh"

using namespace std;




void
metrics::psd::SPPack::
check() const
{
	metrics::SPPack::check();

	if ( welch_window_type > sigproc::TWinType::_total )
		throw invalid_argument ("Invalid window type");

	for ( auto c : {.1, .25, .5} )
		if ( binsize == c )
			return;
	throw invalid_argument ("Invalid binsize");
}

void
metrics::psd::SPPack::
reset()
{
	metrics::SPPack::reset();
	binsize = .25;
	welch_window_type = sigproc::TWinType::welch;
}







metrics::psd::CProfile::
CProfile (const sigfile::CSource& F, int sig_no,
	  const SPPack &fft_params)
	: metrics::CProfile (F, sig_no,
			     fft_params.pagesize,
			     fft_params.compute_n_bins(F.samplerate(sig_no))),
	  Pp (fft_params)
{
	Pp.check();
}



string
metrics::psd::CProfile::
fname_base() const
{
	DEF_UNIQUE_CHARP (_);
	ASPRINTF( &_,
		  "%s.%s-%zu"
		  ":%zu-%g-%c",
		  _using_F.filename(), _using_F.channel_by_id(_using_sig_no),
		  _using_F.dirty_signature( _using_sig_no),
		  Pp.pagesize, Pp.binsize,
		  'a'+(char)Pp.welch_window_type);
	string ret {_};
	return ret;
}



string
metrics::psd::CProfile::
mirror_fname() const
{
	DEF_UNIQUE_CHARP (_);
	string basename_dot = agh::fs::make_fname_base (_using_F.filename(), "", true);
	ASPRINTF( &_,
		  "%s.%s-%zu"
		  ":%zu-%g-%c@%zu"
		  ".psd",
		  basename_dot.c_str(), _using_F.channel_by_id(_using_sig_no),
		  _using_F.dirty_signature( _using_sig_no),
		  Pp.pagesize, Pp.binsize,
		  'a'+(char)Pp.welch_window_type,
		  sizeof(double));
	string ret {_};
	return ret;
}




int
metrics::psd::CProfile::
go_compute()
{
	_data.resize( pages() * _bins);

	size_t	sr = samplerate();
	size_t	spp = sr * Pp.pagesize;
//	double	freq_max = (spp+1)/2 / sr;

      // 0. get signal sample; always use double not TFloat
      // so that saved power is usable irrespective of what TFloat is today
	valarray<double> S = agh::alg::to_vad( _using_F.get_signal_filtered( _using_sig_no));

      // 1. dampen samples marked as artifacts
	// already done in get_signal_filtered()

      // 2. zero-mean and detrend
	// zero-mean already done in CEDFFile::get_signal_filtered()

      // 4. obtain power spectrum
	// prepare

	double
		*fft_Ti = (double*)fftw_malloc( sizeof(double) * spp * 2),
		*fft_To = (double*)fftw_malloc( sizeof(double) * spp * 2);
	valarray<double>	// buffer for PSD
		P (spp+2);

	static fftw_plan fft_plan = NULL;
	static size_t saved_spp = 0;
#ifdef _OPENMP
#pragma omp single
#endif
	{
//		if ( fft_plan == nullptr ) {
//#if defined(HAVE_LIBFFTW3_OMP) && defined(_OPENMP)
//			int n_procs = omp_get_max_threads();
//			fftw_init_threads();
//			fftw_plan_with_nthreads( n_procs);
//			fftw_plan();
//			printf( "Will use %d core(s)\n", n_procs);
//#endif
//		}
		if ( fft_plan == nullptr || spp != saved_spp ) {

			printf( "Preparing fftw plan for %zu samples...", spp);
			saved_spp = spp;

			memcpy( fft_Ti, &S[0], spp * sizeof(double));  // not necessary?
			fft_plan = fftw_plan_dft_r2c_1d( spp, fft_Ti, (fftw_complex*)fft_To, 0 /* FFTW_PATIENT */);
			printf( "done\n");
		}
	}

	// go
	TFloat	f = 0.;
	size_t	p, b, k = 1,
		window = sr;
	// what about some smooth edges?
	valarray<double>
		W (spp);
	{
		size_t	t9 = spp - window,   // start of the last window but one
			t;
		auto wfun = sigproc::winf[Pp.welch_window_type];
		for ( t = 0; t < window/2; ++t )
			W[t] = wfun( t, window);
		for ( t = window/2; t < window; ++t )
			W[t9 + t] = wfun( t, window);
		// AND, connect mid-first to mid-last windows (at lowest value of the window)
		W[ slice(window/2, spp-window, 1) ] = wfun( window/2, window);
	}

	for ( p = 0; p < pages(); ++p ) {
		memcpy( fft_Ti, &S[p*spp], spp * sizeof(double));
		for ( size_t s = 0; s < spp; ++s )
			fft_Ti[s] *= W[s];

		fftw_execute_dft_r2c( fft_plan, fft_Ti, (fftw_complex*)fft_To);

	      // thanks http://www.fftw.org/fftw2_doc/fftw_2.html
		P[0] = fft_To[0] * fft_To[0];		/* DC component */
		for ( k = 1; k < (spp+1)/2; ++k )		/* (k < N/2 rounded up) */
			P[k] =    fft_To[    k] * fft_To[    k]
				+ fft_To[spp-k] * fft_To[spp-k];
		if ( likely (spp % 2 == 0) )			/* N is even */
			P[spp/2] = fft_To[spp/2] * fft_To[spp/2];	/* Nyquist freq. */

	      // 5. collect power into bins
		// the frequency resolution in P is (1/samplerate) Hz, right?
		////memcpy( &_data[p*bins], &P[ThId][0], bins * sizeof(TFloat));
		///printf( "n_bins = %zu, max_freq = %g\n", n_bins(), max_freq);
		for ( f = 0., b = 0; b < _bins; (f += Pp.binsize), ++b ) {
			//printf( "b = %zu, f = %g\n", b, f);
			nmth_bin(p, b) = (TFloat) // brilliant!
				valarray<double>
				(P[ slice( f*sr, (f + Pp.binsize)*sr, 1) ]) . sum();
		}
		/// / (bin_size * sr) // don't; power is cumulative
	}

	fftw_free( fft_Ti);
	fftw_free( fft_To);

	return 0;
}









int
metrics::psd::CProfile::
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
		 pages(), Pp.pagesize, _bins*Pp.binsize, Pp.binsize);

	for ( bin = 0; bin < _bins; ++bin, bum += Pp.binsize )
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
metrics::psd::CProfile::
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
		 pages(), Pp.pagesize, from, upto);

	valarray<TFloat> crs = course( from, upto);
	for ( size_t p = 0; p < pages(); ++p )
		fprintf( f, "%zu\t%g\n", p, crs[p]);

	fclose( f);
	return 0;
}



// eof
