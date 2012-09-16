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
		     && A->a >= next(A)->a ) {
			A->z = max( A->z, next(A)->z);
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
		if ( aa <= A->a && A->z <= az ) {
			obj.erase( A++);
			continue;
		}
		if ( A->a < aa && az < A->z ) {
			obj.emplace( next(A), az, A->z);
			A->z = aa;
			break;
		}
		if ( A->a < aa && aa < A->z )
			A->z = aa;
		if ( A->a < az && az < A->z )
			A->a = az;
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
		if ( ra > A.z )
			continue;
		if ( rz < A.a )
			break;

		if ( A.a < ra && A.z > rz )
			return 1.;
		if ( A.a > ra && A.z < rz ) {
			dirty += (A.z - A.a);
			continue;
		}

		if ( A.a < ra )
			dirty = (A.z - ra);
		else {
			dirty += (A.z - rz);
			break;
		}
	}
	return (float)dirty / (rz - ra);
}


unsigned long
sigfile::SArtifacts::
dirty_signature() const
{
	string sig ("a");
	for ( auto &A : obj )
		sig += (to_string((long long int)A.a) + ':' + to_string((long long int)A.z));
	sig += to_string(factor) + to_string( (long long int)dampen_window_type);
	return hash<std::string>() (sig);
}






sigfile::CSource_base::
CSource_base( CSource_base&& rv)
{
	swap( _filename, rv._filename);
	_status = rv._status;
	_flags = rv._flags;
}


// eof
