// ;-*-C++-*-
/*
 *       File name:  libsigfile/psd.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *                   Parts from PhysioToolKit (http://www.physionet.org/physiotools,
 *                   by George B. Moody (george@mit.edu))
 * Initial version:  2010-04-28
 *
 *         Purpose:  CBinnedPower methods
 *
 *         License:  GPL
 */


#include <cassert>

#if defined(_OPENMP)
#include <omp.h>
#endif

#include <fftw3.h>

#include "../common/misc.hh"
#include "../common/fs.hh"
#include "psd.hh"
#include "source.hh"

using namespace std;




// true if all ok;
// false, and assign defaults, if values are totally looney;
// still false, and clamp bin_size to satisfy constraints if necessary
bool
sigfile::SFFTParamSet::validate()
{
	if ( pagesize < 0 || pagesize > 120 ||
	     //freq_trunc < 4. || freq_trunc > 80. ||
	     welch_window_type > TWinType::_total )
		return false;
	return true;
}






// must match those defined in glade
const array<const char*, 8>
	sigfile::SFFTParamSet::welch_window_type_names = {{
	"Bartlett", "Blackman", "Blackman-Harris",
	"Hamming",  "Hanning",  "Parzen",
	"Square",   "Welch"
}};





// The following window functions have been taken from fft.c, part of WFDB package

#define TWOPI (M_PI*2)

/* See Oppenheim & Schafer, Digital Signal Processing, p. 241 (1st ed.) */
TFloat
__attribute__ ((const))
win_bartlett( size_t j, size_t n)
{
	TFloat a = 2.0/(n-1), w;
	if ( (w = j*a) > 1. )
		w = 2. - w;
	return w;
}

/* See Oppenheim & Schafer, Digital Signal Processing, p. 242 (1st ed.) */
TFloat
__attribute__ ((const))
win_blackman( size_t j, size_t n)
{
	TFloat a = TWOPI/(n-1), w;
	w = 0.42 - .5 * cos(a * j) + .08 * cos(2 * a * j);
	return w;
}

/* See Harris, F.J., "On the use of windows for harmonic analysis with the
   discrete Fourier transform", Proc. IEEE, Jan. 1978 */
TFloat
__attribute__ ((const))
win_blackman_harris( size_t j, size_t n)
{
	TFloat a = TWOPI/(n-1), w;
	w = 0.35875 - 0.48829 * cos(a * j) + 0.14128 * cos(2 * a * j) - 0.01168 * cos(3 * a * j);
	return w;
}

/* See Oppenheim & Schafer, Digital Signal Processing, p. 242 (1st ed.) */
TFloat
__attribute__ ((const))
win_hamming( size_t j, size_t n)
{
	TFloat a = TWOPI/(n-1), w;
	w = 0.54 - 0.46*cos(a*j);
	return w;
}

/* See Oppenheim & Schafer, Digital Signal Processing, p. 242 (1st ed.)
   The second edition of Numerical Recipes calls this the "Hann" window. */
TFloat
__attribute__ ((const))
win_hanning( size_t j, size_t n)
{
	TFloat a = TWOPI/(n-1), w;
	w = 0.5 - 0.5*cos(a*j);
	return w;
}

/* See Press, Flannery, Teukolsky, & Vetterling, Numerical Recipes in C,
   p. 442 (1st ed.) */
TFloat
__attribute__ ((const))
win_parzen( size_t j, size_t n)
{
	TFloat a = (n-1)/2.0, w;
	if ( (w = (j-a)/(a+1)) > 0.0 )
		w = 1 - w;
	else
		w = 1 + w;
	return w;
}

/* See any of the above references. */
TFloat
__attribute__ ((const))
win_square( size_t j, size_t n)
{
	return 1.0;
}

/* See Press, Flannery, Teukolsky, & Vetterling, Numerical Recipes in C,
   p. 442 (1st ed.) or p. 554 (2nd ed.) */
TFloat
__attribute__ ((const))
win_welch( size_t j, size_t n)
{
	TFloat a = (n-1)/2.0, w;
	w = (j-a)/(a+1);
	w = 1 - w*w;
	return w;
}



TFloat (*sigfile::winf[])(size_t, size_t) = {
	win_bartlett,
	win_blackman,
	win_blackman_harris,
	win_hamming,
	win_hanning,
	win_parzen,
	win_square,
	win_welch
};



sigfile::CBinnedPower::CBinnedPower( const CSource& F, int sig_no,
				     const SFFTParamSet &fft_params)
	: CPageMetrics_base (F, sig_no,
			     fft_params.pagesize,
			     fft_params.compute_n_bins(F.samplerate(sig_no))),
	  SFFTParamSet (fft_params)
{
}



string
sigfile::CBinnedPower::fname_base() const
{
	DEF_UNIQUE_CHARP (_);
	assert (asprintf( &_,
			  "%s-%s-%zu-%c%c-%zu",
			  _using_F.filename(), _using_F.channel_by_id(_using_sig_no),
			  SFFTParamSet::pagesize, //freq_trunc,
			  'a'+(char)welch_window_type,
			  'a'+(char)_using_F.artifacts(_using_sig_no).dampen_window_type,
			  _signature) > 1);
	string ret {_};
	return ret;
}




inline namespace {
inline valarray<double>
to_vad( valarray<double>&& rv)
{
	return rv;
}
inline valarray<double>
to_vad( const valarray<float>& rv)
{
	valarray<double> ret;
	ret.resize( rv.size());
	for ( size_t i = 0; i < rv.size(); ++i )
		ret[i] = rv[i];
	return ret;
}
}


