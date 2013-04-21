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
#include <gsl/gsl_math.h>
#include "common/lang.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace sigproc {

enum TFilterDirection { forward, back };

template <typename T>
class CFilter_base {
	DELETE_DEFAULT_METHODS (CFilter_base<T>);

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

	virtual valarray<T>
	apply( const valarray<T>& in, bool) = 0;
	void reset()
		{}
};


template <typename T>
class CFilterIIR
  : public CFilter_base<T> {
	DELETE_DEFAULT_METHODS (CFilterIIR<T>);

    protected:
	CFilterIIR<T> (size_t samplerate_,
		       TFilterDirection direction_,
		       T gain_, T back_polate_)
	      : CFilter_base<T> (samplerate_, direction_),
		anticipate (true),
		gain (gain_),
		back_polate (back_polate_)
		{
			calculate_iir_coefficients();
		}
	virtual void reset()
		{
			CFilter_base<T>::reset();

			calculate_iir_coefficients();

			filter_state_z = 0.;
			filter_state_p = 0.;
		}

	virtual void reset( T xn)
		{
			calculate_iir_coefficients();

			zeros = 0.;
			filter_state_z = xn;
			filter_state_p = xn * zeros.sum() / (1. - poles.sum());
		}


	bool	anticipate;
	valarray<T>
		filter_state_p,
		filter_state_z,
		poles,
		zeros;
	T	gain,
		back_polate;

    public:
	void calculate_iir_coefficients()
		{}
	virtual valarray<T>
	apply( const valarray<T>& in, bool);
};


template <typename T>
class CFilterSE
  : public CFilterIIR<T> {
	DELETE_DEFAULT_METHODS (CFilterSE<T>);

    public:
	CFilterSE<T> (size_t samplerate_, TFilterDirection direction_,
		      T gain_, T back_polate_,
		      T f0_, T fc_, T bandwidth_)
	      : CFilterIIR<T> (samplerate_, direction_, gain_, back_polate_),
		f0 (f0_),
		fc (fc_),
		bandwidth (bandwidth_)
		{
			CFilterIIR<T>::zeros.resize(3); CFilterIIR<T>::filter_state_z.resize(3);
			CFilterIIR<T>::poles.resize(3); CFilterIIR<T>::filter_state_p.resize(4);    // NrPoles+1 !!!!!111адинадин
			calculate_iir_coefficients();
		}

	void calculate_iir_coefficients()
		{
			CFilterIIR<T>::calculate_iir_coefficients();

			T	ts = 1.0 / CFilterIIR<T>::samplerate,
				fprewarp, r, s, t;

			fprewarp = tan( f0 * M_PI * ts) / (M_PI * ts);
			r = gsl_pow_2( 2. * M_PI * fprewarp * ts);
			// From November 1992 prewarping applied because of Arends results !
			// r:=sqr(2.0*pi*f0*Ts);                         No prewarping
			s = 2. * M_PI * bandwidth * ts * 2.;
			t = 4. + r + s;
			CFilterIIR<T>::poles[0] = 1.;
			CFilterIIR<T>::poles[1] = (8.0 - 2.0 * r) / t;
			CFilterIIR<T>::poles[2] = (-4.0 + s - r) / t;

			fprewarp = tan(fc * M_PI * ts) / (M_PI * ts);
			r = 2.0 / (2. * M_PI * fprewarp);
			s = CFilterIIR<T>::gain * 2. * M_PI * bandwidth * 2.;

			CFilterIIR<T>::zeros[0] = s * (r + ts)   / t;
			CFilterIIR<T>::zeros[1] = s * (-2.0 * r) / t;
			CFilterIIR<T>::zeros[2] = s * (r - ts)   / t;
		}

    private:
	T	f0,
		fc,
		bandwidth;
};

template <typename T>
class CFilterDUE
  : public CFilterIIR<T> {
	DELETE_DEFAULT_METHODS (CFilterDUE<T>);

    public:
	CFilterDUE<T> (size_t samplerate_, TFilterDirection direction_,
		       T gain_, T back_polate_,
		       T minus_3db_frequency_)
	      : CFilterIIR<T> (samplerate_, direction_, gain_, back_polate_),
		minus_3db_frequency (minus_3db_frequency_)
		{
			CFilterIIR<T>::zeros.resize(3); CFilterIIR<T>::filter_state_z.resize(3);
			CFilterIIR<T>::poles.resize(1); CFilterIIR<T>::filter_state_p.resize(2);    // NrPoles+1 !!!!!
			calculate_iir_coefficients();
		}

	void calculate_iir_coefficients()
		{
			CFilterIIR<T>::calculate_iir_coefficients();

			T	ts = 1. / CFilterIIR<T>::samplerate,
				fprewarp = tan( M_PI * minus_3db_frequency * ts) / (M_PI * ts),
				r = 1. / (2. * M_PI * fprewarp),
				s = ts / 2.;

			/// this is what m.roussen has in Library/Filters/DUEFilter.cs:
			// FZeros[0] = Gain * (s + r);
			// FZeros[1] = Gain * (s - r);
			// FPoles[0] = 1.0;
			/// note the last assignment is to FPoles -- which isn't used anyway at index 0
			CFilterIIR<T>::zeros[0] = CFilterIIR<T>::gain * (s + r);
			CFilterIIR<T>::zeros[1] = CFilterIIR<T>::gain * (s - r);
			CFilterIIR<T>::zeros[2] = 1.;

			// so I got zeros[2] assigned by transcription mistake (should be to poles[0]) -- and
			// it worked!  *Except* that zeros, through this assignment, has now grown to 3, which
			// causes an invalid read in apply(), and went miraculously undetected until noticed
			// (thank valgrind) at 0.9_rc stage
			CFilterIIR<T>::poles[0] = 1.;
			// Still, FPoles[0] = 1.0 is here for whatever it is intended to do

			// May your life be forever geweldig, mate!
		}

    private:
	T	minus_3db_frequency;
};


#include "ext-filters.ii"

} // namespace sigproc

#endif // _EXT_FILTERS_HH

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
