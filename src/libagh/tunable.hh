// ;-*-C++-*-
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

#include <cassert>
#include <cstring>
#include <vector>
#include <valarray>
#include <string>


#if HAVE_CONFIG_H
#  include "config.h"
#endif


namespace agh {

using namespace std;


typedef unsigned short TTunable_underlying_type;
enum class TTunable : TTunable_underlying_type {
	rs,	rc,
	fcR,	fcW,
	S0,	SU,
	ta,	tp,
	gc,
	_basic_tunables,
	gc1 = gc,
	gc2,
	gc3,
	gc4,
	_all_tunables
};

inline TTunable
operator++( TTunable& b)
{
	TTunable retv = b;
	b = (TTunable) ((size_t)b+1);
	return retv;
}





enum class TTIdx : unsigned {
	val,
	min,
	max,
	step
};




template <class Int>
TTunable
__attribute__ ((const))
operator+( TTunable lv, Int rv)
{
	return (TTunable)((Int)lv + rv);
}

template <class Int>
int
__attribute__ ((const))
operator<( Int lv, TTunable rv)
{
	return (size_t)lv < (size_t)rv;
}

template <class Int>
int
__attribute__ ((const))
operator==( Int lv, TTunable rv)
{
	return (lv == (Int)rv);
}




namespace siman {
	void _siman_print(void*);
};

class STunableSet {
    public:
      // static members
	struct STunableDescription {
		double	def_val, def_min, def_max, def_step;
		float	display_scale_factor,
			adj_step;
		int	is_required;
		int	time_adj;
		const char
			*name,
			*fmt,
			*unit,
			*human_name,
			*description;
	};
	static const STunableDescription stock[(size_t)TTunable::_basic_tunables];

	template <class Int>
	static const string
	tunable_name( Int t)
		{
			if ( (TTunable_underlying_type)t < (TTunable_underlying_type)TTunable::_basic_tunables )
				return stock[(TTunable_underlying_type)t].name;
			else if ( (TTunable_underlying_type)t < (TTunable_underlying_type)TTunable::_all_tunables )
				return string("gc") + to_string((long long unsigned)t);
			else
				return "BAD_TUNABLE";
		}

      // object
    friend class STunableSetFull;
    friend class CModelRun;
    friend class CExpDesign;
    friend void siman::_siman_print( void*);

	STunableSet( const STunableSet& rv)
	      : P (rv.P)
		{}

    protected:
	valarray<double>
		P;

	STunableSet( STunableSet&& rv)
		{
			swap( P, rv.P);
		}

	STunableSet( size_t n_egc = 1)
	      : P ((size_t)TTunable::_basic_tunables + n_egc - 1)
		{}
	STunableSet( size_t n_egc, const double* rv)
	      : P ((size_t)TTunable::_basic_tunables + n_egc - 1)
		{
			memcpy( &P[0], rv, P.size() * sizeof(double));
		}
	STunableSet( const STunableSet &o, size_t n_egc)
	      : P ((size_t)TTunable::_basic_tunables + n_egc - 1)
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
				P[ slice(0, (size_t)TTunable::_basic_tunables, 1) ] =
					o.P[ slice(0, (size_t)TTunable::_basic_tunables, 1)];
				if ( n_egc > 1 )
					P[ slice((size_t)TTunable::gc+1, n_egc-1, 1) ] = o.P[(size_t)TTunable::gc];
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
	TTunable last() const
		{
			return (TTunable) ((TTunable_underlying_type)P.size()-1);
		}

	double& operator[]( TTunable t)
		{
			return P[(size_t)t];
		}
	double& operator[]( size_t t)
		{
			return P[t];
		}
	const double& operator[]( TTunable t) const
		{
			return P[(size_t)t];
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
		state ((size_t)TTunable::_basic_tunables+ n_egc-1)
		{
			assign_defaults();
		}
	STunableSetFull( const STunableSetFull& t0, size_t n_egc = 1)
	      : value (t0.value, n_egc), step (t0.step, n_egc), lo (t0.lo, n_egc), hi (t0.hi, n_egc),
		state ((size_t)TTunable::_basic_tunables+ n_egc-1)
		{}

	bool is_valid() const;
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





} // namespace agh

#endif

// EOF
