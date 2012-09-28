// ;-*-C++-*-
/*
 *       File name:  expdesign/recording.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-09-02
 *
 *         Purpose:  Signal computed metric course
 *
 *         License:  GPL
 */

#include <list>
#include <valarray>

#include "recording.hh"
#include "primaries.hh"

using namespace std;


agh::CRecording::
CRecording (sigfile::CSource& F, int sig_no,
	    const sigfile::SFFTParamSet& fft_params,
	    const sigfile::SMCParamSet& mc_params)
      : CBinnedPower (F, sig_no, fft_params),
	CBinnedMC (F, sig_no, mc_params,
		   fft_params.pagesize),
	uc_params {NAN, NAN, NAN, NAN},
	_status (0), // not computed
	_source (F), _sig_no (sig_no),
	_cached_metric (sigfile::TMetricType::invalid),
	_cached_freq_from (NAN),
	_cached_freq_upto (NAN)
{
	// if ( F.signal_type(sig_no) == sigfile::SChannel::TType::eeg ) {
	// 	CBinnedPower::compute();
	// 	CBinnedMC::compute();
	// }
}



string
agh::CSCourse::
explain_status( int code)
{
	list<const char*> ss;
	if ( code & TFlags::enoscore )
		ss.push_back( "insufficiently scored");
	if ( code & TFlags::efarapart )
		ss.push_back( "episodes too far apart");
	if ( code & TFlags::esigtype )
		ss.push_back( "signal is not an EEG");
	if ( code & TFlags::etoomanymsmt )
		ss.push_back( "too many episodes");
	if ( code & TFlags::enoswa )
		ss.push_back( "no SWA");
	if ( code & TFlags::eamendments_ineffective)
		ss.push_back( "inappropriate amendments");
	if ( code & TFlags::ers_nonsensical )
		ss.push_back( "too few episoded for rs");
	if ( code & TFlags::enegoffset )
		ss.push_back( "negative offset");
	if ( code & TFlags::euneq_pagesize )
		ss.push_back( "wrong page size");
	return agh::str::join( ss, "; ");
}






agh::CSCourse::
CSCourse (CSubject& J, const string& d, const sigfile::SChannel& h,
	  const SSCourseParamSet& params)
      : SSCourseParamSet (params),
	_status (0),
	_sim_start ((size_t)-1), _sim_end ((size_t)-1)
{
	if ( not J.have_session(d) or J.measurements.at(d).size() == 0 )
		throw invalid_argument (string(J.name()) + " has no recordings in session " + d);

	auto& EE = J.measurements.at(d).episodes;
	for ( auto &E : EE )
		_mm_list.push_back( &E.recordings.at(h));

	for ( auto Mi = _mm_list.begin(); Mi != _mm_list.end(); ++Mi ) {
		const auto& M = **Mi;
		const auto& F = M.F();

		if ( Mi == _mm_list.begin() ) {
			_0at = F.start_time();
			_pagesize = M.SFFTParamSet::pagesize;
			_pages_in_bed = 0;
		} else
			if ( _pagesize != F.pagesize() ) {
				_status |= TFlags::euneq_pagesize;
				return;  // this is really serious, so return now
			}

		int	pa = (size_t)difftime( F.start_time(), _0at) / _pagesize,
			pz = (size_t)difftime( F.end_time(), _0at) / _pagesize;
	      // anchor zero page, get pagesize from edf^W CBinnedPower^W either goes
		printf( "CSCourse::CSCourse(): adding %s of [%s, %s, %s] %zu pages (%d indeed) recorded %s",
			sigfile::metric_method(params._profile_type), F.subject(), F.session(), F.episode(),
			F.pages(), pz-pa, ctime( &F.start_time()));

		if ( pz - pa != (int)F.pages() ) {
			fprintf( stderr, "CSCourse::CSCourse(): correct end page to match page count in EDF: %d->%zu\n",
				 pz, pa + F.pages());
			pz = pa + F.pages();
		}
		_pages_in_bed += (pz-pa);

		if ( pa < 0 ) {
			_status |= TFlags::enegoffset;
			return;
		}
		// this condition is checked against already in CSubject::SEpisodeSequence::add_one()
		// if ( _mm_bounds.size() > 0  &&  pa - _mm_bounds.back().second > 4 * 24 * 3600 ) {
		// 	_status |= (int)TSimPrepError::efarapart;
		// 	return;
		// }
		_mm_bounds.emplace_back( TBounds (pa, pz));

		_timeline.resize( pz, sigfile::SPageSimulated {0., 0., 1.});  // fill with WAKE
	}

	create_timeline();

	if ( _sim_start != (size_t)-1 )
		printf( "CSCourse::CSCourse(): sim start-end: %zu-%zu; avg SWA = %.4g (over %zu pp, or %.3g%% of all time in bed); "
			" SWA_L = %g;  SWA[%zu] = %g\n",
			_sim_start, _sim_end, _SWA_100, _pages_with_SWA, (double)_pages_with_SWA / _pages_in_bed * 100,
			_SWA_L, _sim_start, _SWA_0);
	else
		printf( "CSCourse::CSCourse(): status %xd, %s\n", _status, CSCourse::explain_status( _status).c_str());
}




