/*
 *       File name:  aghermann/model/achermann.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  Classes related to Achermann model
 *
 *         License:  GPL
 */

#ifndef AGH_AGHERMANN_MODEL_ACHERMANN_H_
#define AGH_AGHERMANN_MODEL_ACHERMANN_H_

#include <string>
#include <vector>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_siman.h>

#include "libsigfile/forward-decls.hh"
#include "libsigfile/page.hh"
#include "libmetrics/page-metrics-base.hh"
#include "aghermann/expdesign/profile.hh"
#include "achermann-tunable.hh"


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


namespace agh {
namespace ach {

using namespace std;



struct SControlParamSet {
	bool	DBAmendment1,
		DBAmendment2,
		AZAmendment1,
		AZAmendment2,
		ScoreUnscoredAsWake;

	gsl_siman_params_t
		siman_params;
		    // int n_tries
		    // 	The number of points to try for each step.
		    // int iters_fixed_T
		    // 	The number of iterations at each temperature.
		    // double step_size
		    // 	The maximum step size in the random walk.
		    // double k, t_initial, mu_t, t_min

	SControlParamSet()
		{
			reset();
		}

	SControlParamSet& operator=( const SControlParamSet&) = default;
	bool operator==( const SControlParamSet &rv) const;

	void check() const; // throws
	void reset();
};




class CModelRun
  : public agh::CProfile {

    public:
	CModelRun (const CModelRun&)
	      : tx (t0, tstep, tlo, thi)
		{
			throw runtime_error ("nono");
		}
	CModelRun () // oblige map
	      : tx (t0, tstep, tlo, thi)
		{
			throw runtime_error ("nono");
		}
	CModelRun (CModelRun&&);
	CModelRun (CSubject&, const string& session, const sigfile::SChannel&,
		   const agh::SProfileParamSet&,
		   const SControlParamSet&, const STunableSetWithState&);

	enum TModrunFlags { modrun_tried = 1 };
	int	status;
	SControlParamSet
		ctl_params;
	STunableSet<TTRole::d>	tstep;
	STunableSet<TTRole::l>	tlo;
	STunableSet<TTRole::u>	thi;
	STunableSet<TTRole::v>	t0;
	STunableSetWithState	tx;

	int watch_simplex_move( void (*)(void*));
	double snapshot();
	double cf;

  //	double _siman_metric( const void *xp, const void *yp) const;
  //	void _siman_step( const gsl_rng *r, void *xp,
  //			  double step_size);
    private:
	vector<sigfile::SPage>
		_scores2;  // we need shadow to hold scores as modified per Score{MVT,Unscored}As... and by t_{a,p},
			   // and also to avoid recreating it before each stride
	void _restore_scores_and_extend_rem( size_t, size_t);
	void _prepare_scores2();

	const double &_which_gc( size_t p) const; // selects episode egc by page, or returns &gc if !AZAmendment
};




inline const double&
CModelRun::
_which_gc( size_t p) const // selects episode egc by page, or returns &gc if !AZAmendment
{
	if ( ctl_params.AZAmendment1 )
		for ( size_t m = _mm_bounds.size()-1; m >= 1; --m )
			if ( p >= _mm_bounds[m].first )
				return tx[TTunable::gc + m];
	return tx[TTunable::gc];
}



} // namespace ach
} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
