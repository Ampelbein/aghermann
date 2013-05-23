/*
 *       File name:  aghermann/ui/mw/settings_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  SExpDesignUI settings tab callbacks
 *
 *         License:  GPL
 */

#ifdef _OPENMP
#include <omp.h>
#endif

#include "common/globals.hh"
#include "common/string.hh"
#include "aghermann/ui/misc.hh"
#include "aghermann/ui/sf/sf.hh"
#include "mw.hh"

using namespace std;
using namespace aghui;



extern "C" {

void
tDesign_switch_page_cb(
	GtkNotebook*,
	gpointer,
	const guint page_num,
	const gpointer userdata)
{
	using namespace sigfile;
	auto& ED = *(SExpDesignUI*)userdata;

      // save parameters changing which should trigger tree rescan
	if ( page_num == 0 ) {  // switching back from settings tab

	      // collect values from widgets
		ED.W_V1.down();

		ED.ED->fft_params.pagesize = ED.FFTPageSizeValues[ED.pagesize_item];
		ED.ED->fft_params.binsize =  ED.FFTBinSizeValues [ED.binsize_item];

		try { ED.ED->fft_params.check(); }
		catch (invalid_argument ex) {
			pop_ok_message( ED.wMainWindow,
					"Invalid FFT parameters", "Resetting to defaults.");
			ED.ED->fft_params.reset();
		}

		try {
			ED.ED->mc_params.check();
		} catch (invalid_argument ex) {
			pop_ok_message( ED.wMainWindow, "Invalid uC parameters", "Resetting to defaults.");
			ED.ED->mc_params.reset();
		}

		ED.adjust_op_freq_spinbuttons();

#ifdef _OPENMP
		omp_set_num_threads(
			(ED.ED->num_threads == 0)
			? agh::global::num_procs
			: ED.ED->num_threads);
#endif
	      // scan as necessary
		if ( ED.pagesize_item_saved	  		!= ED.pagesize_item ||
		     ED.binsize_item_saved	  		!= ED.binsize_item ||
		     ED.fft_params_plan_type_saved  		!= ED.ED->fft_params.plan_type ||
		     ED.fft_params_welch_window_type_saved  	!= ED.ED->fft_params.welch_window_type ||
		     ED.af_dampen_window_type_saved  		!= ED.ED->af_dampen_window_type ||
		     ED.af_dampen_factor_saved	  		!= ED.ED->af_dampen_factor ||
		     !ED.ED->mc_params.same_as( ED.mc_params_saved) ) {
		      // rescan tree
			ED.do_rescan_tree(); // with populate
		} else if ( ED.tl_height_saved			!= ED.tl_height ||
			    // recalculte mesurements layout as necessary
			    ED.tl_pph_saved			!= ED.tl_pph )
			ED.populate_1();

	} else {
		ED.tl_pph_saved				= ED.tl_pph;
		ED.tl_height_saved			= ED.tl_height;
		ED.pagesize_item_saved			= ED.pagesize_item;
		ED.binsize_item_saved			= ED.binsize_item;
		ED.fft_params_welch_window_type_saved	= ED.ED->fft_params.welch_window_type;
		ED.fft_params_plan_type_saved		= ED.ED->fft_params.plan_type;
		ED.af_dampen_window_type_saved		= ED.ED->af_dampen_window_type;
		ED.af_dampen_factor_saved		= ED.ED->af_dampen_factor;
		ED.mc_params_saved			= ED.ED->mc_params;

		ED.pagesize_item = ED.figure_pagesize_item();
		ED.binsize_item = ED.figure_binsize_item();

		ED.W_V1.up();

		// colours are served specially elsewhere
	}
}







// ================== Simulations part



namespace {

void
__adjust_adjustments( SExpDesignUI& ED)
{
	using namespace agh::ach;
	for ( size_t t = 0; t < (size_t)TTunable::_basic_tunables; ++t ) {
		gtk_adjustment_configure(
			ED.jTunable[t][0],
			stock[t].display_scale_factor * ED.ED->tunables0[t],
			stock[t].display_scale_factor * ED.ED->tlo[t],
			stock[t].display_scale_factor * ED.ED->thi[t],
			stock[t].adj_step, 10 * stock[t].adj_step,
			0);
		gtk_adjustment_configure(
			ED.jTunable[t][1],
			stock[t].display_scale_factor * ED.ED->tlo[t],
			0,
			stock[t].display_scale_factor * ED.ED->thi[t],
			stock[t].adj_step, 10 * stock[t].adj_step,
			0);
		gtk_adjustment_configure(
			ED.jTunable[t][2],
			stock[t].display_scale_factor * ED.ED->thi[t],
			stock[t].display_scale_factor * ED.ED->tlo[t],
			stock[t].display_scale_factor * ED.ED->thi[t] * 1.5,
			stock[t].adj_step, 10 * stock[t].adj_step,
			0);
		gtk_adjustment_configure(
			ED.jTunable[t][3],
			stock[t].display_scale_factor * ED.ED->tstep[t],
			stock[t].display_scale_factor * ED.ED->tunables0[t] / 1000.,
			stock[t].display_scale_factor * ED.ED->tunables0[t] / 2,
			stock[t].adj_step, 10 * stock[t].adj_step,
			0);
	}
}

void
__adjust_tunables_up( SExpDesignUI& ED)
{
	using namespace agh::ach;
	for ( size_t t = 0; t < TTunable::_basic_tunables; ++t ) {
		ED.ED->tunables0 [t] *= stock[t].display_scale_factor;
		ED.ED->tlo       [t] *= stock[t].display_scale_factor;
		ED.ED->thi       [t] *= stock[t].display_scale_factor;
		ED.ED->tstep     [t] *= stock[t].display_scale_factor;
	}
}

void
__adjust_tunables_down( SExpDesignUI& ED)
{
	using namespace agh::ach;
	for ( size_t t = 0; t < TTunable::_basic_tunables; ++t ) {
		ED.ED->tunables0 [t] /= stock[t].display_scale_factor;
		ED.ED->tlo       [t] /= stock[t].display_scale_factor;
		ED.ED->thi       [t] /= stock[t].display_scale_factor;
		ED.ED->tstep     [t] /= stock[t].display_scale_factor;
	}
}

} // namespace

void
tSimulations_switch_page_cb(
	GtkNotebook*,
	gpointer,
	const guint page_num,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	if ( page_num == 1 ) {  // switching to display parameters tab
		agh::str::decompose_double(
			ED.ED->ctl_params0.siman_params.t_min,
			&ED.ctl_params0_siman_params_t_min_mantissa,
			&ED.ctl_params0_siman_params_t_min_exponent);
		agh::str::decompose_double(
			ED.ED->ctl_params0.siman_params.t_initial,
			&ED.ctl_params0_siman_params_t_initial_mantissa,
			&ED.ctl_params0_siman_params_t_initial_exponent);
		__adjust_adjustments( ED);
		__adjust_tunables_up( ED);
		ED.W_Vtunables.up();
		ED.W_V2.up();
		for ( auto& t : {ED.eCtlParamDBAmendment1,
				 ED.eCtlParamDBAmendment2,
				 ED.eCtlParamAZAmendment1} ) {
			g_signal_emit_by_name( t, "toggled");
			g_signal_emit_by_name( t, "toggled");
		}
	} else {
		ED.W_V2.down();
		ED.W_Vtunables.down();
		__adjust_tunables_down( ED);
		ED.ED->ctl_params0.siman_params.t_min =
			ED.ctl_params0_siman_params_t_min_mantissa
			* pow(10, ED.ctl_params0_siman_params_t_min_exponent);
		ED.ED->ctl_params0.siman_params.t_initial =
			ED.ctl_params0_siman_params_t_initial_mantissa
			* pow(10, ED.ctl_params0_siman_params_t_initial_exponent);

	      // for ctlparam changes to take effect on virgin modruns
		ED.populate_2();
	}
}




void
bSimParamRevertTunables_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	ED.ED->tunables0.set_defaults();
	ED.ED->tlo.set_defaults();
	ED.ED->thi.set_defaults();
	ED.ED->tstep.set_defaults();

	__adjust_tunables_up( ED);
	ED.W_Vtunables.up();
}


