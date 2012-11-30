// ;-*-C++-*-
/*
 *       File name:  expdesign/loadsave.cc
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

#include "common/config-validate.hh"
#include "primaries.hh"
#include "model/achermann.hh"


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

		for ( size_t t = 0; t < ach::TTunable::_basic_tunables; ++t ) {
			auto& A = conf.lookup(string("tunable.") + ach::tunable_name(t));
			tunables0[t] = A[0];
			tlo      [t] = A[1];
			thi      [t] = A[2];
			tstep    [t] = A[3];
		}
	} catch (...) {
		fprintf( stderr, "CExpDesign::load_settings(): Something is wrong with %s\n", EXPD_FILENAME);

		_status = _status | load_fail;

		ctl_params0.reset();
		tunables0.set_defaults();
		fft_params.reset();
		mc_params.reset();

		return -1;
	}

	try { ctl_params0.check(); }
	catch (...) {
		ctl_params0.reset();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid ctl params; assigned defaults\n");
	}

	if ( tunables0.check() ) {
		tunables0.set_defaults();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid tunables; assigned defaults\n");
	}

	try { fft_params.check(); }
	catch (invalid_argument ex) {
		fft_params.reset();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid fft params (%s); assigned defaults\n", ex.what());
	}

	try { mc_params.check(); }
	catch (invalid_argument ex) {
		mc_params.reset();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid mc params (%s); assigned defaults\n", ex.what());
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
	for ( size_t t = 0; t < ach::TTunable::_basic_tunables; ++t )
		confval::put( conf, string("tunable.") + ach::tunable_name(t),
			      forward_list<double> {tunables0[t], tlo[t], thi[t], tstep[t]});

	conf.writeFile( EXPD_FILENAME);

	return 0;
}



// eof
