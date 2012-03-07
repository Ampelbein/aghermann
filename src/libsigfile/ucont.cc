// ;-*-C++-*-
/*
 *       File name:  libsigfile/ucont.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  CBinnedMicroConty ("EEG microcontinuity")
 *
 *         License:  GPL
 */

#include <cassert>
#include "../misc.hh"
#include "ucont.hh"
#include "source.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

int
sigfile::CBinnedMicroConty::compute( const SMicroContyParamSet& req_params,
				     bool force)
{
	for ( size_t i = 0; i < _data.size(); ++i )
		_data[i] = sin((double)i/222);
	return 0;
}


string
sigfile::CBinnedMicroConty::fname_base() const
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


// eof
