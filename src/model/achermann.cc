/*
 *       File name:  model/achermann.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-01
 *
 *         Purpose:  simulation model classes
 *
 *         License:  GPL
 */

#include <list>

#include "expdesign/profile.hh"
#include "expdesign/primaries.hh"
#include "achermann-tunable.hh"
#include "achermann.hh"


using namespace std;


void
agh::ach::SControlParamSet::
check() const
{
	if ( siman_params.n_tries < 1 ||
	     siman_params.iters_fixed_T < 1 ||
	     siman_params.step_size <= 0. ||
	     siman_params.k <= 0. ||
	     siman_params.t_initial <= 0. ||
	     siman_params.t_min <= 0. ||
	     siman_params.t_min >= siman_params.t_initial ||
	     siman_params.mu_t <= 0 )
		throw invalid_argument("Bad SControlParamSet");
}

void
agh::ach::SControlParamSet::
reset()
{
	siman_params.n_tries		=   20;
	siman_params.iters_fixed_T	=   10;
	siman_params.step_size		=    3.;
	siman_params.k			=    1.0;
	siman_params.t_initial  	=  200.;
	siman_params.mu_t		=    1.003;
	siman_params.t_min		=    1.;

	DBAmendment1		= true;
	DBAmendment2		= false;
	AZAmendment1		= false;
	AZAmendment2		= false;
}



bool
__attribute__ ((pure))
agh::ach::SControlParamSet::
operator==( const SControlParamSet &rv) const
{
	return	memcmp( &siman_params, &rv.siman_params, sizeof(siman_params)) == 0 &&
		DBAmendment1 == rv.DBAmendment1 &&
		DBAmendment2 == rv.DBAmendment2 &&
		AZAmendment1 == rv.AZAmendment1 &&
		AZAmendment2 == rv.AZAmendment2;
}




int
agh::CExpDesign::
setup_modrun( const string& j, const string& d, const string& h,
	      const SProfileParamSet& profile_params0,
	      agh::ach::CModelRun** Rpp)
{
	try {
		CSubject& J = subject_by_x(j);

		if ( J.measurements[d].size() == 1 && ctl_params0.DBAmendment2 )
			return CProfile::TFlags::eamendments_ineffective;

		if ( J.measurements[d].size() == 1 && tstep[ach::TTunable::rs] > 0. )
			return CProfile::TFlags::ers_nonsensical;

		J.measurements[d].modrun_sets[profile_params0].insert(
			pair<string, ach::CModelRun> (
				h,
				ach::CModelRun (
					J, d, h,
					profile_params0,
					ctl_params0,
					tunables0))
			);
		if ( Rpp )
			*Rpp = &J.measurements[d]
				. modrun_sets[profile_params0][h];

	} catch (invalid_argument ex) { // thrown by CProfile ctor
		fprintf( stderr, "CExpDesign::setup_modrun( %s, %s, %s): %s\n", j.c_str(), d.c_str(), h.c_str(), ex.what());
		return -1;
	} catch (out_of_range ex) {
		fprintf( stderr, "CExpDesign::setup_modrun( %s, %s, %s): %s\n", j.c_str(), d.c_str(), h.c_str(), ex.what());
		return -1;
	} catch (int ex) { // thrown by CModelRun ctor
		log_message( "CExpDesign::setup_modrun( %s, %s, %s): %s\n", j.c_str(), d.c_str(), h.c_str(), CProfile::explain_status(ex).c_str());
		return ex;
	}

	return 0;
}




agh::ach::CModelRun::
CModelRun (CSubject& subject, const string& session, const sigfile::SChannel& channel,
	   const agh::SProfileParamSet& _scourse_params,
	   const SControlParamSet& _ctl_params,
	   const STunableSetWithState& t0)
      : CProfile (subject, session, channel,
		  _scourse_params),
	status (0),
	ctl_params (_ctl_params),
	tstep (ctl_params.AZAmendment1 ? _mm_list.size()-1 : 0),
	tlo   (ctl_params.AZAmendment1 ? _mm_list.size()-1 : 0),
	thi   (ctl_params.AZAmendment1 ? _mm_list.size()-1 : 0),
	t0    (ctl_params.AZAmendment1 ? _mm_list.size()-1 : 0),
	tx (t0, tstep, tlo, thi),
	cf (NAN)
{
	if ( CProfile::_status )
		throw CProfile::_status;
	_prepare_scores2();
}

