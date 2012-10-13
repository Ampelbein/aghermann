// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-patterns.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-14
 *
 *         Purpose:  scoring facility patterns
 *
 *         License:  GPL
 */




#include <dirent.h>
#include <sys/stat.h>

#include "misc.hh"
#include "scoring-facility.hh"

using namespace std;


aghui::SScoringFacility::SFindDialog::
SFindDialog (SScoringFacility& parent)
      : params {2, 1.5, .1, .5, 3, 2},
	params_saved {(unsigned short)-1},
	tolerance_a (.2),
	tolerance_b (.4),
	tolerance_c (.6),
	cpattern (nullptr),
	last_find ((size_t)-1),
	increment (3),
	draw_details (true),
	_p (parent)
{
	W_V.reg( _p.ePatternEnvTightness, 	&params.env_tightness);
	W_V.reg( _p.ePatternFilterOrder, 	&params.bwf_order);
	W_V.reg( _p.ePatternFilterCutoff, 	&params.bwf_cutoff);
	W_V.reg( _p.ePatternDZCDFStep, 		&params.dzcdf_step);
	W_V.reg( _p.ePatternDZCDFSigma, 	&params.dzcdf_sigma);
	W_V.reg( _p.ePatternDZCDFSmooth, 	&params.dzcdf_smooth);
	W_V.reg( _p.ePatternParameterA, 	&tolerance_a);
	W_V.reg( _p.ePatternParameterB, 	&tolerance_b);
	W_V.reg( _p.ePatternParameterC, 	&tolerance_c);
}

aghui::SScoringFacility::SFindDialog::
~SFindDialog ()
{
	// g_object_unref( mPatterns);
	// gtk_widget_destroy( (GtkWidget*)wPattern);
}








void
aghui::SScoringFacility::SFindDialog::
set_pattern_da_width( int width)
{
	g_object_set( (GObject*)_p.daPatternSelection,
		      "width-request", da_wd = width,
		      "height-request", da_ht,
		      NULL);
	g_object_set( (GObject*)_p.vpPatternSelection,
		      "width-request", min( width+5, 600),
		      "height-request", da_ht + 30,
		      NULL);
}



