// ;-*-C++-*-
/*
 *       File name:  ui/mw/mw-loadsave.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  load/save ui-related vars
 *
 *         License:  GPL
 */

#include <forward_list>
//#include <initializer_list>

#include "common/config-validate.hh"
#include "ui/globals.hh"
#include "mw.hh"

using namespace std;
using namespace aghui;

#define CONF_FILE ".aghermann.conf"


inline namespace {

forward_list<pair<const char*, aghui::SExpDesignUI::TColour>>
saving_colors()
{
	using namespace aghui;
	return forward_list<pair<const char*, SExpDesignUI::TColour>>
		({
			{"NONE",	SExpDesignUI::TColour::score_none    },
			{"NREM1",	SExpDesignUI::TColour::score_nrem1   },
			{"NREM2",	SExpDesignUI::TColour::score_nrem2   },
			{"NREM3",	SExpDesignUI::TColour::score_nrem3   },
			{"NREM4",	SExpDesignUI::TColour::score_nrem4   },
			{"REM",		SExpDesignUI::TColour::score_rem     },
			{"Wake",	SExpDesignUI::TColour::score_wake    },
			{"ProfilePsdSF",SExpDesignUI::TColour::profile_psd_sf},
			{"ProfileMcSF",	SExpDesignUI::TColour::profile_mc_sf },
			{"EMG",   	SExpDesignUI::TColour::emg           },
			{"Hypnogram",	SExpDesignUI::TColour::hypnogram     },
			{"Artifacts",	SExpDesignUI::TColour::artifact      },
			{"Annotations",	SExpDesignUI::TColour::annotations   },
			{"Selection",	SExpDesignUI::TColour::selection     },
			{"TicksSF",	SExpDesignUI::TColour::ticks_sf      },
			{"LabelsSF",	SExpDesignUI::TColour::labels_sf     },
			{"BandDelta",	SExpDesignUI::TColour::band_delta    },
			{"BandTheta",	SExpDesignUI::TColour::band_theta    },
			{"BandAlpha",	SExpDesignUI::TColour::band_alpha    },
			{"BandBeta",	SExpDesignUI::TColour::band_beta     },
			{"BandGamma",	SExpDesignUI::TColour::band_gamma    },
			{"Cursor",	SExpDesignUI::TColour::cursor        },

			{"Night",	SExpDesignUI::TColour::night	     },
			{"Day",		SExpDesignUI::TColour::day	     },

			{"TicksMT",	SExpDesignUI::TColour::ticks_mt      },
			{"LabelsMT",	SExpDesignUI::TColour::labels_mt     },
			{"PowerMT",   	SExpDesignUI::TColour::power_mt      },

			{"SWA",		SExpDesignUI::TColour::swa           },
			{"SWASim",	SExpDesignUI::TColour::swa_sim       },
			{"ProcessS",	SExpDesignUI::TColour::process_s     },
			{"PaperMR",	SExpDesignUI::TColour::paper_mr      },
			{"TicksMR",	SExpDesignUI::TColour::ticks_mr      },
			{"LabelsMR",	SExpDesignUI::TColour::labels_mr     }
		});
}
} // inline namespace

int
aghui::SExpDesignUI::
load_settings()
{
	libconfig::Config conf;

	try {
		conf.readFile( CONF_FILE);
		confval::get( config_keys_s, conf);
		confval::get( config_keys_d, conf);
		confval::get( config_keys_g, conf);

		try {
			auto& SC = conf.lookup("ScoreCodes");
			for ( size_t i = sigfile::SPage::TScore::none; i < sigfile::SPage::TScore::_total; ++i )
				ext_score_codes[i].assign( (const char*)SC[i]);
		} catch (...) {
			fprintf( stderr, "SExpDesignUI::load_settings(): Something is wrong with section ScoreCodes in %s\n", CONF_FILE);
		}
		for( auto &p : saving_colors() ) {
			try {
				auto& V = conf.lookup(string("Color.")+p.first);
				auto& C = CwB[p.second];
				C.clr.red   = V[0];
				C.clr.green = V[1];
				C.clr.blue  = V[2];
				C.clr.alpha = V[3];
				gtk_color_chooser_set_rgba( GTK_COLOR_CHOOSER (CwB[p.second].btn), &C.clr);
			} catch (...) {
				fprintf( stderr, "SExpDesignUI::load_settings(): Something is wrong with Color.%s in %s\n", p.first, CONF_FILE);
			}
		}

		try {
			for ( size_t i = metrics::psd::TBand::delta; i < metrics::psd::TBand::_total; ++i ) {
				auto& A = conf.lookup(string("Band.")+FreqBandNames[i]);
				float	f0 = A[0],
					f1 = A[1];
				if ( f0 < f1 ) {
					gtk_spin_button_set_value( eBand[i][0], freq_bands[i][0] = f0);
					gtk_spin_button_set_value( eBand[i][1], freq_bands[i][1] = f1);
				}
			}
		} catch (...) {
			fprintf( stderr, "SExpDesignUI::load_settings(): Something is wrong with section Band in %s\n", CONF_FILE);
		}
	} catch (...) {
		fprintf( stderr, "SExpDesignUI::load_settings(): Something is wrong with %s\n", CONF_FILE);
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
	if ( active_profile_psd_freq_upto <= active_profile_psd_freq_from )
		active_profile_psd_freq_from = 2., active_profile_psd_freq_upto = 3.;

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
	adjust_op_freq_spinbuttons();

	return 0;
}






int
aghui::SExpDesignUI::
save_settings()
{
	libconfig::Config conf;

	_geometry_placeholder.assign(
		to_string( geometry.w) + 'x'
		+ to_string( geometry.h) + '+'
		+ to_string( geometry.x) + '+'
		+ to_string( geometry.y));
	_aghtt_placeholder = AghT();
	_aghdd_placeholder = AghD();

	confval::put( config_keys_s, conf);
	confval::put( config_keys_d, conf);
	confval::put( config_keys_g, conf);

	confval::put( conf, "ScoreCodes", ext_score_codes);

	for ( auto &p : saving_colors() ) {
		auto& C = CwB[p.second];
		confval::put( conf, string("Color.") + p.first,
			      forward_list<double> {C.clr.red, C.clr.green, C.clr.blue, C.clr.alpha});
	}

	for ( unsigned i = metrics::psd::TBand::delta; i < metrics::psd::TBand::_total; ++i )
		confval::put( conf, string("Band.") + FreqBandNames[i],
			      forward_list<double> {freq_bands[i][0], freq_bands[i][1]});

	conf.writeFile( CONF_FILE);

	return 0;
}


// eof
