/*
 *       File name:  libexstrom/exstrom.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-14
 *
 *         Purpose:  explicit template instantiations
 *
 *         License:  GPL
 */

#include "exstrom.hh"

template valarray<TFloat> exstrom::binomial_mult( unsigned, const valarray<TFloat>&);
template valarray<TFloat> exstrom::trinomial_mult( unsigned, const valarray<TFloat>&, const valarray<TFloat>&);
template valarray<TFloat> exstrom::dcof_bwlp( unsigned, TFloat);
template valarray<TFloat> exstrom::dcof_bwbp( unsigned, TFloat, TFloat);
template valarray<TFloat> exstrom::ccof_bwbs( unsigned, TFloat, TFloat);
template TFloat exstrom::sf_bwlp( unsigned, TFloat);
template TFloat exstrom::sf_bwhp( unsigned, TFloat);
template TFloat exstrom::sf_bwbp( unsigned, TFloat, TFloat);
template TFloat exstrom::sf_bwbs( unsigned, TFloat, TFloat);
template valarray<TFloat> exstrom::low_pass( const valarray<TFloat>&, size_t, float, unsigned, bool);
template valarray<TFloat> exstrom::high_pass( const valarray<TFloat>&, size_t, float, unsigned, bool);
template valarray<TFloat> exstrom::band_pass( const valarray<TFloat>&, size_t, float, float, unsigned, bool);
template valarray<TFloat> exstrom::band_stop( const valarray<TFloat>&, size_t, float, float, unsigned, bool);

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