int
sigfile::CBinnedPower::compute( const SFFTParamSet& req_params,
				bool force)
{
      // check if we have it already
	hash_t req_signature = _using_F.artifacts( _using_sig_no).dirty_signature();
	if ( have_data() && (*this) == req_params
	     && _signature == req_signature )
		return 0;

	size_t	sr = samplerate();
	size_t	spp = sr * _pagesize;
	TFloat	freq_max = (TFloat)(spp+1)/2 / sr;
	_data.resize( pages() * _bins);
	printf( "CBinnedPower::compute( %s, %s): %g sec (%zu pp @%zu + %zu sec last incomplete page); bins/size/freq_max = %zu/%g/%g\n",
		_using_F.filename(), _using_F.channel_by_id(_using_sig_no),
		_using_F.recording_time(),
		pages(), _pagesize, (size_t)_using_F.recording_time() - (pages() * _pagesize),
		_bins, binsize, freq_max);
	// fprintf( stderr, "bin_size = %g, page_size = %zu; %zu bins\n",
	// 	 bin_size, page_size, n_bins());

	DEF_UNIQUE_CHARP (old_mirror_fname);
	DEF_UNIQUE_CHARP (new_mirror_fname);

	// insert a .
	string basename_dot = fs::make_fname_base (_using_F.filename(), "", true);

	assert (asprintf( &old_mirror_fname,
			  "%s-%s-%zu-%g-%c%c-%zu.power",
			  basename_dot.c_str(),
			  _using_F.channel_by_id(_using_sig_no), _pagesize, binsize,
			  'a'+(char)welch_window_type, 'a'+(char)_using_F.artifacts(_using_sig_no).dampen_window_type,
			  _signature) > 1);

      // update signature
	*(SFFTParamSet*)this = req_params;
	_signature = req_signature;
	assert (asprintf( &new_mirror_fname,
			  "%s-%s-%zu-%g-%c%c-%zu.power",
			  basename_dot.c_str(),
			  _using_F.channel_by_id(_using_sig_no), _pagesize, binsize,
			  'a'+(char)welch_window_type, 'a'+(char)_using_F.artifacts(_using_sig_no).dampen_window_type,
			  _signature) > 1);

	bool got_it = (_mirror_back( new_mirror_fname) == 0);

      // remove previously saved power
	if ( strcmp( old_mirror_fname, new_mirror_fname) )
		if ( unlink( old_mirror_fname) )
			;

	if ( got_it and not force ) {
		_status |= TFlags::computed;
		return 0;
	}

      // 0. get signal sample; always use double not TFloat
      // so that saved power is usable irrespective of what TFloat is today
	valarray<double> S = to_vad( _using_F.get_signal_filtered( _using_sig_no));

      // 1. dampen samples marked as artifacts
	// already done in get_signal_filtered()

      // 2. zero-mean and detrend
	// zero-mean already done in CEDFFile::get_signal_filtered()

      // 3. apply windowing function
	{
	      // (a) create a static vector of multipliers
		valarray<double>
			W (spp);
		for ( size_t i = 0; i < spp; ++i )
			W[i] = winf[(size_t)welch_window_type]( i, spp);

	      // (b) apply it page by page
		for ( size_t p = 0; p < pages(); ++p )
			S[ slice(p * spp, 1 * spp, 1) ] *= W;
	}
      // 4. obtain power spectrum
	// prepare

	static double
		*fft_Ti = nullptr,
		*fft_To = nullptr;
	static valarray<double>	// buffer for PSD
		P;
	static fftw_plan fft_plan = NULL;
	static size_t saved_spp = 0;

	static int n_procs = 1;
	if ( fft_plan == nullptr ) {
#if defined(HAVE_LIBFFTW3_OMP) && defined(_OPENMP)
		n_procs = omp_get_max_threads();
		fftw_init_threads();
		fftw_plan_with_nthreads( n_procs);
#endif
	}
	if ( fft_plan == nullptr || spp != saved_spp ) {

		fprintf( stderr, "Will use %d core(s)\nPreparing fftw plan for %zu samples...", n_procs, spp);
		saved_spp = spp;

		fftw_free( fft_Ti);
		fftw_free( fft_To);

		fft_Ti = (double*) fftw_malloc( sizeof(double) * spp * 2);
		fft_To = (double*) fftw_malloc( sizeof(double) * spp * 2);
		P.resize( spp+2);
		// and let them lie spare

		memcpy( fft_Ti, &S[0], spp * sizeof(double));  // not necessary?
		fft_plan = fftw_plan_dft_r2c_1d( spp, fft_Ti, (fftw_complex*)fft_To, 0 /* FFTW_PATIENT */);
		fprintf( stderr, "done\n");
	}

	// go
	TFloat	f = 0.;
	size_t	p, b, k = 1;
	for ( p = 0; p < pages(); ++p ) {
		memcpy( fft_Ti, &S[p*spp], spp * sizeof(double));

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
		for ( f = 0., b = 0; b < _bins; (f += binsize), ++b ) {
			//printf( "b = %zu, f = %g\n", b, f);
			nmth_bin(p, b) =
				valarray<double>
				(P[ slice( f*sr, (f + binsize)*sr, 1) ]) . sum();
		}
		/// / (bin_size * sr) // don't; power is cumulative
	}

	if ( _mirror_enable( new_mirror_fname) )
		;

	_status |= TFlags::computed;
	return 0;
}









int
sigfile::CBinnedPower::export_tsv( const string& fname) const
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
sigfile::CBinnedPower::export_tsv( float from, float upto,
				   const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return -1;

	auto sttm = _using_F.start_time();
	char *asctime_ = asctime( localtime( &sttm));
	fprintf( f, "## Subject: %s;  Session: %s, Episode: %s recorded %.*s;  Channel: %s\n"
		 "## Spectral power course (%zu %zu-sec pages) in range %g-%g Hz\n",
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
