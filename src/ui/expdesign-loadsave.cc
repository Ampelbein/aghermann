// ;-*-C++-*-
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

#include "misc.hh"
#include "expdesign.hh"
#include "../core/boost-config-validate.hh"

using namespace std;
using namespace aghui;

#define CONF_FILE ".aghermann.conf"





int
aghui::SExpDesignUI::load_settings()
{
	using namespace sigfile;
	using boost::property_tree::ptree;
	ptree pt;

	try {
		read_xml( CONF_FILE, pt);
		get( config_keys_s, pt);
		get( config_keys_z, pt);
		get( config_keys_b, pt);
		get( config_keys_g, pt);

		for ( size_t i = SPage::TScore::none; i != SPage::TScore::_total; ++i ) {
			string strval = pt.get<string>( string("ScoreCodes.")+SPage::score_name((SPage::TScore)i));
			if ( !strval.empty() )
				ext_score_codes[i].assign( strval);
		}

		auto colours =
			forward_list<pair<const char*, GtkColorButton*>>
			({
				{"NONE",	CwB[TColour::score_none    ].btn},
				{"NREM1",	CwB[TColour::score_nrem1   ].btn},
				{"NREM2",	CwB[TColour::score_nrem2   ].btn},
				{"NREM3",	CwB[TColour::score_nrem3   ].btn},
				{"NREM4",	CwB[TColour::score_nrem4   ].btn},
				{"REM",		CwB[TColour::score_rem     ].btn},
				{"Wake",	CwB[TColour::score_wake    ].btn},
				{"ProfilePsdSF",CwB[TColour::profile_psd_sf].btn},
				{"ProfileMcSF",	CwB[TColour::profile_mc_sf ].btn},
				{"EMG",   	CwB[TColour::emg           ].btn},
				{"Hypnogram",	CwB[TColour::hypnogram     ].btn},
				{"Artifacts",	CwB[TColour::artifact      ].btn},
				{"Annotations",	CwB[TColour::annotations   ].btn},
				{"Selection",	CwB[TColour::selection	   ].btn},
				{"TicksSF",	CwB[TColour::ticks_sf      ].btn},
				{"LabelsSF",	CwB[TColour::labels_sf     ].btn},
				{"BandDelta",	CwB[TColour::band_delta    ].btn},
				{"BandTheta",	CwB[TColour::band_theta    ].btn},
				{"BandAlpha",	CwB[TColour::band_alpha    ].btn},
				{"BandBeta",	CwB[TColour::band_beta     ].btn},
				{"BandGamma",	CwB[TColour::band_gamma    ].btn},
				{"Cursor",	CwB[TColour::cursor        ].btn},

				{"Night",	CwB[TColour::night	   ].btn},
				{"Day",		CwB[TColour::day	   ].btn},

				{"TicksMT",	CwB[TColour::ticks_mt      ].btn},
				{"LabelsMT",	CwB[TColour::labels_mt     ].btn},
				{"PowerMT",   	CwB[TColour::power_mt      ].btn},

				{"SWA",		CwB[TColour::swa           ].btn},
				{"SWASim",	CwB[TColour::swa_sim       ].btn},
				{"ProcessS",	CwB[TColour::process_s     ].btn},
				{"PaperMR",	CwB[TColour::paper_mr      ].btn},
				{"TicksMR",	CwB[TColour::ticks_mr      ].btn},
				{"LabelsMR",	CwB[TColour::labels_mr     ].btn}
			});
		for( auto &p : colours ) {
			GdkColor clr;
			unsigned alpha;
			string strval = pt.get<string>( (string("Colours.")+p.first).c_str());
			if ( !strval.empty() &&
			     sscanf( strval.c_str(), "%x,%x,%x,%x",
				     (unsigned*)&clr.red, (unsigned*)&clr.green, (unsigned*)&clr.blue,
				     (unsigned*)&alpha) == 4 ) {
				gtk_color_button_set_color( p.second, &clr);
				gtk_color_button_set_alpha( p.second, alpha);
			}
		}

		for ( size_t i = TBand::delta; i < TBand::_total; ++i ) {
			auto	f0 = pt.get<double>( (string("Bands.")+FreqBandNames[i]+".[").c_str()),
				f1 = pt.get<double>( (string("Bands.")+FreqBandNames[i]+".]").c_str());
			if ( f0 < f1 ) {
				gtk_spin_button_set_value( eBand[i][0], f0);
				gtk_spin_button_set_value( eBand[i][1], f1);
			}
			g_signal_emit_by_name( eBand[i][0], "value-changed");
			g_signal_emit_by_name( eBand[i][1], "value-changed");
		}
	} catch (...) {
		;
	}

      // plus postprocess and extra checks
	{
		int x, y, w, h;
		if ( not _geometry_placeholder.empty()
		     and sscanf( _geometry_placeholder.c_str(), "%ux%u+%u+%u", &w, &h, &x, &y) == 4 ) {
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
	_AghTi = find( AghTT.begin(), AghTT.end(), _aghtt_placeholder);
	if ( _AghTi == AghTT.end() )
		_AghTi = AghTT.begin();

      // save scan_tree triggers
	// pagesize and binsize not loaded, so their _saved counterparts saved in ctor

      // misc
	__adjust_op_freq_spinbuttons();

	return 0;
}






int
aghui::SExpDesignUI::save_settings()
{
	using boost::property_tree::ptree;
	using namespace sigfile;
	ptree pt;

	_geometry_placeholder.assign(
		to_string( geometry.w) + 'x'
		+ to_string( geometry.h) + '+'
		+ to_string( geometry.x) + '+'
		+ to_string( geometry.y));
	_aghtt_placeholder = AghT();
	_aghdd_placeholder = AghD();

	put( config_keys_s, pt);
	put( config_keys_z, pt);
	put( config_keys_b, pt);
	put( config_keys_g, pt);

	for ( size_t i = SPage::TScore::none; i != SPage::TScore::_total; ++i )
		pt.put( (string("ScoreCodes.") + SPage::score_name((SPage::TScore)i)), ext_score_codes[i]);

	auto colours =
		forward_list<pair<const char*, SManagedColor&>>
		({
			{"NONE",	CwB[TColour::score_none    ]},
			{"NREM1",	CwB[TColour::score_nrem1   ]},
			{"NREM2",	CwB[TColour::score_nrem2   ]},
			{"NREM3",	CwB[TColour::score_nrem3   ]},
			{"NREM4",	CwB[TColour::score_nrem4   ]},
			{"REM",		CwB[TColour::score_rem     ]},
			{"Wake",	CwB[TColour::score_wake    ]},
			{"ProfilePsdSF",CwB[TColour::profile_psd_sf]},
			{"ProfileMcSF",	CwB[TColour::profile_mc_sf ]},
			{"EMG",   	CwB[TColour::emg           ]},
			{"Hypnogram",	CwB[TColour::hypnogram     ]},
			{"Artifacts",	CwB[TColour::artifact      ]},
			{"Annotations",	CwB[TColour::annotations   ]},
			{"Selection",	CwB[TColour::selection	   ]},
			{"TicksSF",	CwB[TColour::ticks_sf      ]},
			{"LabelsSF",	CwB[TColour::labels_sf     ]},
			{"BandDelta",	CwB[TColour::band_delta    ]},
			{"BandTheta",	CwB[TColour::band_theta    ]},
			{"BandAlpha",	CwB[TColour::band_alpha    ]},
			{"BandBeta",	CwB[TColour::band_beta     ]},
			{"BandGamma",	CwB[TColour::band_gamma    ]},
			{"Cursor",	CwB[TColour::cursor        ]},

			{"Night",	CwB[TColour::night	   ]},
			{"Day",		CwB[TColour::day	   ]},

			{"TicksMT",	CwB[TColour::ticks_mt      ]},
			{"LabelsMT",	CwB[TColour::labels_mt     ]},
			{"PowerMT",   	CwB[TColour::power_mt      ]},

			{"SWA",		CwB[TColour::swa           ]},
			{"SWASim",	CwB[TColour::swa_sim       ]},
			{"ProcessS",	CwB[TColour::process_s     ]},
			{"PaperMR",	CwB[TColour::paper_mr      ]},
			{"TicksMR",	CwB[TColour::ticks_mr      ]},
			{"LabelsMR",	CwB[TColour::labels_mr     ]}
		});
	for ( auto &p : colours ) {
		snprintf_buf( "%#x,%#x,%#x,%#x",
			      p.second.clr.red, p.second.clr.green, p.second.clr.blue,
			      p.second.alpha);
		pt.put( (string("Colours.")+p.first).c_str(), __buf__);
	}

	for ( unsigned short i = TBand::delta; i < TBand::_total; ++i ) {
		snprintf_buf( "%g,%g", freq_bands[i][0], freq_bands[i][1]);
		pt.put( (string("Bands.") + FreqBandNames[i]), __buf__);
	}

	write_xml( CONF_FILE, pt);

	return 0;
}


// eof
