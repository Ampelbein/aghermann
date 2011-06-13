// ;-*-C++-*- *  Time-stamp: "2011-06-13 17:08:03 hmmr"
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

#include <forward_list>
#include <initializer_list>

#include "misc.hh"
#include "ui.hh"
#include "measurements.hh"
#include "settings.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;

namespace aghui { namespace settings {


#define CONF_FILE ".aghermann.conf"

int
load()
{
	using boost::property_tree::ptree;
	ptree pt;

	try {
		read_xml( CONF_FILE, pt);

		string	strval;
		double	dblval;
		unsigned
			uintval;

		strval = pt.get<string>( "WindowGeometry.Main");
		{
			guint x, y, w, h;
			if ( sscanf( strval.c_str(), "%ux%u+%u+%u", &w, &h, &x, &y) == 4 ) {
				GeometryMain.x = x;
				GeometryMain.y = y;
				GeometryMain.w = w;
				GeometryMain.h = h;
			}
		}

		dblval = pt.get<double>( "Common.OperatingRangeFrom");
		if ( dblval > 0 )
			OperatingRangeFrom = dblval;

		dblval = pt.get<double>( "Common.OperatingRangeUpto");
		if ( dblval > OperatingRangeFrom )
			OperatingRangeUpto = dblval;
		if ( OperatingRangeUpto <= OperatingRangeFrom || OperatingRangeFrom <= 0. )
			OperatingRangeFrom = 2., OperatingRangeUpto = 3.;

		// this may be too early..
		// no, this function gets called from aghui::populate, called from main where it follows creation of a new AghCC
		_AghDi = find( AghDD.begin(), AghDD.end(), pt.get<string>( "Common.CurrentSession"));
		if ( _AghDi == AghDD.end() )
			_AghDi = AghDD.begin();
		_AghTi = find( AghTT.begin(), AghTT.end(), pt.get<string>( "Common.CurrentChannel"));
		if ( _AghTi == AghTT.end() )
			_AghTi = AghTT.begin();

		dblval = pt.get<float>( "MeasurementsOverview.PixelsPeruV2");
		if ( isfinite(dblval) && dblval > 0. )
			msmtview::PPuV2 = dblval;

		SimRunbatchIncludeAllChannels = pt.get<bool>( "BatchRun.IncludeAllChannels");
		SimRunbatchIncludeAllSessions = pt.get<bool>( "BatchRun.IncludeAllSessions");
		SimRunbatchIterateRanges      = pt.get<bool>( "BatchRun.IterateRanges");

		for ( TScore i = TScore::none; i != TScore::_total; next(i) ) {
			strval = pt.get<string>( string("ScoreCodes.")+agh::SPage::score_name(i));
			if ( !strval.empty() )
				ExtScoreCodes[(TScore_underlying_type)i].assign( strval);
		}

		uintval = pt.get<unsigned>( "WidgetSizes.PageHeight");
		if ( uintval >= 10 && uintval <= 500 )
			WidgetSize_SFPageHeight = uintval;
		uintval = pt.get<unsigned>( "WidgetSizes.HypnogramHeight");
		if ( uintval >= 10 && uintval <= 500 )
			WidgetSize_SFHypnogramHeight = uintval;
		uintval = pt.get<unsigned>( "WidgetSizes.SpectrumWidth");
		if ( uintval >= 10 && uintval <= 500 )
			WidgetSize_SFSpectrumWidth = uintval;
		uintval = pt.get<unsigned>( "WidgetSizes.EMGProfileHeight");
		if ( uintval >= 10 && uintval <= 500 )
			WidgetSize_SFEMGProfileHeight = uintval;

		auto colours =
			forward_list<pair<const char*, GtkColorButton*&>>
			({
				{"NONE",	CwB[TColour::score_none ].btn},
				{"NREM1",	CwB[TColour::score_nrem1].btn},
				{"NREM2",	CwB[TColour::score_nrem2].btn},
				{"NREM3",	CwB[TColour::score_nrem3].btn},
				{"NREM4",	CwB[TColour::score_nrem4].btn},
				{"REM",		CwB[TColour::score_rem  ].btn},
				{"Wake",	CwB[TColour::score_wake ].btn},
				{"MVT",		CwB[TColour::score_mvt  ].btn},
				{"PowerSF",	CwB[TColour::power_sf   ].btn},
				{"EMG",   	CwB[TColour::emg        ].btn},
				{"Hypnogram",	CwB[TColour::hypnogram  ].btn},
				{"Artifacts",	CwB[TColour::artifact   ].btn},
				{"TicksSF",	CwB[TColour::ticks_sf   ].btn},
				{"LabelsSF",	CwB[TColour::labels_sf  ].btn},
				{"BandDelta",	CwB[TColour::band_delta ].btn},
				{"BandTheta",	CwB[TColour::band_theta ].btn},
				{"BandAlpha",	CwB[TColour::band_alpha ].btn},
				{"BandBeta",	CwB[TColour::band_beta  ].btn},
				{"BandGamma",	CwB[TColour::band_gamma ].btn},
				{"Cursor",	CwB[TColour::cursor     ].btn},

				{"TicksMT",	CwB[TColour::ticks_mt   ].btn},
				{"LabelsMT",	CwB[TColour::labels_mt  ].btn},
				{"PowerMT",   	CwB[TColour::power_mt   ].btn},

				{"SWA",		CwB[TColour::swa        ].btn},
				{"SWASim",	CwB[TColour::swa_sim    ].btn},
				{"ProcessS",	CwB[TColour::process_s  ].btn},
				{"PaperMR",	CwB[TColour::paper_mr   ].btn},
				{"TicksMR",	CwB[TColour::ticks_mr   ].btn},
				{"LabelsMR",	CwB[TColour::labels_mr  ].btn}
			});
		for_each( colours.begin(), colours.end(),
			  [&] ( const pair<const char*, GtkColorButton*>& p)
			  {
				  GdkColor clr;
				  guint16  alpha;
				  strval = pt.get<string>( (string("Colours.")+p.first).c_str());
				  if ( !strval.empty() &&
				       sscanf( strval.c_str(), "%x,%x,%x,%x",
					       (unsigned*)&clr.red, (unsigned*)&clr.green, (unsigned*)&clr.blue,
					       (unsigned*)&alpha) == 4 ) {
					  gtk_color_button_set_color( p.second, &clr);
					  gtk_color_button_set_alpha( p.second, alpha);
				  }
				  g_signal_emit_by_name( p.second, "color-set");
			  });

		for ( TBand i = TBand::delta; i != TBand::_total; next(i) ) {
			float	f0 = pt.get<double>( (string("Bands.")+FreqBandNames[(TBand_underlying_type)i]+".[").c_str()),
				f1 = pt.get<double>( (string("Bands.")+FreqBandNames[(TBand_underlying_type)i]+".]").c_str());
			if ( f0 < f1 ) {
				gtk_spin_button_set_value( eBand[(TBand_underlying_type)i][0], f0);
				gtk_spin_button_set_value( eBand[(TBand_underlying_type)i][1], f1);
			}
			g_signal_emit_by_name( eBand[(TBand_underlying_type)i][0], "value-changed");
			g_signal_emit_by_name( eBand[(TBand_underlying_type)i][1], "value-changed");
		}

	} catch (...) {
		return 1;
	}

	return 0;
}






int
save()
{
	using boost::property_tree::ptree;
	ptree pt;

	{
		char b[40];
		snprintf( b, 39, "%ux%u+%u+%u", GeometryMain.w, GeometryMain.h, GeometryMain.x, GeometryMain.y);
		pt.put( "WindowGeometry.Main", b);
	}

	pt.put( "Common.CurrentSession",	AghD());
	pt.put( "Common.CurrentChannel",	AghT());
	pt.put( "Common.OperatingRangeFrom",	OperatingRangeFrom);
	pt.put( "Common.OperatingRangeUpto",	OperatingRangeUpto);

	for ( TScore i = TScore::none; i != TScore::_total; next(i) )
		pt.put( (string("ScoreCodes.") + agh::SPage::score_name(i)), ExtScoreCodes[(TScore_underlying_type)i]);

	pt.put( "MeasurementsOverview.PixelsPeruV2", msmtview::PPuV2);

	pt.put( "BatchRun.IncludeAllChannels",	SimRunbatchIncludeAllChannels);
	pt.put( "BatchRun.IncludeAllSessions",	SimRunbatchIncludeAllSessions);
	pt.put( "BatchRun.IterateRanges",	SimRunbatchIterateRanges);

	auto colours =
		forward_list<pair<const char*, SManagedColor&>>
		({
			{"NONE",	CwB[TColour::score_none ]},
			{"NREM1",	CwB[TColour::score_nrem1]},
			{"NREM2",	CwB[TColour::score_nrem2]},
			{"NREM3",	CwB[TColour::score_nrem3]},
			{"NREM4",	CwB[TColour::score_nrem4]},
			{"REM",		CwB[TColour::score_rem  ]},
			{"Wake",	CwB[TColour::score_wake ]},
			{"MVT",		CwB[TColour::score_mvt  ]},
			{"PowerSF",	CwB[TColour::power_sf   ]},
			{"EMG",   	CwB[TColour::emg        ]},
			{"Hypnogram",	CwB[TColour::hypnogram  ]},
			{"Artifacts",	CwB[TColour::artifact   ]},
			{"TicksSF",	CwB[TColour::ticks_sf   ]},
			{"LabelsSF",	CwB[TColour::labels_sf  ]},
			{"BandDelta",	CwB[TColour::band_delta ]},
			{"BandTheta",	CwB[TColour::band_theta ]},
			{"BandAlpha",	CwB[TColour::band_alpha ]},
			{"BandBeta",	CwB[TColour::band_beta  ]},
			{"BandGamma",	CwB[TColour::band_gamma ]},
			{"Cursor",	CwB[TColour::cursor     ]},

			{"TicksMT",	CwB[TColour::ticks_mt   ]},
			{"LabelsMT",	CwB[TColour::labels_mt  ]},
			{"PowerMT",   	CwB[TColour::power_mt   ]},

			{"SWA",		CwB[TColour::swa        ]},
			{"SWASim",	CwB[TColour::swa_sim    ]},
			{"ProcessS",	CwB[TColour::process_s  ]},
			{"PaperMR",	CwB[TColour::paper_mr   ]},
			{"TicksMR",	CwB[TColour::ticks_mr   ]},
			{"LabelsMR",	CwB[TColour::labels_mr  ]}
		});
	for_each( colours.begin(), colours.end(),
		  [&] ( const pair<const char*, SManagedColor&>& p)
		  {
			  snprintf_buf( "%#x,%#x,%#x,%#x",
					p.second.clr.red, p.second.clr.green, p.second.clr.blue,
					p.second.alpha);
			  pt.put( (string("Colours.")+p.first).c_str(), __buf__);
		  });

	for ( TBand i = TBand::delta; i != TBand::_total; next(i) ) {
		snprintf_buf( "%g,%g", FreqBands[(TBand_underlying_type)i][0], FreqBands[(TBand_underlying_type)i][1]);
		pt.put( (string("Bands.") + FreqBandNames[(TBand_underlying_type)i]), __buf__);
	}

	pt.put( "WidgetSizes.PageHeight",		WidgetSize_SFPageHeight);
	pt.put( "WidgetSizes.HypnogramHeight",		WidgetSize_SFHypnogramHeight);
	pt.put( "WidgetSizes.SpectrumWidth",		WidgetSize_SFSpectrumWidth);
	pt.put( "WidgetSizes.EMGProfileHeight",		WidgetSize_SFEMGProfileHeight);

	write_xml( CONF_FILE, pt);

	return 0;
}

}}



// eof
