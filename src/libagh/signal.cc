// ;-*-C++-*- *  Time-stamp: "2010-12-27 13:54:06 hmmr"
/*
 *       File name:  signal.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-12-26
 *
 *         Purpose:  various standalone signal processing functions
 *
 *         License:  GPL
 */

#include "signal.hh"



size_t
signal_envelope( const valarray<float>& filtered,
		 vector<size_t>& env_l,
		 vector<size_t>& env_u,
		 size_t over)
{
	size_t	i, j, dh = (over-1)/2+1,
		n_samples = filtered.size();

	env_l.resize( 0);
	env_u.resize( 0);

	for ( i = dh; i < n_samples-dh; ++i ) {
		for ( j = 1; j <= dh; ++j )
			if ( filtered[i-j] <= filtered[i] )  // [i] is not a local min
				goto inner_continue;
		for ( j = 1; j <= dh; ++j )
			if ( filtered[i+j] <= filtered[i] )  // [i] is not
				goto inner_continue;
		env_l.push_back( i);
		continue;
	inner_continue:
		for ( j = 1; j <= dh; ++j )
			if ( filtered[i-j] >= filtered[i] )  // [i] is not a local max
				goto outer_continue;
		for ( j = 1; j <= dh; ++j )
			if ( filtered[i+j] >= filtered[i] )  // [i] is not
				goto outer_continue;
		env_u.push_back( i);
	outer_continue:
		;
	}

	return env_u.size();
}


size_t
signal_breadth( const valarray<float>& signal,
		const vector<size_t>& env_u,
		const vector<size_t>& env_l,
		valarray<float>& sig_breadth)
{
	size_t	ia = max( *env_u. begin(), *env_l. begin()),
		iz = min( *env_u.rbegin(), *env_l.rbegin());
	sig_breadth.resize( signal.size());

	auto Iu = env_u.begin(), Il = env_l.begin();
	for ( size_t i = ia; i < iz; ++i ) {
		float	frac_u = (float)(i - *Iu)/(*next(Iu) - *Iu),
			frac_l = (float)(i - *Il)/(*next(Il) - *Il);
		float	dyu = signal[*next(Iu)] - signal[*Iu],
			dyl = signal[*next(Il)] - signal[*Il];
		sig_breadth[i] =
			 (signal[*Iu] + frac_u * dyu) +
			-(signal[*Il] + frac_l * dyl);
		if ( i == *Iu )
			++Iu;
		if ( i == *Il )
			++Il;
	}

	return iz - ia;
}



int
low_pass( const valarray<float>& signal,
	  size_t samplerate,
	  float cutoff,
	  valarray<float>& out)
{
	return 0;
}



size_t
find_pattern( const CSignalPattern<float>& pattern,
	      valarray<float>& signal,
	      size_t start,
	      float tolerance)
{
      // low-pass signal being searched, too
	valarray<float> course;
	low_pass( signal, pattern.samplerate, pattern.cutoff,
		  course);

      // prepare for comparison by other criteria:
	// signal breadth
	vector<size_t> env_u, env_l;
	signal_envelope( signal, env_u, env_l, pattern.tightness);

	// instantaneous rate
	;

	size_t diff;
	size_t iz = signal.size() - pattern.size();
	for ( size_t i = 0; i < iz; ++i ) {
		diff = 0;
		for ( size_t j = 0; j < pattern.size(); ++j )
			diff += fabs( pattern.course[j] - course[i+j]);
		float likeness = (float)diff / pattern.size();
	}

	return 0;
}

// eof
