/*
 *       File name:  tools/agh-profile-gen.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-06-18
 *
 *         Purpose:  Standalone profile generator
 *
 *         License:  GPL
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <utime.h>
#include <stdlib.h>

#include <stdexcept>
#include <set>

#include "common/alg.hh"
#include "common/fs.hh"
#include "common/string.hh"
#include "libsigfile/typed-source.hh"
#include "libmetrics/all.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

#include <argp.h>


using namespace std;

#define ARGV0 "agh-profile-gen"

const char
	*argp_program_version = ARGV0 " " VERSION,
	*argp_program_bug_address = "<" PACKAGE_BUGREPORT ">";

static char doc[] =
       ARGV0 " -- sleep profile (PSD, MC, SWU) generator";

enum TOptChar : char {
	o_profile    = 't',
	o_page       = 'p',
	o_step       = 'i',
	o_psd_params = 'P',
	o_mc_params  = 'M',
	o_swu_params = 'S',
	o_channel    = 'h',
};

static struct argp_option options[] = {
       {"channel",		o_channel,    "CHANNEL",		0, "use this channel (0-based)"			},
       {"profile",		o_profile,    "pms",			0, "profile(s) to generate (p=PSD, m=MC, s=SWU)"},
       {"page",			o_page,       "PAGESIZE",		0, "page size (sec)"				},
       {"step",			o_step,       "STEP",			0, "step (sec)"					},
       {"psd-params",		o_psd_params, "BINSIZE",		0, "PSD: binsize (sec, one of .1, .25, .5)"	},
       {"mc-params",		o_mc_params,  "SCOPE:F0FC:BANDWIDTH:IIR_BACKPOLATE:GAIN:SMOOTH",
									0, "MC parameters"				},
       {"swu-params",		o_swu_params, "MIN_UPSWING_LEN",	0, "SWU parameters"				},
       { 0 }
     };

static char args_doc[] = "FILE";

static error_t parse_opt( int, char*, struct argp_state*);

static struct argp argp = {
	options,
	parse_opt,
	args_doc,
	doc
};


struct SArguments {
	string	file;
	int	h;

	double	pagesize, // will propagate to any *_pp
		step;

	set<metrics::TType>
		types;

	metrics::psd::SPPack psd_pp;
	metrics::mc ::SPPack mc_pp;
	metrics::swu::SPPack swu_pp;

	SArguments()
	      : h (-1),
		pagesize (NAN),
		step (NAN)
		{}
};


static error_t
parse_opt( int key, char *arg, struct argp_state *state)
{
	auto& Q = *(SArguments*)state->input;

	switch ( key ) {
	case TOptChar::o_profile:
		if ( strchr( arg, 'p') )
			Q.types.insert(metrics::TType::psd);
		if ( strchr( arg, 'm') )
			Q.types.insert(metrics::TType::mc);
		if ( strchr( arg, 's') )
			Q.types.insert(metrics::TType::swu);
		break;

	case TOptChar::o_page:
		Q.pagesize = atof( arg);
		break;

	case TOptChar::o_step:
		Q.step = atof( arg);
		break;

	case TOptChar::o_channel:
		Q.h = atoi( arg);
		break;

	case TOptChar::o_psd_params:
		sscanf( arg, "%lg",
			&Q.psd_pp.binsize);
		break;

	case TOptChar::o_mc_params:
		sscanf( arg, "%lg:%lg:%lg:%lg:%lg",
			&Q.mc_pp.scope, &Q.mc_pp.f0fc, &Q.mc_pp.bandwidth, &Q.mc_pp.iir_backpolate, &Q.mc_pp.mc_gain);
		break;

	case TOptChar::o_swu_params:
		sscanf( arg, "%lg",
			&Q.swu_pp.min_upswing_duration);
		break;

	case ARGP_KEY_ARG:
		if ( Q.file.empty() )
			Q.file = arg;
		else
			throw invalid_argument ("Can only process one file/channel at a time");
		break;

	case ARGP_KEY_END:
		if ( state->arg_num < 1 )
			argp_usage( state);
		break;
	default:
		return (error_t)ARGP_ERR_UNKNOWN;
	}
	return (error_t)0;
}




int
main( int argc, char **argv)
{
	SArguments A;
	try {
		argp_parse( &argp, argc, argv, 0, NULL, (void*)&A);

		if ( A.h == -1 )
			throw invalid_argument ("Invalid or missing channel");

		if ( A.types.empty() )
			throw invalid_argument ("Which profiles do you want?");

		if ( !isfinite(A.pagesize) )
			throw invalid_argument ("Missing or invalid pagesize");
		if ( !isfinite(A.step) )
			throw invalid_argument ("Missing or invalid step");

		bool	do_psd = A.types.find( metrics::TType::psd) != A.types.end(),
			do_mc  = A.types.find( metrics::TType:: mc) != A.types.end(),
			do_swu = A.types.find( metrics::TType::swu) != A.types.end();

		if ( do_psd )
			A.psd_pp.pagesize = A.pagesize, A.psd_pp.step = A.step, A.psd_pp.check();
		if ( do_mc )
			A.mc_pp.pagesize  = A.pagesize, A.mc_pp.step  = A.step, A.mc_pp.check();
		if ( do_swu )
			A.swu_pp.pagesize = A.pagesize, A.swu_pp.step = A.step, A.swu_pp.check();

		if ( A.file.empty() )
			throw invalid_argument ("Missing file name");

		sigfile::CTypedSource F (A.file, A.pagesize, 0|sigfile::CSource::no_ancillary_files);
		if ( do_psd ) {
			metrics::psd::CProfile P (F, A.h, A.psd_pp);
			if ( P.go_compute() )
				throw runtime_error ("Failed to compute PSD");
			P.export_tsv( A.file + ".psd");
		}
		if ( do_mc ) {
			metrics::mc::CProfile P (F, A.h, A.mc_pp);
			if ( P.go_compute() )
				throw runtime_error ("Failed to compute MC");
			P.export_tsv( A.file + ".mc");
		}
		if ( do_swu ) {
			metrics::swu::CProfile P (F, A.h, A.swu_pp);
			if ( P.go_compute() )
				throw runtime_error ("Failed to compute SWU");
			P.export_tsv( A.file + ".swu");
		}

		return 0;

	} catch ( exception& ex ) {
		fprintf( stderr, "Error: %s\n", ex.what());
		return 1;
	}
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