agh::ach::CModelRun::
CModelRun (CModelRun&& rv)
      : CProfile (move(rv)),
	status (rv.status),
	ctl_params (rv.ctl_params),
	tstep (rv.tstep),
	tlo (rv.tlo),
	thi (rv.thi),
	t0  (rv.t0),
	tx (tstep, tlo, thi),
	cf (NAN)
{
	_prepare_scores2();
}








// create a copy of original scores; also patch Unscored and MVT
void
agh::ach::CModelRun::
_prepare_scores2()
{
	size_t p, pp;

	_scores2.assign( _timeline.begin(), _timeline.end());

	if ( ctl_params.ScoreUnscoredAsWake ) {
		for ( p = _sim_start; p < _timeline.size(); ++p )
			if ( _scores2[p].NREM + _scores2[p].REM + _scores2[p].Wake == 0 ) { // this is Unscored
//				printf( " (patching Unscored as Wake at %d)\n", p);
				_scores2[p].Wake = 1.;
			}
	} else {  // ... as prev page
		for ( p = (_sim_start > 0) ?_sim_start :1; p < _timeline.size(); ++p )
			if ( _scores2[p].NREM + _scores2[p].REM + _scores2[p].Wake == 0 ) {
//				printf( " (patching Unscored as prev at %d)\n", p);
				pp = p-1;
				do
					_scores2[p] = _scores2[pp];
				while ( ++p < _timeline.size() &&
					_scores2[p].NREM + _scores2[p].REM + _scores2[p].Wake == 0. );
			}
	}

	if ( ctl_params.AZAmendment2 )
		for ( p = 0; p < _timeline.size(); ++p )
			_timeline[p].NREM = ceil(_timeline[p].NREM);
}





// recreate timeline REM values from _scores2, extend REM bouts per _ta_ and _tp_
void
agh::ach::CModelRun::
_restore_scores_and_extend_rem( size_t da, size_t dz)
{
	size_t	a  =                    da,
		z  = _timeline.size() - dz,
		p, pi, di;

	for ( p = 0; p < _timeline.size(); ++p )
		_timeline[p].REM = _scores2[p].REM;

	for ( p = a; p < z; ++p )
		if ( _scores2[p].REM > 0.33 ) {  // only deal with some substantial REM
			for ( pi = p; _timeline[pi].REM > .33 && pi < z; ++pi ) {

				// pull front
				di = da;
				do  {
					if ( _scores2[pi].REM > _scores2[pi - di].REM )
						_timeline[pi - di].REM = _scores2[pi].REM;
				} while ( di-- );

				// push end
				di = dz;
				do {
					if ( _scores2[pi].REM > _scores2[pi + di].REM )
						_timeline[pi + di].REM = _scores2[pi].REM;
				} while ( di-- );

			}  // perhaps, in addition to spreading the boundary value to regions before and after existing REM,
			   // we should also bump the existing values inside it?
			p = pi;
		}
}






