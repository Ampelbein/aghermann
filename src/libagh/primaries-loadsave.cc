// ;-*-C++-*- *  Time-stamp: "2011-05-18 00:42:52 hmmr"
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

#include <glib.h>

#include <memory>
#include "primaries.hh"
#include "model.hh"
#include "edf.hh"
#include "tunable.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace agh {

using namespace std;


#define EXPD_FILENAME ".expdesign.xml"

int
CExpDesign::load()
{
	using boost::property_tree::ptree;
	ptree pt;

	// Load the XML file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	try {
		read_xml( EXPD_FILENAME, pt);

		ctl_params0.siman_params.n_tries		= pt.get<size_t>( "ctlp.NTries");
		ctl_params0.siman_params.iters_fixed_T		= pt.get<double>( "ctlp.ItersFixedT");
		ctl_params0.siman_params.step_size		= pt.get<double>( "ctlp.StepSize");
		ctl_params0.siman_params.k			= pt.get<double>( "ctlp.Boltzmannk");
		ctl_params0.siman_params.t_initial		= pt.get<double>( "ctlp.TInitial");
		ctl_params0.siman_params.mu_t			= pt.get<double>( "ctlp.DampingMu");
		ctl_params0.siman_params.t_min			= pt.get<double>( "ctlp.TMin");
		ctl_params0.DBAmendment1			= pt.get<bool>( "ctlp.DBAmendment1");
		ctl_params0.DBAmendment2			= pt.get<bool>( "ctlp.DBAmendment2");
		ctl_params0.AZAmendment				= pt.get<bool>( "ctlp.AZAmendment");
		ctl_params0.ScoreMVTAsWake			= pt.get<bool>( "ctlp.ScoreMVTAsWake");
		ctl_params0.ScoreUnscoredAsWake			= pt.get<bool>( "ctlp.ScoreUnscoredAsWake");
		ctl_params0.req_percent_scored			= pt.get<float>( "ctlp.ReqScoredPC");
		ctl_params0.swa_laden_pages_before_SWA_0	= pt.get<size_t>( "ctlp.NSWALadenPagesBeforeSWA0");

		for ( size_t t = 0; t < TTunable::_basic_tunables; ++t ) {
			tunables0.value[t]	= pt.get<double>( string("tunable.") + agh::STunableSet::tunable_name(t) + ".value");
			tunables0.lo[t]		= pt.get<double>( string("tunable.") + agh::STunableSet::tunable_name(t) + ".lo");
			tunables0.hi[t]		= pt.get<double>( string("tunable.") + agh::STunableSet::tunable_name(t) + ".hi");
			tunables0.step[t]	= pt.get<double>( string("tunable.") + agh::STunableSet::tunable_name(t) + ".step");
		}

		fft_params.welch_window_type	= (TFFTWinType)pt.get<int>( "fftp.WelchWindowType");
		fft_params.bin_size		= pt.get<double>( "fftp.BinSize");
		fft_params.page_size		= pt.get<size_t>( "fftp.PageSize");

		af_dampen_window_type		= (TFFTWinType)pt.get<int>( "artifacts.DampenWindowType");

	} catch (...) {
		_status = _status | TExpDesignState::load_fail;
		return -1;
	}

	return 0;
}





int
CExpDesign::save() const
{
	using boost::property_tree::ptree;
	ptree pt;

	pt.put( "ctlp.NTries",				ctl_params0.siman_params.n_tries);
	pt.put( "ctlp.ItersFixedT",			ctl_params0.siman_params.iters_fixed_T);
	pt.put( "ctlp.StepSize",			ctl_params0.siman_params.step_size);
	pt.put( "ctlp.Boltzmannk",			ctl_params0.siman_params.k);
	pt.put( "ctlp.TInitial",			ctl_params0.siman_params.t_initial);
	pt.put( "ctlp.DampingMu",			ctl_params0.siman_params.mu_t);
	pt.put( "ctlp.TMin",				ctl_params0.siman_params.t_min);
	pt.put( "ctlp.DBAmendment1",			ctl_params0.DBAmendment1);
	pt.put( "ctlp.DBAmendment2",			ctl_params0.DBAmendment2);
	pt.put( "ctlp.AZAmendment",			ctl_params0.AZAmendment);
	pt.put( "ctlp.ScoreMVTAsWake",			ctl_params0.ScoreMVTAsWake);
	pt.put( "ctlp.ScoreUnscoredAsWake",		ctl_params0.ScoreUnscoredAsWake);
	pt.put( "ctlp.ReqScoredPC",			ctl_params0.req_percent_scored);
	pt.put( "ctlp.NSWALadenPagesBeforeSWA0",	ctl_params0.swa_laden_pages_before_SWA_0);

      // only save _agh_basic_tunables_
	for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t ) {
		pt.put( string("tunable.") + agh::STunableSet::tunable_name(t) + ".value", tunables0.value[t]);
		pt.put( string("tunable.") + agh::STunableSet::tunable_name(t) + ".lo",    tunables0.lo[t]);
		pt.put( string("tunable.") + agh::STunableSet::tunable_name(t) + ".hi",    tunables0.hi[t]);
		pt.put( string("tunable.") + agh::STunableSet::tunable_name(t) + ".step",  tunables0.step[t]);
	}

	pt.put( "fftp.WelchWindowType",		(unsigned short)fft_params.welch_window_type);
	pt.put( "fftp.BinSize",			fft_params.bin_size);
	pt.put( "fftp.PageSize",		fft_params.page_size);

	pt.put( "artifacts.DampenWindowType",	(unsigned short)af_dampen_window_type);

	write_xml( EXPD_FILENAME, pt);

	return 0;
}


}


// EOF
