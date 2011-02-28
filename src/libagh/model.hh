// ;-*-C++-*- *  Time-stamp: "2011-03-01 01:18:47 hmmr"

/*
 * Author: Andrei Zavada (johnhommer@gmail.com)
 *
 * License: GPL
 *
 * Initial version: 2010-04-28
 */

#ifndef _AGH_MODEL_H
#define _AGH_MODEL_H


#if HAVE_CONFIG_H
#  include "config.h"
#endif


#include <string>
#include <vector>
#include <list>
#include <valarray>
#include <functional>
#include <stdexcept>

#include <sys/time.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_siman.h>

#include "misc.hh"
#include "page.hh"
#include "tunable.hh"

using namespace std;



typedef size_t sid_type;
typedef size_t hash_key;

class CSubject;



class CRecording;

class CSCourse {

	CSCourse();

    public:
	size_t	_sim_start,
		_sim_end,
		_baseline_end,
		_pages_with_SWA,
		_pages_in_bed;
	double	_SWA_L,
		_SWA_0,	_SWA_100;
	float	freq_from, freq_upto;

	time_t	_0at;
	vector<SPageSimulated>
		timeline;
	typedef pair<size_t, size_t> TBounds;
	vector<TBounds>  // in pages
		mm_bounds;

	typedef vector<const CRecording*> TMsmtPtrList;
	TMsmtPtrList mm_list;

	CSCourse( TMsmtPtrList& MM, // size_t start_page, size_t end_page,
		  float ifreq_from, float ifreq_upto,
		  float req_percent_scored = 90,
		  size_t swa_laden_pages_before_SWA_0 = 3)
	      : _sim_start (-1), _sim_end (-1),
		freq_from (ifreq_from), freq_upto (ifreq_upto),
		mm_list (MM)
		{
			int retval = layout_measurements( MM,
							  freq_from, freq_upto,
							  req_percent_scored,
							  swa_laden_pages_before_SWA_0);
			if ( retval )
				throw retval;
//				throw logic_error (string (simprep_perror(retval)));
		}

	time_t nth_msmt_start_time( size_t n) const
		{
			return _0at + mm_bounds[n].first * _pagesize;
		}
	time_t nth_msmt_end_time( size_t n) const
		{
			return _0at + mm_bounds[n].second * _pagesize;
		}

	int layout_measurements( TMsmtPtrList&,
//				 size_t start_page, size_t end_page,
				 float freq_from, float freq_upto,
				 float req_percent_scored = 90,
				 size_t swa_laden_pages_before_SWA_0 = 3);
    private:
	size_t	_pagesize;  // since power is binned each time it is
			    // collected in layout_measurements() and
			    // then detached, we keep it here
			    // privately
    public:
	size_t pagesize() const
		{
			return _pagesize;
		}

	const char
		*subject,   // plus these bits, because this saves a lot of lookups
	        *channel,   // They are filled out in layout_measurements
		*session;

	bool matches( const char* j, const char* d, const char* h,
		      float ffrom, float fupto) const
		{
			return	strcmp( subject, j) == 0 &&
				strcmp( session, d) == 0 &&
				strcmp( channel, h) == 0 &&
				freq_from == ffrom && freq_upto == fupto;
		}
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

	bool	DBAmendment1:1,
		DBAmendment2:1,
		AZAmendment:1,
		ScoreMVTAsWake:1,
		ScoreUnscoredAsWake:1;

	SControlParamSet()
		{
			assign_defaults();
		}

	bool is_sane()
		{
			return	siman_params.n_tries > 1 &&
				siman_params.iters_fixed_T > 1 &&
				siman_params.step_size > 0. &&
				siman_params.k > 0. &&
				siman_params.t_initial > 0. &&
				siman_params.t_min > 0. &&
				siman_params.t_min < siman_params.t_initial &&
				siman_params.mu_t > 0;
		}

	void assign_defaults()
		{
			siman_params.n_tries		= 200;
			siman_params.iters_fixed_T	= 100;
			siman_params.step_size		= 1.;
			siman_params.k		=    1.0;
			siman_params.t_initial  =  200.;
			siman_params.mu_t	=    1.003;
			siman_params.t_min	=    1.;

			DBAmendment1 = true;
			DBAmendment2 = false;
			ScoreMVTAsWake = false;
			ScoreUnscoredAsWake = true;
		}
	bool operator==( const SControlParamSet &rv) const
		{
			return	memcmp( &siman_params, &rv.siman_params, sizeof(siman_params)) == 0 &&
				DBAmendment1 == rv.DBAmendment1 &&
				DBAmendment2 == rv.DBAmendment2 &&
				ScoreMVTAsWake == rv.ScoreMVTAsWake &&
				ScoreUnscoredAsWake == rv.ScoreUnscoredAsWake;
		}
};




class CExpDesign;

