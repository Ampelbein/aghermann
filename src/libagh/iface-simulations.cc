// ;-*-C++-*- *  Time-stamp: "2011-02-27 18:19:53 hmmr"
/*
 *       File name:  libagh/iface-simulations.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-01
 *
 *         Purpose:  
 *
 *         License:  GPL
 */



#include <cassert>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include "iface.h"
#include "primaries.hh"
#include "tunable.hh"

extern CExpDesign *AghCC;


#ifdef __cplusplus
extern "C" {
#endif


float
agh_req_percent_scored_get()
{
	return AghCC -> req_percent_scored;
}

void
agh_req_percent_scored_set( float v)
{
	if ( v > 0.2 && v <= 1. )
		AghCC -> req_percent_scored = v;
}







const char*
agh_modelrun_get_subject( TModelRef Ri)
{
	return static_cast<CSimulation*>(Ri) -> subject;
}
const char*
agh_modelrun_get_session( TModelRef Ri)
{
	return static_cast<CSimulation*>(Ri) -> session;
}
const char*
agh_modelrun_get_channel( TModelRef Ri)
{
	return static_cast<CSimulation*>(Ri) -> channel;
}


float
agh_modelrun_get_freqrange( TModelRef Ri, float *from_p, float *upto_p)
{
	if ( from_p )
		*from_p = static_cast<CSimulation*>(Ri) -> freq_from;
	if ( upto_p )
		*upto_p = static_cast<CSimulation*>(Ri) -> freq_upto;
	return static_cast<CSimulation*>(Ri) -> freq_upto -
		static_cast<CSimulation*>(Ri) -> freq_from;
}


int
agh_modelrun_setup( const char *j_name, const char *d_name, const char *h_name,
		    float from, float upto,
		    TModelRef *Rp,
		    const char **error_p)
{
	try {
		return AghCC -> setup_modrun( j_name, d_name, h_name,
					      from, upto,
					      (CSimulation*&)*Rp);
//	} catch (logic_error ex) {
	} catch (int ex) {
		fprintf( stderr, "agh_modelrun_setup( %s, %s, %s, %g, %g): %s\n",
			 j_name, d_name, h_name, from, upto, simprep_perror(ex));
		if ( error_p )
			*error_p = AghCC->last_error();
		return ex;
	}
}


static struct SConsumerSCourseSetupInfo
	__scourse_setup_info_consumable;

const struct SConsumerSCourseSetupInfo*
agh_modelrun_get_scourse_setup_info( TModelRef Ri,
				     SConsumerSCourseSetupInfo *out_p)
{
	__scourse_setup_info_consumable._sim_start      = static_cast<CSimulation*>(Ri) -> _sim_start;
	__scourse_setup_info_consumable._sim_end        = static_cast<CSimulation*>(Ri) -> _sim_end;
	__scourse_setup_info_consumable._baseline_end   = static_cast<CSimulation*>(Ri) -> _baseline_end;
	__scourse_setup_info_consumable._pages_with_SWA = static_cast<CSimulation*>(Ri) -> _pages_with_SWA;
	__scourse_setup_info_consumable._pages_in_bed   = static_cast<CSimulation*>(Ri) -> _pages_in_bed;
	__scourse_setup_info_consumable._SWA_L          = static_cast<CSimulation*>(Ri) -> _SWA_L;
	__scourse_setup_info_consumable._SWA_0          = static_cast<CSimulation*>(Ri) -> _SWA_0;
	__scourse_setup_info_consumable._SWA_100        = static_cast<CSimulation*>(Ri) -> _SWA_100;
	if ( out_p )
		memcpy( out_p, &__scourse_setup_info_consumable, sizeof(SConsumerSCourseSetupInfo));
	return &__scourse_setup_info_consumable;
}



int
agh_modelrun_run( TModelRef Ri, void (*printer)(void*))
{
	return static_cast<CSimulation*>(Ri) -> watch_simplex_move( printer);
}


double
agh_modelrun_snapshot( TModelRef Ri)
{
	return static_cast<CSimulation*>(Ri) -> snapshot();
}




TModelRef
agh_modelrun_find_by_jdhq( const char *j_name, const char *d_name, const char *h_name,
			   float from, float upto)
{
	try {
		auto SL =
			AghCC -> subject_by_x(j_name)
			. measurements.at(d_name)
			. modrun_sets[h_name];
		for ( auto I = SL.begin(); I != SL.end(); ++I )
			if ( pair<float,float>(from, upto) == I->first )
				return static_cast<TModelRef>(&I->second);
		return NULL;
	} catch (invalid_argument ex) {
		return NULL;
	}
}


void
agh_modelrun_reset( TModelRef Ri)
{
	AghCC -> reset_modrun( *(static_cast<CSimulation*>(Ri)));
}




size_t
agh_modelrun_get_n_episodes( TModelRef Ri)
{
	return (static_cast<CSimulation*>(Ri)) -> mm_bounds.size();
}




size_t
agh_modelrun_get_nth_episode_start_p( TModelRef Ri, size_t e)
{
	return (static_cast<CSimulation*>(Ri)) -> mm_bounds[e].first;
}

size_t
agh_modelrun_get_nth_episode_end_p( TModelRef Ri, size_t e)
{
	return static_cast<CSimulation*>(Ri) -> mm_bounds[e].second;
}

size_t
agh_modelrun_get_pagesize( TModelRef Ri)
{
	return (static_cast<CSimulation*>(Ri)) -> pagesize();
}






size_t
agh_modelrun_get_all_courses_as_double( TModelRef Ri,
					double **SWA_out, double **S_out, double **SWAsim_out,
					char **scores_out)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);
	size_t p;
	*SWA_out    = (double*)malloc( __R->timeline.size() * sizeof(double));
	*S_out      = (double*)malloc( __R->timeline.size() * sizeof(double));
	*SWAsim_out = (double*)malloc( __R->timeline.size() * sizeof(double));
	*scores_out = (char*)  malloc( __R->timeline.size() * sizeof(char));
	for ( p = 0; p < __R->timeline.size(); ++p ) {
		SPageSimulated &P = __R->timeline[p];
		(*SWA_out)   [p] = P.SWA;
		(*S_out)     [p] = P.S;
		(*SWAsim_out)[p] = P.SWA_sim;
		(*scores_out)[p] = P.p2score();
	}
	return __R->timeline.size();
}

