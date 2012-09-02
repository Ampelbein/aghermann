// ;-*-C++-*-
/*
 *       File name:  model/achermann-tunable.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  tunables
 *
 *         License:  GPL
 */



#include <cassert>
#include <stdexcept>
#include "achermann-tunable.hh"


using namespace std;

const agh::ach::STunableSet::STunableDescription
	agh::ach::STunableSet::stock[(size_t)TTunable::_basic_tunables] = {
	{
		.918e-3,	.100e-3,	2.000e-3,	.001e-3,
		1e3, 0.001,
		true,	-1,
		"rs",		"%5.3f",	"1/min",
		"<i>rs</i>",
		"S Rise rate",
		"S Rise rate",
	},
	{
		.283,		.01,		4.,		.002,
		1, .001,
		true,	-1,
		"rc",	 	"%5.3f",	"1/min",
		"<i>rc</i>",
		"SWA rise rate",
		"SWA rise rate",
	},
	{
		.24,		.04,		4.,		.001,
		1, .01,
		true,	-1,
		"fc_R",		"%4.2f",	"1/min",
		"<i>fc</i><sub>R</sub>",
		"SWA decay rate in REM",
		"SWA decay rate in REM",
	},
	{
		.6,		.04,		4.,		.001,
		1, .01,
		true,	-1,
		"fc_W",		"%4.2f",	"1/min",
		"<i>fc</i><sub>W</sub>",
		"SWA decay rate in Wake",
		"SWA decay rate in Wake",
	},
	{
		300,		9,		1005,		1,
		1, 1,
		true,	0,
		"S_0",		"%5.1f",	"%",
		"<i>S</i><sub>0</sub>",
		"Starting value of S",
		"Starting value of S",
	},
	{
		500,		18,		2010,		1,
		1, 1,
		true,	0,
		"S_U",	 	"%5.1f",	"%",
		"<i>S</i><sub>U</sub>",
		"S upper asymptote",
		"S upper asymptote",
	},
	{
		3,		.4,		22,		1,
		1, .1,
		false,	1,
		"t_a",		"%3.1f",	"min",
		"<i>rem</i><sub>pre</sub>",
		"REM anticipation time",
		"REM anticipation time",
	},
	{
		3,		.4,		22,		1,
		1, .1,
		false,	1,
		"t_p",		"%3.1f",	"min",
		"<i>rem</i><sub>post</sub>",
		"REM extension time",
		"REM extension time",
	},
	{
		.835e-2,	.010e-2,	4.000e-2,	.004e-2,
		1e2, .001,
		true,	-1,
		"gc",		"%5.3f",	"1/min",
		"<i>gc</i>",
		"Gain constant",
		"SWA self-dissipation efficiency"
	},
};





agh::ach::STunableSet::
STunableSet( const STunableSet &o, size_t n_egc)
      : P ((size_t)TTunable::_basic_tunables + n_egc - 1)
{
	// we are certain about this far
	assert (n_egc > 0);
	// theoreticlly, o.P has all egc's
	if ( o.P.size() == P.size() ) {
//				printf( "o.P.size() eq = %zu\n", o.P.size());
		P = o.P;
	} else {
		// except when initilising from a basic set
//				printf( "o.P.size() = %zu\n", o.P.size());
		P[ slice(0, (size_t)TTunable::_basic_tunables, 1) ] =
			o.P[ slice(0, (size_t)TTunable::_basic_tunables, 1)];
		if ( n_egc > 1 )
			P[ slice((size_t)TTunable::gc+1, n_egc-1, 1) ] = o.P[(size_t)TTunable::gc];
	}
}




string
agh::ach::STunableSet::
tunable_name( size_t t)
{
	if ( t < ach::TTunable::_basic_tunables )
		return stock[t].name;
	else if ( t < ach::TTunable::_all_tunables )
		return string("gc")
			+ to_string((long long unsigned)t - ach::TTunable::gc + 1);
	else
		return "BAD_TUNABLE";
}


string
agh::ach::STunableSet::
tunable_pango_name( size_t t)
{
	if ( t < TTunable::_basic_tunables )
		return stock[t].pango_name;
	else if ( t < ach::TTunable::_all_tunables )
		return string("<i>gc</i><sub>")
			+ to_string((long long unsigned)t - ach::TTunable::gc + 1)
			+ "</sub>";
	else
		return "BAD_TUNABLE";
}





void
__attribute__ ((pure))
agh::ach::STunableSet::
check() const
{
	size_t t = 0;
	for ( ; t < ach::TTunable::_basic_tunables; ++t )
		if ( P[t] <= stock[t].def_min || P[t] >= stock[t].def_max )
			throw invalid_argument ("Bad Tunable");
	for ( ; t < P.size(); ++t )
		if ( P[t] <= stock[ach::TTunable::gc].def_min || P[t] >= stock[ach::TTunable::gc].def_max )
			throw invalid_argument ("Bad Tunable");
}



void
agh::ach::STunableSet::
reset()
{
	size_t t = 0;
	for ( ; t < ach::TTunable::_basic_tunables; ++t )
		P[t] = stock[t].def_val;
	for ( ; t < size(); ++t )
		P[t] = stock[ach::TTunable::gc].def_val;
}





void
agh::ach::STunableSet::
adjust_for_ppm( double ppm)
{
	for ( size_t t = 0; t < size(); ++t )
		P[t] *= pow( ppm, stock[min(t, (size_t)TTunable::gc)].time_adj);
}

void
agh::ach::STunableSet::
unadjust_for_ppm( double ppm)
{
	for ( size_t t = 0; t < size(); t++ )
		P[t] /= pow( ppm, stock[min(t, (size_t)TTunable::gc)].time_adj);
}








void
agh::ach::STunableSetFull::
reset()
{
	size_t t;
	for ( t = 0; t < (int)ach::TTunable::_basic_tunables; ++t ) {
		value[t] =  ach::STunableSet::stock[t].def_val;
		step [t] =  ach::STunableSet::stock[t].def_step;
		lo   [t] =  ach::STunableSet::stock[t].def_min;
		hi   [t] =  ach::STunableSet::stock[t].def_max;
		state[t] =  0;
	}
	for ( ; t < size(); ++t ) {
		value[t] =  ach::STunableSet::stock[ach::TTunable::gc].def_val;
		step [t] =  ach::STunableSet::stock[ach::TTunable::gc].def_step;
		lo   [t] =  ach::STunableSet::stock[ach::TTunable::gc].def_min;
		hi   [t] =  ach::STunableSet::stock[ach::TTunable::gc].def_max;
		state[t] =  0;
	}
}


void
__attribute__ ((pure))
agh::ach::STunableSetFull::
check() const
{
	for ( size_t t = 0; t < value.size(); ++t )
		if ( lo[t] >= hi[t] || step[t] >= (hi[t] - lo[t])/2 )
			throw invalid_argument ("Bad Tunable");
}







void
agh::ach::STunableSetFull::
randomise()
{
	for ( size_t t = 0; t < value.P.size(); t++ )
		if ( step[t] > 0. )
			value[t] = lo[t] + (double)(rand() - RAND_MAX)/RAND_MAX * (hi[t] - lo[t]);
}


// eof
