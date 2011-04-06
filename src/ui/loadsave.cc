// ;-*-C++-*- *  Time-stamp: "2011-04-05 02:19:34 hmmr"
/*
 *       File name:  ui/loadsave.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  load/save ui-related vars
 *
 *         License:  GPL
 */

#include <cstring>

#include "misc.hh"
#include "settings.hh"
#include "ui.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace aghui {

using namespace std;
using namespace agh;


#define CONF_FILE ".aghermann.conf"

int
settings_load()
{
	using boost::property_tree::ptree;
	ptree pt;

	try {
		read_xml( CONF_FILE, pt);

		string	strval;
		double	dblval;
		int	intval;
		unsigned
			uintval;

		strval = pt.get<string>( "WindowGeometry.Main");
		{
			guint x, y, w, h;
			if ( sscanf( strval.c_str(), "%ux%u+%u+%u", &w, &h, &x, &y) == 4 ) {
				AghGeometryMain.x = x;
				AghGeometryMain.y = y;
				AghGeometryMain.w = w;
				AghGeometryMain.h = h;
			}
		}

		dblval = pt.get<double>( "Common.OperatingRangeFrom");
		if ( dblval > 0 )
			AghOperatingRangeFrom = dblval;

		dblval = pt.get<double>( "Common.OperatingRangeUpto");
		if ( dblval > AghOperatingRangeFrom )
			AghOperatingRangeUpto = dblval;

		_AghDi = find( AghDD.begin(), AghDD.end(), pt.get<string>( "Common.CurrentSession"));
		_AghTi = find( AghTT.begin(), AghTT.end(), pt.get<string>( "Common.CurrentChannel"));

		uintval = pt.get<unsigned>( "SignalAnalysis.EnvTightness");
		if ( uintval > 1 && uintval <= 50 )
			AghEnvTightness = uintval;
		uintval = pt.get<unsigned>( "SignalAnalysis.BWFOrder");
		if ( uintval > 0 )
			AghBWFOrder = uintval;
		dblval = pt.get<double>( "SignalAnalysis.BWFCutoff");
		if ( dblval > 0. )
			AghBWFCutoff = dblval;
		dblval = pt.get<double>( "SignalAnalysis.DZCDFStep");
		if ( dblval > 0. )
			AghDZCDFStep = dblval;
		dblval = pt.get<double>( "SignalAnalysis.DZCDFSigma");
		if ( dblval > 0. )
			AghDZCDFSigma = dblval;
		uintval = pt.get<unsigned>( "SignalAnalysis.DZCDFSmooth");
		if ( uintval >= 0 )
			AghDZCDFSmooth = uintval;
		AghUseSigAnOnNonEEGChannels = pt.get<bool>( "SignalAnalysis.UseSigAnOnNonEEGChannels");

		dblval = pt.get<unsigned>( "MeasurementsOverview.PixelsPeruV2");
		if ( dblval != 0 )
			AghPPuV2 = dblval;

		AghSimRunbatchIncludeAllChannels = pt.get<bool>( "BatchRun.IncludeAllChannels");
		AghSimRunbatchIncludeAllSessions = pt.get<bool>( "BatchRun.IncludeAllSessions");
		AghSimRunbatchIterateRanges      = pt.get<bool>( "BatchRun.IterateRanges");

		for ( TScore_underlying_type i = 0; i < TScore::_total; ++i ) {
			strval = pt.get<string>( (string("ScoreCodes.")+AghScoreNames[i]).c_str());
			if ( !strval.empty() )
				AghExtScoreCodes[i] = strval;
		}

		uintval = pt.get<unsigned>( "WidgetSizes.PageHeight");
		if ( uintval >= 10 && uintval <= 500 )
			AghSFDAPageHeight = uintval;
		uintval = pt.get<unsigned>( "WidgetSizes.PowerProfileHeight");
		if ( uintval >= 10 && uintval <= 500 )
			AghSFDAPowerProfileHeight = uintval;
		uintval = pt.get<unsigned>( "WidgetSizes.SpectrumWidth");
		if ( uintval >= 10 && uintval <= 500 )
			AghSFDASpectrumWidth = uintval;
		uintval = pt.get<unsigned>( "WidgetSizes.EMGProfileHeight");
		if ( uintval >= 10 && uintval <= 500 )
			AghSFDAEMGProfileHeight = uintval;

		auto colours =
			forward_list<pair<const char*, GtkWidget*>>
			({
				{"NONE",	bColourNONE},
				{"NREM1",	bColourNREM1},
				{"NREM2",	bColourNREM2},
				{"NREM3",	bColourNREM3},
				{"NREM4",	bColourNREM4},
				{"REM",		bColourREM},
				{"Wake",	bColourWake},
				{"PowerSF",	bColourPowerSF},
				{"EMG",   	bColourEMG},
				{"Hypnogram",	bColourHypnogram},
				{"Artifacts",	bColourArtifacts},
				{"TicksSF",	bColourTicksSF},
				{"LabelsSF",	bColourLabelsSF},
				{"BandDelta",	bColourBandDelta},
				{"BandTheta",	bColourBandTheta},
				{"BandAlpha",	bColourBandAlpha},
				{"BandBeta",	bColourBandBeta},
				{"BandGamma",	bColourBandGamma},
				{"Cursor",	bColourCursor},

				{"TicksMT",	bColourTicksMT},
				{"LabelsMT",	bColourLabelsMT},
				{"PowerMT",   	bColourPowerMT},

				{"SWA",		bColourSWA},
				{"SWASim",	bColourSWASim},
				{"ProcessS",	bColourProcessS},
				{"PaperMR",	bColourPaperMR},
				{"TicksMR",	bColourTicksMR},
				{"LabelsMR",	bColourLabelsMR}
			});
		for_each( colours.begin(), colours.end(),
			  [&] ( const pair<const char*, GtkWidget>>& p)
			  {
				  GdkColor clr;
				  guint16  alpha;
				  strval = pt.get<string>( (string("Colours.")+p.first).c_str());
				  if ( !strval.empty() &&
				       sscanf( strval.c_str(), "%x,%x,%x,%x",
					       (unsigned*)&clr.red, (unsigned*)&clr.green, (unsigned*)&clr.blue,
					       (unsigned*)&alpha) == 4 ) {
					  gtk_color_button_set_color( GTK_COLOR_BUTTON (p.second), &clr);
					  gtk_color_button_set_alpha( GTK_COLOR_BUTTON (p.second), alpha);
				  }
				  g_signal_emit_by_name( p.second, "color-set");
			  });

		for ( gushort i = 0; i < TBand::_total; ++i ) {
			float	f0 = pt.get<double>( (string("Bands.")+AghFreqBandsNames[i]+".[").c_str()),
				f1 = pt.get<double>( (string("Bands.")+AghFreqBandsNames[i]+".]").c_str());
			if ( f0 < f1 ) {
				gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[i][0]), f0);
				gtk_spin_button_set_value( GTK_SPIN_BUTTON (eBand[i][1]), f1);
			}
			g_signal_emit_by_name( GTK_SPIN_BUTTON (eBand[i][0]), "value-changed");
			g_signal_emit_by_name( GTK_SPIN_BUTTON (eBand[i][1]), "value-changed");
		}

	} catch (...) {
		return 1;
	}

	return 0;
}






