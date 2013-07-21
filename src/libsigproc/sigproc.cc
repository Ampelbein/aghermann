/*
 *       File name:  libsigproc/sigproc.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-12-26
 *
 *         Purpose:  various standalone signal processing functions
 *
 *         License:  GPL
 */

#include <vector>
#include <valarray>

#include "sigproc.hh"

using namespace std;


template void sigproc::smooth( valarray<TFloat>&, size_t);
template void sigproc::normalize( valarray<TFloat>&);
template valarray<TFloat> sigproc::derivative( const valarray<TFloat>&);
template size_t sigproc::envelope( const SSignalRef<float>&, double, double, valarray<float>*, valarray<float>*, vector<size_t>*, vector<size_t>*);
template size_t sigproc::envelope( const SSignalRef<double>&, double, double, valarray<double>*, valarray<double>*, vector<size_t>*, vector<size_t>*);
template valarray<TFloat> sigproc::dzcdf( const SSignalRef<TFloat>&, double, double, size_t);
template double sigproc::sig_diff( const valarray<TFloat>&, const valarray<TFloat>&, int);
template double sigproc::phase_diff( const SSignalRef<TFloat>&, const SSignalRef<TFloat>&, size_t, size_t, double, double, unsigned, size_t);


valarray<float>
sigproc::
resample_f( const valarray<float>& signal,
	    size_t start, size_t end,
	    size_t to_size,
	    int alg)
{
	if ( unlikely (end <= start) || (end > signal.size()) )
		throw runtime_error ("bad args for resample");

	valarray<float>	resampled (to_size);
	SRC_DATA S;
	S.data_in       = const_cast<float*>(&signal[start]);
	S.input_frames  = end - start;
	S.output_frames = to_size;
	S.data_out      = &resampled[0];
	S.src_ratio     = (double)S.output_frames / S.input_frames;

	src_simple( &S, alg, 1);

	return resampled;
}

valarray<double>
sigproc::
resample( const valarray<double>& signal,
	  size_t start, size_t end,
	  size_t to_size,
	  int alg)
{
	valarray<float> tmp1 (end - start);
	for ( size_t i = start; i < end; ++i )
		tmp1[i] = signal[i];

	valarray<float> tmp2
		= sigproc::resample_f( tmp1, 0, end-start, to_size, alg);

	valarray<double> ret (end-start);
	for ( size_t i = 0; i < end-start; ++i )
		ret[i] = tmp2[i];

	return ret;
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
