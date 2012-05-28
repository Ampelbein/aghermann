// ;-*-C++-*-
/*
 *       File name:  core/primaries-loadsave.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  CExpDesign::{load,save}.
 *
 *         License:  GPL
 */

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include <memory>
#include "primaries.hh"
#include "model.hh"
#include "tunable.hh"

#include "../common/config-validate.hh"


using namespace std;
using namespace agh;

#define EXPD_FILENAME ".expdesign.conf"





int
agh::CExpDesign::load_settings()
{
	libconfig::Config conf;

	// Load the XML file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	try {
		conf.readFile( EXPD_FILENAME);

		using namespace confval;
		get( config_keys_d, conf);
		get( config_keys_g, conf);
		get( config_keys_b, conf);

		for ( size_t t = 0; t < TTunable::_basic_tunables; ++t ) {
			auto& A = conf.lookup(string("tunable.") + STunableSet::tunable_name(t));
			tunables0.value[t] = A[0];
			tunables0.lo   [t] = A[1];
			tunables0.hi   [t] = A[2];
			tunables0.step [t] = A[3];
		}
	} catch (...) {
		fprintf( stderr, "CExpDesign::load_settings(): Something is wrong with %s\n", EXPD_FILENAME);

		_status = _status | load_fail;

		ctl_params0.reset();
		tunables0.reset();
		fft_params.reset();
		mc_params.reset( fft_params.pagesize);

		return -1;
	}

	try { ctl_params0.check(); }
	catch (...) {
		ctl_params0.reset();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid ctl params; assigned defaults\n");
	}

	try { tunables0.check(); }
	catch (...) {
		tunables0.reset();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid tunables; assigned defaults\n");
	}

	try { fft_params.check(); }
	catch (...) {
		fft_params.reset();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid fft params; assigned defaults\n");
	}

	try { mc_params.check( fft_params.pagesize); }
	catch (...) {
		mc_params.reset( fft_params.pagesize);
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid mc params; assigned defaults\n");
	}

	return 0;
}





int
agh::CExpDesign::save_settings()
{
	libconfig::Config conf;

	confval::put( config_keys_d, conf);
	confval::put( config_keys_g, conf);
	confval::put( config_keys_b, conf);

      // only save _agh_basic_tunables_
	for ( size_t t = 0; t < TTunable::_basic_tunables; ++t )
		confval::put( conf, string("tunable.") + STunableSet::tunable_name(t),
			      forward_list<double> {tunables0.value[t], tunables0.lo[t], tunables0.hi[t], tunables0.step[t]});

	conf.writeFile( EXPD_FILENAME);

	return 0;
}



// eof