void
eCtlParamDBAmendment1_toggled_cb(
	GtkToggleButton *button,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_label_set_markup(
		ED.lCtlParamDBAmendment1,
		gtk_toggle_button_get_active( button)
		? "<small>Let SWA be affected by <i>S</i> at all times</small>"
		: "<small>Cancel <i>rc</i>-dependent SWA increase in Wake</small>");
}

void
eCtlParamDBAmendment2_toggled_cb(
	GtkToggleButton *button,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_label_set_markup(
		ED.lCtlParamDBAmendment2,
		gtk_toggle_button_get_active( button)
		? "<small>Assume sleep homeostat is stable (<i>S</i><sub>24h</sub> = <i>S</i><sub>0</sub>)</small>"
		: "<small>Don't assume <i>S</i><sub>24h</sub> = <i>S</i><sub>0</sub></small>");
}

void
eCtlParamAZAmendment1_toggled_cb(
	GtkToggleButton *button,
	const gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_label_set_markup(
		ED.lCtlParamAZAmendment1,
		gtk_toggle_button_get_active( button)
		? "<small>Compute <i>gc</i> per-episode</small>"
		: "<small>Assume <i>gc</i> is not variable across episodes</small>");
}

// -------- colours
void
bColourX_color_set_cb(
	GtkColorButton*,
	const gpointer userdata)
{
	auto& mc = *(SManagedColor*)userdata;
	mc.acquire();
}

} // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
