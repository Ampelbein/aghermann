/*
 *       File name:  aghermann/ui/mw/loadsave.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-04-28
 *
 *         Purpose:  load/save ui-related vars
 *
 *         License:  GPL
 */

#include <forward_list>

#include "common/config-validate.hh"
#include "aghermann/ui/globals.hh"
#include "mw.hh"

using namespace std;
using namespace aghui;

#define CONF_FILE ".aghermann.conf"


namespace {

forward_list<pair<const char*, aghui::SExpDesignUI::TColour>>
saving_colors()
{
	using namespace aghui;
	return forward_list<pair<const char*, SExpDesignUI::TColour>>
		({
			{"MWNight",	SExpDesignUI::TColour::mw_night	   },
			{"MWDay",	SExpDesignUI::TColour::mw_day	   },
			{"MWTicks",	SExpDesignUI::TColour::mw_ticks    },
			{"MWLabels",	SExpDesignUI::TColour::mw_labels   },
			{"MWProfile",  	SExpDesignUI::TColour::mw_profile  },

			{"ScoreNONE",	SExpDesignUI::TColour::score_none  },
			{"ScorNeREM1",	SExpDesignUI::TColour::score_nrem1 },
			{"ScoreNREM2",	SExpDesignUI::TColour::score_nrem2 },
			{"ScoreNREM3",	SExpDesignUI::TColour::score_nrem3 },
			{"ScoreNREM4",	SExpDesignUI::TColour::score_nrem4 },
			{"ScoreREM",	SExpDesignUI::TColour::score_rem   },
			{"ScoreWake",	SExpDesignUI::TColour::score_wake  },

			{"SFProfilePSD",  SExpDesignUI::TColour::sf_profile_psd},
			{"SFProfileSWU",  SExpDesignUI::TColour::sf_profile_swu},
			{"SFProfileMC",	  SExpDesignUI::TColour::sf_profile_mc },

			{"SFPhasicSpindle",  SExpDesignUI::TColour::sf_phasic_spindle},
			{"SFPhasicKComplex", SExpDesignUI::TColour::sf_phasic_Kcomplex},
			{"SFPhasicEyeBlink", SExpDesignUI::TColour::sf_phasic_eyeblink},

			{"SFEMG",   	  SExpDesignUI::TColour::sf_emg        },
			{"SFHypnogram",	  SExpDesignUI::TColour::sf_hypnogram  },
			{"SFArtifacts",	  SExpDesignUI::TColour::sf_artifact   },
			{"SFAnnotations", SExpDesignUI::TColour::sf_annotations},
			{"SFEmbeddedAnnotations", SExpDesignUI::TColour::sf_embedded_annotations},
			{"SFSelection",	  SExpDesignUI::TColour::sf_selection  },
			{"SFTicks",	  SExpDesignUI::TColour::sf_ticks      },
			{"SFLabels",	  SExpDesignUI::TColour::sf_labels     },
			{"SFCursor",	  SExpDesignUI::TColour::sf_cursor     },

			{"BandDelta",	SExpDesignUI::TColour::band_delta    },
			{"BandTheta",	SExpDesignUI::TColour::band_theta    },
			{"BandAlpha",	SExpDesignUI::TColour::band_alpha    },
			{"BandBeta",	SExpDesignUI::TColour::band_beta     },
			{"BandGamma",	SExpDesignUI::TColour::band_gamma    },

			{"MFSWA",	SExpDesignUI::TColour::mf_swa        },
			{"MFSWASim",	SExpDesignUI::TColour::mf_swa_sim    },
			{"MFProcessS",	SExpDesignUI::TColour::mf_process_s  },
			{"MFPaper",	SExpDesignUI::TColour::mf_paper      },
			{"MFTicks",	SExpDesignUI::TColour::mf_ticks      },
			{"MFLabels",	SExpDesignUI::TColour::mf_labels     }
		});
}
} // namespace

int
aghui::SExpDesignUI::
load_settings()
{
	libconfig::Config conf;

	try {
		conf.readFile( CONF_FILE);
		agh::confval::get( config_keys_s, conf);
		agh::confval::get( config_keys_b, conf);
		agh::confval::get( config_keys_d, conf);
		agh::confval::get( config_keys_g, conf);

		try {
			auto& SC = conf.lookup("ScoreCodes");
			for ( size_t i = sigfile::SPage::TScore::none; i < sigfile::SPage::TScore::TScore_total; ++i )
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
	_AghTi = find( AghTT.begin(), AghTT.end(), sigfile::SChannel (_aghtt_placeholder));
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

	agh::confval::put( config_keys_s, conf);
	agh::confval::put( config_keys_b, conf);
	agh::confval::put( config_keys_d, conf);
	agh::confval::put( config_keys_g, conf);

	agh::confval::put( conf, "ScoreCodes", ext_score_codes);

	for ( auto &p : saving_colors() ) {
		auto& C = CwB[p.second];
		agh::confval::put( conf, string("Color.") + p.first,
			      forward_list<double> {C.clr.red, C.clr.green, C.clr.blue, C.clr.alpha});
	}

	conf.writeFile( CONF_FILE);

	return 0;
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
