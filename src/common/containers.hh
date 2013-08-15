/*
 *       File name:  common/containers.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-04-25
 *
 *         Purpose:  misc supporting algorithms (containers)
 *
 *         License:  GPL
 */

#ifndef _AGH_COMMON_CONTAINERS_H
#define _AGH_COMMON_CONTAINERS_H

#include <list>
#include <forward_list>
#include <vector>
#include <map>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {
namespace alg {

template <typename T>
bool
member( const T& x, const list<T>& v)
{
	return any( v.begin(), v.end(), x);
}

template <typename T>
bool
member( const T& x, const forward_list<T>& v)
{
	return any( v.begin(), v.end(), x);
}

template <typename T>
bool
member( const T& x, const vector<T>& v)
{
	return any( v.begin(), v.end(), x);
}

template <typename K, typename V>
bool
member( const K& x, const map<K, V>& m)
{
	return m.find(x) != m.end();
}

} // namespace alg
} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
