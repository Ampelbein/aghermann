// ;-*-C++-*- *  Time-stamp: "2011-03-14 00:50:49 hmmr"
/*
 *       File name:  core/primaries-loadsave.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-04-28
 *
 *         Purpose:  CExpDesign::{load,save}.
 *                   These are the two methods relying heavily on glib facilities
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
//#include "iface.h"

using namespace std;




int
CExpDesign::load()
{
	GKeyFile *kf = g_key_file_new();

	GError *kf_error = NULL;

	gint	intval;
	gdouble	dblval;
	GString *ext_msg = g_string_sized_new( 120);

	UNIQUE_CHARP (_);

	reset_error_log();

	if ( !g_key_file_load_from_file( kf, "ExpDesign", G_KEY_FILE_KEEP_COMMENTS, &kf_error) ) {
		printf( "%s\n", kf_error->message);

		switch ( kf_error->code ) {
		case G_KEY_FILE_ERROR_NOT_FOUND:
			if ( asprintf( &_,
				       "The file ExpDesign could not be read from \"%s\" (%s)\n",
				       session_dir(), kf_error->message) )
				;
			break;
		case G_KEY_FILE_ERROR_PARSE:
			if ( asprintf( &_,
				       "The file ExpDesign in %s could not be parsed\n",
				       session_dir()) )
				;
			break;
		default:
			if ( asprintf( &_,
				       "There was an error reading or parsing the ExpDesign file in %s (%s)\n",
				       session_dir(), kf_error->message) )
				;
		}

		_error_log += _;
		_status = AGH_SESSION_STATE_LOADFAIL;
		return -1;
	}


	dblval = g_key_file_get_double( kf, "ReqPercentScored", "Value", NULL);
	if ( dblval == 0 ) {
		g_string_append_printf( ext_msg, "Bad value for ReqPercentScored.\n");
		req_percent_scored = 90.;
	} else
		req_percent_scored = dblval;

	intval = g_key_file_get_integer( kf, "SWALadenPagesBeforeSWA0", "Value", NULL);
	if ( intval == 0 ) {
		g_string_append_printf( ext_msg, "Bad value for SWALadenPagesBeforeSWA0.\n");
		swa_laden_pages_before_SWA_0 = 3;
	} else
		swa_laden_pages_before_SWA_0 = intval;


	intval = g_key_file_get_integer( kf, "FFT", "WelchWindowType", NULL);
	if ( intval > AGH_WT_N_TYPES || intval < 0 ) {
		g_string_append_printf( ext_msg, "Bad WelchWindowType.\n");
		fft_params.welch_window_type = AGH_WT_WELCH;
	} else
		fft_params.welch_window_type = (TFFTWinType)intval;

	dblval = g_key_file_get_double( kf, "FFT", "BinSize", NULL);
	if ( dblval <= 0 ) {
		g_string_append_printf( ext_msg, "BinSize must be a positive float.\n");
		fft_params.bin_size = 1.;
	} else
		fft_params.bin_size = dblval;

	intval = g_key_file_get_integer( kf, "FFT", "PageSize", NULL);
	if ( intval <= 0 || intval > 600 ) {
		g_string_append_printf( ext_msg, "PageSize must be an integer in the range (0..600].\n");
		fft_params.page_size = 30;
	} else
		fft_params.page_size = intval;


	intval = g_key_file_get_integer( kf, "Artifacts", "DampenWindowType", NULL);
	if ( intval > AGH_WT_N_TYPES || intval < 0 ) {
		g_string_append_printf( ext_msg, "Bad DampenWindowType.\n");
		af_dampen_window_type = AGH_WT_WELCH;
	} else
		af_dampen_window_type = (TFFTWinType)intval;


	if ( g_key_file_has_group( kf, "Control parameters") ) {
		control_params.siman_params.n_tries		= g_key_file_get_integer( kf, "Control parameters", "NTries", NULL);
		control_params.siman_params.iters_fixed_T	= g_key_file_get_integer( kf, "Control parameters", "ItersFixedT", NULL);
		control_params.siman_params.step_size		= g_key_file_get_double ( kf, "Control parameters", "StepSize", NULL);
		control_params.siman_params.k			= g_key_file_get_double ( kf, "Control parameters", "Boltzmannk", NULL);
		control_params.siman_params.t_initial		= g_key_file_get_double ( kf, "Control parameters", "TInitial", NULL);
		control_params.siman_params.mu_t		= g_key_file_get_double ( kf, "Control parameters", "DampingMu", NULL);
		control_params.siman_params.t_min		= g_key_file_get_double ( kf, "Control parameters", "TMin", NULL);

		control_params.DBAmendment1           = g_key_file_get_boolean( kf, "Control parameters", "DBAmendment1", NULL);
		control_params.DBAmendment2           = g_key_file_get_boolean( kf, "Control parameters", "DBAmendment2", NULL);
		control_params.AZAmendment            = g_key_file_get_boolean( kf, "Control parameters", "AZAmendment", NULL);
		control_params.ScoreMVTAsWake         = g_key_file_get_boolean( kf, "Control parameters", "ScoreMVTAsWake", NULL);
		control_params.ScoreUnscoredAsWake    = g_key_file_get_boolean( kf, "Control parameters", "ScoreUnscoredAsWake", NULL);

		if ( !control_params.is_sane() ) {
			g_string_append_printf( ext_msg,
						"Failed to read all control parameters, or some values were missing or "
						"those supplied invalid.\n");
			control_params.assign_defaults();
		}
	}


	if ( !g_key_file_has_group( kf, "Tunables") )
		g_string_append_printf( ext_msg, "No [Tunables] group found.\n");
	else {
		for ( guint t = 0; t < _gc_; t++ ) {
			gdouble *x;
			gsize n;
			x = g_key_file_get_double_list( kf, "Tunables", __AGHTT[t].name, &n, NULL);
			if ( n == 5 ) {
				tunables.value.P[t]	= x[0];
				tunables.lo.P[t]	= x[1];
				tunables.hi.P[t]	= x[2];
				tunables.step.P[t]	= x[3];
				tunables.state[t]	= ((gboolean)x[4] ? STunableSetFull::T_REQUIRED : 0);
			} else // no matter which assignment failed
				g_string_append_printf( ext_msg,
							"Failed to read all fields for tunable %s.\n",
							__AGHTT[t].name);
			g_free( x);
		}
		if ( !tunables.check_consisitent() ) {
			g_string_append_printf( ext_msg,
						"Tunable definitions inconsistent; defaults will be assigned.\n");
			tunables.assign_defaults();
		}
	}


	if ( ext_msg->len ) {
		_error_log += ext_msg->str;
		fprintf( stderr, "CExpDesign::load: completed, with warnings:\n%s\n",
			 ext_msg->str);
	}

	g_string_free( ext_msg, TRUE);
	g_key_file_free( kf);
	if ( kf_error)
		g_error_free( kf_error);

	return 0;
}






gint
CExpDesign::save() const
{
	GKeyFile *kf = g_key_file_new();

	GString *agg = g_string_sized_new( 200);

	g_key_file_set_integer( kf, "Control parameters", "NTries",		control_params.siman_params.n_tries);
	g_key_file_set_integer( kf, "Control parameters", "ItersFixedT",	control_params.siman_params.iters_fixed_T);
	g_key_file_set_double( kf, "Control parameters", "StepSize",		control_params.siman_params.step_size);
	g_key_file_set_double( kf, "Control parameters", "Boltzmannk",		control_params.siman_params.k);
	g_key_file_set_double( kf, "Control parameters", "TInitial",		control_params.siman_params.t_initial);
	g_key_file_set_double( kf, "Control parameters", "DampingMu",		control_params.siman_params.mu_t);
	g_key_file_set_double( kf, "Control parameters", "TMin",		control_params.siman_params.t_min);

	g_key_file_set_boolean( kf, "Control parameters", "DBAmendment1",		control_params.DBAmendment1);
	g_key_file_set_boolean( kf, "Control parameters", "DBAmendment2",		control_params.DBAmendment2);
	g_key_file_set_boolean( kf, "Control parameters", "AZAmendment",		control_params.AZAmendment);
	g_key_file_set_boolean( kf, "Control parameters", "ScoreMVTAsWake",		control_params.ScoreMVTAsWake);
	g_key_file_set_boolean( kf, "Control parameters", "ScoreUnscoredAsWake",	control_params.ScoreUnscoredAsWake);


	for ( guint t = 0; t < tunables.value.P.size(); ++t ) {
		gdouble x[5] = { tunables.value.P[t],
				 tunables.lo.P[t],
				 tunables.hi.P[t],
				 tunables.step.P[t],
				 (gdouble)(tunables.state[t] & STunableSetFull::T_REQUIRED) };
		char gcx_buf[6];
		g_key_file_set_double_list( kf, "Tunables",
					    (t <= _gc_) ? __AGHTT[t].name : (snprintf( gcx_buf, 5, "gc%u", t-_gc_),
									     gcx_buf),
					    x, 5);
	}


	g_key_file_set_integer( kf, "SWALadenPagesBeforeSWA0", "Value", swa_laden_pages_before_SWA_0);
	g_key_file_set_double(  kf, "ReqPercentScored", "Value", req_percent_scored);

	g_key_file_set_integer( kf, "FFT", "WelchWindowType",		fft_params.welch_window_type);
	g_key_file_set_double(  kf, "FFT", "BinSize",			fft_params.bin_size);
	g_key_file_set_integer( kf, "FFT", "PageSize",			fft_params.page_size);

	g_key_file_set_integer( kf, "Artifacts", "DampenWindowType",	af_dampen_window_type);

	g_string_free( agg, TRUE);

	gchar *towrite = g_key_file_to_data( kf, NULL, NULL);
	g_file_set_contents( "ExpDesign", towrite, -1, NULL);
	g_free( towrite);

	g_key_file_free( kf);

	return 0;
}





// EOF
