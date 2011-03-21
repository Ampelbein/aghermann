// ;-*-C++-*- *  Time-stamp: "2011-03-21 01:22:33 hmmr"
/*
 *       File name:  libagh/tunable.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-29
 *
 *         Purpose:  tunable classes
 *
 *         License:  GPL
 */


#ifndef _AGH_TUNABLE_H
#define _AGH_TUNABLE_H


#if HAVE_CONFIG_H
#  include "config.h"
#endif


#include <cassert>
#include <cstring>
#include <vector>
#include <valarray>

#include "../common.h"


using namespace std;



extern const STunableDescription __AGHTT[_agh_basic_tunables_];



namespace NSSiman {
	void _siman_print(void*);
};

class STunableSet {
    friend class STunableSetFull;
    friend class CModelRun;
    friend class CExpDesign;
    friend void NSSiman::_siman_print( void*);

    protected:
	valarray<double>
		P;

	STunableSet( STunableSet&& rv)
		{
			swap( P, rv.P);
		}

	STunableSet( size_t n_egc = 1)
	      : P (_agh_basic_tunables_ + n_egc - 1)
		{}
	STunableSet( size_t n_egc, const double* rv)
	      : P (_agh_basic_tunables_ + n_egc - 1)
		{
			memcpy( &P[0], rv, P.size() * sizeof(double));
		}
	STunableSet( const STunableSet &o, size_t n_egc)
	      : P (_agh_basic_tunables_ + n_egc - 1)
		{
		      // we are certain about this far
			assert (n_egc > 0);
		      // theoreticlly, o.P has all egc's
			if ( o.P.size() == P.size() ) {
//				printf( "o.P.size() eq = %zu\n", o.P.size());
				P = o.P;
			} else {
		      // except when initilising from a basic set
//				printf( "o.P.size() = %zu\n", o.P.size());
				P[ slice(0, _agh_basic_tunables_, 1) ] = o.P[ slice(0, _agh_basic_tunables_, 1)];
				if ( n_egc > 1 )
					P[ slice(_gc_+1, n_egc-1, 1) ] = o.P[_gc_];
			}
		}
	STunableSet& operator=( void* pp)
		{
			memcpy( &P[0], pp, P.size()*sizeof(double));
			return *this;
		}

    public:
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
			state.resize(n);
		}

	STunableSetFull( STunableSetFull&& rv)
	      : value ((STunableSet&&)rv.value),
		step ((STunableSet&&)rv.step),
		lo ((STunableSet&&)rv.lo),
		hi ((STunableSet&&)rv.hi)
		{
			swap( state, rv.state);
		}

	STunableSetFull( size_t n_egc = 1)
	      : value (n_egc), step (n_egc), lo (n_egc), hi (n_egc),
		state (_agh_basic_tunables_+ n_egc-1)
		{
			assign_defaults();
		}
	STunableSetFull( const STunableSetFull& t0, size_t n_egc = 1)
	      : value (t0.value, n_egc), step (t0.step, n_egc), lo (t0.lo, n_egc), hi (t0.hi, n_egc),
		state (_agh_basic_tunables_+ n_egc-1)
		{}

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
};




inline string tunable_name( size_t t)
{
	if ( t < _agh_basic_tunables_ )
		return __AGHTT[t].name;
	else if ( t < _agh_n_tunables_ )
		return string("gc") + to_string(t);
	else
		return "BAD_TUNABLE";
}


#endif

// EOF
