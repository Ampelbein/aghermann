// ;-*-C++-*-
/*
 *       File name:  model/tunable.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-29
 *
 *         Purpose:  tunable classes
 *
 *         License:  GPL
 */


#ifndef _AGH_MODEL_TUNABLE_H
#define _AGH_MODEL_TUNABLE_H

#include <cstring>
#include <vector>
#include <valarray>
#include <string>


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {
namespace ach {

using namespace std;


enum TTunable : unsigned short {
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



enum class TTIdx : unsigned short {
	val,
	min,
	max,
	step
};





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
			*pango_name,
			*human_name,
			*description;
	};
	static const STunableDescription stock[TTunable::_basic_tunables];

	static string
	tunable_name( size_t t);

	static string
	tunable_pango_name( size_t t);

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
	STunableSet( const STunableSet &o, size_t n_egc);
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
			return (TTunable) (P.size()-1);
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

	void check() const;  // throws
	void reset();

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
			reset();
		}
	STunableSetFull( const STunableSetFull& t0, size_t n_egc = 1)
	      : value (t0.value, n_egc), step (t0.step, n_egc), lo (t0.lo, n_egc), hi (t0.hi, n_egc),
		state ((size_t)TTunable::_basic_tunables+ n_egc-1)
		{}

	void check() const;
	void reset();

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





} // namespace ach
} // namespace agh

#endif

// eof
