// ;-*-C++-*- *  Time-stamp: "2011-03-13 01:52:10 hmmr"

/*
 * Author: Andrei Zavada <johnhommer@gmail.com>
 *
 * License: GPL
 *
 * Initial version: 2010-04-29
 */


#ifndef _AGH_TUNABLE_H
#define _AGH_TUNABLE_H


#if HAVE_CONFIG_H
#  include "config.h"
#endif


#include <cstring>
#include <vector>
#include <valarray>

#include "../common.h"


using namespace std;



extern const STunableDescription __AGHTT[_agh_basic_tunables_];





class STunableSet {
    public:
	valarray<double>
		P;

	STunableSet( size_t n_egc = 0)
	      : P (_agh_basic_tunables_+ n_egc)
		{}
	STunableSet( size_t n_egc, const double* rv)
	      : P (_agh_basic_tunables_ + n_egc)
		{
			memcpy( &P[0], rv, P.size() * sizeof(double));
		}
	STunableSet( const STunableSet &o)
		{
			P = o.P;
		}
	STunableSet& operator=( void* pp)
		{
			memcpy( &P[0], pp, P.size()*sizeof(double));
			return *this;
		}

	size_t size() const
		{
			return P.size();
		}

	double& operator[]( size_t t)
		{
			return P[t];
		}
	const double& operator[]( size_t t) const
		{
			return P[t];
		}

	void assign_defaults();
	// bool all_in_range() const;

	void adjust_for_ppm( double ppm);
	void unadjust_for_ppm( double ppm);

	valarray<double> normalize( const STunableSet& step) const
		{
			valarray<double> Px (P);
			for ( size_t t = 0; t < Px.size(); ++t )
				Px[t] = P[t] / step[t];
			return Px;
		}
	double distance( const STunableSet& rv, const STunableSet& step) const
		{
			return sqrt( pow( normalize(step) - rv.normalize(step), 2.).sum());
		}
};




class STunableSetFull {
    public:
	STunableSet
		value, step, lo, hi;

	static const int T_REQUIRED = 1;
	vector<int>
		state;

	size_t size() const
		{
			return value.size();
		}
	void resize( size_t n)
		{
			value.P.resize(n);
			step.P.resize(n);
			lo.P.resize(n);
			hi.P.resize(n);
		}

	STunableSetFull( size_t n_egc = 0)
	      : value (n_egc), step (n_egc), lo (n_egc), hi (n_egc),
		state (_gc_+1+ n_egc)
		{
			assign_defaults();
		}

	bool check_consisitent() const;
	void assign_defaults();

	void adjust_for_ppm( double ppm)
		{
			value.adjust_for_ppm( ppm);
			step.adjust_for_ppm( ppm);
			lo.adjust_for_ppm( ppm);
			hi.adjust_for_ppm( ppm);
		}
	void unadjust_for_ppm( double ppm)
		{
			value.unadjust_for_ppm( ppm);
			step.unadjust_for_ppm( ppm);
			lo.unadjust_for_ppm( ppm);
			hi.unadjust_for_ppm( ppm);
		}

	void randomise();

//	void dump();
};




#endif

// EOF
