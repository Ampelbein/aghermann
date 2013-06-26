/*
 *       File name:  libsigfile/edf-io.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-21
 *
 *         Purpose:  CEDFFile bulk data io
 *
 *         License:  GPL
 */

#include "common/string.hh"
#include "edf.hh"

using namespace std;
using sigfile::CEDFFile;

valarray<TFloat>
CEDFFile::
get_region_original_smpl( const int h,
			  const size_t sa, const size_t sz) const
{
	if ( unlikely (_status & (TStatus::bad_header | TStatus::bad_version)) )
		throw invalid_argument ("CEDFFile::get_region_original(): broken source");
	if ( unlikely (_mmapping == NULL) )
		throw invalid_argument ("CEDFFile::get_region_original(): no data");
	if ( unlikely (sa >= sz || sz > samplerate(h) * recording_time()) )
		throw range_error (agh::str::sasprintf(
					   "CEDFFile::get_region_original(%s[%s]): bad region (req %zu:%zu, avail end %zu x %g sec = %g, or %zu x %zu = %zu)",
					   filename(), operator[](h).ucd.name(),
					   sa, sz, samplerate(h), recording_time(), samplerate(h) * recording_time(),
					   n_data_records, operator[](h).samples_per_record, n_data_records * operator[](h).samples_per_record));

	valarray<TFloat> recp;

	const SSignal& H = operator[](h);
	size_t	r0    =                        (   sa) / H.samples_per_record,
		r_cnt = (size_t) ceilf( (float)(sz-sa) / H.samples_per_record);

	int16_t* tmp;
	tmp = (int16_t*)malloc( r_cnt * H.samples_per_record * 2);  // 2 is sizeof(sample) sensu edf

	while ( r_cnt-- )
		memcpy( &tmp[ r_cnt * H.samples_per_record ],

			(char*)_mmapping + header_length
			+ (r0 + r_cnt) * _total_samples_per_record * 2	// full records before
			+ H._at * 2,				// offset to our samples

			H.samples_per_record * 2);	// our precious ones

	recp.resize( sz - sa);

      // repackage for shipping, and scale
	size_t sa_off = sa - r0 * H.samples_per_record;
	for ( size_t s = 0; s < recp.size(); ++s )
		recp[s] = tmp[sa_off + s] * H.scale;

	free( tmp);

	return move(recp);
}



valarray<TFloat>
CEDFFile::
get_region_filtered_smpl( const int h,
			  const size_t smpla, const size_t smplz) const
{
	valarray<TFloat> recp =
		get_region_original_smpl( h, smpla, smplz);
	if ( recp.size() == 0 )
		return valarray<TFloat> (0);
	// and zeromean
       	recp -= (recp.sum() / recp.size());

	const SSignal& H = operator[](h);

      // artifacts
	size_t this_samplerate = H.samples_per_record / data_record_size;
	for ( auto &A : H.artifacts() ) {
		size_t	Aa = A.a * this_samplerate,
			Az = A.z * this_samplerate;
		if ( unlikely (Aa >= smplz) )
			break;
		size_t	run = (Az - Aa),
			window = min( run, this_samplerate),
			t;
		if ( unlikely (Az > smplz) )
			run = smplz - Aa;
		valarray<TFloat>
			W (run);

		if ( run > window ) {
			// construct a vector of multipliers using an INVERTED windowing function on the
			// first and last windows of the run
			size_t	t0;
			for ( t = 0; t < window/2; ++t )
				W[t] = (1 - sigproc::winf[(size_t)H.artifacts.dampen_window_type]( t, window));
			t0 = run-window;  // start of the last window but one
			for ( t = window/2; t < window; ++t )
				W[t0 + t] = (1 - sigproc::winf[(size_t)H.artifacts.dampen_window_type]( t, window));
			// AND, connect mid-first to mid-last windows (at lowest value of the window)
			TFloat minimum = sigproc::winf[(size_t)H.artifacts.dampen_window_type]( window/2, window);
			W[ slice(window/2, run-window, 1) ] =
				(1. - minimum);
		} else  // run is shorter than samplerate (1 sec)
			for ( t = 0; t < window; ++t )
				W[t] = (1 - sigproc::winf[(size_t)H.artifacts.dampen_window_type]( t, window));

		// now gently apply the multiplier vector onto the artifacts
		recp[ slice(Aa, run, 1) ] *= (W * (TFloat)H.artifacts.factor);
	}

      // filters
	if ( H.filters.low_pass_cutoff > 0. && H.filters.high_pass_cutoff > 0. &&
	     H.filters.low_pass_order > 0 && H.filters.high_pass_order > 0 ) {
		auto tmp (exstrom::band_pass(
				  recp, this_samplerate,
				  H.filters.high_pass_cutoff, H.filters.low_pass_cutoff,
				  H.filters.low_pass_order, true));
		recp = tmp;
	} else {
		if ( H.filters.low_pass_cutoff > 0. && H.filters.low_pass_order > 0 ) {
			auto tmp (exstrom::low_pass(
					  recp, this_samplerate,
					  H.filters.low_pass_cutoff, H.filters.low_pass_order, true));
			recp = tmp;
		}
		if ( H.filters.high_pass_cutoff > 0. && H.filters.high_pass_order > 0 ) {
			auto tmp (exstrom::high_pass(
					  recp, this_samplerate,
					  H.filters.high_pass_cutoff, H.filters.high_pass_order, true));
			recp = tmp;
		}
	}

	switch ( H.filters.notch_filter ) {
	case SFilterPack::TNotchFilter::at50Hz:
		recp = exstrom::band_stop( recp, this_samplerate,
					   48, 52, 1, true);
	    break;
	case SFilterPack::TNotchFilter::at60Hz:
		recp = exstrom::band_stop( recp, this_samplerate,
					   58, 62, 1, true);
	    break;
	case SFilterPack::TNotchFilter::none:
	default:
	    break;
	}

	// filters happen to append samples, so
	return move(recp[ slice (0, smplz-smpla, 1)]);
}





