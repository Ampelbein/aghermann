/*
 *       File name:  common/config-validate.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-30
 *
 *         Purpose:  libconfig-bound validator
 *
 *         License:  GPL
 */

#ifndef _AGH_COMMON_CONFIG_VALIDATE_H
#define _AGH_COMMON_CONFIG_VALIDATE_H

#include <limits.h>

#include <forward_list>
#include <array>
#include <functional>
#include <stdexcept>

#include <libconfig.h++>

#include "string.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

namespace confval {

using namespace std;

template <typename T> libconfig::Setting::Type libconfig_type_id();

template <> libconfig::Setting::Type inline libconfig_type_id<bool>  	   () { return libconfig::Setting::Type::TypeBoolean; }
template <> libconfig::Setting::Type inline libconfig_type_id<int>   	   () { return libconfig::Setting::Type::TypeInt;     }
template <> libconfig::Setting::Type inline libconfig_type_id<size_t>	   () { return libconfig::Setting::Type::TypeInt64;   }
template <> libconfig::Setting::Type inline libconfig_type_id<const char*> () { return libconfig::Setting::Type::TypeString;  }
template <> libconfig::Setting::Type inline libconfig_type_id<string>	   () { return libconfig::Setting::Type::TypeString;  }
template <> libconfig::Setting::Type inline libconfig_type_id<double>	   () { return libconfig::Setting::Type::TypeFloat;   }
template <> libconfig::Setting::Type inline libconfig_type_id<float> 	   () { return libconfig::Setting::Type::TypeFloat;   }



inline
libconfig::Setting&
ensure_path( libconfig::Setting& S, libconfig::Setting::Type type, const string& key)
{
	auto pe = agh::str::tokens( key, ".");
	auto Si = &S;
	// ensure path
	for ( auto K = pe.begin(); K != pe.end(); ++K )
		if ( not Si->exists(*K) ) {
			Si = &Si->add( *K, (next(K) == pe.end()) ? type : libconfig::Setting::TypeGroup);
		} else
			Si = &(*Si)[*K];
	return *Si;
}


template <typename T>
void
put( libconfig::Config& C, const string& key, const T& value)
{
	ensure_path( C.getRoot(), libconfig_type_id<T>(), key) = value;
}
template <> // specialise for size_t
inline void
put( libconfig::Config& C, const string& key, const size_t& value)
{
	if ( value > INT_MAX )
		fprintf( stderr, "Value being saved is way too long for any practical purpose (unintialized?): %zu\n", value);
	ensure_path( C.getRoot(), libconfig_type_id<int>(), key) = (int)value;
}

template <typename T>
void
put( libconfig::Config& C, const string& key, const forward_list<T>& vl)
{
	auto& S = ensure_path( C.getRoot(), libconfig::Setting::Type::TypeList, key);
	for ( auto& V : vl )
		S.add( libconfig_type_id<T>()) = V;
}

template <typename T, size_t N>
void
put( libconfig::Config& C, const string& key, const array<T, N>& vl)
{
	auto& S = ensure_path( C.getRoot(), libconfig::Setting::Type::TypeList, key);
	for ( auto& V : vl )
		S.add( libconfig_type_id<T>()) = V;
}







template <typename T>
struct SValidator {
	string key;
	T* rcp;
	struct SVFTrue {
		bool operator() ( const T&) const { return true; }
	};
	struct SVFRangeEx {
	        T lo, hi;
		SVFRangeEx( const T& _lo, const T& _hi) : lo(_lo), hi(_hi) {};
		bool operator() ( const T& v) const { return v > lo && v < hi; }
	};
	struct SVFRangeIn {
	        T lo, hi;
		SVFRangeIn( const T& _lo, const T& _hi) : lo(_lo), hi(_hi) {};
		bool operator() ( const T& v) const { return v >= lo && v <= hi; }
	};
	function<bool(const T&)> valf;

	template <typename K>
	SValidator( const K& _key, T* _rcp)
	      : key (_key), rcp (_rcp), valf {SVFTrue()}
		{}
	template <typename K>
	SValidator( const K& _key, T* _rcp, function<bool (const T&)> _valf)
	      : key (_key), rcp (_rcp), valf (_valf)
		{}

	void get( const libconfig::Config& C) const
		{
			T tmp;
			if ( not C.lookupValue( key, tmp) ) {
				fprintf( stderr, "SValidator::get(): key %s not found\n", key.c_str());
				return; // leave at default
			}
			if ( not valf(tmp) )
				throw invalid_argument( string("Bad value for \"") + key + "\"");
			*rcp = tmp;
		}
	void put( libconfig::Config& C) const
		{
			confval::put( C, key, *rcp);
		}
};


// // specialise for FP types to have an additional isfinite check
// // is it obviously redundant?
// template <> inline bool SValidator<float>::SVFRangeIn::
// operator()( const float& v)
// { return isfinite(v) and v > lo && v < hi; }

// template <> inline bool SValidator<double>::SVFRangeIn::
// operator()( const float& v)
// { return isfinite(v) and v > lo && v < hi; }

// template <> inline bool SValidator<float>::SVFRangeEx::
// operator()( const float& v)
// { return isfinite(v) and v > lo && v < hi; }

// template <> inline bool SValidator<double>::SVFRangeEx::
// operator()( const float& v)
// { return isfinite(v) and v > lo && v < hi; }


template <>
inline void
SValidator<size_t>::get( const libconfig::Config& C) const
{
	int tmp; // libconfig doesn't deal in unsigned values
	if ( not C.lookupValue( key, tmp) ) {
		fprintf( stderr, "SValidator::get(): key %s not found\n", key.c_str());
		return; // leave at default
	}
	if ( not valf(tmp) )
		throw invalid_argument( string("Bad value for \"") + key + "\"");
	*rcp = tmp;
}


template <typename T>
void
get( forward_list<SValidator<T>>& vl,
     libconfig::Config& conf,
     bool nothrow = true)
{
	for ( auto& V : vl )
		if ( nothrow )
			try {
				V.get( conf);
			} catch ( exception& ex) {
				fprintf( stderr, "confval::get(list): %s\n", ex.what());
			}
		else
			V.get( conf);
}

template <typename T>
void
put( forward_list<SValidator<T>>& vl,
     libconfig::Config& conf)
{
	for ( auto& V : vl )
		V.put( conf);
}



} // namespace confval

#endif // _AGH_CONFIG_VALIDATOR_H

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
