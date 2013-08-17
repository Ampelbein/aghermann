/*
 *       File name:  aghermann/expdesign/loadsave.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  CExpDesign::{load,save}.
 *
 *         License:  GPL
 */

#include "common/config-validate.hh"
#include "aghermann/model/achermann.hh"
#include "primaries.hh"


using namespace std;
using namespace agh;

#define EXPD_FILENAME ".expdesign.conf"



int
CExpDesign::
load_settings()
{
	libconfig::Config conf;

	// Load the XML file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	try {
		conf.readFile( EXPD_FILENAME);

		using namespace confval;
		get( config_keys_d, conf);
		get( config_keys_z, conf);
		get( config_keys_g, conf);
		get( config_keys_b, conf);
		get( config_keys_s, conf);

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

	try {
		for ( size_t i = metrics::TBand::delta; i < metrics::TBand::TBand_total; ++i ) {
			auto& A = conf.lookup(string("Band.")+FreqBandNames[i]);
			float	f0 = A[0],
				f1 = A[1];
			if ( f0 < f1 ) {
				freq_bands[i][0] = f0;
				freq_bands[i][1] = f1;
			} else
				fprintf( stderr, "agh::SExpDesign::load_settings(): Invalid Band range\n");
		}
	} catch (...) {
		fprintf( stderr, "agh::SExpDesign::load_settings(): Something is wrong with section Band\n");
	}

	try { ctl_params0.check(); }
	catch (...) {
		ctl_params0.reset();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid ctl params\n");
	}

	if ( tunables0.check() ) {
		tunables0.set_defaults();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid tunables\n");
	}

	try { fft_params.check(); }
	catch (invalid_argument ex) {
		fft_params.reset();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid fft params (%s)\n", ex.what());
	}

	try { mc_params.check(); }
	catch (invalid_argument ex) {
		mc_params.reset();
		fprintf( stderr, "agh::CExpDesign::load_settings(): Invalid mc params (%s)\n", ex.what());
	}

	return 0;
}





int
CExpDesign::
save_settings()
{
	libconfig::Config conf;

	confval::put( config_keys_d, conf);
	confval::put( config_keys_g, conf);
	confval::put( config_keys_b, conf);
	confval::put( config_keys_s, conf);
	confval::put( config_keys_z, conf);

	// only save _agh_basic_tunables_
	for ( size_t t = 0; t < ach::TTunable::_basic_tunables; ++t )
		confval::put( conf, string("tunable.") + ach::tunable_name(t),
			      forward_list<double> {tunables0[t], tlo[t], thi[t], tstep[t]});

	for ( unsigned i = metrics::TBand::delta; i < metrics::TBand::TBand_total; ++i )
		confval::put( conf, string("Band.") + FreqBandNames[i],
			      forward_list<double> {freq_bands[i][0], freq_bands[i][1]});

	conf.writeFile( EXPD_FILENAME);

	return 0;
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