agh::CSCourse::
CSCourse (CRecording& M,
	  const SSCourseParamSet& params)
      : SSCourseParamSet (params),
	_status (0),
	_sim_start ((size_t)-1), _sim_end ((size_t)-1)
{
	_mm_list.push_back( &M);

	_0at = M.F().start_time();
	_pagesize = M.SFFTParamSet::pagesize;
	_pages_in_bed = 0;

	int	pa = (size_t)difftime( M.F().start_time(), _0at) / _pagesize,
		pz = (size_t)difftime( M.F().end_time(), _0at) / _pagesize;
	printf( "CSCourse::CSCourse(): adding single recording %s of [%s, %s, %s] %zu pages (%d indeed) recorded %s",
		sigfile::metric_method(params._profile_type), M.F().subject(), M.F().session(), M.F().episode(),
		M.F().pages(), pz-pa, ctime( &M.F().start_time()));

	if ( pz - pa != (int)M.F().pages() ) {
		fprintf( stderr, "CSCourse::CSCourse(): correct end page to match page count in EDF: %d->%zu\n",
			 pz, pa + M.F().pages());
		pz = pa + M.F().pages();
	}
	_pages_in_bed += (pz-pa);

	if ( pa < 0 ) {
		_status |= TFlags::enegoffset;
		return;
	}
	// this condition is checked against already in CSubject::SEpisodeSequence::add_one()
	// if ( _mm_bounds.size() > 0  &&  pa - _mm_bounds.back().second > 4 * 24 * 3600 ) {
	// 	_status |= (int)TSimPrepError::efarapart;
	// 	return;
	// }
	_mm_bounds.emplace_back( TBounds (pa, pz));

	_timeline.resize( pz, sigfile::SPageSimulated {0., 0., 1.});  // fill with WAKE

	create_timeline();

	if ( _sim_start != (size_t)-1 )
		printf( "CSCourse::CSCourse(): sim start-end: %zu-%zu; avg SWA = %.4g (over %zu pp, or %.3g%% of all time in bed); "
			" SWA_L = %g;  SWA[%zu] = %g\n",
			_sim_start, _sim_end, _SWA_100, _pages_with_SWA, (double)_pages_with_SWA / _pages_in_bed * 100,
			_SWA_L, _sim_start, _SWA_0);
	else
		printf( "CSCourse::CSCourse(): status %xd, %s\n", _status, CSCourse::explain_status( _status).c_str());
}




agh::CSCourse::
CSCourse( CSCourse&& rv)
      : SSCourseParamSet (rv),
	_sim_start (rv._sim_start), _sim_end (rv._sim_end),
	_baseline_end (rv._baseline_end),
	_pages_with_SWA (rv._pages_with_SWA),
	_pages_in_bed (rv._pages_in_bed),
	_SWA_L (rv._SWA_L), _SWA_0 (rv._SWA_0), _SWA_100 (rv._SWA_100),
	_0at (rv._0at),
	_pagesize (rv._pagesize)
{
	swap( _timeline,  rv._timeline);
	swap( _mm_bounds, rv._mm_bounds);
	swap( _mm_list,   rv._mm_list);
}




void
agh::CSCourse::
create_timeline()
{
	_metric_avg = 0.;
	for ( auto Mi = _mm_list.begin(); Mi != _mm_list.end(); ++Mi ) {
		auto& M = **Mi;
		const auto& F = M.F();

		if ( F.percent_scored() < _req_percent_scored )
			_status |= TFlags::enoscore;

	      // collect M's power and scores
		valarray<TFloat>
			lumped_bins = M.course<TFloat>( _profile_type, _freq_from, _freq_upto);

		size_t	pa = (size_t)difftime( F.start_time(), _0at) / _pagesize,
			pz = (size_t)difftime( F.end_time(), _0at) / _pagesize;
		for ( size_t p = pa; p < pz; ++p ) {
			_timeline[p] = sigfile::SPageSimulated {F[p-pa]};
		      // fill unscored/MVT per user setting
			if ( !_timeline[p].is_scored() ) {
				if ( _ScoreUnscoredAsWake )
					_timeline[p].mark( sigfile::SPage::TScore::wake);
				else
					if ( p > 0 )
						_timeline[p] = _timeline[p-1];
			}
		      // put SWA, compute avg PSD
			_metric_avg +=
				(_timeline[p].metric = lumped_bins[p-pa]);
		}

	      // determine SWA_0
		if ( Mi == _mm_list.begin() ) {
			_baseline_end = pz;

			// require some length of swa-containing pages to happen before sim_start
			for ( size_t p = 0; p < pz; ++p ) {
				for ( size_t pp = p; pp < pz; ++pp ) {
					if ( _timeline[pp].NREM < 1./3 ) {
						p = pp;
						goto outer_continue;
					}
					if ( (pp-p) >= _swa_laden_pages_before_SWA_0 ) {
						_sim_start = pp;
						goto outer_break;
					}
				}
			outer_continue:
				;
			}
		outer_break:

			if ( _sim_start == (size_t)-1 )
				_status |= TFlags::enoswa;
			else
				_SWA_0 = _timeline[_sim_start].metric;
		}

		_sim_end = pz-1;
	}
	_metric_avg /= _pages_in_bed;

      // determine SWA metrics
	_pages_with_SWA = _pages_non_wake = 0;
	_SWA_L = _SWA_100 = 0.;

	if ( _sim_start != (size_t)-1 ) {
		size_t REM_pages_cnt = 0;
		for ( size_t p = _sim_start; p < _sim_end; ++p ) {
			auto& P = _timeline[p];
			if ( P.REM > .5 ) {
				_SWA_L += P.metric;
				++REM_pages_cnt;
			}
			if ( P.NREM > 1./3 ) {
				_SWA_100 += P.metric;
				++_pages_with_SWA;
			}
			if ( P.Wake == 0. )
				++_pages_non_wake;
		}
		if ( REM_pages_cnt )
			_SWA_L /= (REM_pages_cnt / .95);
		if ( _pages_with_SWA )
			_SWA_100 /= _pages_with_SWA;
	}
}


// eof

