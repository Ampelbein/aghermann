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





int
CEDFFile::
put_region_smpl( const int h,
		 const valarray<TFloat>& src,
		 const size_t offset)
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



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
