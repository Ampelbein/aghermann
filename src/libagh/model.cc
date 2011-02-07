// ;-*-C++-*- *  Time-stamp: "2011-02-05 01:56:11 hmmr"
/*
 *       File name:  model.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-05-01
 *
 *         Purpose:  simulation model classes definitions
 *
 *         License:  GPL
 */

#include <functional>
#include <algorithm>

#include "tunable.hh"
#include "primaries.hh"
#include "model.hh"


using namespace std;



int
CSCourse::layout_measurements( TMsmtPtrList& MM,
//			       size_t start_page, size_t end_page,
			       float freq_from, float freq_upto,
			       float req_percent_scored,
			       size_t swa_laden_pages_before_SWA_0)
{
	timeline.clear();
	mm_bounds.clear();

	for ( auto Mi = MM.begin(); Mi != MM.end(); ++Mi ) {
		const CRecording& M = **Mi;
		const CEDFFile& F = M.F();

		if ( !F.percent_scored() < req_percent_scored )
			return AGH_SIMPREP_ENOSCORE;

	      // anchor zero page, get pagesize from edf^W CBinnedPower
		if ( Mi == MM.begin() ) {
			_0at = F.start_time;
			_pagesize = M.pagesize();
			_pages_in_bed = 0;
		} else
			if ( _pagesize != F.pagesize() )
				return AGH_SIMPREP_EUNEQ_PAGESIZE;

		double	pa = (size_t)difftime( F.start_time, _0at) / _pagesize,
			pz = (size_t)difftime( F.end_time, _0at) / _pagesize;
		_pages_in_bed += (pz-pa);

		if ( pa < 0 )
			return AGH_SIMPREP_ENEGOFFSET;
		if ( mm_bounds.size() > 0  &&  pa - mm_bounds.back().second > 4 * 24 * 3600 )
			return AGH_SIMPREP_EFARAPART;
		mm_bounds.emplace_back( TBounds (pa,pz));


//	printf( asctime( &F->timestamp_struct));
//	printf( "m = %d, offset = %d h\n", measurements->len, (guint)(offset/3600));

		timeline.resize( (size_t)pz, SPageSimulated(0., 0., 1.));  // fill with WAKE

	      // collect M's power
		size_t	b = freq_from / M.binsize(),
			bz = min ((size_t)(freq_upto / M.binsize()), M.n_bins());

		valarray<double>
			lumped_bins (pz-pa);
		for ( float f = freq_from; b <= bz; b++, f += M.binsize() )
			lumped_bins += M.power_course( b);

		for ( size_t p = pa; p < pz; p++ )
			timeline[p].SWA = lumped_bins[ p-pa ];

		fprintf( stderr,
			 "CSCourse::layout_measurements(): added [%s, %s, %s] recorded %s at page %zu;  timeline now is %zu pages\n",
			 F.PatientID_raw, F.Session.c_str(), F.Episode.c_str(),
			 ctime( &F.start_time), (size_t)pa, (size_t)pz);

	      // determine SWA_0 etc
		if ( Mi == MM.begin() ) {
		restart:
			for ( size_t p = swa_laden_pages_before_SWA_0; p < pz; p++ ) {
				for ( size_t pp = swa_laden_pages_before_SWA_0; pp; pp-- )
					if ( timeline[pp].NREM < 1./3 ) {
						p++;
						goto restart;
					}
				_sim_start = p;
				if ( _sim_start == pz )
					return AGH_SIMPREP_ENOSWA;
				break;
			}

			_baseline_end = pz;

			_pages_with_SWA = 0;
			_SWA_L = _SWA_100 = 0.;
		}

		for ( size_t p = pa; p < pz; p++ ) {
			SPageSimulated& P = timeline[p];
			if ( P.REM > .5 )
				_SWA_L = max(_SWA_L, P.SWA);
			if ( P.NREM > 1./3 )
				_pages_with_SWA++;
		}

		_SWA_100 = 0.;
		for ( size_t p = 0; p < pz; p++ ) {
			SPageSimulated& P = timeline[p];
			if ( P.NREM > 1./3 )
				_SWA_100 += P.SWA;
		}
		_SWA_L *= .95;
		_SWA_100 /= _pages_with_SWA;

		_sim_end = pz;
	}

	subject = MM.front()->subject();
	session = MM.front()->session();
	channel = MM.front()->channel();

	fprintf( stderr,
		 "CSCourse::layout_measurements(): avg SWA = %.4g (over %zu pp, or %.3g%% of all time in bed); "
		 " SWA_L = %g;  SWA[%zu] = %.4g\n",
		 _SWA_100, _pages_with_SWA, (double)_pages_with_SWA / _pages_in_bed * 100,
		 _SWA_L, _sim_start, _SWA_0);

	return 0;
}








