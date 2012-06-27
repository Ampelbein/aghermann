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

using namespace std;

void
sigfile::SArtifacts::
mark_artifact( size_t aa, size_t az)
{
	if ( aa >= az )
		return;
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
sigfile::SArtifacts::
clear_artifact( size_t aa, size_t az)
{
	auto A = obj.begin();
	while ( A != obj.end() ) {
		if ( aa <= A->first && A->second <= az ) {
			obj.erase( A++);
			continue;
		}
		if ( A->first < aa && az < A->second ) {
			obj.emplace( next(A), az, A->second);
			A->second = aa;
			break;
		}
		if ( A->first < aa && aa < A->second )
			A->second = aa;
		if ( A->first < az && az < A->second )
			A->first = az;
		++A;
	}
}





float
__attribute__ ((pure))
sigfile::SArtifacts::
region_dirty_fraction( size_t ra, size_t rz) const
{
	size_t	dirty = 0;
	for ( auto& A : obj ) {
		if ( ra > A.second )
			continue;
		if ( rz < A.first )
			break;

		if ( A.first < ra && A.second > rz )
			return 1.;
		if ( A.first > ra && A.second < rz ) {
			dirty += (A.second - A.first);
			continue;
		}

		if ( A.first < ra )
			dirty = (A.second - ra);
		else {
			dirty += (A.second - rz);
			break;
		}
	}
	return (float)dirty / (rz - ra);
}


agh::hash_t
sigfile::SArtifacts::
dirty_signature() const
{
	string sig ("a");
	for ( auto &A : obj )
		sig += (to_string((long long int)A.first) + ':' + to_string((long long int)A.second));
	sig += to_string(factor) + to_string( (long long int)dampen_window_type);
	return HASHKEY (sig);
}






sigfile::CSource_base::
CSource_base( CSource_base&& rv)
{
	swap( _filename, rv._filename);
	_status = rv._status;
	_flags = rv._flags;
}


// eof
