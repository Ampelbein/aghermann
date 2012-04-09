// ;-*-C++-*-
/*
 *       File name:  core/model.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  Classes related to Achermann model
 *
 *         License:  GPL
 */

#ifndef _AGH_MODEL_H
#define _AGH_MODEL_H

#include <string>
#include <vector>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_siman.h>

#include "../libsigfile/forward-decls.hh"
#include "../libsigfile/page-metrics-base.hh"
#include "../libsigfile/page.hh"
#include "forward-decls.hh"
#include "tunable.hh"


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {

using namespace std;



typedef size_t sid_type;
typedef size_t hash_key;




struct SSCourseParamSet {
	sigfile::TProfileType
		_profile_type;
	float	_freq_from,
		_freq_upto;
	float	_req_percent_scored;
	size_t	_swa_laden_pages_before_SWA_0;
	bool	_ScoreMVTAsWake:1,
		_ScoreUnscoredAsWake:1;
};


class CSCourse
  : private SSCourseParamSet {

    public:
	CSCourse( CSubject& J, const string& d, const sigfile::SChannel& h,
		  const SSCourseParamSet& params);
	void create_timeline( const SSCourseParamSet& params)
		{
			*(SSCourseParamSet*)this = params;
			create_timeline();
		}
	void create_timeline();

	static string explain_status( int);

	sigfile::TProfileType profile_type() const
					{ return _profile_type; }
	float freq_from() const		{ return _freq_from; }
	float freq_upto() const		{ return _freq_upto; }
	size_t sim_start() const	{ return _sim_start; }
	size_t sim_end() const		{ return _sim_end; }
	size_t baseline_end() const	{ return _baseline_end; }
	size_t pages_with_swa() const	{ return _pages_with_SWA; }
	size_t pages_non_wake() const	{ return _pages_non_wake; }
	size_t pages_in_bed() const	{ return _pages_in_bed; }
	double SWA_L() const		{ return _SWA_L; }
	double SWA_0() const		{ return _SWA_0; }
	double SWA_100() const		{ return _SWA_100; }
	double PSD_avg() const		{ return _PSD_avg; }

	const vector<sigfile::SPageSimulated>&
	timeline() const		{ return _timeline; }

	typedef pair<size_t, size_t> TBounds;
	const vector<TBounds>&
	mm_bounds() const		{ return _mm_bounds; }

	const vector<const CRecording*>&
	mm_list() const			{ return _mm_list; }

	const sigfile::SPageSimulated&
	operator[]( size_t p) const
		{
			return _timeline[p];
		}

	time_t nth_episode_start_time( size_t n) const
		{
			return _0at + _mm_bounds[n].first * _pagesize;
		}
	time_t nth_episode_end_time( size_t n) const
		{
			return _0at + _mm_bounds[n].second * _pagesize;
		}

	size_t nth_episode_start_page( size_t n) const
		{
			return _mm_bounds[n].first;
		}
	size_t nth_episode_end_page( size_t n) const
		{
			return _mm_bounds[n].second;
		}

	size_t pagesize() const
		{
			return _pagesize;
		}
	const char* subject() const;
	const char* session() const;
	const char* channel() const;

    protected:
	int	_status;

	CSCourse( const CSCourse&) = delete;
	CSCourse()
		{} // easier than the default; not used anyway

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
	size_t	_sim_start,
		_sim_end,
		_baseline_end,
		_pages_with_SWA,
		_pages_non_wake,
		_pages_in_bed;
	double	_SWA_L,
		_SWA_0,	_SWA_100,
		_PSD_avg;

	time_t	_0at;
	vector<sigfile::SPageSimulated>
		_timeline;
	vector<TBounds>  // in pages
		_mm_bounds;

	vector<const CRecording*>
		_mm_list;
    private:
	size_t	_pagesize;  // since power is binned each time it is
			    // collected in layout_measurements() and
			    // then detached, we keep it here
			    // privately
};







struct SControlParamSet {

	gsl_siman_params_t
		siman_params;
		    // int n_tries
		    // 	The number of points to try for each step.
		    // int iters_fixed_T
		    // 	The number of iterations at each temperature.
		    // double step_size
		    // 	The maximum step size in the random walk.
		    // double k, t_initial, mu_t, t_min

	bool	DBAmendment1,
		DBAmendment2,
		AZAmendment1,
		AZAmendment2,
		ScoreUnscoredAsWake;

	sigfile::TProfileType
		profile_type;
	float	req_percent_scored;
	size_t	swa_laden_pages_before_SWA_0;

	SControlParamSet()
		{
			reset();
		}

	SControlParamSet& operator=( const SControlParamSet&) = default;
	bool operator==( const SControlParamSet &rv) const;

	void check() const; // throws
	void reset();
};




class CModelRun;
namespace siman {
	extern CModelRun *modrun;
	double	_cost_function( void *xp);
	void	_siman_step( const gsl_rng *r, void *xp, double step_size);
	double	_siman_metric( void *xp, void *yp);
	void	_siman_print( void *xp);
};

class CModelRun
  : public CSCourse {

	friend class CExpDesign;
	friend class CSimulation;

	CModelRun(const CModelRun& rv) = delete;
	//void operator=( const CModelRun&) = delete;

    public:
	CModelRun( CModelRun&& rv)
	      : CSCourse ((CSCourse&&)rv),
		tt ((STunableSetFull&&)rv.tt),
		cur_tset ((STunableSet&&)rv.cur_tset)
		{
			ctl_params = rv.ctl_params,
			status = rv.status;
			_prepare_scores2();
		}

	static const int
		ok			= 0,
		enoscore		= 1,
		efarapart		= 2,
		esigtype		= 4,
		etoomanymsmt		= 8,
		enoswa			= 16,
		eamendments_ineffective	= 32,
		ers_nonsensical		= 64,
		enegoffset		= 128,
		euneq_pagesize		= 256;

	CModelRun() // oblige map
		{}

	enum TModrunFlags { modrun_tried = 1 };
	int	status;
	SControlParamSet
		ctl_params;
	STunableSetFull
		tt;
	STunableSet
		cur_tset;

	CModelRun( CSubject& subject, const string& session, const sigfile::SChannel& channel,
		   float freq_from, float freq_upto,
		   const SControlParamSet& _ctl_params,
		   const STunableSetFull& t0)
	      : CSCourse( subject, session, channel,
			  agh::SSCourseParamSet {_ctl_params.profile_type,
					  	 freq_from, freq_upto,
					  	 _ctl_params.req_percent_scored,
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

	int watch_simplex_move( void (*)(void*));
	double snapshot()
		{
			return _cost_function( &cur_tset.P[0]);
		}

    private:
	vector<sigfile::SPage>
		_scores2;  // we need shadow to hold scores as modified per Score{MVT,Unscored}As... and by t_{a,p},
			   // and also to avoid recreating it before each stride
//	size_t	_pagesize;
	// pagesize is taken as held in CSCourse, collected from edf sources during layout_measurements()
	// the pagesize used for displaying the EEGs is a totally different matter and it
	// does not even belong in core

	void _restore_scores_and_extend_rem( size_t, size_t);
	void _prepare_scores2();

	friend double agh::siman::_cost_function( void*);
	double _cost_function( const void *xp);  // aka fit
	friend double agh::siman::_siman_metric( void*, void*);
	double _siman_metric( const void *xp, const void *yp) const
		{
			return STunableSet (tt.value.P.size() - (size_t)TTunable::gc, (double*)xp).distance(
				STunableSet (tt.value.P.size() - (size_t)TTunable::gc, (double*)yp),
				tt.step);
		}
	friend void agh::siman::_siman_step( const gsl_rng *r, void *xp, double step_size);
	void _siman_step( const gsl_rng *r, void *xp,
			  double step_size);

	const double &_which_gc( size_t p) const // selects episode egc by page, or returns &gc if !AZAmendment
		{
			if ( ctl_params.AZAmendment1 )
				for ( size_t m = _mm_bounds.size()-1; m >= 1; --m )
					if ( p >= _mm_bounds[m].first )
						return cur_tset[TTunable::gc + m];
			return cur_tset[TTunable::gc];
		}
};



extern gsl_rng *__agh_rng;
void init_global_rng();

} // namespace agh

#endif

// eof
