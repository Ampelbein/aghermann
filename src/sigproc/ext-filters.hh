// ;-*-C++-*-
/*
 *       File name:  sigproc/ext-filters.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-03-11
 *
 *         Purpose:  some filters used in microcontinuity code
 *
 *         License:  GPL
 */

#ifndef _SIGPROC_EXT_FILTERS_HH
#define _SIGPROC_EXT_FILTERS_HH

#include <valarray>
#include <stdexcept>
#include "common/lang.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigproc {

class CFilter_base {
	DELETE_DEFAULT_METHODS (CFilter_base);

    public:
	enum TFilterDirection { forward, back };

    protected:
	size_t samplerate;
	TFilterDirection direction;

	CFilter_base (size_t samplerate_,
		      TFilterDirection direction_ = forward)
	      : samplerate (samplerate_),
		direction (direction_)
		{
			if ( samplerate_ == 0 )
				throw invalid_argument ("CFilter_base(): samplerate is 0");
		}

	virtual valarray<TFloat>
	apply( const valarray<TFloat>& in, bool) = 0;
	void reset()
		{}
};


class CFilterIIR
  : public CFilter_base {
	DELETE_DEFAULT_METHODS (CFilterIIR);

    protected:
	CFilterIIR (size_t samplerate_,
		    TFilterDirection direction_,
		    double gain_, double back_polate_)
	      : CFilter_base (samplerate_, direction_),
		anticipate (true),
		gain (gain_),
		back_polate (back_polate_)
		{
			calculate_iir_coefficients();
		}
	virtual void reset();
	virtual void reset( double use_this);

	bool anticipate;
	valarray<double>
		filter_state_p,
		filter_state_z,
		poles,
		zeros;
	double	gain,
		back_polate;

    public:
	void calculate_iir_coefficients()
		{}
	virtual valarray<TFloat>
	apply( const valarray<TFloat>& in, bool);
};


class CFilterSE
  : public CFilterIIR {
	DELETE_DEFAULT_METHODS (CFilterSE);

    public:
	CFilterSE (size_t samplerate_, TFilterDirection direction_,
		   double gain_, double back_polate_,
		   double f0_, double fc_, double bandwidth_)
	      : CFilterIIR (samplerate_, direction_, gain_, back_polate_),
		f0 (f0_),
		fc (fc_),
		bandwidth (bandwidth_)
		{
			zeros.resize(3); filter_state_z.resize(3);
			poles.resize(3); filter_state_p.resize(4);    // NrPoles+1 !!!!!111адинадин
			calculate_iir_coefficients();
		}

	void calculate_iir_coefficients();

    private:
	double	f0,
		fc,
		bandwidth;
};

class CFilterDUE
  : public CFilterIIR {
	DELETE_DEFAULT_METHODS (CFilterDUE);

    public:
	CFilterDUE (size_t samplerate_, TFilterDirection direction_,
		    double gain_, double back_polate_,
		    double minus_3db_frequency_)
	      : CFilterIIR (samplerate_, direction_, gain_, back_polate_),
		minus_3db_frequency (minus_3db_frequency_)
		{
			zeros.resize(2); filter_state_z.resize(2);
			poles.resize(1); filter_state_p.resize(2);    // NrPoles+1 !!!!!
			calculate_iir_coefficients();
		}

	void calculate_iir_coefficients();

    private:
	double	minus_3db_frequency;
};

} // namespace sigproc

#endif // _EXT_FILTERS_HH

// eof
