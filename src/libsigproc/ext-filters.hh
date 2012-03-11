// ;-*-C++-*-
/*
 *       File name:  libexstrom/ext-filters.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-03-11
 *
 *         Purpose:  some filters directly for use in microcontinuity code
 *
 *         License:  GPL
 */

#ifndef _EXT_FILTERS_HH
#define _EXT_FILTERS_HH

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
	enum TFilterDirection { forward, backward };
	TFilterDirection direction;
	bool use_first_sample_to_reset;

	CFilter_base( size_t samplerate_, TFilterDirection direction_ = forward, bool use_first_sample_to_reset_ = false)
		: samplerate (samplerate_),
		  direction (direction_),
		  use_first_sample_to_reset (use_first_sample_to_reset_)
		{
			if ( samplerate_ == 0 )
				throw invalid_argument ("CFilter_base(): samplerate is 0");
		}

	virtual void apply( const valarray<TFloat>& in, valarray<TFloat>& out,
			    size_t ia, size_t iz, size_t outIdxStart = (size_t)-1) = 0;
	void reset()
		{}
};


class CFilterIIR : public CFilter_base {
	CFilterIIR() = delete;
    protected:
	CFilterIIR( size_t samplerate_,
		    TFilterDirection direction_ = forward, bool use_first_sample_to_reset_ = false)
	      : CFilter_base (samplerate_, direction_, use_first_sample_to_reset_),
		anticipate (true),
		back_polate (0.),
		gain (1.)
		{
			calculate_iir_coefficients();
		}
	virtual void reset();
	virtual void reset( bool use_next_sample);
	virtual void reset( TFloat use_this);

	virtual void apply( const valarray<TFloat>& in, valarray<TFloat>& out,
			    size_t ia, size_t iz, size_t outIdxStart = (size_t)-1);
	bool anticipate;
	valarray<TFloat>
		filter_state_p,
		filter_state_z,
		poles,
		zeros;
	TFloat	back_polate,
		gain;

	void calculate_iir_coefficients()
		{}
};


class CFilterSE : public CFilterIIR {
    public:
	void calculate_iir_coefficients();
	CFilterSE( size_t samplerate_,
		   TFilterDirection direction_ = forward, bool use_first_sample_to_reset_ = false)
	      : CFilterIIR (samplerate_, direction_, use_first_sample_to_reset_)
		{
			calculate_iir_coefficients();
		}
};

class CFilterDUE : public CFilterIIR {
    public:
	void calculate_iir_coefficients();
	CFilterDUE( TFloat _3db_frequency_, size_t samplerate_,
		    TFilterDirection direction_ = forward, bool use_first_sample_to_reset_ = false)
	      : CFilterIIR (samplerate_, direction_, use_first_sample_to_reset_),
		_3db_frequency (_3db_frequency_)
		{
			zeros.resize(2); filter_state_z.resize(2);
			poles.resize(1); filter_state_p.resize(2);    // NrPoles+1 !!!!!
			calculate_iir_coefficients();
		}
    private:
	TFloat	_3db_frequency;
};

} // namespace sigproc

#endif // _EXT_FILTERS_HH

// eof
