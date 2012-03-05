// ;-*-C++-*-
/*
 *       File name:  libsigfile/ucont.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-03-04
 *
 *         Purpose:  CBinnedMicroConty ("EEG microcontinuity")
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_UCONT_H
#define _SIGFILE_UCONT_H

#include "../misc.hh"
#include "forward-decls.hh"
#include "page-metrics-base.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigfile {



struct SMicroContyParamSet {

	size_t	pagesize;

	SMicroContyParamSet& operator=( const SMicroContyParamSet& rv)
		{
			return *this;
			// don't touch samplerate
		}
	bool operator==( const SMicroContyParamSet& rv) const
		{
			return false;
		}
	bool validate();
	void assign_defaults()
		{
		}

	SMicroContyParamSet( const SMicroContyParamSet& rv) = default;
	SMicroContyParamSet()
		{
			assign_defaults();
		}
};





class CBinnedMicroConty
  : public CPageMetrics_base, SMicroContyParamSet {

	CBinnedMicroConty() = delete;

    protected:
	CBinnedMicroConty( const CSource& F, int sig_no,
			   const SMicroContyParamSet &params)
	      : CPageMetrics_base (F, sig_no, params.pagesize, 1),
		SMicroContyParamSet (params)
		{}

    public:
      // obtain
	int compute( const SMicroContyParamSet& req_params,
		     bool force = false);
	// possibly reuse that already obtained unless factors affecting signal or fft are different
	void compute( bool force = false)
		{
			compute( *this, force);
		}

	string fname_base() const;
};


} // namespace sigfile


#endif // _SIGFILE_UCONT_H

// eof
