// ;-*-C++-*-
/*
 *       File name:  libsigfile/mc.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  CBinnedMC ("EEG microcontinuity")
 *
 *         License:  GPL
 */

#include <unistd.h>
#include <limits.h>
#include <cassert>
#include <functional>
#include <stdexcept>

#include "../common/misc.hh"
#include "mc.hh"
#include "source.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


string
sigfile::CBinnedMC::
fname_base() const
{
	DEF_UNIQUE_CHARP (_);
	assert (asprintf( &_,
			  "%s-%s-%zu-%g-%c%c-%zu",
			  _using_F.filename(), _using_F.channel_by_id(_using_sig_no),
			  _using_F.pagesize(), 42.,
			  'a'+(char)0,
			  'a'+(char)0,
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
sigfile::SMCParamSet::
check( size_t pagesize) const
{
        if ( mc_gain < 1.0 )
		throw invalid_argument ("mc_gain must be >= 1.0");
}


void
sigfile::SMCParamSet::
reset( size_t pagesize)
{
	f0			=     1.;
	fc			=     1.8;
	bandwidth		=     1.5;
	iir_backpolate		=     0.5;	// 0.0 < Backpolate < 1.0 on s: standard 0.5
}





sigfile::CBinnedMC::
CBinnedMC( const CSource& F, int sig_no,
	   const SMCParamSet &params,
	   size_t pagesize)
      : CPageMetrics_base (F, sig_no,
			   pagesize, // acting 'pagesize' for CPageMetrics_base
			   1), // one bin only
	SMCParamSet (params),
	ss (pages()),
	su (pages()),
	due_filter (samplerate(), sigproc::CFilterIIR::TFilterDirection::Forward,
		    params.mc_gain, params.iir_backpolate,
		    params.fc),
	se_filter (samplerate(), sigproc::CFilterIIR::TFilterDirection::Forward,
		   params.mc_gain, params.iir_backpolate,
		   params.f0, params.fc, params.bandwidth)
{
	SMCParamSet::check( pagesize); // throw if not ok
}


// agh::VAF
// sigfile::CBinnedMC::make_sssu_template() const
// {
// 	agh::VAF
// 		vaf (pib_correlation_function_buffer_size);
// 	// todo: Marco: "gewoon" maximum zoeken, geen moeilijke functies.
// 	// geweldig, епта!
// 	// Construct the template to detect desired piB value peak in the smoothed histogram
// 	for ( size_t k = 0; k < vaf.size(); ++k ) {
// 		auto	x = ((TFloat)k / vaf.size()) * 3 * M_PI - 2 * M_PI;  // and when x is 0...
// 		vaf[k] =
// 			2.0 / 3 * -0.5894 * (heaviside(x + 2 * M_PI) - heaviside(x + (M_PI / 2)))
// 			+ (heaviside(x + (M_PI / 2)) - heaviside(x)) * (sin(2 * x) / (2 * x))
// 			+ (heaviside(x) - heaviside(x - M_PI / 2)) * (sin(2 * x) / (2 * x));
// 	}
// 	// patch nan
// 	size_t badk = 2./3 * vaf.size();
// 	vaf[badk] = (vaf[badk-1] + vaf[badk+1]) / 2;

// 	vaf -= (TFloat)vaf.sum() / vaf.size();
// 	agh::vaf_dump( vaf, fname_base() + ".sssu_template");

// 	return vaf;
// }





int
sigfile::CBinnedMC::
compute( const SMCParamSet& req_params,
	 bool force)
{
	agh::hash_t req_signature = _using_F.artifacts( _using_sig_no).dirty_signature();
	if ( have_data() && (*this) == req_params
	     && _signature == req_signature )
		return 0;

	_data.resize( pages() * _bins);

	printf( "CBinnedMC::compute( %s, %s): %g sec (%zu pp @%zu + %zu sec last incomplete page)\n",
		_using_F.filename(), _using_F.channel_by_id(_using_sig_no),
		_using_F.recording_time(),
		pages(), _pagesize, (size_t)_using_F.recording_time() - (pages() * _pagesize));

	DEF_UNIQUE_CHARP (old_mirror_fname);
	DEF_UNIQUE_CHARP (new_mirror_fname);

	// insert a .
	string basename_dot = fs::make_fname_base (_using_F.filename(), "", true);

	assert (asprintf( &old_mirror_fname,
			  "%s-%s-%zu:"
			  "_%g" "_%g" "_%g_%g_%g"
			  ":%zu.mc",
			  basename_dot.c_str(),
			  _using_F.channel_by_id(_using_sig_no), _pagesize,
			  iir_backpolate,
			  mc_gain,
			  f0, fc, bandwidth,
			  _signature)
		> 1);

      // update signature
	*(SMCParamSet*)this = req_params;
	_signature = req_signature;
	assert (asprintf( &new_mirror_fname,
			  "%s-%s-%zu:"
			  "_%g" "_%g" "_%g_%g_%g"
			  ":%zu.mc",
			  basename_dot.c_str(),
			  _using_F.channel_by_id(_using_sig_no), _pagesize,
			  iir_backpolate,
			  mc_gain,
			  f0, fc, bandwidth,
			  _signature)
		> 1);

	bool got_it = (_mirror_back( new_mirror_fname) == 0);

      // remove previously saved power
	if ( strcmp( old_mirror_fname, new_mirror_fname) )
		if ( unlink( old_mirror_fname) )
			;

	if ( got_it and not force ) {
		_status |= TFlags::computed;
		return 0;
	}


	//cout << "Performing SU and SS reduction...\n";
	// DoSSSUReduction();
	do_sssu_reduction();


//	_data = ss - su;
	for ( size_t i = 0; i < _data.size(); ++i )
		_data[i] = ss[i] - su[i];

/*
	//cout << "Computing PiB value...\n";
	do_detect_pib();

	//cout << "Detecting artifacts...\n";
	// DoComputeArtifactTraces();
	do_compute_artifact_traces();

	//cout << "Smoothing SU and SS...\n";
	do_smooth_sssu();

	//cout << "Detecting events...\n";
	// DoDetectMCEvents();
	do_detect_mc_events();

	//cout << "Re-smoothing signals and detecting jumps...\n";
	// DoResmoothSSSU();
	do_smooth_sssu();

	//cout << "Computing final gains...\n";
	// DoComputeMC();
	do_compute_mc();

	if ( dump_buffers )
		do_dump_buffers();

	for ( size_t i = 0; i < _data.size(); ++i )
		_data[i] = mc[i];
*/
	if ( _mirror_enable( new_mirror_fname) )
		;

	_status |= TFlags::computed;

	return 0;
}


void
sigfile::CBinnedMC::
do_sssu_reduction()
{
	valarray<TFloat>
		due_filtered,
		se_filtered;
	{
		auto signal = _using_F.get_signal_filtered(_using_sig_no);
		due_filtered = due_filter.apply( signal, false);
		se_filtered  =  se_filter.apply( signal, false);
	}

	size_t	integrate_samples = pagesize() * samplerate();

	for ( size_t p = 0; p < pages(); ++p ) {
		auto range = slice (p * integrate_samples, integrate_samples, 1);
		su[p] =
			(valarray<TFloat> {due_filtered[range]} * valarray<TFloat> {se_filtered[range]})
			.sum()
			/ integrate_samples;
		ss[p] =
			pow(valarray<TFloat> {se_filtered[range]}, (TFloat)2.)
			.sum() / samplerate()
			/ integrate_samples;
	}
}






// eof