int
settings_save()
{
	using boost::property_tree::ptree;
	ptree pt;

	{
		char b[40];
		snprintf( b, 39, "%ux%u+%u+%u", AghGeometryMain.w, AghGeometryMain.h, AghGeometryMain.x, AghGeometryMain.y);
		pt.put( "WindowGeometry.Main", b);
	}

	pt.put( "Common.CurrentSession",	AghDi->c_str());
	pt.put( "Common.CurrentChannel",	AghTi->c_str());
	pt.put( "Common.OperatingRangeFrom",	AghOperatingRangeFrom);
	pt.put( "Common.OperatingRangeUpto",	AghOperatingRangeUpto);

	pt.put( "SignalAnalysis.EnvTightness",	AghEnvTightness);
	pt.put( "SignalAnalysis.BWFOrder",	AghBWFOrder);
	pt.put( "SignalAnalysis.BWFCutoff",	AghBWFCutoff);
	pt.put( "SignalAnalysis.DZCDFStep",	AghDZCDFStep);
	pt.put( "SignalAnalysis.DZCDFSigma",	AghDZCDFSigma);
	pt.put( "SignalAnalysis.DZCDFSmooth",	AghDZCDFSmooth);
	pt.put( "SignalAnalysis.UseSigAnOnNonEEGChannels",
						AghUseSigAnOnNonEEGChannels);

	for ( TScore_underlying_type i = 0; i < TScore::_total; ++i )
		pt.put( (string("ScoreCodes.") + AghScoreNames[i]).c_str(), AghExtScoreCodes[i]);

	pt.put( "MeasurementsOverview.PixelsPeruV2", AghPPuV2);

	pt.put( "BatchRun.IncludeAllChannels",	AghSimRunbatchIncludeAllChannels);
	pt.put( "BatchRun.IncludeAllSessions",	AghSimRunbatchIncludeAllSessions);
	pt.put( "BatchRun.IterateRanges",	AghSimRunbatchIterateRanges);

	auto colours = forward_list<pair<const char*, GdkColor&>>
		({
			{"TicksMT",	__fg0__[cTICKS_MT]},
			{"LabelsMT",	__fg0__[cLABELS_MT]},
			{"PowerMT",   	__fg0__[cPOWER_MT]},

			{"NONE",	__bg1__[cSIGNAL_SCORE_NONE]},
			{"NREM1",	__bg1__[cSIGNAL_SCORE_NREM1]},
			{"NREM2",	__bg1__[cSIGNAL_SCORE_NREM2]},
			{"NREM3",	__bg1__[cSIGNAL_SCORE_NREM3]},
			{"NREM4",	__bg1__[cSIGNAL_SCORE_NREM4]},
			{"REM",		__bg1__[cSIGNAL_SCORE_REM]},
			{"Wake",	__bg1__[cSIGNAL_SCORE_WAKE]},
			{"PowerSF",   	__fg1__[cPOWER_SF]},
			{"EMG",   	__fg1__[cEMG]},
			{"Hypnogram",	__bg1__[cHYPNOGRAM]},
			{"Artifacts",	__fg1__[cARTIFACT]},
			{"TicksSF",	__fg1__[cTICKS_SF]},
			{"LabelsSF",	__fg1__[cLABELS_SF]},
			{"BandDelta",	__fg1__[cBAND_DELTA]},
			{"BandTheta",   __fg1__[cBAND_THETA]},
			{"BandAlpha",	__fg1__[cBAND_ALPHA]},
			{"BandBeta",	__fg1__[cBAND_BETA]},
			{"BandGamma",	__fg1__[cBAND_GAMMA]},

			{"SWA",		__fg2__[cSWA]},
			{"SWASim",	__fg2__[cSWA_SIM]},
			{"ProcessS",	__fg2__[cPROCESS_S]},
			{"PaperMR",	__bg2__[cPAPER_MR]},
			{"TicksMR",	__fg2__[cTICKS_MR]},
			{"LabelsMR",	__fg2__[cLABELS_MR]}
		});
	for_each( colours.begin(), colours.end(),
		  [&] ( const colours::value_type& p)
		  {
			  snprintf_buf( "%#x,%#x,%#x,%#x", p.second.red, p.second.green, p.second.blue, 0);
			  pt.put( (string("Colours.")+p.first).c_str(), __buf__);
		  });

	for ( TBand_underlying_type i = 0; i < TBand::_total; ++i ) {
		snprintf_buf( "%g,%g", AghFreqBands[i][0], AghFreqBands[i][1]);
		pt.put( (string("Bands.") + AghFreqBandsNames[i]).c_str(), __buf__);
	}

	pt.put( "WidgetSizes.PageHeight",		AghSFDAPageHeight);
	pt.put( "WidgetSizes.PowerProfileHeight",	AghSFDAPowerProfileHeight);
	pt.put( "WidgetSizes.SpectrumWidth",		AghSFDASpectrumWidth);
	pt.put( "WidgetSizes.EMGProfileHeight",		AghSFDAEMGProfileHeight);

	write_xml( CONF_FILE, pt);

	return 0;
}


}





// EOF
