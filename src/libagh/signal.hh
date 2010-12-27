// ;-*-C++-*- *  Time-stamp: "2010-12-27 02:44:10 hmmr"
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
		  float cutoff,
		  valarray<float>& out);


template <class T>
class CSignalPattern {
    private:
	CSignalPattern();

    public:
	size_t	samplerate;
	float	cutoff,
		sigma;
	size_t	tightness;

	valarray<T>
		course,
		insta_freq,
		breadth;

	size_t size() const
		{
			return course.size();
		}

      // the complete pattern signature is made of:
      // (a) course of the mean (low-freq component);
      // (b) instantaneous frequency at fine intervals;
      // (c) signal breadth at given tightness.
	CSignalPattern( const valarray<T>& pattern,
			size_t _samplerate,
			float _cutoff,
			float _sigma,
			size_t _tightness)
	      : samplerate (_samplerate),
		cutoff (_cutoff), sigma (_sigma), tightness (_tightness)
		{
			low_pass( pattern, samplerate, cutoff, course);

			vector<size_t> env_u, env_l;
			signal_envelope( pattern, env_u, env_l, tightness);

			// signal breadth
			signal_breadth( pattern, env_u, env_l, breadth);
		}
};

size_t	find_pattern( const CSignalPattern<float>& pattern,
		      valarray<float>& signal,
		      size_t start,
		      float tolerance);

#endif

// eof
