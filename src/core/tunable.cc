// ;-*-C++-*-
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

namespace agh {


const STunableSet::STunableDescription STunableSet::stock[(size_t)TTunable::_basic_tunables] = {
	{
		.918e-3,	.100e-3,	2.000e-3,	.001e-3,
		1e3, 0.001,
		true,	-1,
		"rs",		"%5.3f",	"1/min",
		"S Rise rate",
		"S Rise rate",
	},
	{
		.283,		.01,		4.,		.002,
		1, .001,
		true,	-1,
		"rc",	 	"%5.3f",	"1/min",
		"SWA rise rate",
		"SWA rise rate",
	},
	{
		.24,		.04,		4.,		.001,
		1, .0001,
		true,	-1,
		"fc_R",		"%4.2f",	"1/min",
		"SWA decay rate in REM",
		"SWA decay rate in REM",
	},
	{
		.6,		.04,		4.,		.001,
		1, .01,
		true,	-1,
		"fc_W",		"%4.2f",	"1/min",
		"SWA decay rate in Wake",
		"SWA decay rate in Wake",
	},
	{
		300,		9,		1005,		1,
		1, 1,
		true,	0,
		"S_0",		"%5.1f",	"%",
		"Starting value of S",
		"Starting value of S",
	},
	{
		500,		18,		2010,		1,
		1, 1,
		true,	0,
		"S_U",	 	"%5.1f",	"%",
		"S upper asymptote",
		"S upper asymptote",
	},
	{
		3,		.4,		22,		1,
		1, .1,
		false,	1,
		"t_a",		"%3.1f",	"min",
		"REM anticipation time",
		"REM anticipation time",
	},
	{
		3,		.4,		22,		1,
		1, .1,
		false,	1,
		"t_p",		"%3.1f",	"min",
		"REM extension time",
		"REM extension time",
	},
	{
		.835e-2,	.010e-2,	4.000e-2,	.004e-2,
		1e2, .001,
		true,	-1,
		"gc",		"%5.3f",	"1/min"
	},
};





void
STunableSet::assign_defaults()
{
	size_t t;
	for ( t = 0; t < TTunable::_basic_tunables; ++t )
		P[t] = stock[t].def_val;
	for ( ; t < size(); ++t )
		P[t] = stock[t].def_val;
}





void
STunableSet::adjust_for_ppm( double ppm)
{
	for ( size_t t = 0; t < size(); ++t )
		P[t] *= pow( ppm, stock[t].time_adj);
}

void
STunableSet::unadjust_for_ppm( double ppm)
{
	for ( size_t t = 0; t < size(); t++ )
		P[t] /= pow( ppm, stock[t].time_adj);
}








void
STunableSetFull::assign_defaults()
{
	size_t t;
	for ( t = 0; t < (int)TTunable::_basic_tunables; ++t ) {
		value[t] =  STunableSet::stock[t].def_val;
		step [t] =  STunableSet::stock[t].def_step;
		lo   [t] =  STunableSet::stock[t].def_min;
		hi   [t] =  STunableSet::stock[t].def_max;
		state[t] =  0;
	}
	for ( ; t < size(); ++t ) {
		value[t] =  STunableSet::stock[(size_t)TTunable::gc].def_val;
		step [t] =  STunableSet::stock[(size_t)TTunable::gc].def_step;
		lo   [t] =  STunableSet::stock[(size_t)TTunable::gc].def_min;
		hi   [t] =  STunableSet::stock[(size_t)TTunable::gc].def_max;
		state[t] =  0;
	}
}


bool
__attribute__ ((pure))
STunableSetFull::is_valid()
const
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


}

// EOF
