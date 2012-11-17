// ;-*-C++-*-
/*
 *       File name:  libsigproc/ext-filters.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-03-11
 *
 *         Purpose:  some filters directly for use in microcontinuity code
 *
 *         License:  GPL
 */


#include <gsl/gsl_math.h>

#include "../common/lang.hh"
#include "ext-filters.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif



void
sigproc::CFilterIIR::
reset()
{
	CFilter_base::reset();

	calculate_iir_coefficients();

	filter_state_z = 0.;
	filter_state_p = 0.;
}


void
sigproc::CFilterIIR::
reset( TFloat xn)
{
	calculate_iir_coefficients();

	filter_state_z = xn;
	filter_state_p = xn * zeros.sum() / (1. - poles.sum());
}





void
sigproc::CFilterDUE::
calculate_iir_coefficients()
{
	CFilterIIR::calculate_iir_coefficients();

	TFloat	ts = 1. / samplerate,
		fprewarp = tan( M_PI * minus_3db_frequency * ts) / (M_PI * ts),
		r = 1. / (2. * M_PI * fprewarp),
		s = ts / 2.;
	zeros = {(TFloat)(gain * (s + r)),
		 (TFloat)(gain * (s - r)),
		 (TFloat)1.};
}

void
sigproc::CFilterSE::
calculate_iir_coefficients()
{
	CFilterIIR::calculate_iir_coefficients();

	TFloat	ts = 1.0 / samplerate,
		fprewarp, r, s, t;

	fprewarp = tan(f0 * M_PI * ts) / (M_PI * ts);
	r = gsl_pow_2(2. * M_PI * fprewarp * ts);
	// From November 1992 prewarping applied because of Arends results !
	// r:=sqr(2.0*pi*f0*Ts);                         No prewarping
	s = 2. * M_PI * bandwidth * ts * 2.;
	t = 4. + r + s;
	poles = {(TFloat)1.,
		 (TFloat)((8.0 - 2.0 * r) / t),
		 (TFloat)((-4.0 + s - r) / t)};

	fprewarp = tan(fc * M_PI * ts) / (M_PI * ts);
	r = 2.0 / (2. * M_PI * fprewarp);
	s = gain * 2. * M_PI * bandwidth * 2.;
	zeros = {(TFloat)(s * (r + ts)   / t),
		 (TFloat)(s * (-2.0 * r) / t),
		 (TFloat)(s * (r - ts)   / t)};
}




valarray<TFloat>
sigproc::CFilterIIR::
apply( const valarray<TFloat>& in, bool use_first_sample_to_reset)
{
	if ( unlikely (poles.size() == 0) )
		throw runtime_error ("Unitialized CFilterIIR");

	valarray<TFloat> out (in.size());

	size_t i, l;

	int d;
	switch ( direction ) {
	case forward:
		i = 0;
		l = in.size();
		d = 1;
	    break;
	case back:
		i = in.size()-1;
		l = 0-1;
		d = -1;
	    break;
	default:
		throw invalid_argument ("sigproc::CFilterIIR::apply(): direction?");
	}

	for ( ; i != l; i += d ) {
		filter_state_z[0] = in[i];
		if ( unlikely (use_first_sample_to_reset) ) {
			reset( filter_state_z[0]);
			use_first_sample_to_reset = false;
		}
		// Compute new output sample
		TFloat r = 0.;
		// Add past output-values
		size_t j;
		for ( j = 1; j < poles.size(); ++j )
			r += poles[j] * filter_state_p[j];
		// Not anticipate = do not include current input sample in output value
		if ( not anticipate)
			out[i] = r;
		// Add past input-values
		for ( j = 0; j < zeros.size(); ++j )
			r += zeros[j] * filter_state_z[j];
		// Anticipate = include current input sample in output value
		if ( anticipate )
			out[i] = r;
		// Do backpolation (FilterStateP[1] = Last out-sample)
		out[i] = back_polate * filter_state_p[1] + (1.0 - back_polate) * out[i];
		// Scale result
		// TODO: Check if removing extra checks was ok
		// Update filter state
		for ( j = poles.size()-1; j >= 2; --j )
			filter_state_p[j] = filter_state_p[j-1];
		filter_state_p[1] = r;
		for ( j = zeros.size()-1; j >= 1; --j )
			filter_state_z[j] = filter_state_z[j-1];
	}

	return out;
}


// eof