int
CEDFFile::
put_region_smpl( const int h,
		 const valarray<TFloat>& src,
		 const size_t offset) const
{
	if ( unlikely (_status & (TStatus::bad_header | TStatus::bad_version)) )
		throw invalid_argument("CEDFFile::put_region_(): broken source");
	if ( unlikely (_mmapping == NULL) )
		throw invalid_argument("CEDFFile::put_region_(): no data");
	if ( unlikely (offset >= samplerate(h) * recording_time()) )
		throw range_error("CEDFFile::put_region_(): offset beyond end of file");
	if ( unlikely (offset + src.size() > samplerate(h) * recording_time()) ) {
		fprintf( stderr, "CEDFFile::put_region_(): attempt to write past end of file (%zu + %zu > %zu * %g)\n",
			 offset, src.size(), samplerate(h), recording_time());
		throw range_error("CEDFFile::put_region_(): attempt to write past end of file");
	}

	const SSignal& H = operator[](h);
	size_t	r0    =                            offset  / H.samples_per_record,
		r_cnt = (size_t) ceilf( (double)src.size() / H.samples_per_record);

	valarray<int16_t> tmp (src.size());
	for ( size_t i = 0; i < tmp.size(); ++i )
		tmp[i] = // clamp
			agh::alg::value_within(
				(double)src[i] / H.scale, (double)INT16_MIN, (double)INT16_MAX);

	size_t r;
	for ( r = 0; r < r_cnt - 1; ++r ) // minus one
		memcpy( (char*)_mmapping + header_length
			+ (r0 + r) * _total_samples_per_record * 2	// full records before
			+ H._at * 2,				// offset to our samples

			&tmp[ r * H.samples_per_record ],

			H.samples_per_record * 2);	// our precious ones
	// last record is underfull
	memcpy( (char*)_mmapping + header_length
		+ (r0 + r) * _total_samples_per_record * 2
		+ H._at * 2,

		&tmp[ r * H.samples_per_record ],

		(tmp.size() - r * H.samples_per_record) * 2);

	return 0;
}




int
CEDFFile::
export_original( const int h,
		 const string& fname) const
{
	valarray<TFloat> signal = get_signal_original( h);
	FILE *fd = fopen( fname.c_str(), "w");
	if ( fd ) {
		for ( size_t i = 0; i < signal.size(); ++i )
			fprintf( fd, "%g\n", signal[i]);
		fclose( fd);
		return 0;
	} else
		return -1;
}


int
CEDFFile::
export_filtered( const int h,
		 const string& fname) const
{
	valarray<TFloat> signal = get_signal_filtered( h);
	FILE *fd = fopen( fname.c_str(), "w");
	if ( fd ) {
		for ( size_t i = 0; i < signal.size(); ++i )
			fprintf( fd, "%g\n", signal[i]);
		fclose( fd);
		return 0;
	} else
		return -1;
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
