// ;-*-C++-*-
/*
 *       File name:  common/alg.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-25
 *
 *         Purpose:  misc supporting algorithms
 *
 *         License:  GPL
 */

#ifndef _AGH_COMMON_ALG_H
#define _AGH_COMMON_ALG_H

#include <valarray>
#include <list>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {
namespace alg {

template <typename T>
inline void
pod_swap( T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}



template <typename T>
inline bool
overlap( const T& a, const T& b,
	 const T& c, const T& d)
{
	return not ((a < c && b < c) || (a > d && b > d));
}

template <typename T>
bool
between( const T& a, const T& b, const T&c)
{
	return a <= b && b <= c;
}



// using TRegion = class pair<size_t, size_t>;  // come gcc 4.7, come!
template <typename T>
struct SSpan {
	T a, z;
	SSpan (const T& a_, const T& z_)
	      : a (a_), z (z_)
		{}
	bool operator==( const SSpan<T>& rv) const
		{
			return a == rv.a && z == rv.z;
		}
	bool operator<( const SSpan<T>& rv) const
		{
			return a < rv.a;
		}
	T
	size() const
		{
			return z - a;
		}
	template <typename U>
	SSpan<U>
	operator*( const U& f) const
		{
			return {(U)(a * f), (U)(z * f)};
		}
	template <typename U>
	SSpan<U>
	operator/( const U& f) const
		{
			return {(U)(a / f), (U)(z / f)};
		}
	float
	dirty( const SSpan<T>& b) const
		{
			if ( between(a, b.a, z) && between(b.a, z, b.z) )      // aa .. ba .. az .. bz
				return (float)(z - b.a) / size();
			else if ( between(b.a, a, b.z) && between(a, b.z, z) ) // ba .. aa .. bz .. az
				return (float)(b.z - a) / size();
			else if ( between(a, b.a, z) && between(a, b.z, z) )   // b entirely inside a
				return (float)b.size() / size();
			else if ( between(b.a, a, b.z) && between(b.a, z, b.z) ) // a entirely inside b
				return 1.f;
			else
				return 0.f;
		}

	float
	dirty( const list<SSpan<T>>& B) const
		{
			float q = 0.;
			for ( auto& b : B )
				q += dirty(b);
			return q;
		}

};







template <typename T>
void
ensure_within( T& v, const T& l, const T& h)
{
	if ( v < l )
		v = l;
	else if ( v > h )
		v = h;
}

template <typename T>
T
value_within( const T& v, const T& l, const T& h)
{
	T o {v};
	if ( v < l )
		o = l;
	else if ( v > h )
		o = h;
	return o;
}



inline valarray<double>
to_vad( valarray<double>&& rv)
{
	return move(rv);
}

inline valarray<double>
to_vad( const valarray<float>& rv)
{
	valarray<double> ret;
	ret.resize( rv.size());
	for ( size_t i = 0; i < rv.size(); ++i )
		ret[i] = rv[i];
	return ret;
}




inline float
__attribute__ ((pure))
calibrate_display_scale( const valarray<TFloat>& signal,
			 size_t over, float fit)
{
	return fit / (abs(signal[ slice (0, over, 1) ]).sum() / over) / 8;
}


double sensible_scale_reduction_factor( double display_scale,
					double constraint_max, double constraint_min = 8.);  // 8 pixels

} // namespace alg
} // namespace agh

#endif // _AGH_COMMON_ALG_H

// eof