double
__attribute__ ((hot))
agh::ach::CModelRun::
snapshot()
{
//	printf( "AZAmendment = %d; cur_tset.size = %zu\n", AZAmendment, cur_tset.size());
	const float ppm = 60. / pagesize();
	auto _tset = tx;
	_tset.adjust_for_ppm( ppm);

	_restore_scores_and_extend_rem( (int)round( _tset[TTunable::ta]), (int)round( _tset[TTunable::tp]));

      // substitute S_0 and S_U, expressed in %, with abs values
	_tset[TTunable::S0] *= _SWA_100/100;
	_tset[TTunable::SU] *= _SWA_100/100;

	if ( ctl_params.DBAmendment2 )
		_timeline[_baseline_end].S = _SWA_100 * 3; // will be overwritten at completion of the first iteration

      // prime S and swa_sim
	_timeline[_sim_start].metric_sim = _SWA_0;
	_timeline[_sim_start].S = _tset[TTunable::S0];

	double _fit = 0.;

#define CF_CYCLE_COMMON_DB1 \
	int	WT = (_timeline[p].Wake > 0.33);			\
	TFloat	pS = _timeline[p].S / _tset[TTunable::SU];		\
	TFloat	pSWA =							\
		_tset[TTunable::rc] * _timeline[p].metric_sim * pS		\
		* (1. - _timeline[p].metric_sim / _timeline[p].S)		\
		* (1. - _timeline[p].Wake);				\
	_timeline[p+1].metric_sim =					\
		_timeline[p].metric_sim					\
		+ pSWA							\
		- _tset[TTunable::fcR] * (_timeline[p].metric_sim - _SWA_L) * _timeline[p].REM \
		- _tset[TTunable::fcW] * (_timeline[p].metric_sim - _SWA_L) * _timeline[p].Wake; \
									\
	_timeline[p+1].S =						\
		_timeline[p].S + ( WT					\
				   ? 0					\
				   : (-_which_gc(p) * _timeline[p].metric_sim) ) \
		+ (_tset[TTunable::SU] - _timeline[p].S) * _tset[TTunable::rs]; \
									\
	if ( _timeline[p].has_swa() )					\
		_fit += gsl_pow_2( _timeline[p].metric - _timeline[p].metric_sim); \
	if ( _timeline[p].S < 0 ) \
		_fit += 1e9;

#define CF_CYCLE_COMMON_NODB1 \
	int	WT = (_timeline[p].Wake > 0.33);			\
	double	pS = _timeline[p].S / _tset[TTunable::SU];		\
	double	pSWA =							\
		_tset[TTunable::rc] * _timeline[p].metric_sim * pS * (1. - _timeline[p].metric_sim / _timeline[p].S); \
	_timeline[p+1].metric_sim =					\
		_timeline[p].metric_sim					\
		+ pSWA							\
		- _tset[TTunable::fcR] * (_timeline[p].metric_sim - _SWA_L) * _timeline[p].REM \
		- _tset[TTunable::fcW] * (_timeline[p].metric_sim - _SWA_L) * _timeline[p].Wake; \
									\
	_timeline[p+1].S =						\
		_timeline[p].S + ( WT					\
				   ? 0					\
				   : (-_which_gc(p) * _timeline[p].metric_sim) ) \
		+ (_tset[TTunable::SU] - _timeline[p].S) * _tset[TTunable::rs]; \
									\
	if ( _timeline[p].has_swa() )					\
		_fit += gsl_pow_2( _timeline[p].metric - _timeline[p].metric_sim); \
	if ( _timeline[p].S < 0 ) \
		_fit += 1e9;
// define end

	if ( ctl_params.DBAmendment2 )
		if ( ctl_params.DBAmendment1 )
			for ( size_t p = _sim_start; p < _sim_end; ++p ) {
				double edt = exp( -(24*60*ppm + _sim_start - _baseline_end) * _tset[TTunable::rs]);
				_tset[TTunable::SU] = (_timeline[_sim_start].S - _timeline[_baseline_end].S * edt) / (1. - edt);

				CF_CYCLE_COMMON_DB1;
			}
		else
			for ( size_t p = _sim_start; p < _sim_end; ++p ) {
				double edt = exp( -(24*60*ppm + _sim_start - _baseline_end) * _tset[TTunable::rs]);
				_tset[TTunable::SU] = (_timeline[_sim_start].S - _timeline[_baseline_end].S * edt) / (1. - edt);

				CF_CYCLE_COMMON_NODB1;
			}
	else
		if ( ctl_params.DBAmendment1 )
			for ( size_t p = _sim_start; p < _sim_end; ++p ) {
				CF_CYCLE_COMMON_DB1;
			}
		else
			for ( size_t p = _sim_start; p < _sim_end; ++p ) {
				CF_CYCLE_COMMON_NODB1;
			}

#undef CF_CYCLE_COMMON_DB1
#undef CF_CYCLE_COMMON_NODB1

	return cf = sqrt( _fit/_pages_with_SWA);
}





// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
