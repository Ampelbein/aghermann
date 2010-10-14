// ;-*-C++-*- *  Time-stamp: "2010-10-15 01:49:08 hmmr"
/*
 *       File name:  core/iface.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  C++ core structures and functions made accessible for ui
 *
 *         License:  GPL
 */


#ifndef _AGH_IFACE_H
#define _AGH_IFACE_H

#include <time.h>
#include <gsl/gsl_siman.h>
#include "../common.h"

#if HAVE_CONFIG_H
#  include "config.h"
#endif



#ifdef __cplusplus
extern "C" {
#endif



// api: expdesign

typedef void (*TProgressIndicatorFun) (const char*, size_t, size_t);

int		agh_expdesign_init( const char* dir, TProgressIndicatorFun fun);
void		agh_expdesign_shutdown();
int		agh_expdesign_status();
const char*	agh_expdesign_messages();
const char*	agh_expdesign_last_message();
void		agh_expdesign_scan_tree( TProgressIndicatorFun);


// enumerate used names
size_t	agh_enumerate_groups( char***);
size_t	agh_enumerate_subjects( char***);
size_t	agh_enumerate_sessions( char***);
size_t	agh_enumerate_episodes( char***);
size_t	agh_enumerate_all_channels( char***);
size_t	agh_enumerate_eeg_channels( char***);
void	agh_free_enumerated_array( char**);


// edf sources
struct SSignal_lite {
	const char
		*Channel,
		*SignalType;
	size_t	SamplesPerRecord;
};
struct SEDFFile {

	int	status;
	const char*
		filename;

      // static fields (raw)
	const char
		*PatientID,
		*Session,
		*Episode;
	size_t	NDataRecords,
		DataRecordSize,
		NSignals;
	struct tm
		timestamp_struct;
	time_t	start_time,
		end_time;

	struct SSignal_lite*
		signals;
};

typedef void* TEDFRef;

// get info on a yet unattached edf file
const struct SEDFFile*	agh_edf_get_info_from_file( const char* fname, char** recp);
// get info on an edf that has been registered
const struct SEDFFile*	agh_edf_get_info_from_sourceref( TEDFRef, char**);
const struct SEDFFile*	agh_edf_find_by_jdeh( const char* j, const char* d, const char* e, const char* h,
					      TEDFRef*);
const struct SEDFFile*	agh_edf_find_first( TEDFRef*);
const struct SEDFFile*	agh_edf_find_next( TEDFRef*);
int	agh_edf_write_header( TEDFRef);
size_t	agh_edf_get_scores( TEDFRef, char**, size_t *pagesize_p);
int 	agh_edf_put_scores( TEDFRef, char*);
int	agh_edf_import_scores( TEDFRef, const char *fname);
int	agh_edf_export_scores( TEDFRef, const char *fname);

int	agh_edf_add_or_mod_unfazer( TEDFRef,
				    const char *target_channel,
				    const char *offending_channel,
				    double factor);
double	agh_edf_get_unfazer_factor( TEDFRef, const char*, const char*);
void	agh_edf_remove_unfazer( TEDFRef, const char*, const char*);

void	agh_explain_edf_status( int status, char **out_p);



// group

const char*	agh_group_find_first();
const char*	agh_group_find_next();


// subject

typedef void* TRecRef;

struct SEpisode {  // is synonymous with C++ class in CSubject, but nevermind
	const char
		*name;
	size_t	length;  // in seconds
	time_t	start, start_rel,
		end, end_rel;
	size_t	n_recordings;
	TRecRef	*recordings;
};

struct SSession {
	const char
		*name;
	size_t n_episodes;
	struct SEpisode *episodes;
};

struct SSubject {
	const char
		*name,
		*group;
	TGender gender;
	int age;
	const char *comment;
	size_t n_sessions;
	struct SSession *sessions;
};

size_t	agh_subject_get_n_of();
size_t	agh_subject_get_n_of_in_group(const char*);

// if you pass a recp argument for the agh_subject_find_* functions
// below, make sure you call agh_SSubject_destruct on it when unneeded
// or before a next call to agh_subject_find_*, or you will leak
// memory
const struct SSubject*	agh_subject_find_by_name( const char* j_name, struct SSubject*);
const struct SSubject*	agh_subject_find_first_in_group( const char* which_group, struct SSubject*);
const struct SSubject*	agh_subject_find_next_in_group( struct SSubject*);

void	agh_SSubject_destruct( struct SSubject*);

// measurement


TRecRef		agh_msmt_find_by_jdeh( const char *j,
				       const char *d,
				       const char *e,
				       const char *h);
TEDFRef		agh_msmt_get_source( TRecRef);
const char*	agh_msmt_get_signal_type( TRecRef);
const char*	agh_msmt_get_signal_name( TRecRef);
size_t		agh_msmt_get_pagesize( TRecRef);
size_t		agh_msmt_get_signal_data_as_double( TRecRef,
						    double **out_p,
						    size_t *samplerate,
						    float *signal_scale);
size_t		agh_msmt_get_signal_data_as_float( TRecRef,
						   float **out_p,
						   size_t *samplerate,
						   float *signal_scale);
size_t		agh_msmt_get_signal_data_unfazed_as_double( TRecRef,
							    double **out_p,
							    size_t *samplerate,
							    float *signal_scale);
size_t		agh_msmt_get_signal_data_unfazed_as_float( TRecRef,
							   float **out_p,
							   size_t *samplerate,
							   float *signal_scale);

size_t		agh_msmt_get_track( TRecRef, char**);
void		agh_msmt_put_track( TRecRef, char*);

size_t		agh_msmt_get_power_spectrum_as_double( TRecRef, size_t p, double**);
size_t		agh_msmt_get_power_spectrum_as_float( TRecRef, size_t p, float**);

size_t		agh_msmt_get_power_course_as_double_direct( TRecRef, double*);
size_t		agh_msmt_get_power_course_as_float_direct( TRecRef, float*);
size_t		agh_msmt_get_power_course_as_double( TRecRef, double**);
size_t		agh_msmt_get_power_course_as_float( TRecRef, float**);
size_t		agh_msmt_get_power_course_in_range_as_double( TRecRef, float, float,
							      double**);
size_t		agh_msmt_get_power_course_in_range_as_float( TRecRef, float, float,
							     float**);
size_t		agh_msmt_get_power_course_in_range_as_double_direct( TRecRef, float, float,
								     double*);
size_t		agh_msmt_get_power_course_in_range_as_float_direct( TRecRef, float, float,
								    float*);
char*		agh_msmt_fname_base( TRecRef);
int		agh_msmt_export_power( TRecRef, const char *fname);
int		agh_msmt_export_power_in_range( TRecRef, float, float, const char *fname);

size_t		agh_fft_get_pagesize();
void		agh_fft_set_pagesize( size_t);
float		agh_fft_get_binsize();
void		agh_fft_set_binsize( float);
TFFTWinType	agh_fft_get_window_type();
void		agh_fft_set_window_type( TFFTWinType);

TFFTWinType	agh_af_get_window_type();
void		agh_af_set_window_type( TFFTWinType);
size_t		agh_af_get_smoothover();
void		agh_af_set_smoothover( size_t);

typedef /* list<CSimulation>::iterator */ void* TModelRef;
TModelRef	agh_modelrun_find_first();
TModelRef	agh_modelrun_find_next();
TModelRef	agh_modelrun_find_by_jdhq( const char *j_name, const char *d_name, const char *h_name,
					  float from, float upto);
int		agh_modelrun_exist( const char *j_name, const char *d_name, const char *h_name,
				    float from, float upto);

const char*	agh_modelrun_get_subject( TModelRef);
const char*	agh_modelrun_get_session( TModelRef);
const char*	agh_modelrun_get_channel( TModelRef);
size_t		agh_modelrun_get_n_episodes( TModelRef);
size_t		agh_modelrun_get_nth_episode_start_p( TModelRef, size_t);
size_t		agh_modelrun_get_nth_episode_end_p( TModelRef, size_t);
size_t		agh_modelrun_get_pagesize( TModelRef);

void		agh_modelrun_get_all_courses_as_double( TModelRef,
							double **SWA_out, double **S_out, double **SWAsim_out,
							char **scores_out);
void		agh_modelrun_get_mutable_courses_as_double( TModelRef,
							    double **S_out, double **SWAsim_out);

int		agh_modelrun_setup( const char *j, const char *d, const char *h,
				    float from, float upto,
				    TModelRef*);
void		agh_modelrun_reset( TModelRef);
int		agh_modelrun_run( TModelRef);
void		agh_modelrun_snapshot( TModelRef);  // do a single cycle to recreate variable courses
void		agh_modelrun_save( TModelRef);

int		agh_modelrun_tsv_export_one( TModelRef, const char *fname);
int		agh_modelrun_tsv_export_all( const char *fname);

void		agh_modelrun_collect_from_tree( float, float);

struct SConsumerTunableSet {
	size_t	n_tunables;
	double	*tunables;
};

void		agh_modelrun_get_tunables( TModelRef Ri, struct SConsumerTunableSet*);
void		agh_modelrun_put_tunables( TModelRef Ri, const struct SConsumerTunableSet*);

struct SConsumerCtlParams {
	gsl_siman_params_t
		siman_params;
	int	DBAmendment1:1,
		DBAmendment2:1,
		AZAmendment:1,
		ScoreMVTAsWake:1,
		ScoreUnscoredAsWake:1;
};

void		agh_modelrun_get_ctlparams( TModelRef Ri, struct SConsumerCtlParams*);
void		agh_modelrun_put_ctlparams( TModelRef Ri, const struct SConsumerCtlParams*);


const struct STunableDescription*
		agh_tunable_get_description( size_t);

struct SConsumerTunableSetFull {
	size_t	n_tunables;
	double	*tunables,
		*upper_bounds,
		*lower_bounds,
		*steps;
	int	*states;
};
#define T_REQUIRED_C 1

void		agh_tunables0_get( struct SConsumerTunableSetFull*);
void		agh_tunables0_put( const struct SConsumerTunableSetFull*);
void		agh_tunables0_stock_defaults();

void		agh_ctlparams0_get( struct SConsumerCtlParams*);
void		agh_ctlparams0_put( const struct SConsumerCtlParams*);

int		agh_channel_follows_1020( const char* h);
const char*	agh_signal_type_following_Kemp( const char* h);
int		agh_signal_type_is_fftable( const char*);



#ifdef __cplusplus
}
#endif


#endif

// EOF