namespace NSSiman {
	double	_cost_function( void *xp);
	void	_siman_step( const gsl_rng *r, void *xp, double step_size);
	double	_siman_metric( void *xp, void *yp);
};

class CModelRun
  : public SControlParamSet, public STunableSetFull,
    public CSCourse {

    friend class CExpDesign;
    friend class CSimulation;

	CModelRun();

    public:
	int	status;

	STunableSet
		cur_tset;

	CModelRun( CSCourse::TMsmtPtrList& MM,
		   float freq_from, float freq_upto,
		   const SControlParamSet& ctl_params, const STunableSetFull& t0,
		   float req_percent_scored = 90,
		   size_t swa_laden_pages_before_SWA_0 = 3)
//		throw (logic_error)
	      : SControlParamSet (ctl_params),
		STunableSetFull (t0),
		CSCourse( MM, freq_from, freq_upto,
			  req_percent_scored, swa_laden_pages_before_SWA_0),
		status (0),
		cur_tset (t0.value)
		{
			_prepare_scores2();
		}

	int watch_simplex_move( void (*)(void*));
	double snapshot()
		{
			return _cost_function( &cur_tset.P[0]);
		}

    private:
	vector<SPage>
		_scores2;  // we need shadow to hold scores as modified per Score{MVT,Unscored}As... and by t_{a,p},
			   // and also to avoid recreating it before each stride
//	size_t	_pagesize;
	// pagesize is taken as held in CSCourse, collected from edf sources during layout_measurements()
	// the pagesize used for displaying the EEGs is a totally different matter and it
	// does not even belong in core

	void _restore_scores_and_extend_rem( size_t, size_t);
	void _prepare_scores2();

	friend double NSSiman::_cost_function( void*);
	double _cost_function( const void *xp);  // aka fit
	friend double NSSiman::_siman_metric( void*, void*);
	double _siman_metric( const void *xp, const void *yp) const
		{
			return STunableSet (value.P.size() - _gc_, (double*)xp).distance(
				STunableSet (value.P.size() - _gc_, (double*)yp),
				step);
		}
	friend void NSSiman::_siman_step( const gsl_rng *r, void *xp, double step_size);
	void _siman_step( const gsl_rng *r, void *xp,
			  double step_size);

	double &_which_gc( size_t p)  // selects episode egc by page, or returns &gc if !AZAmendment
		{
			if ( AZAmendment )
				for ( size_t m = mm_bounds.size()-1; m >= 1; --m )
					if ( p >= mm_bounds[m].first )
						return cur_tset[_gc_ + m];
			return cur_tset[_gc_];
		}
};



class CSimId {
    public:
	hash_key
		_subject,
		_session,
		_channel;
	float	_from,
		_upto;
	CSimId( const char *j, const char *d, const char *h,
		float from, float upto)
	      : _subject (HASHKEY(j)), _session (HASHKEY(d)), _channel (HASHKEY(h)),
		_from (from), _upto (upto)
		{}
	CSimId( hash_key j, hash_key d, hash_key h,
		float from, float upto)
	      : _subject (j), _session (d), _channel (h),
		_from (from), _upto (upto)
		{}
};

class CSubject;

class CSimulation
  : public CModelRun {

	CSimulation();

	string	_backup_fname;

    public:
	bool matches( const char* j, const char* d, const char* h,
		      float ffrom, float fupto,
		      SControlParamSet& cset) const
		{
			return	strcmp( subject, j) == 0 &&
				strcmp( session, d) == 0 &&
				strcmp( channel, h) == 0 &&
				freq_from == ffrom && freq_upto == fupto &&
				((SControlParamSet)(*this)) == cset;
		}
	bool operator==( const CSimId& rv) const
		{
			return	HASHKEY(subject) == rv._subject &&
				HASHKEY(session) == rv._session &&
				HASHKEY(channel) == rv._channel &&
				freq_from == rv._from &&
				freq_upto == rv._upto;
		}

	CSimulation( CSCourse::TMsmtPtrList& MM,
		     float freq_from, float freq_upto,
		     const SControlParamSet& ctl_params,
		     const STunableSetFull& t0,
		     const char* backup_fname,
		     float req_percent_scored = 90,
		     size_t swa_laden_pages_before_SWA_0 = 3)
//		throw (logic_error)
	      : CModelRun( MM,
			   freq_from, freq_upto,
			   ctl_params, t0,
			   req_percent_scored,
			   swa_laden_pages_before_SWA_0),
		_backup_fname (backup_fname)
		{
//			if ( load() != 0 )
//				;
		}

       ~CSimulation()
		{
//			save();
		}

      // will only save tunables, S and SWA_sim
	int save( const char*, bool binary = true);
	int save( bool binary = true)
		{
			return save( _backup_fname.c_str(), binary);
		}
	int load( const char*);
	int load( bool binary = true)
		{
			return load( _backup_fname.c_str());
		}
};


extern gsl_rng *__agh_rng;
void init_global_rng();

#endif

// EOF
