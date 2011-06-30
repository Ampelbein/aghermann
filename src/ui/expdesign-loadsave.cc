// ;-*-C++-*- *  Time-stamp: "2011-06-30 01:18:51 hmmr"
/*
 *       File name:  ui/expdesign-loadsave.cc
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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "expdesign.hh"
#include "../libagh/boost-ptree-validator.hh"

using namespace std;
using namespace aghui;

#define CONF_FILE ".aghermann.conf"



vector<::SValidator> aghui::SExpDesignUI::config_keys = {
	{"WindowGeometry.Main",		_geometry_placeholder},
	{"Common.CurrentSession",	_aghdd_placeholder},
	{"Common.CurrentChannel",	_aghtt_placeholder},
	{"Common.OperatingRangeFrom",	operating_range_from,	aghui::SExpDesignUI::SValidator::SVFRange (0., 20.)},
	{"Common.OperatingRangeUpto",	operating_range_upto,	aghui::SExpDesignUI::SValidator::SVFRange (0., 20.)},

	{"Measurements.TimelineHeight",	timeline_height,	aghui::SExpDesignUI::SValidator::SVFRange (10, 600)},
	{"Measurements.TimelinePPuV2",	ppuv2,			aghui::SExpDesignUI::SValidator::SVFRange (1e-10, 1e10)},
	{"Measurements.TimelinePPH",	timeline_height,	aghui::SExpDesignUI::SValidator::SVFRange (10, 600)},

	{"ScoringFacility.NeighPagePeek",	SScoringFacility::NeighPagePeek,	aghui::SExpDesignUI::SValidator::SVFRange (0., 40.)},
	{"ScoringFacility.IntersignalSpace",	SScoringFacility::IntersignalSpace,	aghui::SExpDesignUI::SValidator::SVFRange (10, 800)},
	{"ScoringFacility.SpectrumWidth",	SScoringFacility::SpectrumWidth,	aghui::SExpDesignUI::SValidator::SVFRange (10, 800)},
	{"ScoringFacility.HypnogramHeight",	SScoringFacility::HypnogramHeight,	aghui::SExpDesignUI::SValidator::SVFRange (10, 300)},

	{"BatchRun.IncludeAllChannels",	runbatch_include_all_channels},
	{"BatchRun.IncludeAllSessions",	runbatch_include_all_sessions},
	{"BatchRun.IterateRanges",	runbatch_iterate_ranges},
};






int
aghui::SExpDesignUI::load_settings()
{
	using boost::property_tree::ptree;
	ptree pt;

	try {
		read_xml( CONF_FILE, pt);

		for_each( config_keys.begin(), config_keys.end(),
			  //bind (function(&SValidator::get), pt, _1));
			  [&] ( ::SValidator& V)
			  {
				  try {
					  V.get( pt);
				  } catch (invalid_argument ex) {
					  fprintf( stderr, "SExpDesignUI::load_settings(): %s\n", ex.what());
				  }
			  });

	      // plus postprocess and extra checks
		{
			int x, y, w, h;
			if ( sscanf( _geometry_placeholder.c_str(), "%ux%u+%u+%u", &w, &h, &x, &y) == 4 ) {
				geometry.x = x;
				geometry.y = y;
				geometry.w = w;
				geometry.h = h;
			}
		}
		if ( operating_range_upto <= operating_range_from || operating_range_from <= 0. )
			operating_range_from = 2., operating_range_upto = 3.;

		// make sure ED has been created
		_AghDi = find( AghDD.begin(), AghDD.end(), _aghdd_placeholder);
		if ( _AghDi == AghDD.end() )
			_AghDi = AghDD.begin();
		_AghTi = find( AghTT.begin(), AghTT.end(), _aghtt_placeholder));
		if ( _AghTi == AghTT.end() )
			_AghTi = AghTT.begin();

		for ( auto i = agh::SPage::TScore::none; i != agh::SPage::TScore::_total; agh::SPage::next(i) ) {
			strval = pt.get<string>( string("ScoreCodes.")+agh::SPage::score_name(i));
			if ( !strval.empty() )
				ext_score_codes[(agh::SPage::TScore_underlying_type)i].assign( strval);
		}

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

		for ( agh::TBand i = agh::TBand::delta; i != agh::TBand::_total; next(i) ) {
			float	f0 = pt.get<double>( (string("Bands.")+FreqBandNames[(agh::TBand_underlying_type)i]+".[").c_str()),
				f1 = pt.get<double>( (string("Bands.")+FreqBandNames[(agh::TBand_underlying_type)i]+".]").c_str());
			if ( f0 < f1 ) {
				gtk_spin_button_set_value( eBand[(agh::TBand_underlying_type)i][0], f0);
				gtk_spin_button_set_value( eBand[(agh::TBand_underlying_type)i][1], f1);
			}
			g_signal_emit_by_name( eBand[(agh::TBand_underlying_type)i][0], "value-changed");
			g_signal_emit_by_name( eBand[(agh::TBand_underlying_type)i][1], "value-changed");
		}

	} catch (...) {
		return 1;
	}

	return 0;
}






int
aghui::settings::save()
{
	using boost::property_tree::ptree;
	using namespace agh;
	ptree pt;

	{
		char b[40];
		snprintf( b, 39, "%ux%u+%u+%u", GeometryMain.w, GeometryMain.h, GeometryMain.x, GeometryMain.y);
		pt.put( "WindowGeometry.Main", b);
	}

	pt.put( "Common.CurrentSession",	AghD());
	pt.put( "Common.CurrentChannel",	AghT());
	pt.put( "Common.OperatingRangeFrom",	msmt::OperatingRangeFrom);
	pt.put( "Common.OperatingRangeUpto",	msmt::OperatingRangeUpto);

	for ( SPage::TScore i = SPage::TScore::none; i != SPage::TScore::_total; agh::SPage::next(i) )
		pt.put( (string("ScoreCodes.") + SPage::score_name(i)), ExtScoreCodes[(SPage::TScore_underlying_type)i]);

	pt.put( "MeasurementsOverview.PixelsPeruV2", msmt::PPuV2);

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

	pt.put( "ScoringFcility.NeighPagePeek",		SFNeighPagePeek);

	pt.put( "WidgetSizes.PageHeight",		sf::SScoringFacility::WidgetSize_PageHeight);
	pt.put( "WidgetSizes.HypnogramHeight",		sf::SScoringFacility::WidgetSize_HypnogramHeight);
	pt.put( "WidgetSizes.SpectrumWidth",		sf::SScoringFacility::WidgetSize_SpectrumWidth);
	pt.put( "WidgetSizes.EMGProfileHeight",		sf::SScoringFacility::WidgetSize_EMGProfileHeight);

	write_xml( CONF_FILE, pt);

	return 0;
}





// eof
