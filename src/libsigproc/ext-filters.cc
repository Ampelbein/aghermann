// ;-*-C++-*-
/*
 *       File name:  libexstrom/ext-filters.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-03-11
 *
 *         Purpose:  some filters directly for use in microcontinuity code
 *
 *         License:  GPL
 */

#include "ext-filters.hh"




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
reset( bool use_next_sample)
{
	if ( not use_next_sample )
		reset();
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
sigproc::CFilterIIR::
apply( const valarray<TFloat>& in, valarray<TFloat>& out,
       size_t ia, size_t iz, size_t oa)
{
	if ( ia <= iz )
		throw invalid_argument ("sigproc::CFilterIIR::apply(): ia < iz");

	size_t i, l;

	if ( oa == (size_t)-1 )
		oa = ia;

	int d;
	switch ( direction ) {
	case forward:
		i = ia;
		l = iz + 1; // overshoot?
		d = 1;
	    break;
	case backward:
		i = iz;
		l = ia - 1;
		d = -1;
	    break;
	default:
		throw invalid_argument ("sigproc::CFilterIIR::apply(): direction?");
	}

	filter_state_z[0] = in[i];
	if ( use_first_sample_to_reset )
		reset( filter_state_z[0]);
	i += d;

	while ( i != l ) {
		TFloat& s = out[i];
		// Compute new output sample
		TFloat r = 0.;
		// Add past output-values
		size_t j;
		for ( j = 1; j < poles.size(); ++j )
			r += poles[j] * filter_state_p[j];
		// Not anticipate = do not include current input sample in output value
		if ( not anticipate)
			s = r;
		// Add past input-values
		for ( j = 0; j < zeros.size(); ++j )
			r += zeros[j] * filter_state_z[j];
		// Anticipate = include current input sample in output value
		if ( anticipate )
			s = r;
		// Do backpolation (FilterStateP[1] = Last out-sample)
		s = back_polate * filter_state_p[1] + (1.0 - back_polate) * s;
		// Scale result 
		// TODO: Check if removing extra checks was ok
		// Update filter state
		for ( j = poles.size()-1; j >= 2; --j )
			filter_state_p[j] = filter_state_p[j-1];
		filter_state_p[1] = r;
		for ( j = zeros.size()-1; j >= 1; --j )
			filter_state_z[j] = filter_state_z[j-1];

		i += d;
	}
}


void
sigproc::CFilterDUE::
calculate_iir_coefficients()
{
	CFilterIIR::calculate_iir_coefficients();

	// Settings: 1=SampleFreq, 2=Gain, 3=FilterFreq
	TFloat	ts = 1. / samplerate,
		fprewarp = tan( M_PI * _3db_frequency * ts) / (M_PI * ts),
		r = 1. / (2. * M_PI * fprewarp),
		s = ts / 2.;
	zeros[0] = gain * (s + r);
	zeros[1] = gain * (s - r);
	poles[0] = 1.;
}

void
sigproc::CFilterSE::
calculate_iir_coefficients()
{
}





// eof