size_t
agh_modelrun_get_mutable_courses_as_double( TModelRef Ri,
					    double **S_out, double **SWAsim_out)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);
	size_t p;
	*S_out      = (double*)malloc( __R->timeline.size() * sizeof(double));
	*SWAsim_out = (double*)malloc( __R->timeline.size() * sizeof(double));
	for ( p = 0; p < __R->timeline.size(); ++p ) {
		SPageSimulated &P = __R->timeline[p];
		(*S_out)     [p] = P.S;
		(*SWAsim_out)[p] = P.SWA_sim;
	}
	return __R->timeline.size();
}



void
agh_modelrun_get_all_courses_as_double_direct( TModelRef Ri,
					       double *SWA_out, double *S_out, double *SWAsim_out,
					       char *scores_out)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);
	size_t p;
	for ( p = 0; p < __R->timeline.size(); ++p ) {
		SPageSimulated &P = __R->timeline[p];
		SWA_out   [p] = P.SWA;
		S_out     [p] = P.S;
		SWAsim_out[p] = P.SWA_sim;
		scores_out[p] = P.p2score();
	}
}

void
agh_modelrun_get_mutable_courses_as_double_direct( TModelRef Ri,
						   double *S_out, double *SWAsim_out)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);
	size_t p;
	for ( p = 0; p < __R->timeline.size(); ++p ) {
		SPageSimulated &P = __R->timeline[p];
		S_out     [p] = P.S;
		SWAsim_out[p] = P.SWA_sim;
	}
}






