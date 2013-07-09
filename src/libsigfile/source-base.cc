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
#include "libsigproc/sigproc.hh"
#include "source-base.hh"

using namespace std;
using namespace sigfile;

void
SArtifacts::
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
SArtifacts::
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
SArtifacts::
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
SArtifacts::
dirty_signature() const
{
	string sig ("a");
	for ( auto &A : obj )
		sig += (to_string((long long int)A.a) + ':' + to_string((long long int)A.z));
	sig += to_string(factor) + to_string( (long long int)dampen_window_type);
	return hash<std::string>() (sig);
}


unsigned long
SFilterPack::
dirty_signature() const
{
	char *tmp;
	ASPRINTF( &tmp, "%g%d%g%d%d",
		  low_pass_cutoff, low_pass_order, high_pass_cutoff, high_pass_order, (int)notch_filter);
	string t2 {tmp};
	free( tmp);
	return hash<string>() (t2);
}





int
CSource::
load_ancillary_files()
{
	int retval = 0;

	FAFA;
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

	FAFA;
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
	FAFA;

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

	FAFA;
	return retval;
}





int
CSource::
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








tuple<string, string, int>
CSource::
figure_session_and_episode()
{
	int status = 0;
	string session, episode;

	// (a) parsed from RecordingID_raw
	char int_session[81], int_episode[81];
	string rec_id_isolated (agh::str::trim( recording_id()));
#define T "%80[-a-zA-Z0-9 _]"
	if ( sscanf( rec_id_isolated.c_str(), T "," T,      int_episode, int_session) == 2 ||
	     sscanf( rec_id_isolated.c_str(), T ":" T,      int_session, int_episode) == 2 ||
	     sscanf( rec_id_isolated.c_str(), T "/"  T,     int_session, int_episode) == 2 ||
	     sscanf( rec_id_isolated.c_str(), T " (" T ")", int_session, int_episode) == 2 )
		;
	else
		status = 1;
#undef T

	// (b) identified from file name
	///// if ( sscanf( agh::fs::path_elements( filename).back().c_str(), "%*s-%d.%s", ) == 3 )
	size_t	basename_start = _filename.rfind( '/'),
		dot = _filename.rfind('.');
	string fn_episode =
		_filename.substr(
			basename_start + 1,
			dot - basename_start - 1);
	// chip away '-1' if present
	if ( fn_episode.size() >= 3 /* strlen("a-1") */ ) {
		size_t sz = fn_episode.size();
		if ( fn_episode[sz-2] == '-' && isdigit(fn_episode[sz-1]) )
			fn_episode.erase( sz-2, 2);
	}

	if ( status ) { // (a) failed
		episode.assign( fn_episode);    // use RecordingID_raw as Session
		session.assign( rec_id_isolated);
	} else {
		episode.assign( int_episode);
		session.assign( int_session);
	}

	return make_tuple( session, episode, status);
}









valarray<TFloat>
CSource::
get_region_filtered_smpl( const int h,
			  const size_t smpla, const size_t smplz) const
{
	valarray<TFloat> recp =
		get_region_original_smpl( h, smpla, smplz);
	if ( recp.size() == 0 )
		return valarray<TFloat> (0);
	// and zeromean
       	recp -= (recp.sum() / recp.size());

	size_t this_samplerate = samplerate(h);

      // artifacts
	const auto& AA = artifacts(h);
	for ( const auto& A : AA() ) {
		size_t	Aa = A.a * this_samplerate,
			Az = A.z * this_samplerate;
		if ( unlikely (Aa >= smplz) )
			break;
		size_t	run = (Az - Aa),
			window = min( run, this_samplerate),
			t;
		if ( unlikely (Az > smplz) )
			run = smplz - Aa;
		valarray<TFloat>
			W (run);

		if ( run > window ) {
			// construct a vector of multipliers using an INVERTED windowing function on the
			// first and last windows of the run
			size_t	t0;
			for ( t = 0; t < window/2; ++t )
				W[t] = (1 - sigproc::winf[(size_t)AA.dampen_window_type]( t, window));
			t0 = run-window;  // start of the last window but one
			for ( t = window/2; t < window; ++t )
				W[t0 + t] = (1 - sigproc::winf[(size_t)AA.dampen_window_type]( t, window));
			// AND, connect mid-first to mid-last windows (at lowest value of the window)
			TFloat minimum = sigproc::winf[(size_t)AA.dampen_window_type]( window/2, window);
			W[ slice(window/2, run-window, 1) ] =
				(1. - minimum);
		} else  // run is shorter than samplerate (1 sec)
			for ( t = 0; t < window; ++t )
				W[t] = (1 - sigproc::winf[(size_t)AA.dampen_window_type]( t, window));

		// now gently apply the multiplier vector onto the artifacts
		recp[ slice(Aa, run, 1) ] *= (W * (TFloat)AA.factor);
	}

      // filters
	const auto& ff = filters(h);
	if ( ff.low_pass_cutoff > 0. && ff.high_pass_cutoff > 0. &&
	     ff.low_pass_order  > 0  && ff.high_pass_order  > 0 ) {
		auto tmp (exstrom::band_pass(
				  recp, this_samplerate,
				  ff.high_pass_cutoff, ff.low_pass_cutoff,
				  ff.low_pass_order, true));
		recp = tmp;
	} else {
		if ( ff.low_pass_cutoff > 0. && ff.low_pass_order > 0 ) {
			auto tmp (exstrom::low_pass(
					  recp, this_samplerate,
					  ff.low_pass_cutoff, ff.low_pass_order, true));
			recp = tmp;
		}
		if ( ff.high_pass_cutoff > 0. && ff.high_pass_order > 0 ) {
			auto tmp (exstrom::high_pass(
					  recp, this_samplerate,
					  ff.high_pass_cutoff, ff.high_pass_order, true));
			recp = tmp;
		}
	}

	switch ( ff.notch_filter ) {
	case SFilterPack::TNotchFilter::at50Hz:
		recp = exstrom::band_stop( recp, this_samplerate,
					   48, 52, 1, true);  // hardcoded numerals spotted!
	    break;
	case SFilterPack::TNotchFilter::at60Hz:
		recp = exstrom::band_stop( recp, this_samplerate,
					   58, 62, 1, true);
	    break;
	case SFilterPack::TNotchFilter::none:
	default:
	    break;
	}

	// filters happen to append samples, so
	return move(recp[ slice (0, smplz-smpla, 1)]);
}




int
CSource::
export_original( const int h,
		 const string& fname) const
{
	valarray<TFloat> signal = get_signal_original( h);
	FILE *fd = fopen( fname.c_str(), "w");
	if ( fd ) {
		for ( size_t i = 0; i < signal.size(); ++i )
			fprintf( fd, "%g\n", signal[i]);
		fclose( fd);
		return 0;
	} else
		return -1;
}


int
CSource::
export_filtered( const int h,
		 const string& fname) const
{
	valarray<TFloat> signal = get_signal_filtered( h);
	FILE *fd = fopen( fname.c_str(), "w");
	if ( fd ) {
		for ( size_t i = 0; i < signal.size(); ++i )
			fprintf( fd, "%g\n", signal[i]);
		fclose( fd);
		return 0;
	} else
		return -1;
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
