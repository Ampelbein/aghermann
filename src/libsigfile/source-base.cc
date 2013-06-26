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


#include <fstream>
#include "common/string.hh"
#include "source-base.hh"

using namespace std;

void
sigfile::SArtifacts::
mark_artifact( const double aa, const double az)
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
clear_artifact( const double aa, const double az)
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
region_dirty_fraction( const double ra, const double rz) const
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
//	DEF_UNIQUE_CHARP (tmp);
	char *tmp;
	ASPRINTF( &tmp, "%g%d%g%d%d",
		  low_pass_cutoff, low_pass_order, high_pass_cutoff, high_pass_order, (int)notch_filter);
	string t2 {tmp};
	free( tmp);
	return hash<string>() (t2);
}





int
sigfile::CSource::
load_ancillary_files()
{
	int retval = 0;

	for ( int h = 0; h < (int)n_channels(); ++h ) {
		auto& H = channel_by_id(h);

	      // 1. artifacts
		{
			ifstream thomas (make_fname_artifacts( H));
			if ( not thomas.good() )
				goto step2;

			auto& AA = artifacts(h);
			while ( !thomas.eof() ) {
				double aa = NAN, az = NAN;
				thomas >> aa >> az;
				if ( not isfinite(aa) || not isfinite(az) ) {
					retval = -1;
					break;
				}
				AA.mark_artifact( aa, az);
			}
		}

	step2:
	      // 2. annotations
		{
			ifstream fd (make_fname_annotations( H));
			if ( not fd.good() )
				goto step3;

			auto& AA = annotations(h);
			while ( fd.good() and not fd.eof() ) {
				int type = -1;
				double aa = NAN, az = NAN;
				string an;
				fd >> type >> aa >> az;
				getline( fd, an, SAnnotation::EOA);
				if ( isfinite(aa) and isfinite(az) and
				     aa < az and az <= recording_time()
				     and type < SAnnotation::TType_total and type >= 0 )
					AA.emplace_back(
						aa, az,
						agh::str::trim(an),
						(SAnnotation::TType)type);
				else {
					retval = -1;
					break;
				}
			}
			AA.sort();
		}
	step3:
		;
	}

      // 3. filters
	{
		ifstream thomas (make_fname_filters(_filename));
		if ( !thomas.good() )
			for ( int h = 0; h < (int)n_channels(); ++h ) {
				auto& AA = filters(h);

				unsigned lpo = -1, hpo = -1, nf = -1;
				double lpc = 0., hpc = 0.;
				thomas >> lpc >> lpo
				       >> hpc >> hpo >> nf;
				AA = {lpc, lpo, hpc, hpo, (SFilterPack::TNotchFilter)nf};
				if ( not AA.is_valid() )
					AA.reset();
			}
	}

	return retval;
}





int
sigfile::CSource::
save_ancillary_files()
{
	int retval = 0;
	for ( int h = 0; h < (int)n_channels(); ++h ) {
		auto& H = channel_by_id(h);
		{
			auto& AA = artifacts(h);
			if ( not AA.empty() ) {
				ofstream thomas (make_fname_artifacts( H), ios_base::trunc);
				for ( auto &A : AA() )
					thomas << A.a << ' ' << A.z << endl;
				if ( not thomas.good() )
					retval = -1;
			} else
				if ( unlink( make_fname_artifacts( H).c_str()) ) {}
		}

		{
			auto& AA = annotations(h);

			auto fname = make_fname_annotations( H);

			if ( not AA.empty() ) {
				ofstream thomas (fname, ios_base::trunc);
				for ( auto &A : AA ) {
					thomas << (int)A.type << ' '
					       << A.span.a << ' ' << A.span.z << ' '
					       << A.label << SAnnotation::EOA << endl;
					if ( not thomas.good() )
						retval = -1;
				}

			} else
				if ( unlink( fname.c_str()) ) {}
		}
	}
	ofstream thomas (make_fname_filters( filename()), ios_base::trunc);
	if ( thomas.good() )
		for ( int h = 0; h < (int)n_channels(); ++h ) {
			auto& AA = filters(h);
			thomas << AA.low_pass_cutoff << ' ' << AA.low_pass_order << ' '
			       << AA.high_pass_cutoff << ' ' << AA.high_pass_order << ' '
			       << (int)AA.notch_filter << endl;
			if ( not thomas.good() )
				retval = -1;
		}

	return retval;
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
// tab-width: 8
// End:
