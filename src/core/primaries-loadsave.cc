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
		auto& cfroot = conf.getRoot();

		using namespace confval;
		get( config_keys_d, cfroot);
		get( config_keys_g, cfroot);
		get( config_keys_b, cfroot);

		for ( size_t t = 0; t < TTunable::_basic_tunables; ++t ) {
			conf.lookupValue( string("tunable.") + STunableSet::tunable_name(t) + ".value", tunables0.value[t]);
			conf.lookupValue( string("tunable.") + STunableSet::tunable_name(t) + ".lo",    tunables0.lo   [t]);
			conf.lookupValue( string("tunable.") + STunableSet::tunable_name(t) + ".hi",    tunables0.hi   [t]);
			conf.lookupValue( string("tunable.") + STunableSet::tunable_name(t) + ".step",  tunables0.step [t]);
		}
	} catch (...) {
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
	auto& cfroot = conf.getRoot();

	FAFA;
	using namespace confval;
	put( config_keys_d, cfroot);
	put( config_keys_g, cfroot);
	put( config_keys_b, cfroot);

      // only save _agh_basic_tunables_
	FAFA;
	for ( size_t t = 0; t < TTunable::_basic_tunables; ++t ) {
		confval::put( cfroot, string("tunable.") + STunableSet::tunable_name(t) + ".value", tunables0.value[t]);
		confval::put( cfroot, string("tunable.") + STunableSet::tunable_name(t) + ".lo",    tunables0.lo   [t]);
		confval::put( cfroot, string("tunable.") + STunableSet::tunable_name(t) + ".hi",    tunables0.hi   [t]);
		confval::put( cfroot, string("tunable.") + STunableSet::tunable_name(t) + ".step",  tunables0.step [t]);
	}
	FAFA;

	conf.writeFile( EXPD_FILENAME);

	return 0;
}



// eof
