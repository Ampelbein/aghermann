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

#include "../common/config-validate.hh"
#include "misc.hh"
#include "expdesign.hh"

using namespace std;
using namespace aghui;

#define CONF_FILE ".aghermann.conf"





int
aghui::SExpDesignUI::load_settings()
{
	libconfig::Config conf;

	try {
		conf.readFile( CONF_FILE);
		auto& cfroot = conf.getRoot();
		confval::get( config_keys_s, cfroot);
		confval::get( config_keys_d, cfroot);
		confval::get( config_keys_g, cfroot);

		for ( size_t i = sigfile::SPage::TScore::none; i != sigfile::SPage::TScore::_total; ++i ) {
			string strval = cfroot[string("ScoreCodes.")+sigfile::SPage::score_name((sigfile::SPage::TScore)i)];
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
			auto& V = cfroot[string("Color.")+p.first];
			clr.red   = (int)V[0];
			clr.green = (int)V[1];
			clr.blue  = (int)V[2];
			alpha     = (int)V[3];
			gtk_color_button_set_color( p.second, &clr);
			gtk_color_button_set_alpha( p.second, alpha);
		}

		for ( size_t i = sigfile::TBand::delta; i < sigfile::TBand::_total; ++i ) {
			float	f0 = cfroot[string("Bands.")+FreqBandNames[i]][0],
				f1 = cfroot[string("Bands.")+FreqBandNames[i]][1];
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
	libconfig::Config conf;
	auto& cfroot = conf.getRoot();

	FAFA;
	_geometry_placeholder.assign(
		to_string( geometry.w) + 'x'
		+ to_string( geometry.h) + '+'
		+ to_string( geometry.x) + '+'
		+ to_string( geometry.y));
	_aghtt_placeholder = AghT();
	_aghdd_placeholder = AghD();

	FAFA;
	confval::put( config_keys_s, cfroot);
	confval::put( config_keys_d, cfroot);
	confval::put( config_keys_g, cfroot);
	FAFA;

//	for ( size_t i = sigfile::SPage::TScore::none; i != sigfile::SPage::TScore::_total; ++i )
		confval::put( cfroot,
			      "ScoreCodes",
			      //string("ScoreCodes.") + sigfile::SPage::score_name((sigfile::SPage::TScore)i),
			      ext_score_codes);

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
	FAFA;
	for ( auto &p : colours )
		confval::put( cfroot, string("Color.") + p.first,
			      forward_list<int> {p.second.clr.red, p.second.clr.green, p.second.clr.blue, p.second.alpha});

	FAFA;
	for ( unsigned short i = sigfile::TBand::delta; i < sigfile::TBand::_total; ++i )
		confval::put( cfroot, string("Band.") + FreqBandNames[i],
			      forward_list<double> {freq_bands[i][0], freq_bands[i][1]});

	FAFA;
	conf.writeFile( CONF_FILE);

	return 0;
}


// eof