int
CSimulation::load( const char *fname)
{
	return 0;
}

int
CSimulation::save( const char *fname, bool binary)
{
	return 0;
}



int
CExpDesign::setup_modrun( const char* j, const char* d, const char* h,
			  float freq_from, float freq_upto,
			  CSimulation* &R_ref)
{
	try {
		CSubject& J = subject_by_x(j);
		string	sim_fname (make_fname_simulation( J.name(), h, d,
							  //0, J.measurements.size(),
							  freq_from, freq_upto).c_str());

		// list<CSimulation>::iterator R = find_if( simulations.begin(), simulations.end(),
		//  					 bind( &CSimulation::matches, j, h, d, freq_from, freq_upto, control_params));
		// ниасилил!

		if ( J.measurements[d].size() == 1 && control_params.DBAmendment2 )
			return AGH_SIMPREP_EAMENDMENTS_INEFFECTIVE;

		if ( J.measurements[d].size() == 1 && tunables.step[_rs_] > 0. )
			return AGH_SIMPREP_ERS_NONSENSICAL;

	      // collect measurements in requested session and channel
		CSCourse::TMsmtPtrList MM;
		for ( auto E = J.measurements[d].episodes.begin(); E != J.measurements[d].episodes.end(); ++E )
			MM.push_back( &(E->recordings.at(h)));

		J.measurements[d]
			. modrun_sets[h]
			. emplace_back( //pair< pair<float, float>, CSimulation>
				pair< pair<float, float>, CSimulation> (
				pair< float, float>(freq_from, freq_upto),
				CSimulation (
					MM,
					freq_from, freq_upto,
					control_params, tunables,
					sim_fname.c_str(),
					req_percent_scored,
					swa_laden_pages_before_SWA_0)));
		R_ref = &J.measurements[d]
			. modrun_sets[h].rbegin()->second;

		// if ( R -> load( sim_fname.c_str()) )
		// 	;  // load SWA_sim and S and tunables, if they exist

		return 0;

	} catch (invalid_argument ex) {
		fprintf( stderr, "%s\n", ex.what());
		return -1;
	} catch (logic_error ex) {
		return -5;
	} catch (int retval) {
		return retval;
	}
}


void
CExpDesign::reset_modrun( CSimulation& Y)
{
	// R->fft_params = fft_params;
	// *(SControlParamSet*)R = control_params;
	// *(STunableSetFull*)R  = tunables;
	// R->randomize();

	// R->_fit = 0.;

	// if ( R->_swa_sim_stack->len )
	// 	for ( guint n = 0; n < R->_swa_sim_stack->len; n++ ) {
	// 		GArray* &A = R->nth_swa_sim_course(n);
	// 		GArray* &B = R->nth_s_course(n);
	// 		if ( A )  g_array_free( A, TRUE), A = NULL;
	// 		if ( B )  g_array_free( B, TRUE), B = NULL;
	// 	}

	// R->status  = 0;
	// R->_stride = 0;
}




gsl_rng *__agh_rng;

void
init_global_rng()
{
	const gsl_rng_type *T;
	gsl_rng_env_setup();
	T = gsl_rng_default;
	if ( gsl_rng_default_seed == 0 ) {
		struct timeval tp = { 0L, 0L };
		gettimeofday( &tp, NULL);
		gsl_rng_default_seed = tp.tv_usec;
	}
	__agh_rng = gsl_rng_alloc( T);
}


// EOF