void
agh_modelrun_save( TModelRef Ri)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);
	__R->save( AghCC -> make_fname_simulation( __R->subject, __R->session, __R->channel,
						   __R->freq_from, __R->freq_upto).c_str());
}


int
agh_modelrun_get_status( TModelRef Ri)
{
	return static_cast<CSimulation*>(Ri) -> status;
}


void
agh_modelrun_remove_untried()
{
	for ( auto Gi = AghCC->groups_begin(); Gi != AghCC->groups_end(); ++Gi )
		for ( auto Ji = Gi->second.begin(); Ji != Gi->second.end(); ++Ji )
			for ( auto Di = Ji->measurements.begin(); Di != Ji->measurements.end(); ++Di )
			retry_modruns:
				for ( auto RSi = Di->second.modrun_sets.begin(); RSi != Di->second.modrun_sets.end(); ++RSi ) {
				retry_this_modrun_set:
					for ( auto Ri = RSi->second.begin(); Ri != RSi->second.end(); ++Ri )
						if ( !(Ri->second.status & AGH_MODRUN_TRIED) ) {
							RSi->second.erase( Ri);
							goto retry_this_modrun_set;
						}
					if ( RSi->second.empty() ) {
						Di->second.modrun_sets.erase( RSi);
						goto retry_modruns;
					}
				}
}




void
agh_modelrun_get_tunables( TModelRef Ri, struct SConsumerTunableSet *t_set)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);
	t_set->n_tunables = __R->cur_tset.P.size();
	memcpy( t_set->tunables, &__R->cur_tset.P[0], t_set->n_tunables * sizeof(double));
}

void
agh_modelrun_put_tunables( TModelRef Ri, const struct SConsumerTunableSet *t_set)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);
	assert (t_set->n_tunables == __R->cur_tset.P.size());
	memcpy( &__R->cur_tset.P[0], t_set->tunables, t_set->n_tunables * sizeof(double));
}



void
agh_modelrun_get_ctlparams( TModelRef Ri, struct SConsumerCtlParams *ctl_params)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);
	memcpy( &ctl_params->siman_params, &__R->siman_params, sizeof(gsl_siman_params_t));
	ctl_params->DBAmendment1 = __R->DBAmendment1;
	ctl_params->DBAmendment2 = __R->DBAmendment2;
	ctl_params->AZAmendment  = __R->AZAmendment;
	ctl_params->ScoreMVTAsWake = __R->ScoreMVTAsWake;
	ctl_params->ScoreUnscoredAsWake = __R->ScoreUnscoredAsWake;
}
void
agh_modelrun_put_ctlparams( TModelRef Ri, const struct SConsumerCtlParams *ctl_params)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);
	memcpy( &__R->siman_params, &ctl_params->siman_params, sizeof(gsl_siman_params_t));
	__R->DBAmendment1 = ctl_params->DBAmendment1;
	__R->DBAmendment2 = ctl_params->DBAmendment2;
	__R->AZAmendment  = ctl_params->AZAmendment;
	__R->ScoreMVTAsWake = ctl_params->ScoreMVTAsWake;
	__R->ScoreUnscoredAsWake = ctl_params->ScoreUnscoredAsWake;
}



int
agh_modelrun_tsv_export_one( TModelRef Ri, const char *fname)
{
	CSimulation* __R = static_cast<CSimulation*>(Ri);

	FILE *f = fopen( fname, "w");
	if ( !f )
		return -1;

	fprintf( f, "#Subject: %s;  Session: %s;  Channel: %s;  Range: %g-%g\n#",
		 __R->subject, __R->session, __R->channel,
		 __R->freq_from, __R->freq_upto);
	size_t t;
	for ( t = 0; t < _gc_+1; ++t )
		fprintf( f, "\t%s", __AGHTT[t].name);
	for ( ; t < __R->cur_tset.P.size(); ++t )
		fprintf( f, "\tgc%zu", t - _gc_);
	fprintf( f, "\n");

	for ( t = 0; t < __R->cur_tset.P.size(); ++t )
		fprintf( f, "\t%g", __R->cur_tset.P[t]);

	fclose( f);

	return 0;
}

