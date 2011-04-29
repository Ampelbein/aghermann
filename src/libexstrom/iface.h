// ;-*-C++-*- *  Time-stamp: "2011-04-21 23:33:10 hmmr"
/*
 *       File name:  libexstrom/iface.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-26
 *
 *         Purpose:  C wrappers for the signal/exstrom library
 *
 *         License:  GPL
 */


#ifndef _EXSTROM_IFACE_H
#define _EXSTROM_IFACE_H

#include <unistd.h>

#if HAVE_CONFIG_H
#  include "config.h"
#endif




// signal processing
size_t	exstrom_low_pass( const float *in, size_t n_samples, size_t samplerate,
			  float cutoff, unsigned order, int scale,
			  float **course);

double	signal_phasediff( const float *sig1, const float *sig2,
			  size_t samplerate,
			  size_t sa, size_t sz,
			  float fa, float fz,
			  unsigned order,
			  size_t scope);

size_t	signal_envelope( const float *in, size_t n_samples, size_t samplerate,
			 float **upper_p, float **lower_p,
			 size_t tightness,
			 float **breadth);  // this last pointer can be NULL

size_t	signal_dzcdf( const float *in, size_t n_samples, size_t samplerate,
		      float dt, float sigma, size_t smooth,
		      float **buffer_p);

struct SSignalPatternPrimer {
	float	*data;
	size_t	n_samples,  // includes contexts on both sides
		context_before,
		context_after,
		samplerate;

        // Butterworth low-pass filter fields
	unsigned
		bwf_order;
	float	bwf_cutoff;
	int	bwf_scale;

        // ZC density function fields
	float 	dzcdf_step,
		dzcdf_sigma;
	size_t	dzcdf_smooth;

        // envelope
	size_t	env_tightness;

	float	a,
		b,
		c;
	float	match_a,
		match_b,
		match_c;
};
size_t		signal_find_pattern( struct SSignalPatternPrimer *pattern,
				     const float *field, size_t n_samples_field,
				     ssize_t start, int inc);
size_t		signal_find_pattern_( struct SSignalPatternPrimer *pattern,
				      const float *course,
				      const float *breadth,
				      const float *dzcdf,
				      size_t n_samples_field,
				      ssize_t start, int inc);

#ifdef __cplusplus
}
#endif

#endif

// eof