void
aghui::SScoringFacility::SFindDialog::
draw( cairo_t *cr)
{
	if ( pattern.size() == 0 ) {
		set_pattern_da_width( 200);
		aghui::cairo_put_banner( cr, da_wd, da_ht, "(no selection)");
		enable_controls( false);
		return;
	} else {
		enable_controls( true);
	}

      // ticks
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 9);
	float	seconds = (float)pattern.size() / samplerate;
	for ( size_t i8 = 0; (float)i8 / 8 < seconds; ++i8 ) {
		_p._p.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgba( cr);
		cairo_set_line_width( cr, (i8%8 == 0) ? 1. : (i8%4 == 0) ? .6 : .3);
		guint x = (float)i8/8 / seconds * da_wd;
		cairo_move_to( cr, x, 0);
		cairo_rel_line_to( cr, 0, da_ht);
		cairo_stroke( cr);

		if ( i8 % 8 == 0 ) {
			_p._p.CwB[SExpDesignUI::TColour::labels_sf].set_source_rgba( cr);
			cairo_move_to( cr, x + 5, da_ht-2);
			snprintf_buf( "%g", (float)i8/8);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}
	}

	size_t	run = pattern_size_essential();

      // snippet
	cairo_set_source_rgb( cr, 0.1, 0.1, 0.1);
	cairo_set_line_width( cr, .8);
	aghui::cairo_draw_signal( cr, pattern, 0, pattern.size(),
				  da_wd, 0, da_ht/3, display_scale);
	cairo_stroke( cr);

	// lines marking out context
	cairo_set_source_rgba( cr, 0.9, 0.9, 0.9, .5);
	cairo_set_line_width( cr, 1.);
	cairo_rectangle( cr, 0., 0., (float)context_before / pattern.size() * da_wd, da_ht);
	cairo_rectangle( cr, (float)(context_before + run) / pattern.size() * da_wd, 0,
			 (float)(context_after) / pattern.size() * da_wd, da_ht);
	cairo_fill( cr);
	cairo_stroke( cr);

	// thing if found
	if ( last_find != (size_t)-1 ) {
		cairo_set_source_rgba( cr, 0.1, 0.1, 0.1, .9);
		cairo_set_line_width( cr, .7);
		aghui::cairo_draw_signal( cr, field_channel->signal_filtered,
					  last_find - context_before, last_find + run + context_after,
					  da_wd, 0, da_ht*2/3, display_scale);
		cairo_stroke( cr);
	}

	if ( draw_details ) {
		valarray<TFloat>
			env_u, env_l,
			course,
			dzcdf;
	      // envelope
		{
			if ( sigproc::envelope( pattern, params.env_tightness, samplerate,
						1./samplerate,
						env_l, env_u) == 0 ) {
				aghui::cairo_put_banner( cr, da_wd, da_ht, "Selection is too short");
				enable_controls( false);
				goto out;
			}
			enable_controls( true);

			cairo_set_source_rgba( cr, 1, 1, 1, .5);
			aghui::cairo_draw_signal( cr, env_u, 0, env_u.size(),
						  da_wd, 0, da_ht/3, display_scale);
			aghui::cairo_draw_signal( cr, env_l, 0, env_l.size(),
						  da_wd, 0, da_ht/3, display_scale, 1, aghui::TDrawSignalDirection::Backward, true);
			cairo_close_path( cr);
			cairo_fill( cr);
			cairo_stroke( cr);
		}

	      // low-pass filter
		{
			course = exstrom::low_pass( pattern, samplerate,
						    params.bwf_cutoff, params.bwf_order, true);

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
			cairo_set_line_width( cr, 3.);
			aghui::cairo_draw_signal( cr, course, 0, course.size(),
						  da_wd, 0, da_ht/3, display_scale);
			cairo_stroke( cr);
		}

	      // dzcdf
		{
			if ( samplerate < 10 ) {
				aghui::cairo_put_banner( cr, da_wd, da_ht, "Samplerate is too low");
				enable_controls( false);
				goto out;
			}
			if ( params.dzcdf_step * 10 > pattern_length() ) { // require at least 10 dzcdf points
				aghui::cairo_put_banner( cr, da_wd, da_ht, "Selection is too short");
				enable_controls( false);
				goto out;
			}
			enable_controls( true);

			dzcdf = sigproc::dzcdf( pattern, samplerate,
						params.dzcdf_step, params.dzcdf_sigma, params.dzcdf_smooth);
			float	dzcdf_display_scale = da_ht/4. / dzcdf.max();

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
			cairo_set_line_width( cr, 1.);
			aghui::cairo_draw_signal( cr, dzcdf, 0, dzcdf.size(),
						  da_wd, 0, da_ht/2-5, dzcdf_display_scale);
			cairo_stroke( cr);
			cairo_set_line_width( cr, .5);
			cairo_rectangle( cr, 0, da_ht/2-5, da_wd, da_ht/2-4);
			cairo_stroke( cr);
		}
	}
out:
	;
}




void
aghui::SScoringFacility::SFindDialog::
load_pattern( SScoringFacility::SChannel& field)
{
	// double check, possibly redundant after due check in callback
	size_t	run = field.selection_end - field.selection_start;
	if ( run == 0 )
		return;

	field_channel = &field;
	context_before = (field.selection_start < context_pad)
		? context_pad - field.selection_start
		: context_pad;
	context_after  = (field.selection_end + context_pad > field.n_samples())
		? field.n_samples() - field.selection_end
		: context_pad;
	size_t	full_sample = context_before + run + context_after;

	pattern.resize( full_sample);
	pattern = field.signal_filtered[ slice (field.selection_start - context_before,
						full_sample, 1) ];
				// or _p.selection_*
	samplerate = field.samplerate();
	display_scale = field.signal_display_scale;

	set_pattern_da_width( full_sample / field.spp());

	last_find = (size_t)-1;

	preselect_channel( field.name);
	preselect_entry( NULL, 0);
	gtk_label_set_markup( _p.lPatternSimilarity, "");

	gtk_widget_queue_draw( (GtkWidget*)_p.daPatternSelection);
}




