// ;-*-C++-*- *  Time-stamp: "2010-12-29 03:15:43 hmmr"
/*
 *       File name:  signal.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-12-26
 *
 *         Purpose:  various standalone signal processing functions
 *
 *         License:  GPL
 */

#ifndef _AGH_SIGNAL_H
#define _AGH_SIGNAL_H

#include <vector>
#include <valarray>

#if HAVE_CONFIG_H
#  include "config.h"
#endif

using namespace std;

size_t	signal_envelope( const valarray<float>& filtered,
			 vector<size_t>& recp_l,
			 vector<size_t>& recp_u,
			 size_t tightness);

size_t	signal_breadth( const valarray<float>& signal,
			const vector<size_t>& env_u,
			const vector<size_t>& env_l,
			valarray<float>& sig_breadth);

int	low_pass( const valarray<float>& signal,
		  size_t samplerate,
		  size_t order, float cutoff, bool scale,
		  valarray<float>& out);


template <class T>
class CSignalPattern {
    private:
	CSignalPattern();

    public:
	size_t	samplerate;

      // the complete pattern signature is made of:
      // (a) course of the mean (low-freq component);
      // (b) instantaneous frequency at fine intervals;
      // (c) signal breadth at given tightness.

      // data for individual constituents of the pattern:
        // Butterworth low-pass filter
	size_t	bwf_order;
	float	bwf_cutoff;
	bool	bwf_scale;

        // ZC density function fields
	float 	zc_sigma;

        // envelope
	size_t	env_tightness;

	valarray<T>
		course,
		insta_freq,
		breadth;

	size_t size() const
		{
			return course.size();
		}

	CSignalPattern( const valarray<T>& pattern,
			size_t _samplerate,
			size_t _order, float _cutoff, bool _scale,
			float _sigma,
			size_t _tightness)
	      : samplerate (_samplerate),
		bwf_order (_order), bwf_cutoff (_cutoff), bwf_scale (_scale),
		zc_sigma (_sigma),
		env_tightness (_tightness)
		{
			::low_pass( pattern, samplerate,
				    bwf_order, bwf_cutoff, bwf_scale,
				    course);

			vector<size_t> env_u, env_l;
			signal_envelope( pattern, env_u, env_l, env_tightness);

			; // zc part

			signal_breadth( pattern, env_u, env_l, breadth);
		}
};

size_t	find_pattern( const CSignalPattern<float>& pattern,
		      valarray<float>& signal,
		      size_t start,
		      float tolerance);

#endif

// eof
