// ;-*-C++-*- *  Time-stamp: "2010-12-20 02:07:48 hmmr"
/*
 *       File name:  core/iface-glib.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-09-26
 *
 *         Purpose:  C++ core structures and functions made accessible for ui
 *
 *         License:  GPL
 */


#ifndef _AGH_IFACE_GLIB_H
#define _AGH_IFACE_GLIB_H

#include <glib.h>

#include "iface.h"

#ifdef __cplusplus
extern "C" {
#endif


#define Ai(A,B,C) g_array_index(A,B,C)


size_t		agh_edf_get_scores_as_garray( TEDFRef, GArray*, size_t *pagesize_p);
int 		agh_edf_put_scores_as_garray( TEDFRef, GArray*);


// size_t		agh_edf_get_artifacts_as_garray( TEDFRef ref, const char *channel,
// 						 GArray *out);
// void		agh_edf_put_artifacts_as_garray( TEDFRef ref, const char *channel,
// 						 GArray *in);


size_t		agh_msmt_get_power_spectrum_as_double_garray( TRecRef, size_t p, GArray*);
size_t		agh_msmt_get_power_spectrum_as_float_garray( TRecRef, size_t p, GArray*);


size_t		agh_msmt_get_power_course_in_range_as_double_garray( TRecRef ref,
								     float from, float upto,
								     GArray *out);
size_t		agh_msmt_get_power_course_in_range_as_float_garray( TRecRef ref,
								    float from, float upto,
								    GArray *out);

void		agh_modelrun_get_all_courses_as_double_garray( TModelRef,
							       GArray *SWA_out, GArray *S_out, GArray *SWAsim_out,
							       GArray *scores_out);
void		agh_modelrun_get_mutable_courses_as_double_garray( TModelRef,
								   GArray *S_out, GArray *SWAsim_out);

#ifdef __cplusplus
}
#endif

#endif

// EOF