void
aghui::SScoringFacility::SFindDialog::
load_pattern( const char *label, bool do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns/%s", _p._p.ED->session_dir(), label);
	} else {
		string j_dir = _p._p.ED->subject_dir( _p.csubject());
		snprintf_buf( "%s/.patterns/%s", j_dir.c_str(), label);
	}

	FILE *fd = fopen( __buf__, "r");
	if ( fd ) {
		size_t	full_sample;
		if ( fscanf( fd,
			     "%u  %u %lg  %lg %lg %u  %lg %lg %lg\n"
			     "%zu %zu %zu %zu\n",
			     &params.env_tightness,
			     &params.bwf_order, &params.bwf_cutoff,
			     &params.dzcdf_step, &params.dzcdf_sigma, &params.dzcdf_smooth,
			     &tolerance_a, &tolerance_b, &tolerance_c,
			     &samplerate, &full_sample, &context_before, &context_after) == 13 ) {
			if ( samplerate != field_channel->samplerate() ) {
				snprintf_buf( "Loaded pattern has samplerate different from the current samplerate (%zu vs %zu)",
					      samplerate, field_channel->samplerate());
				pop_ok_message( (GtkWindow*)_p.wPattern, __buf__);
			}
			pattern.resize( full_sample);
			for ( size_t i = 0; i < full_sample; ++i ) {
				double tmp;
				if ( fscanf( fd, "%la", &tmp) != 1 ) {
					fprintf( stderr, "SScoringFacility::SFindDialog::load_pattern(): short read at sample %zu from %s\n", i, __buf__);
					pattern.resize( 0);
					break;
				} else
					pattern[i] = tmp;
			}

			display_scale = field_channel->signal_display_scale;
			W_V.up();

			set_pattern_da_width( full_sample / field_channel->spp());
		} else
			pattern.resize( 0);

		fclose( fd);
	}
}




void
aghui::SScoringFacility::SFindDialog::
save_pattern( const char *label, bool do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns", _p._p.ED->session_dir());
		if ( mkdir( __buf__, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
			fprintf( stderr, "SScoringFacility::SFindDialog::save_pattern(): mkdir('%s') failed\n", __buf__);
		snprintf_buf( "%s/.patterns/%s", _p._p.ED->session_dir(), label);
	} else {
		string j_dir = _p._p.ED->subject_dir( _p.csubject());
		snprintf_buf( "%s/.patterns", j_dir.c_str());
		if ( mkdir( __buf__, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
			fprintf( stderr, "SScoringFacility::SFindDialog::save_pattern(): mkdir('%s') failed\n", __buf__);
		snprintf_buf( "%s/.patterns/%s", j_dir.c_str(), label);
	}
	FILE *fd = fopen( __buf__, "w");
	if ( fd ) {
		fprintf( fd,
			 "%u  %u %g  %g %g %u  %g %g %g\n"
			 "%zu %zu %zu %zu\n",
			 params.env_tightness, params.bwf_order, params.bwf_cutoff,
			 params.dzcdf_step, params.dzcdf_sigma, params.dzcdf_smooth, tolerance_a, tolerance_b, tolerance_c,
			 samplerate, pattern.size(), context_before, context_after);
		for ( size_t i = 0; i < pattern.size(); ++i )
			fprintf( fd, "%a\n", (double)pattern[i]);
		fclose( fd);
	}
}



void
aghui::SScoringFacility::SFindDialog::
discard_pattern( const char *label, bool do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns/%s", _p._p.ED->session_dir(), label);
	} else {
		string j_dir = _p._p.ED->subject_dir( _p.csubject());
		snprintf_buf( "%s/.patterns/%s", j_dir.c_str(), label);
	}
	unlink( __buf__);
	enumerate_patterns_to_combo();
}



bool
aghui::SScoringFacility::SFindDialog::
search( ssize_t from)
{
	if ( field_channel && pattern.size() > 0 ) {
		if ( !(params == params_saved) ) {
			field_channel->compute_lowpass( params.bwf_cutoff, params.bwf_order);
			field_channel->compute_tightness( params.env_tightness);
			field_channel->compute_dzcdf( params.dzcdf_step, params.dzcdf_sigma, params.dzcdf_smooth);
			params_saved = params;
		}
		cpattern = new sigproc::CPattern<TFloat>
			(pattern, context_before, context_after,
			 field_channel->samplerate(),
			 params,
			 tolerance_a, tolerance_b, tolerance_c);
		last_find = cpattern->find(
			field_channel->signal_lowpass.data,
			valarray<TFloat>
				(field_channel->signal_envelope.upper
				 - field_channel->signal_envelope.lower),
			field_channel->signal_dzcdf.data,
			from, increment);
		match_a = cpattern->match_a;
		match_b = cpattern->match_b;
		match_c = cpattern->match_c;

		delete cpattern;
		cpattern = nullptr;
		return last_find != (size_t)-1;
	} else
		return false;
}




void
aghui::SScoringFacility::SFindDialog::
enable_controls( bool indeed)
{
	gtk_widget_set_sensitive( (GtkWidget*)_p.bPatternFindNext, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)_p.bPatternFindPrevious, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)_p.bPatternSave, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)_p.bPatternDiscard, (gboolean)indeed);
}






