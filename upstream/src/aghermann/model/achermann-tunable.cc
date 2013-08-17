/*
 *       File name:  aghermann/model/achermann-tunable.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  tunables
 *
 *         License:  GPL
 */



#include <cassert>
#include <gsl/gsl_rng.h>
#include "aghermann/globals.hh"
#include "achermann-tunable.hh"


using namespace std;

const agh::ach::STunableDescription
	agh::ach::stock[TTunable::_basic_tunables] = {
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








string
agh::ach::
tunable_name( size_t t)
{
	if ( t < TTunable::_basic_tunables )
		return stock[t].name;
	else if ( t < TTunable::_all_tunables )
		return string("gc")
			+ to_string((long long unsigned)t - TTunable::gc + 1);
	else
		return "BAD_TUNABLE";
}


string
agh::ach::
tunable_pango_name( size_t t)
{
	if ( t < TTunable::_basic_tunables )
		return stock[t].pango_name;
	else if ( t < TTunable::_all_tunables )
		return string("<i>gc</i><sub>")
			+ to_string((long long unsigned)t - TTunable::gc + 1)
			+ "</sub>";
	else
		return "BAD_TUNABLE";
}


string
agh::ach::
tunable_unit( size_t t)
{
	if ( t < TTunable::_basic_tunables )
		return stock[t].unit;
	else if ( t < TTunable::_all_tunables )
		return stock[(size_t)TTunable::gc].unit;
	else
		return "BAD_TUNABLE";
}



// int
// __attribute__ ((pure))
// agh::ach::STunableSetWithState::
// validate_range() const
// {
// 	for ( size_t t = 0; t < size(); ++t )
//  		if ( lo[t] >= hi[t] || step[t] >= (hi[t] - lo[t])/2 )
// 			return -1;
// 	return 0;
// }



namespace agh {
namespace ach {
using namespace agh::ach;

template <>
void
STunableSet<TTRole::v>::
set_defaults()
{
	size_t t = 0;
	for ( ; t < ach::TTunable::_basic_tunables; ++t )
		P[t] = stock[t].def_val;
	for ( ; t < size(); ++t )
		P[t] = stock[ach::TTunable::gc].def_val;
}

template <>
void
STunableSet<TTRole::l>::
set_defaults()
{
	size_t t = 0;
	for ( ; t < ach::TTunable::_basic_tunables; ++t )
		P[t] = stock[t].def_min;
	for ( ; t < size(); ++t )
		P[t] = stock[ach::TTunable::gc].def_min;
}

template <>
void
STunableSet<TTRole::u>::
set_defaults()
{
	size_t t = 0;
	for ( ; t < ach::TTunable::_basic_tunables; ++t )
		P[t] = stock[t].def_max;
	for ( ; t < size(); ++t )
		P[t] = stock[ach::TTunable::gc].def_max;
}

template <>
void
STunableSet<TTRole::d>::
set_defaults()
{
	size_t t = 0;
	for ( ; t < ach::TTunable::_basic_tunables; ++t )
		P[t] = stock[t].def_step;
	for ( ; t < size(); ++t )
		P[t] = stock[ach::TTunable::gc].def_step;
}


template <>
int
__attribute__ ((pure))
STunableSet<TTRole::v>::
check() const
{
	size_t t = 0;
	for ( ; t < ach::TTunable::_basic_tunables; ++t )
		if ( P[t] <= stock[t].def_min || P[t] >= stock[t].def_max )
			return 1;
	for ( ; t < size(); ++t )
		if ( P[t] <= stock[ach::TTunable::gc].def_min || P[t] >= stock[ach::TTunable::gc].def_max )
			return 1;
	return 0;
}

} // namespace ach
} // namespace agh


void
agh::ach::STunableSetWithState::
randomise()
{
	for ( size_t t = 0; t < size(); ++t )
		if ( d[t] > 0. )
			P[t] = l[t] + gsl_rng_uniform( agh::global::rng) * range( (TTunable)t);
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
