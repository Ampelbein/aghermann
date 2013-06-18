/*
 *       File name:  sigproc/ext-filters.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-03-11
 *
 *         Purpose:  some filters used in microcontinuity code (template instantiations)
 *
 *         License:  GPL
 */


#include "ext-filters.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

template valarray<float>   sigproc::CFilterIIR<float>::apply( const valarray<float>&, bool);
template valarray<double>  sigproc::CFilterIIR<double>::apply( const valarray<double>&, bool);

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
