// ;-*-C++-*-
/*
 *       File name:  libsigfile/source-base.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-13
 *
 *         Purpose:  base class for various biosignals (edf, edf+ etc)
 *
 *         License:  GPL
 */


#include "source-base.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;

void
sigfile::SArtifacts::mark_artifact( size_t aa, size_t az)
{
	obj.emplace_back( aa, az);
	obj.sort();
startover:
	for ( auto A = obj.begin(); A != obj.end(); ++A )
		if ( next(A) != obj.end()
		     && A->second >= next(A)->first ) {
			A->second = max( A->second, next(A)->second);
			obj.erase( next(A));
			goto startover;
		 }
 }


void
sigfile::SArtifacts::clear_artifact( size_t aa, size_t az)
{
startover:
	for ( auto A = obj.begin(); A != obj.end(); ++A ) {
		if ( aa < A->first && A->second < az ) {
			obj.erase( A);
			goto startover;
		}
		if ( A->first < aa && az < A->second ) {
			obj.emplace( next(A), az, A->second);
			A->second = aa;
			break;
		}
		if ( A->first < aa && aa < A->second ) {
			A->second = aa;
		}
		if ( A->first < az && az < A->second ) {
			A->first = az;
		}
	}
}


size_t
sigfile::SArtifacts::dirty_signature() const
{
	string sig ("a");
	for ( auto &A : obj )
		sig += (to_string((long long int)A.first) + ':' + to_string((long long int)A.second));
	sig += to_string(factor) + to_string( (long long int)dampen_window_type);
	return HASHKEY (sig);
}




// eof