inline namespace {
	int
	scandir_filter( const struct dirent *e)
	{
		return strcmp( e->d_name, ".") && strcmp( e->d_name, "..");
	}
}

const char
	*aghui::SScoringFacility::SFindDialog::globally_marker = "[global] ";

void
aghui::SScoringFacility::SFindDialog::
enumerate_patterns_to_combo()
{
	g_signal_handler_block( _p.ePatternList, _p.ePatternList_changed_cb_handler_id);
	gtk_list_store_clear( _p.mPatterns);

	GtkTreeIter iter;

	struct dirent **eps;
	int n;
	snprintf_buf( "%s/.patterns", _p._p.ED->session_dir());
	n = scandir( __buf__, &eps, scandir_filter, alphasort);
//	printf( "n = %d in %s\n", n, __buf__);
	if ( n >= 0 ) {
		for ( int cnt = 0; cnt < n; ++cnt ) {
			snprintf_buf( "%s%s", globally_marker, eps[cnt]->d_name);
			gtk_list_store_append( _p.mPatterns, &iter);
			gtk_list_store_set( _p.mPatterns, &iter,
					    0, __buf__,
					    -1);
			free( eps[cnt]);
		}
		free( (void*)eps);
	}
	string j_dir = _p._p.ED->subject_dir( _p.csubject());
	snprintf_buf( "%s/.patterns", j_dir.c_str());
	n = scandir( __buf__, &eps, scandir_filter, alphasort);
//	printf( "n = %d in %s\n", n, __buf__);
	if ( n >= 0 ) {
		for ( int cnt = 0; cnt < n; ++cnt ) {
			gtk_list_store_append( _p.mPatterns, &iter);
			gtk_list_store_set( _p.mPatterns, &iter,
					    0, eps[cnt]->d_name,
					    -1);
			free( eps[cnt]);
		}
		free( (void*)eps);
	}
	gtk_combo_box_set_active_iter( _p.ePatternList, NULL);
	g_signal_handler_unblock( _p.ePatternList, _p.ePatternList_changed_cb_handler_id);
}



void
aghui::SScoringFacility::SFindDialog::
preselect_entry( const char *label, bool do_globally)
{
	if ( label == NULL ) {
		gtk_combo_box_set_active_iter( _p.ePatternList, NULL);
		return;
	}

	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( (GtkTreeModel*)_p.mPatterns, &iter);
	while ( valid ) {
		char *entry;
		gtk_tree_model_get( (GtkTreeModel*)_p.mPatterns, &iter,
				    0, &entry,
				    -1);
		if ( (!do_globally && strcmp( entry, label) == 0) ||
		     (do_globally && (strlen( entry) > strlen( globally_marker) && strcmp( entry+strlen(globally_marker), label) == 0)) ) {
			gtk_combo_box_set_active_iter( _p.ePatternList, &iter);
			free( entry);
			return;
		}
		free( entry);
		valid = gtk_tree_model_iter_next( (GtkTreeModel*)_p.mPatterns, &iter);
	}
}

void
aghui::SScoringFacility::SFindDialog::
preselect_channel( const char *ch)
{
	if ( ch == NULL ) {
		gtk_combo_box_set_active_iter( _p.ePatternChannel, NULL);
		return;
	}

	GtkTreeModel *model = gtk_combo_box_get_model( _p.ePatternChannel);
	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( model, &iter);
	while ( valid ) {
		DEF_UNIQUE_CHARP (entry);
		gtk_tree_model_get( model, &iter,
				    0, &entry,
				    -1);
		if ( strcmp( entry, ch) == 0 ) {
			gtk_combo_box_set_active_iter( _p.ePatternChannel, &iter);
			return;
		}
		valid = gtk_tree_model_iter_next( model, &iter);
	}
}



// eof
