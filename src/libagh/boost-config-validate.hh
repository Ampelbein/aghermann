// ;-*-C++-*- *  Time-stamp: "2011-06-30 15:02:46 hmmr"
/*
 *       File name:  libagh/boost-config-validate.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-30
 *
 *         Purpose:  boost::property_tree validator
 *
 *         License:  GPL
 */

#ifndef _BOOST_PTREE_VALIDATOR_H
#define _BOOST_PTREE_VALIDATOR_H

#include <cstdio>
#include <list>
#include <functional>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;

template <class T>
struct SValidator {
	const char *key;
	T& rcp;
	struct SVFTrue {
		bool operator() ( const T& any) const { return true; }
	};
	struct SVFRange {
	        T lo, hi;
		SVFRange( const T& _lo, const T& _hi) : lo(_lo), hi(_hi) {};
		bool operator() ( const T& v) const { return v > lo && v < hi; }
	};
	function<bool(const T&)> valf;

	SValidator( const char* _key, T& _rcp)
	      : key (_key), rcp (_rcp), valf {SVFTrue()}
		{}
	SValidator( const char* _key, T& _rcp, function<bool (const T&)> _valf)
	      : key (_key), rcp (_rcp), valf (_valf)
		{}

	void get( boost::property_tree::ptree& pt)
		{
			using boost::property_tree::ptree;
			auto tmp = pt.get<T>( key);
			if ( valf(tmp) )
				throw invalid_argument( string("Bad value for \"") + key + "\"");
			rcp = tmp;
		}
};


template <class T>
void
get( list<SValidator<T>>& vl,
     boost::property_tree::ptree& pt,
     bool nothrow = true)
{
	for_each( vl.begin(), vl.end(),
		  [&] ( SValidator<T>& V)
		  {
			  if ( nothrow )
				  try {
					  V.get( pt);
				  } catch (...) {
					  ; //fprintf( stderr, "CExpDesign::load_settings(): %s\n", ex.what());
				  }
			  else
				  V.get( pt);
		  });
}




#endif

// eof
