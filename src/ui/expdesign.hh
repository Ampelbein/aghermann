// ;-*-C++-*-
/*
 *       File name:  ui/expdesign.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-13
 *
 *         Purpose:  class SExpDesignUI
 *
 *         License:  GPL
 */

#ifndef _AGHUI_EXPDESIGN_H
#define _AGHUI_EXPDESIGN_H

#include <string>
#include <list>
#include <forward_list>
#include <map>
#include <stdexcept>

#include <cairo/cairo.h>

#include "../common/lang.hh"
#include "../common/config-validate.hh"
#include "../model/forward-decls.hh"
#include "../expdesign/primaries.hh"
#include "ui.hh"
#include "ui++.hh"
#include "forward-decls.hh"
#include "expdesign-widgets.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;

namespace aghui {

class SExpDesignUI
  : public SExpDesignUIWidgets {

	DELETE_DEFAULT_METHODS(SExpDesignUI);

    public:
	agh::CExpDesign
		*ED;
	aghui::SSessionChooser
		*_p;

      // forward decl
	class SGroupPresentation;

      // contained classes
	class SSubjectPresentation {
		DELETE_DEFAULT_METHODS (SSubjectPresentation);

	    public:
		agh::CSubject&  // can't have it declared const due to CMSessionSet operator[] not permitting
			csubject;
		agh::CSCourse // a shortcut
			*cscourse;
		void create_cscourse();

		list<agh::CSubject::SEpisode>&
		sepisodesequence() const
			{
				return csubject.measurements[*_p._p._AghDi].episodes;
			}

		agh::CSubject::SEpisode
			*using_episode;
		bool is_episode_focused() const
			{
				return using_episode != nullptr;
			}
		bool	is_focused;

		bool get_episode_from_timeline_click( unsigned along);  // possibly sets episode_focused

		time_t	tl_start;

		time_t timeline_start() const	{ return _p._p.timeline_start; }
		time_t timeline_end() const	{ return _p._p.timeline_end; }
		size_t timeline_width() const	{ return _p._p.timeline_width; }
		size_t timeline_height() const	{ return _p._p.timeline_height; }
		size_t tl_left_margin() const	{ return _p._p.tl_left_margin; }
		size_t tl_right_margin() const	{ return _p._p.tl_right_margin; }

		void draw_timeline( cairo_t *cr) const;
		void draw_timeline( const char *fname) const;

		SGroupPresentation& _p;
		SSubjectPresentation (agh::CSubject& _j, SGroupPresentation& parent);
	       ~SSubjectPresentation ();

		GtkWidget
			*da;
	};

	class SGroupPresentation
	      : public list<SSubjectPresentation> {
		DELETE_DEFAULT_METHODS (SGroupPresentation);

	    public:
		friend class SSubjectPresentation;
		friend class SExpDesignUI;

		const char* name() const
			{
				return cjgroup->first.c_str();
			}
		agh::CJGroup& group()
			{
				return cjgroup->second;
			}
		SGroupPresentation (agh::CExpDesign::TJGroups::iterator& _g,
				    SExpDesignUI& parent)
		      : cjgroup (_g),
			_p (parent),
			visible (true)
			{}

		SExpDesignUI&
			_p;

		bool	visible;
		GtkExpander
			*expander,
			*vbox;

	    private:
		agh::CExpDesign::TJGroups::iterator cjgroup;
	};
	list<SGroupPresentation>
		groups;
	SSubjectPresentation
		*using_subject;
	SSubjectPresentation*
	subject_presentation_by_csubject( const agh::CSubject&);

      // ctor, dtor
	SExpDesignUI (aghui::SSessionChooser *parent, const string& dir);
       ~SExpDesignUI ();

      // flags
	bool	draw_nremrem_cycles;
	bool	finalize_ui:1,
		suppress_redraw:1,
		nodestroy_by_cb:1;

      // populate
	string	last_used_version;
	void show_changelog();

	int populate( bool do_load);
	void depopulate( bool do_save);
	void populate_1();  // measurements
	void populate_2();  // simulations
	void cleanup_2();
	void do_rescan_tree( bool with_progress_bar = true);
	void do_purge_computed();
	void do_detect_ultradian_cycle( agh::CRecording&);

	void update_subject_details_interactively( agh::CSubject&);
	void show_empty_experiment_blurb();
	int try_download();

      // collected ED info
        // ED strings (channels, sessions, etc)
	list<string>
		AghDD,	AghGG,	AghEE;
	list<sigfile::SChannel>
		AghHH,	AghTT;
	list<sigfile::SChannel>::iterator
		_AghHi,	_AghTi;
	list<string>::iterator
		_AghGi,	_AghDi,	_AghEi;
	const char* AghH() const { return (_AghHi != AghHH.end()) ? _AghHi->c_str() : "(no channels)"; }
	const char* AghT() const { return (_AghTi != AghTT.end()) ? _AghTi->c_str() : "(no channels)"; }
	const char* AghG() const { return (_AghGi != AghGG.end()) ? _AghGi->c_str() : "(no groups)"; }
	const char* AghD() const { return (_AghDi != AghDD.end()) ? _AghDi->c_str() : "(no sessions)"; }
	const char* AghE() const { return (_AghEi != AghEE.end()) ? _AghEi->c_str() : "(no episodes)"; }
	int AghTi() const;
	int AghDi() const;

	// collected full-path annotations
	struct SAnnotation
	      : public agh::CSubject::SEpisode::SAnnotation {
		agh::CSubject& csubject;
		const string& session;
		agh::CSubject::SEpisode& sepisode;
		SAnnotation (agh::CSubject& j, const string& d, agh::CSubject::SEpisode& e,
			     agh::CSubject::SEpisode::SAnnotation& a)
		      : agh::CSubject::SEpisode::SAnnotation (a),
			csubject (j), session (d), sepisode (e)
			{}
	};
	forward_list<SAnnotation>
		global_annotations;

	// samplerates
	list<size_t>
		used_samplerates,
		used_eeg_samplerates;

	list<aghui::SScoringFacility*>
		open_scoring_facilities;
	aghui::SScoringFacility
		*close_this_SF_now;

	// common artifact detection profiles
	struct SDetectArtifactsParamPack {
		double	scope,
			upper_thr, lower_thr,
			f0, fc, bandwidth,
			mc_gain, iir_backpolate;
		double	E, dmin, dmax;
		size_t	sssu_hist_size,
			smooth_side;
		bool	estimate_E,
			use_range;
		SDetectArtifactsParamPack ()
			: scope (4.),
			  upper_thr (9.), lower_thr (-9.),
			  f0 (1.), fc (1.8), bandwidth (1.5),
			  mc_gain (10.), iir_backpolate (.5),
			  E (4.), dmin (-10), dmax (20),
			  sssu_hist_size (100), smooth_side (0),
			  estimate_E (true), use_range (false)
			{}
	};
	map<string, SDetectArtifactsParamPack>
		artifact_detection_profiles;

      // own variables aka saved settings
	sigfile::TMetricType
		display_profile_type;
	double	operating_range_from,
		operating_range_upto;

	double	uc_accuracy_factor;
	static const array<unsigned, 4>
		FFTPageSizeValues;
	static const array<double, 3>
		FFTBinSizeValues;
	int	pagesize_item,
		binsize_item;
	size_t figure_pagesize_item(); // from corresponding ED->fft_params.* fields
	size_t figure_binsize_item();
	size_t pagesize() const
		{
			return FFTPageSizeValues[pagesize_item];
		}
	double binsize() const
		{
			return FFTBinSizeValues[binsize_item];
		}

	sigfile::CHypnogram::TCustomScoreCodes
		ext_score_codes;

	static const char
		*const FreqBandNames[(size_t)sigfile::TBand::_total];
	double	freq_bands[(size_t)sigfile::TBand::_total][2];

	double	profile_scale_psd,
		profile_scale_mc;
	void calculate_profile_scale();
	bool	autoscale;
	size_t	smooth_profile;

	size_t	timeline_height;
	size_t	timeline_pph;

	time_t	timeline_start,
		timeline_end;
	size_t T2P( time_t t) const
		{
			return difftime( t, timeline_start) / 3600. * timeline_pph;
		}
	time_t P2T( int p) const
		{
			return p * 3600. / timeline_pph + timeline_start;
		}
	size_t	tl_left_margin,
		tl_right_margin,
		timeline_width,
		timeline_pages;

      // config
	string	_geometry_placeholder,
		_aghdd_placeholder,
		_aghtt_placeholder;

	size_t	timeline_pph_saved,
		timeline_height_saved;
	size_t	pagesize_item_saved,
		binsize_item_saved;
	sigfile::SFFTParamSet::TWinType
		fft_params_welch_window_type_saved,
		af_dampen_window_type_saved;
	double	af_dampen_factor_saved;

	// sigfile::SFFTParamSet
	// 	fft_params_saved; // members not represented in widgets as is
	sigfile::SMCParamSet
		mc_params_saved;
	SUIVarCollection
		W_V;

      // status bar bits
	void sb_main_progress_indicator( const char*, size_t n, size_t i);
	void buf_on_main_status_bar();

      // dnd
	struct SDndIface {
		GtkTargetEntry
			target_list[];
		size_t	n_targets;
	};
	SDndIface
		dnd;
	int dnd_maybe_admit_one( const char* fname);

	string	browse_command;

	SGeometry
		geometry;

	forward_list<confval::SValidator<string>>	config_keys_s;
	forward_list<confval::SValidator<int>>		config_keys_d;
	forward_list<confval::SValidator<double>>	config_keys_g;
	int load_settings();
	int save_settings();

	void populate_mSessions();
	void populate_mChannels();
	void populate_mGlobalAnnotations();
	void __adjust_op_freq_spinbuttons();
	void __reconnect_channels_combo();
	void __reconnect_sessions_combo();
};


inline int
__attribute__ ((pure))
SExpDesignUI::AghTi() const
{
	int i = 0;
	for ( auto Ti = AghTT.begin(); Ti != AghTT.end(); ++Ti, ++i )
		if ( Ti == _AghTi )
			return i;
	return -1;
}
inline int
__attribute__ ((pure))
SExpDesignUI::AghDi() const
{
	int i = 0;
	for ( auto Di = AghDD.begin(); Di != AghDD.end(); ++Di, ++i )
		if ( Di == _AghDi )
			return i;
	return -1;
}


} // namespace aghui

#endif

// eof
