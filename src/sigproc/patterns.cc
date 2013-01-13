// ;-*-C++-*-
/*
 *       File name:  sigproc/patterns.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-09
 *
 *         Purpose:  CPattern explicit pattern instantiations be here
 *
 *         License:  GPL
 */

#include "patterns.hh"

using namespace std;

template pattern::CPattern<TFloat>::CPattern( const sigproc::SSignalRef<TFloat>&, size_t, size_t, const SPatternPPack<TFloat>&);
template int pattern::CPattern<TFloat>::do_search( const valarray<TFloat>&, const valarray<TFloat>&, const valarray<TFloat>&, const valarray<TFloat>&, size_t);
template int pattern::CPattern<TFloat>::do_search( const sigproc::SSignalRef<TFloat>&, size_t);
template int pattern::CPattern<TFloat>::do_search( const valarray<TFloat>&, size_t);



// eof
