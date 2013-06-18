/*
 *       File name:  aghermann/model/achermann-tunable.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-29
 *
 *         Purpose:  tunable classes
 *
 *         License:  GPL
 */


#ifndef AGH_AGHERMANN_MODEL_ACHERMANN_TUNABLE_H_
#define AGH_AGHERMANN_MODEL_ACHERMANN_TUNABLE_H_

#include <cstring>
#include <vector>
#include <array>
#include <valarray>
#include <string>


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {
namespace ach {

using namespace std;


enum TTunable {
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



enum class TTRole {
	v, d, l, u
};


struct STunableDescription {
	double	def_val, def_min, def_max, def_step;
	float	display_scale_factor,
		adj_step;
	bool	is_required;
	int	time_adj;
	const char
		*name,
		*fmt,
		*unit,
		*pango_name,
		*human_name,
		*description;
};
extern const STunableDescription
	stock[TTunable::_basic_tunables];

string tunable_name( size_t);
string tunable_pango_name( size_t);
string tunable_unit( size_t);



template <TTRole Of = TTRole::v>
struct STunableSet {
    public:
	double	P[TTunable::_all_tunables];
	size_t	n_egc;

	STunableSet (const STunableSet&) = default;
	STunableSet (size_t n_egc_ = 0)
	      : n_egc (n_egc_)
		{
			set_defaults();
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

	int check() const;

	void set_defaults();

	void adjust_for_ppm( double ppm)
		{
			for ( size_t t = 0; t < size(); ++t )
				P[t] *= pow( ppm, stock[min(t, (size_t)TTunable::gc)].time_adj);
		}

	void unadjust_for_ppm( double ppm)
		{
			for ( size_t t = 0; t < size(); t++ )
				P[t] /= pow( ppm, stock[min(t, (size_t)TTunable::gc)].time_adj);
		}

	valarray<double>
	normalize( const double fac[]) const
		{
			valarray<double> Px (size());
			for ( size_t t = 0; t < size(); ++t )
				Px[t] = P[t] / fac[t];
			return Px;
		}
};





inline double
distance( const STunableSet<TTRole::v>& lv,
	  const STunableSet<TTRole::v>& rv,
	  const double f[])
{
	return sqrt( pow( lv.normalize(f) - rv.normalize(f), 2.).sum());
}



struct STunableSetWithState
  : public STunableSet<TTRole::v> {

    public:
	const STunableSet<TTRole::d>& d;
	const STunableSet<TTRole::l>& l;
	const STunableSet<TTRole::u>& u;

	array<int, TTunable::_all_tunables>
 		state;

	STunableSetWithState (const STunableSet<TTRole::v>& v_,
			      const STunableSet<TTRole::d>& d_,
			      const STunableSet<TTRole::l>& l_,
			      const STunableSet<TTRole::u>& u_)
	      : STunableSet (v_), d (d_), l (l_), u (u_)
		{}
	STunableSetWithState (const STunableSet<TTRole::d>& d_,
			      const STunableSet<TTRole::l>& l_,
			      const STunableSet<TTRole::u>& u_)
	      : STunableSet (d_.n_egc), d (d_), l (l_), u (u_)
		{}
	STunableSetWithState (const STunableSetWithState&) = default;

	double range( TTunable t) const
		{
			return u[(size_t)t] - l[(size_t)t];
		}

	void center();
	void randomise();
};





} // namespace ach
} // namespace agh

#endif // AGH_AGHERMANN_MODEL_ACHERMANN_TUNABLE_H_

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
