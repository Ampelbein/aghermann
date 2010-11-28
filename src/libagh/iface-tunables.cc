// ;-*-C++-*- *  Time-stamp: "2010-11-21 03:00:22 hmmr"
/*
 *       File name:  iface-tunables.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2010-05-01
 *
 *         Purpose:  C wrappers to deal with tunables
 *
 *         License:  GPL
 */


#include <cassert>
#include "tunable.hh"
#include "primaries.hh"
#include "iface.h"


#ifdef __cplusplus
extern "C" {
#endif


extern CExpDesign *AghCC;


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
agh_tunables0_get( struct SConsumerTunableSetFull *t_set)
{
	t_set->n_tunables = AghCC->tunables.size();
	t_set->tunables =
		(double*)realloc( t_set->tunables, t_set->n_tunables * sizeof(double));
	t_set->upper_bounds =
		(double*)realloc( t_set->upper_bounds, t_set->n_tunables * sizeof(double));
	t_set->lower_bounds =
		(double*)realloc( t_set->lower_bounds, t_set->n_tunables * sizeof(double));
	t_set->steps =
		(double*)realloc( t_set->steps, t_set->n_tunables * sizeof(double));
	t_set->states =
		(int*)realloc( t_set->states, t_set->n_tunables * sizeof(int));
	memcpy( t_set->tunables,     &AghCC->tunables.value[0], t_set->n_tunables * sizeof(double));
	memcpy( t_set->upper_bounds, &AghCC->tunables.hi[0],    t_set->n_tunables * sizeof(double));
	memcpy( t_set->lower_bounds, &AghCC->tunables.lo[0],    t_set->n_tunables * sizeof(double));
	memcpy( t_set->steps,        &AghCC->tunables.step[0],  t_set->n_tunables * sizeof(double));
	memcpy( t_set->states,       &AghCC->tunables.state[0], t_set->n_tunables * sizeof(int));
}

void
agh_tunables0_put( const struct SConsumerTunableSetFull *t_set)
{
	assert (t_set->n_tunables == AghCC->tunables.size());
	memcpy( &AghCC->tunables.value[0], t_set->tunables,     t_set->n_tunables * sizeof(double));
	memcpy( &AghCC->tunables.hi[0],    t_set->upper_bounds, t_set->n_tunables * sizeof(double));
	memcpy( &AghCC->tunables.lo[0],    t_set->lower_bounds, t_set->n_tunables * sizeof(double));
	memcpy( &AghCC->tunables.step[0],  t_set->steps,        t_set->n_tunables * sizeof(double));
	memcpy( &AghCC->tunables.state[0], t_set->states,       t_set->n_tunables * sizeof(int));
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
