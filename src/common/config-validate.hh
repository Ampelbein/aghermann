// ;-*-C++-*-
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

#ifndef _AGH_CONFIG_VALIDATOR_H
#define _AGH_CONFIG_VALIDATOR_H

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

template <> libconfig::Setting::Type inline libconfig_type_id<bool>  	  	  () { return libconfig::Setting::Type::TypeBoolean; }
template <> libconfig::Setting::Type inline libconfig_type_id<int>   	  	  () { return libconfig::Setting::Type::TypeInt;     }
template <> libconfig::Setting::Type inline libconfig_type_id<size_t>	  	  () { return libconfig::Setting::Type::TypeInt64;   }
template <> libconfig::Setting::Type inline libconfig_type_id<const char*>	  () { return libconfig::Setting::Type::TypeString;  }
template <> libconfig::Setting::Type inline libconfig_type_id<string>	  	  () { return libconfig::Setting::Type::TypeString;  }
template <> libconfig::Setting::Type inline libconfig_type_id<double>	  	  () { return libconfig::Setting::Type::TypeFloat;   }
template <> libconfig::Setting::Type inline libconfig_type_id<float> 	  	  () { return libconfig::Setting::Type::TypeFloat;   }



inline
libconfig::Setting&
ensure_path( libconfig::Setting& S, libconfig::Setting::Type type, const string& key)
{
	auto pe = string_tokens( key, ".");
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
put( libconfig::Setting& S0, const string& key, const T& value)
{
	ensure_path( S0, libconfig_type_id<T>(), key) = value;
}

template <typename T>
void
put( libconfig::Setting& S0, const string& key, const forward_list<T>& vl)
{
	auto& S = ensure_path( S0, libconfig_type_id<T>(), key);
	for ( auto& V : vl )
		S.add( libconfig_type_id<T>()) = V;
}

template <typename T, size_t N>
void
put( libconfig::Setting& S0, const string& key, const array<T, N>& vl)
{
	auto& S = ensure_path( S0, libconfig_type_id<T>(), key);
	for ( auto& V : vl )
		S.add( libconfig_type_id<T>()) = V;
}







template <typename T>
struct SValidator {
	const char *key;
	T* rcp;
	struct SVFTrue {
		bool operator() ( const T& any) const { return true; }
	};
	struct SVFRange {
	        T lo, hi;
		SVFRange( const T& _lo, const T& _hi) : lo(_lo), hi(_hi) {};
		bool operator() ( const T& v) const { return v > lo && v < hi; }
	};
	function<bool(const T&)> valf;

	SValidator( const char* _key, T* _rcp)
	      : key (_key), rcp (_rcp), valf {SVFTrue()}
		{}
	SValidator( const char* _key, T* _rcp, function<bool (const T&)> _valf)
	      : key (_key), rcp (_rcp), valf (_valf)
		{}

	void get( const libconfig::Config& S) const
		{
			get( S.getRoot());
		}
	void get( const libconfig::Setting& S) const
		{
			if ( not S.lookupValue( key, *rcp) )
				return; // leave at default
			if ( valf(*rcp) )
				throw invalid_argument( string("Bad value for \"") + key + "\"");
		}
	void put( libconfig::Config& C) const
		{
			put( C.getRoot());
		}
	void put( libconfig::Setting& S) const
		{
			confval::put( S, key, *rcp);
		}
};


template <typename T>
void
get( forward_list<SValidator<T>>& vl,
     libconfig::Setting& root,
     bool nothrow = true)
{
	for ( auto& V : vl )
		if ( nothrow )
			try {
				V.get( root);
			} catch (...) {
				; //printf( "CExpDesign::load_settings(): %s\n", ex.what());
			}
		else
			V.get( root);
}

template <typename T>
void
put( forward_list<SValidator<T>>& vl,
     libconfig::Setting& root)
{
	for ( auto& V : vl )
		V.put( root);
}



} // namespace confval

#endif // _AGH_CONFIG_VALIDATOR_H

// eof
