// ;-*-C++-*- *  Time-stamp: "2011-03-19 18:24:55 hmmr"
/*
 *       File name:  libagh/tunable.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  tunables
 *
 *         License:  GPL
 */



#include "tunable.hh"

using namespace std;


const STunableDescription __AGHTT[_agh_basic_tunables_] = {
	{
		.918e-3,	.100e-3,	2.000e-3,	.001e-3,
		1e3,
		true,	-1,
		"rs",		"%5.3f",	"1/min"
	},
	{
		.283,		.01,		4.,		.002,
		1,
		true,	-1,
		"rc",	 	"%5.3f",	"1/min"
	},
	{
		.24,		.04,		4.,		.001,
		1,
		true,	-1,
		"fc_R",		"%4.2f",	"1/min"
	},
	{
		.6,		.04,		4.,		.001,
		1,
		true,	-1,
		"fc_W",		"%4.2f",	"1/min"
	},
	{
		300,		9,		1005,		1,
		1,
		true,	0,
		"S_0",		"%5.1f",	"%"
	},
	{
		500,		18,		2010,		1,
		1,
		true,	0,
		"S_U",	 	"%5.1f",	"%"
	},
	{
		3,		.4,		22,		1,
		1,
		false,	1,
		"t_a",		"%3.1f",	"min"
	},
	{
		3,		.4,		22,		1,
		1,
		false,	1,
		"t_p",		"%3.1f",	"min"
	},
	{
		.835e-2,	.010e-2,	4.000e-2,	.004e-2,
		1e2,
		true,	-1,
		"gc",		"%5.3f",	"1/min"
	},
};





void
STunableSet::assign_defaults()
{
	size_t t;
	for ( t = 0; t < _agh_basic_tunables_; ++t )
		P[t] = __AGHTT[t].def_val;
	for ( ; t < size(); ++t )
		P[t] = __AGHTT[t].def_val;
}





void
STunableSet::adjust_for_ppm( double ppm)
{
	for ( size_t t = 0; t < size(); ++t )
		P[t] *= pow( ppm, __AGHTT[t].time_adj);
}

void
STunableSet::unadjust_for_ppm( double ppm)
{
	for ( size_t t = 0; t < size(); t++ )
		P[t] /= pow( ppm, __AGHTT[t].time_adj);
}








void
STunableSetFull::assign_defaults()
{
	size_t t;
	for ( t = 0; t < _agh_basic_tunables_; ++t ) {
		value[t] =  __AGHTT[t].def_val;
		step [t] =  __AGHTT[t].def_step;
		lo   [t] =  __AGHTT[t].def_min;
		hi   [t] =  __AGHTT[t].def_max;
		state[t] =  0;
	}
	for ( ; t < size(); ++t ) {
		value[t] =  __AGHTT[_gc_].def_val;
		step [t] =  __AGHTT[_gc_].def_step;
		lo   [t] =  __AGHTT[_gc_].def_min;
		hi   [t] =  __AGHTT[_gc_].def_max;
		state[t] =  0;
	}
}


bool
STunableSetFull::check_consisitent() const
{
	for ( size_t t = 0; t < value.size(); t++ )
		if ( lo[t] >= hi[t] || step[t] >= (hi[t] - lo[t])/2 )
			return false;
	return true;
}







void
STunableSetFull::randomise()
{
	for ( size_t t = 0; t < value.P.size(); t++ )
		if ( step[t] > 0. )
			value[t] = lo[t] + (double)(rand() - RAND_MAX)/RAND_MAX * (hi[t] - lo[t]);
}




// EOF
