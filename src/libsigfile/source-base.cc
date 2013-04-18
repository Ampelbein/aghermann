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
mark_artifact( double aa, double az)
{
	if ( aa >= az )
		return;
	obj.emplace_back( aa, az);
	obj.sort();
	auto A = obj.begin();
	while ( next(A) != obj.end() ) {
		if ( agh::alg::overlap(A->a, A->z, next(A)->a, next(A)->z) ) {
			A->z = max( A->z, next(A)->z);
			obj.erase( next(A));
			continue;
		}
		++A;
	}
}



void
sigfile::SArtifacts::
clear_artifact( double aa, double az)
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
region_dirty_fraction( double ra, double rz) const
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
	return dirty / (rz - ra);
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


unsigned long
sigfile::SFilterPack::
dirty_signature() const
{
	DEF_UNIQUE_CHARP (tmp);
	ASPRINTF( &tmp, "%g%d%g%d%d",
		  low_pass_cutoff, low_pass_order, high_pass_cutoff, high_pass_order, (int)notch_filter);
	return hash<std::string>() (tmp);
}







sigfile::CSource::
CSource (CSource&& rv)
      : _subject (move(rv._subject))
{
	swap( _filename, rv._filename);
	_status = rv._status;
	_flags = rv._flags;
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

