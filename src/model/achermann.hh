// ;-*-C++-*-
/*
 *       File name:  model/achermann.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  Classes related to Achermann model
 *
 *         License:  GPL
 */

#ifndef _AGH_MODEL_ACHERMANN_H
#define _AGH_MODEL_ACHERMANN_H

#include <string>
#include <vector>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_siman.h>

#include "../libsigfile/forward-decls.hh"
#include "../libsigfile/page-metrics-base.hh"
#include "../libsigfile/page.hh"
#include "../expdesign/recording.hh"
#include "achermann-tunable.hh"


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {
namespace ach {

using namespace std;



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

	double	req_percent_scored;
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
	extern ach::CModelRun *modrun;
	double	_cost_function( void *xp);
	void	_siman_step( const gsl_rng *r, void *xp, double step_size);
	double	_siman_metric( void *xp, void *yp);
	void	_siman_print( void *xp);
};

class CModelRun
  : public agh::CSCourse {

	friend class agh::CExpDesign;

	void operator=( const CModelRun&) = delete;

    public:
	CModelRun (const CModelRun&)
	      : CSCourse ()
		{
			throw runtime_error (
				"CModelRun::CModelRun() is defined solely to enable it to be the"
				" mapped type in a container and must never be called, implicitly or explicitly");
		}
	CModelRun (CModelRun&&);
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

	CModelRun (CSubject& subject, const string& session, const sigfile::SChannel& channel,
		   sigfile::TMetricType,
		   float freq_from, float freq_upto,
		   const SControlParamSet&, const STunableSetFull&);

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

	friend double agh::ach::siman::_cost_function( void*);
	friend double agh::ach::siman::_siman_metric( void*, void*);
	double _cost_function( const void *xp);  // aka fit
	double _siman_metric( const void *xp, const void *yp) const;

	friend void agh::ach::siman::_siman_step( const gsl_rng *r, void *xp, double step_size);
	void _siman_step( const gsl_rng *r, void *xp,
			  double step_size);

	const double &_which_gc( size_t p) const; // selects episode egc by page, or returns &gc if !AZAmendment
};





inline double
CModelRun::
_siman_metric( const void *xp, const void *yp) const
{
	return STunableSet (tt.value.P.size() - (size_t)TTunable::gc, (double*)xp).distance(
		STunableSet (tt.value.P.size() - (size_t)TTunable::gc, (double*)yp),
		tt.step);
}

inline const double&
CModelRun::
_which_gc( size_t p) const // selects episode egc by page, or returns &gc if !AZAmendment
{
	if ( ctl_params.AZAmendment1 )
		for ( size_t m = _mm_bounds.size()-1; m >= 1; --m )
			if ( p >= _mm_bounds[m].first )
				return cur_tset[TTunable::gc + m];
	return cur_tset[TTunable::gc];
}





} // namespace ach
} // namespace agh

#endif

// eof