int
agh_modelrun_tsv_export_all( const char* fname)
{
	FILE *f = fopen( fname, "w");
	if ( !f )
		return -1;


	fclose( f);

	return 0;
}

#undef __R






void
agh_ctlparams0_get( struct SConsumerCtlParams *ctl_params)
{
	memcpy( &ctl_params->siman_params, &AghCC->control_params.siman_params, sizeof(gsl_siman_params_t));
	ctl_params->DBAmendment1 = AghCC->control_params.DBAmendment1;
	ctl_params->DBAmendment2 = AghCC->control_params.DBAmendment2;
	ctl_params->AZAmendment  = AghCC->control_params.AZAmendment;
	ctl_params->ScoreMVTAsWake = AghCC->control_params.ScoreMVTAsWake;
	ctl_params->ScoreUnscoredAsWake = AghCC->control_params.ScoreUnscoredAsWake;
}
void
agh_ctlparams0_put( const struct SConsumerCtlParams *ctl_params)
{
	memcpy( &AghCC->control_params.siman_params, &ctl_params->siman_params, sizeof(gsl_siman_params_t));
	AghCC->control_params.DBAmendment1 = ctl_params->DBAmendment1;
	AghCC->control_params.DBAmendment2 = ctl_params->DBAmendment2;
	AghCC->control_params.AZAmendment  = ctl_params->AZAmendment;
	AghCC->control_params.ScoreMVTAsWake = ctl_params->ScoreMVTAsWake;
	AghCC->control_params.ScoreUnscoredAsWake = ctl_params->ScoreUnscoredAsWake;
}






const struct STunableDescription*
agh_tunable_get_description( size_t t)
{
	static STunableDescription gcn (__AGHTT[_gc_]);
	if ( t <= _gc_ )
		return &__AGHTT[t];
	else {
		static char gc_adlib[10];
		snprintf( gc_adlib, 9, "gc(%zd)", t - _gc_);
		gcn.name = const_cast<const char*> (gc_adlib);
		return &gcn;
	}
}


void
agh_tunables0_get( struct SConsumerTunableSetFull *t_set, size_t n)
{
	assert ((t_set->n_tunables = n) < _agh_n_tunables_);
	memcpy( t_set->tunables,     &AghCC->tunables.value[0], n * sizeof(double));
	memcpy( t_set->upper_bounds, &AghCC->tunables.hi   [0], n * sizeof(double));
	memcpy( t_set->lower_bounds, &AghCC->tunables.lo   [0], n * sizeof(double));
	memcpy( t_set->steps,        &AghCC->tunables.step [0], n * sizeof(double));
	memcpy( t_set->states,       &AghCC->tunables.state[0], n * sizeof(int));
}

void
agh_tunables0_put( const struct SConsumerTunableSetFull *t_set, size_t n)
{
	assert (n < _agh_n_tunables_);
	AghCC->tunables.resize(n);
	memcpy( &AghCC->tunables.value[0], t_set->tunables,     n * sizeof(double));
	memcpy( &AghCC->tunables.hi   [0], t_set->upper_bounds, n * sizeof(double));
	memcpy( &AghCC->tunables.lo   [0], t_set->lower_bounds, n * sizeof(double));
	memcpy( &AghCC->tunables.step [0], t_set->steps,        n * sizeof(double));
	memcpy( &AghCC->tunables.state[0], t_set->states,       n * sizeof(int));
}


void
agh_tunables0_stock_defaults()
{
	AghCC->tunables.assign_defaults();
}



#ifdef __cplusplus
}
#endif


// EOF
