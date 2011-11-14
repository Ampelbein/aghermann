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
	artifacts.emplace_back( aa, az);
	artifacts.sort();
startover:
	for ( auto A = artifacts.begin(); A != artifacts.end(); ++A )
		if ( next(A) != artifacts.end()
		     && A->second >= next(A)->first ) {
			A->second = max( A->second, next(A)->second);
			artifacts.erase( next(A));
			goto startover;
		 }
 }


void
sigfile::clear_artifact( list<TRegion>& artifacts,
			 size_t aa, size_t az)
{
startover:
	for ( auto A = artifacts.begin(); A != artifacts.end(); ++A ) {
		if ( aa < A->first && A->second < az ) {
			artifacts.erase( A);
			goto startover;
		}
		if ( A->first < aa && az < A->second ) {
			artifacts.emplace( next(A), az, A->second);
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
sigfile::SArtifacts::dirty_signature()
{
	string sig ("a");
	for ( auto &A : artifacts )
		sig += (to_string((long long int)A.first) + ':' + to_string((long long int)A.second));
	sig += to_string(factor) + to_string( (long long int)dampen_window_type);
	return HASHKEY (sig);
}





// eof
