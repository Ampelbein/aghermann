// ;-*-C++-*-
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

#include "../expdesign/recording.hh"
#include "../expdesign/primaries.hh"
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
	     siman_params.mu_t <= 0 ||
	     (req_percent_scored < 50. || req_percent_scored > 100. ) )
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

	ScoreUnscoredAsWake	= true;

	req_percent_scored = 90.;
	swa_laden_pages_before_SWA_0 = 3;
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
		AZAmendment2 == rv.AZAmendment2 &&
		ScoreUnscoredAsWake == rv.ScoreUnscoredAsWake &&
		req_percent_scored == rv.req_percent_scored &&
		swa_laden_pages_before_SWA_0 == rv.swa_laden_pages_before_SWA_0;
}




int
agh::CExpDesign::
setup_modrun( const char* j, const char* d, const char* h,
	      sigfile::TMetricType metric_type,
	      float freq_from, float freq_upto,
	      agh::ach::CModelRun* &R_ref)
{
	try {
		CSubject& J = subject_by_x(j);

		if ( J.measurements[d].size() == 1 && ctl_params0.DBAmendment2 )
			return CSCourse::TFlags::eamendments_ineffective;

		if ( J.measurements[d].size() == 1 && tunables0.step[ach::TTunable::rs] > 0. )
			return CSCourse::TFlags::ers_nonsensical;

		auto freq_idx = pair<float,float> (freq_from, freq_upto);
		J.measurements[d]
			. modrun_sets[metric_type][h].insert(
				pair<pair<float, float>, ach::CModelRun>
				(freq_idx, agh::ach::CModelRun (J, d, h,
							   metric_type, freq_from, freq_upto,
							   ctl_params0, tunables0)));
		R_ref = &J.measurements[d]
			. modrun_sets[metric_type][h][freq_idx];

	} catch (invalid_argument ex) { // thrown by CSCourse ctor
		fprintf( stderr, "CExpDesign::setup_modrun( %s, %s, %s): %s\n", j, d, h, ex.what());
		return -1;
	} catch (int ex) { // thrown by CModelRun ctor
		log_message( "CExpDesign::setup_modrun( %s, %s, %s): %s\n", j, d, h, CSCourse::explain_status(ex).c_str());
		return ex;
	}

	return 0;
}




agh::ach::CModelRun::
CModelRun( CSubject& subject, const string& session, const sigfile::SChannel& channel,
	   sigfile::TMetricType metric_type,
	   float freq_from, float freq_upto,
	   const SControlParamSet& _ctl_params,
	   const STunableSetFull& t0)
      : CSCourse( subject, session, channel,
		  agh::SSCourseParamSet {metric_type,
				  freq_from, freq_upto, (float)_ctl_params.req_percent_scored,
				  _ctl_params.swa_laden_pages_before_SWA_0,
				  _ctl_params.ScoreUnscoredAsWake}),
	status (0),
	ctl_params (_ctl_params),
	tt (t0, ctl_params.AZAmendment1 ? _mm_list.size() : 1),
	cur_tset (t0.value, ctl_params.AZAmendment1 ? _mm_list.size() : 1)
{
	if ( CSCourse::_status )
		throw CSCourse::_status;
	_prepare_scores2();
}

agh::ach::CModelRun::
CModelRun( CModelRun&& rv)
      : CSCourse (move(rv)),
	status (rv.status),
	ctl_params (rv.ctl_params),
	tt (move(rv.tt)),
	cur_tset (move(rv.cur_tset))
{
	_prepare_scores2();
}





// eof
