// ;-*-C++-*- *  Time-stamp: "2011-01-30 20:22:58 hmmr"
/*
 *       File name:  libexstrom/iface.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-12-31
 *
 *         Purpose:  wrappers to exstrom and signal processing functions
 *
 *         License:  GPL
 */


#include <cassert>
#include <cstring>

#include "iface.h"
#include "exstrom.hh"
#include "signal.hh"

extern "C" {




size_t
exstrom_low_pass( const float *in, size_t n_samples, size_t samplerate,
		  float cutoff, unsigned order, int scale,
		  float **course)
{
	valarray<float>
		in_va (in, n_samples),
		out_va = NExstrom::low_pass( in_va, samplerate,
					     cutoff, order, (bool)scale);

	assert ((*course) = (float*)malloc( n_samples * sizeof(float)));
	memcpy( *course, &out_va[0], n_samples * sizeof(float));
	return out_va.size();
}






size_t
signal_envelope( const float *in, size_t n_samples, size_t samplerate,
		 float **env_l_p, float **env_u_p,
		 size_t radius,
		 float **breadth_p)
{
	valarray<float> in_va (in, n_samples),
		env_l, env_u;
	if ( NSignal::envelope( in_va, radius, samplerate,
			       1./samplerate,
			       env_l, env_u) == 0 ) // don't need anchor points, nor their count
		return 0;

	// assert (env_l.size() == env_u.size());
	size_t env_size = env_l.size();
	(*env_l_p) = (float*)malloc( env_size * sizeof(float));
	(*env_u_p) = (float*)malloc( env_size * sizeof(float));
	assert (*env_l_p && *env_u_p);
	memcpy( *env_l_p, &env_l[0], sizeof(float) * env_size);
	memcpy( *env_u_p, &env_u[0], sizeof(float) * env_size);

	if ( breadth_p ) {
		assert ((*breadth_p) = (float*)malloc( env_size * sizeof(float)));
		valarray<float> b = env_u - env_l;
		memcpy( *breadth_p, &b[0], env_size * sizeof(float));
	}

	return env_size;
}




size_t
signal_dzcdf( const float *in, size_t n_samples, size_t samplerate,
	      float dt, float sigma, size_t smooth,
	      float **out_p)
{
	valarray<float>
		out_va = NSignal::dzcdf( valarray<float>(in, n_samples), samplerate, dt, sigma, smooth);

	assert ((*out_p) = (float*)malloc( out_va.size() * sizeof(float)));
	memcpy( *out_p, &out_va[0], sizeof(float) * out_va.size());
	return out_va.size();
}





size_t
signal_find_pattern( struct SSignalPatternPrimer *primer,
		     const float *field, size_t n_samples_field,
		     ssize_t start,
		     int inc)
{
	NSignal::CPattern<float> pattern (valarray<float> (primer->data, primer->n_samples),
					  primer->context_before, primer->context_after,
					  primer->samplerate,
					  primer->bwf_order, primer->bwf_cutoff, (bool)primer->bwf_scale,
					  primer->env_tightness,
					  primer->dzcdf_step, primer->dzcdf_sigma, primer->dzcdf_smooth,
					  primer->a, primer->b, primer->c);
	size_t found =
		pattern.find( valarray<float> (field, n_samples_field),
			      start,
			      inc);
	primer->match_a = pattern.match_a;
	primer->match_b = pattern.match_b;
	primer->match_c = pattern.match_c;

	return found;
}

size_t
signal_find_pattern_( struct SSignalPatternPrimer *primer,
		      const float *course,
		      const float *breadth,
		      const float *dzcdf,
		      size_t n_samples_field,
		      ssize_t start,
		      int inc)
{
	NSignal::CPattern<float> pattern (valarray<float> (primer->data, primer->n_samples),
					  primer->context_before, primer->context_after,
					  primer->samplerate,
					  primer->bwf_order, primer->bwf_cutoff, (bool)primer->bwf_scale,
					  primer->env_tightness,
					  primer->dzcdf_step, primer->dzcdf_sigma, primer->dzcdf_smooth,
					  primer->a, primer->b, primer->c);
	size_t found =
		pattern.find( valarray<float> (course,  n_samples_field),
			      valarray<float> (breadth, n_samples_field),
			      valarray<float> (dzcdf,   n_samples_field),
			      start,
			      inc);
	primer->match_a = pattern.match_a;
	primer->match_b = pattern.match_b;
	primer->match_c = pattern.match_c;

	return found;
}




double
signal_phasediff( const float *sig1, const float *sig2,
		  size_t samplerate,
		  size_t sa, size_t sz,
		  float fa, float fz,
		  unsigned order,
		  size_t scope)
{
	return NSignal::phase_diff( valarray<float> (&sig1[sa], sz-sa),
				    valarray<float> (&sig2[sa], sz-sa),
				    samplerate,
				    0, sz - sa,
				    fa, fz, order,
				    scope);
}

} // extern "C"


// EOF
