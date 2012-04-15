// ;-*-C++-*-
/*
 *       File name:  libsigproc/ext-filters.hh
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

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigproc {

class CFilter_base {
	CFilter_base() = delete;
    protected:
	size_t samplerate;
	enum TFilterDirection { Forward, Back };
	TFilterDirection direction;

	CFilter_base( size_t samplerate_,
		      TFilterDirection direction_ = Forward)
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


class CFilterIIR : public CFilter_base {
	CFilterIIR() = delete;
    protected:
	CFilterIIR( size_t samplerate_,
		    TFilterDirection direction_ = Forward)
	      : CFilter_base (samplerate_, direction_),
		anticipate (true),
		back_polate (0.),
		gain (1.)
		{
			calculate_iir_coefficients();
		}
	virtual void reset();
	virtual void reset( bool use_next_sample);
	virtual void reset( TFloat use_this);

	bool anticipate;
	valarray<TFloat>
		filter_state_p,
		filter_state_z,
		poles,
		zeros;
	TFloat	back_polate,
		gain;

    public:
	void calculate_iir_coefficients()
		{}
	virtual valarray<TFloat>
	apply( const valarray<TFloat>& in, bool);
};


class CFilterSE : public CFilterIIR {
    public:
	void calculate_iir_coefficients();
	CFilterSE( size_t samplerate_,
		   TFilterDirection direction_ = Forward)
	      : CFilterIIR (samplerate_, direction_)
		{
			zeros.resize(3); filter_state_z.resize(3);
			poles.resize(3); filter_state_p.resize(4);    // NrPoles+1 !!!!!
			calculate_iir_coefficients();
		}
};

class CFilterDUE : public CFilterIIR {
    public:
	void calculate_iir_coefficients();
	CFilterDUE( TFloat minus_3db_frequency_, size_t samplerate_,
		    TFilterDirection direction_ = Forward)
	      : CFilterIIR (samplerate_, direction_),
		minus_3db_frequency (minus_3db_frequency_)
		{
			zeros.resize(2); filter_state_z.resize(2);
			poles.resize(1); filter_state_p.resize(2);    // NrPoles+1 !!!!!
			calculate_iir_coefficients();
		}
    private:
	TFloat	minus_3db_frequency;
};

} // namespace sigproc

#endif // _EXT_FILTERS_HH

// eof
