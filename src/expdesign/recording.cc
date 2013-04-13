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
#include "model/beersma.hh"

using namespace std;


agh::CRecording::
CRecording (sigfile::CTypedSource& F, int sig_no,
	    const metrics::psd::SPPack& fft_params,
	    const metrics::swu::SPPack& swu_params,
	    const metrics::mc::SPPack& mc_params)
      : psd_profile (F, sig_no, fft_params),
        swu_profile (F, sig_no, swu_params),
	mc_profile  (F, sig_no, mc_params),
	uc_params (nullptr),
	_status (0), // not computed
	_source (F), _sig_no (sig_no)
{
	// if ( F.signal_type(sig_no) == sigfile::SChannel::TType::eeg ) {
	// 	CBinnedPower::compute();
	// 	CBinnedMC::compute();
	// }
}


agh::CRecording::
~CRecording ()
{
	if ( uc_params )
		delete uc_params;
}






string
agh::SProfileParamSet::
display_name() const
{
	DEF_UNIQUE_CHARP (_);
	switch ( metric ) {
	case metrics::TType::psd: ASPRINTF( &_, "%s (%g-%g Hz)", metric_name(), P.psd.freq_from, P.psd.freq_upto); break;
	case metrics::TType::swu: ASPRINTF( &_, "%s (%g Hz)",    metric_name(), P.swu.f0); break;
	case metrics::TType::mc : ASPRINTF( &_, "%s (%g Hz)",    metric_name(), P.mc.f0); break;
	default: ASPRINTF( &_, "(invalid metric: %d)", metric); break;
	}
	string ret {_};
	return ret;
}

string
agh::CProfile::
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






agh::CProfile::
CProfile (CSubject& J, const string& d, const sigfile::SChannel& h,
	  const SProfileParamSet& params)
      : SProfileParamSet (params),
	_status (0),
	_sim_start ((size_t)-1), _sim_end ((size_t)-1)
{
	if ( not J.have_session(d) or J.measurements.at(d).size() == 0 )
		throw invalid_argument (J.id + " has no recordings in session " + d);

	auto& EE = J.measurements.at(d).episodes;
	for ( auto &E : EE )
		_mm_list.push_back( &E.recordings.at(h));

	for ( auto Mi = _mm_list.begin(); Mi != _mm_list.end(); ++Mi ) {
		const auto& M = **Mi;
		const auto& F = M.F();

		if ( Mi == _mm_list.begin() ) {
			_0at = F.start_time();
			_pagesize = M.psd_profile.Pp.pagesize;
			_pages_in_bed = 0;
		} else
			if ( _pagesize != M.pagesize() ) {
				_status |= TFlags::euneq_pagesize;
				return;  // this is really serious, so return now
			}

		int	pa = (size_t)difftime( F.start_time(), _0at) / _pagesize,
//			pz = (size_t)difftime( F.end_time(), _0at) / _pagesize;
			pz = pa + M.hypnogram().pages();
	      // anchor zero page, get pagesize from edf^W CBinnedPower^W either goes
		time_t dima = F.start_time();
		printf( "CProfile::CProfile(): adding %s of [%s, %s, %s] %zu pages (%zu full, %zu in hypnogram) recorded %s",
			metrics::name(params.metric), F.subject().id.c_str(), F.session(), F.episode(),
			M.total_pages(), M.full_pages(), M.hypnogram().pages(), ctime( &dima));

		if ( pz - pa != (int)M.full_pages() ) {
			fprintf( stderr, "CProfile::CProfile(): correcting end page to match full page count in EDF: %d->%zu\n",
				 pz, pa + M.full_pages());
			pz = pa + M.full_pages();
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
		printf( "CProfile::CProfile(): sim start-end: %zu-%zu; avg SWA = %.4g (over %zu pp, or %.3g%% of all time in bed); "
			" SWA_L = %g;  SWA[%zu] = %g\n",
			_sim_start, _sim_end, _SWA_100, _pages_with_SWA, (double)_pages_with_SWA / _pages_in_bed * 100,
			_SWA_L, _sim_start, _SWA_0);
	else
		printf( "CProfile::CProfile(): status %xd, %s\n", _status, CProfile::explain_status( _status).c_str());
}




agh::CProfile::
CProfile (CRecording& M,
	  const SProfileParamSet& params)
      : SProfileParamSet (params),
	_status (0),
	_sim_start ((size_t)-1), _sim_end ((size_t)-1)
{
	_mm_list.push_back( &M);

	_0at = M.F().start_time();
	_pagesize = M.psd_profile.Pp.pagesize;
	_pages_in_bed = 0;

	int	pa = (size_t)difftime( M.F().start_time(), _0at) / _pagesize,
		pz = (size_t)difftime( M.F().end_time(), _0at) / _pagesize;
	time_t	dima = M.F().start_time();
	printf( "CProfile::CProfile(): adding single recording %s of [%s, %s, %s] %zu pages (%zu full, %zu in hypnogram) recorded %s",
		metrics::name(params.metric), M.F().subject().id.c_str(), M.F().session(), M.F().episode(),
		M.total_pages(), M.full_pages(), M.hypnogram().pages(), ctime( &dima));

	if ( pz - pa != (int)M.full_pages() ) {
		fprintf( stderr, "CProfile::CProfile(): correcting end page to match full page count in EDF: %d->%zu\n",
			 pz, pa + M.full_pages());
		pz = pa + M.full_pages();
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
		printf( "CProfile::CProfile(): sim start-end: %zu-%zu; avg SWA = %.4g (over %zu pp, or %.3g%% of all time in bed); "
			" SWA_L = %g;  SWA[%zu] = %g\n",
			_sim_start, _sim_end, _SWA_100, _pages_with_SWA, (double)_pages_with_SWA / _pages_in_bed * 100,
			_SWA_L, _sim_start, _SWA_0);
	else
		printf( "CProfile::CProfile(): status %xd, %s\n", _status, CProfile::explain_status( _status).c_str());
}




agh::CProfile::
CProfile (CProfile&& rv)
      : SProfileParamSet (rv),
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



bool
agh::CProfile::
need_compute( const SProfileParamSet& P)
{
	for ( auto Mi = _mm_list.begin(); Mi != _mm_list.end(); ++Mi )
		if ( (*Mi)->need_compute( P) )
			return true;
	return false;
}


void
agh::CProfile::
create_timeline()
{
	_metric_avg = 0.;
	for ( auto Mi = _mm_list.begin(); Mi != _mm_list.end(); ++Mi ) {
		auto& M = **Mi;
		const auto& F = M.F();
		const auto& Y = M.hypnogram();

		if ( Y.percent_scored() < req_percent_scored )
			_status |= TFlags::enoscore;

	      // collect M's power and scores
		valarray<TFloat>
			lumped_bins = M.course( *(SProfileParamSet*)this);

		size_t	pa = (size_t)difftime( F.start_time(), _0at) / _pagesize,
			pz = (size_t)difftime( F.end_time(), _0at) / _pagesize;
		for ( size_t p = pa; p < pz; ++p ) {
			_timeline[p] = sigfile::SPageSimulated {Y[p-pa]};
		      // fill unscored/MVT per user setting
			if ( !_timeline[p].is_scored() ) {
				if ( score_unscored_as_wake )
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
					if ( (pp-p) >= swa_laden_pages_before_SWA_0 ) {
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


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

