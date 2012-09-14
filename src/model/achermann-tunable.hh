// ;-*-C++-*-
/*
 *       File name:  model/achermann-tunable.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-29
 *
 *         Purpose:  tunable classes
 *
 *         License:  GPL
 */


#ifndef _AGH_MODEL_ACHERMANN_TUNABLE_H
#define _AGH_MODEL_ACHERMANN_TUNABLE_H

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





class STunableSet {
    public:
      // static members
	struct SDescription {
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
	static const SDescription stock[TTunable::_basic_tunables];

	static string
	tunable_name( size_t t);

	static string
	tunable_pango_name( size_t t);

      // object
	double	P[TTunable::_all_tunables];
	size_t	n_egc;

	STunableSet (const STunableSet&) = default;
	STunableSet (size_t n_egc_ = 0)
	      : n_egc (n_egc_)
		{
			reset();
		}

	STunableSet& operator=( const STunableSet&) = default;

	size_t size() const
		{
			return TTunable::_basic_tunables + n_egc;
		}

	double& operator[]( size_t t)
		{
			return P[t];
		}
	const double& operator[]( size_t t) const
		{
			return P[t];
		}

	void check() const;  // throws
	void reset();

	void adjust_for_ppm( double ppm);
	void unadjust_for_ppm( double ppm);

	valarray<double>
	normalize( const STunableSet& step) const
		{
			valarray<double> Px (size());
			for ( size_t t = 0; t < size(); ++t )
				Px[t] = P[t] / step[t];
			return Px;
		}
};


inline double
distance( const STunableSet& lv, const STunableSet& rv, const STunableSet& step)
{
	return sqrt( pow( lv.normalize(step) - rv.normalize(step), 2.).sum());
}



class STunableSetWithState
  : public STunableSet {

    public:
	const STunableSet&
		step, lo, hi;

	array<int, TTunable::_all_tunables>
 		state;

	STunableSetWithState (const STunableSet& step_, const STunableSet& lo_, const STunableSet& hi_)
	      : step (step_), lo (lo_), hi (hi_)
		{}
	STunableSetWithState (const STunableSetWithState&) = default;

	void randomise();
};





} // namespace ach
} // namespace agh

#endif

// eof
