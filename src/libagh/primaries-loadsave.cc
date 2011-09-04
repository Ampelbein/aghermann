// ;-*-C++-*-
/*
 *       File name:  libagh/primaries-loadsave.cc
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
#include "edf.hh"
#include "tunable.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "boost-config-validate.hh"


using namespace std;
using namespace agh;

#define EXPD_FILENAME ".expdesign.conf"





int
agh::CExpDesign::load_settings()
{
	using boost::property_tree::ptree;
	ptree pt;

	// Load the XML file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	try {
		read_xml( EXPD_FILENAME, pt);

		get( config_keys_d, pt);
		get( config_keys_z, pt);
		get( config_keys_g, pt);
		get( config_keys_b, pt);

		for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t ) {
			tunables0.value[t]	= pt.get<double>( string("tunable.") + STunableSet::tunable_name(t) + ".value");
			tunables0.lo[t]		= pt.get<double>( string("tunable.") + STunableSet::tunable_name(t) + ".lo");
			tunables0.hi[t]		= pt.get<double>( string("tunable.") + STunableSet::tunable_name(t) + ".hi");
			tunables0.step[t]	= pt.get<double>( string("tunable.") + STunableSet::tunable_name(t) + ".step");
		}
	} catch (...) {
		_status = _status | load_fail;
		return -1;
	}

	if ( not ctl_params0.is_valid() )
		ctl_params0.assign_defaults();

	if ( not tunables0.is_valid() )
		tunables0.assign_defaults();

	if ( not fft_params.validate() )
		;

	return 0;
}





int
agh::CExpDesign::save_settings()
{
	using boost::property_tree::ptree;
	ptree pt;

	put( config_keys_d, pt);
	put( config_keys_z, pt);
	put( config_keys_g, pt);
	put( config_keys_b, pt);

      // only save _agh_basic_tunables_
	for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t ) {
		pt.put( string("tunable.") + STunableSet::tunable_name(t) + ".value", tunables0.value[t]);
		pt.put( string("tunable.") + STunableSet::tunable_name(t) + ".lo",    tunables0.lo[t]);
		pt.put( string("tunable.") + STunableSet::tunable_name(t) + ".hi",    tunables0.hi[t]);
		pt.put( string("tunable.") + STunableSet::tunable_name(t) + ".step",  tunables0.step[t]);
	}

	write_xml( EXPD_FILENAME, pt);

	return 0;
}





// EOF
