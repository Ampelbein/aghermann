// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-patterns.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-14
 *
 *         Purpose:  scoring facility patterns
 *
 *         License:  GPL
 */

#include <tuple>
#include <dirent.h>
#include <sys/stat.h>

#include "ui/misc.hh"
#include "sf.hh"

using namespace std;


aghui::SScoringFacility::SFindDialog::
SFindDialog (SScoringFacility& parent)
      : Pp {2,  0., 1.5, 1,  .1, .5, 3},
	Pp2 (Pp),
	cpattern (nullptr),
	increment (.05),
	draw_details (true),
	_p (parent)
{
	W_V.reg( _p.ePatternEnvTightness, 	&Pp.env_tightness);
	W_V.reg( _p.ePatternBandPassOrder, 	&Pp.bwf_order);
	W_V.reg( _p.ePatternBandPassFrom, 	&Pp.bwf_ffrom);
	W_V.reg( _p.ePatternBandPassUpto, 	&Pp.bwf_fupto);
	W_V.reg( _p.ePatternDZCDFStep, 		&Pp.dzcdf_step);
	W_V.reg( _p.ePatternDZCDFSigma, 	&Pp.dzcdf_sigma);
	W_V.reg( _p.ePatternDZCDFSmooth, 	&Pp.dzcdf_smooth);

	W_V.reg( _p.ePatternParameterA, 	&get<0>(criteria));
	W_V.reg( _p.ePatternParameterB, 	&get<1>(criteria));
	W_V.reg( _p.ePatternParameterC, 	&get<2>(criteria));
	W_V.reg( _p.ePatternParameterD, 	&get<3>(criteria));

	gtk_widget_set_visible( (GtkWidget*)_p.daPatternField, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.bPatternSearch, TRUE);
}

aghui::SScoringFacility::SFindDialog::
~SFindDialog ()
{
	if ( cpattern )
		delete cpattern;
	// g_object_unref( mPatterns);
	gtk_widget_destroy( (GtkWidget*)_p.wPattern);
}






void
aghui::SScoringFacility::SFindDialog::
search()
{
	if ( unlikely (not field_channel or thing.size() == 0) )
		return;

	if ( !(Pp == Pp2) || field_channel != field_channel_saved) {
		Pp2 = Pp;
		field_channel_saved = field_channel;
	}
	cpattern = new pattern::CPattern<TFloat>
		({thing, field_channel->samplerate()},
		 context_before, context_after,
		 Pp);
	diff_line =
		(cpattern->do_search(
			field_channel->signal_envelope( Pp.env_tightness).first,
			field_channel->signal_envelope( Pp.env_tightness).second,
			field_channel->signal_bandpass( Pp.bwf_ffrom, Pp.bwf_fupto, Pp.bwf_order),
			field_channel->signal_dzcdf( Pp.dzcdf_step, Pp.dzcdf_sigma, Pp.dzcdf_smooth),
			increment * samplerate),
		 cpattern->diff);

	delete cpattern;
	cpattern = nullptr;

	set_field_da_width( _p.total_pages() * 3);
	field_display_scale =
		agh::alg::calibrate_display_scale(
			field_channel->psd.course, _p.total_pages(),
			da_field_ht);
	gtk_widget_set_visible( (GtkWidget*)_p.daPatternField, TRUE);
	gtk_widget_set_visible( (GtkWidget*)_p.bPatternSearch, FALSE);
}


size_t
aghui::SScoringFacility::SFindDialog::
find_occurrences()
{
	occurrences.resize(0);
	for ( size_t i = 0; i < diff_line.size(); ++i )
		if ( diff_line[i].good_enough( criteria) )
			occurrences.push_back(i);
	return occurrences.size();
}







void
aghui::SScoringFacility::SFindDialog::
set_pattern_da_width( int width)
{
	g_object_set( (GObject*)_p.daPatternSelection,
		      "width-request", da_thing_wd = width,
		      "height-request", da_thing_ht,
		      NULL);
	g_object_set( (GObject*)_p.vpPatternSelection,
		      "width-request", min( width+5, 600),
		      "height-request", da_thing_ht + 30,
		      NULL);
}

void
aghui::SScoringFacility::SFindDialog::
set_field_da_width( int width)
{
	g_object_set( (GObject*)_p.daPatternField,
		      "width-request", da_field_wd = width,
		      "height-request", da_field_ht,
		      NULL);
	g_object_set( (GObject*)_p.vpPatternField,
		      "width-request", min( width+5, 600),
		      "height-request", da_thing_ht + 30,
		      NULL);
}



void
aghui::SScoringFacility::SFindDialog::
draw_thing( cairo_t *cr)
{
	if ( thing.size() == 0 ) {
		set_pattern_da_width( 200);
		aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "(no selection)");
		enable_controls( false);
		return;
	} else {
		enable_controls( true);
	}

      // ticks
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 9);
	double	seconds = (double)thing.size() / samplerate;
	for ( size_t i8 = 0; (float)i8 / 8 < seconds; ++i8 ) {
		_p._p.CwB[SExpDesignUI::TColour::sf_ticks].set_source_rgba( cr);
		cairo_set_line_width( cr, (i8%8 == 0) ? 1. : (i8%4 == 0) ? .6 : .3);
		guint x = (float)i8/8 / seconds * da_thing_wd;
		cairo_move_to( cr, x, 0);
		cairo_rel_line_to( cr, 0, da_thing_ht);
		cairo_stroke( cr);

		if ( i8 % 8 == 0 ) {
			_p._p.CwB[SExpDesignUI::TColour::sf_labels].set_source_rgba( cr);
			cairo_move_to( cr, x + 5, da_thing_ht-2);
			snprintf_buf( "%g", (float)i8/8);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
		}
	}

	size_t	run = pattern_size_essential();

      // snippet
	cairo_set_source_rgb( cr, 0., 0., 0.);
	cairo_set_line_width( cr, .8);
	aghui::cairo_draw_signal( cr, thing, 0, thing.size(),
				  da_thing_wd, 0, da_thing_ht/3, thing_display_scale);
	cairo_stroke( cr);

	// lines marking out context
	cairo_set_source_rgba( cr, 0.9, 0.9, 0.9, .5);
	cairo_set_line_width( cr, 1.);
	cairo_rectangle( cr, 0., 0., (float)context_before / thing.size() * da_thing_wd, da_thing_ht);
	cairo_rectangle( cr, (float)(context_before + run) / thing.size() * da_thing_wd, 0,
			 (float)(context_after) / thing.size() * da_thing_wd, da_thing_ht);
	cairo_fill( cr);
	cairo_stroke( cr);

	if ( draw_details ) {
		valarray<TFloat>
			env_u, env_l,
			course,
			dzcdf;
	      // envelope
		{
			if ( sigproc::envelope( {thing, samplerate}, Pp.env_tightness,
						1./samplerate,
						&env_l, &env_u) == 0 ) {
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Selection is too short");
				enable_controls( false);
				goto out;
			}
			enable_controls( true);

			_p._p.CwB[SExpDesignUI::TColour::sf_selection].set_source_rgba_contrasting( cr, .3);
			aghui::cairo_draw_signal( cr, env_u, 0, env_u.size(),
						  da_thing_wd, 0, da_thing_ht/2, thing_display_scale);
			aghui::cairo_draw_signal( cr, env_l, 0, env_l.size(),
						  da_thing_wd, 0, da_thing_ht/2, thing_display_scale, 1,
						  aghui::TDrawSignalDirection::backward, true);
			cairo_close_path( cr);
			cairo_fill( cr);
			cairo_stroke( cr);
		}

	      // target frequency
		{
			if ( Pp.bwf_ffrom >= Pp.bwf_fupto ) {
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Bad band-pass range");
				enable_controls( false);
				goto out;
			}
			course = exstrom::band_pass(
				thing, samplerate,
				Pp.bwf_ffrom, Pp.bwf_fupto, Pp.bwf_order, true);

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
			cairo_set_line_width( cr, 3.);
			aghui::cairo_draw_signal( cr, course, 0, course.size(),
						  da_thing_wd, 0, da_thing_ht/3, thing_display_scale);
			cairo_stroke( cr);
		}

	      // dzcdf
		{
			if ( samplerate < 10 ) {
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Samplerate is too low");
				enable_controls( false);
				goto out;
			}
			if ( Pp.dzcdf_step * 10 > pattern_length() ) { // require at least 10 dzcdf points
				aghui::cairo_put_banner( cr, da_thing_wd, da_thing_ht, "Selection is too short");
				enable_controls( false);
				goto out;
			}
			enable_controls( true);

			dzcdf = sigproc::dzcdf( sigproc::SSignalRef<TFloat> {thing, samplerate},
						Pp.dzcdf_step, Pp.dzcdf_sigma, Pp.dzcdf_smooth);
			float	dzcdf_display_scale = da_thing_ht/4. / dzcdf.max();

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
			cairo_set_line_width( cr, 1.);
			aghui::cairo_draw_signal( cr, dzcdf, 0, dzcdf.size(),
						  da_thing_wd, 0, da_thing_ht/2-5, dzcdf_display_scale);
			cairo_stroke( cr);
		}
	}
out:
	;
}

void
aghui::SScoringFacility::SFindDialog::
draw_field( cairo_t *cr)
{
	aghui::cairo_draw_signal(
		cr,
		field_channel->psd.course, 0, field_channel->psd.course.size(),
		da_field_wd, 0., da_field_ht/2, field_display_scale);

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

	thing.resize( full_sample);
	thing = field.signal_filtered[ slice (field.selection_start - context_before,
						full_sample, 1) ];
				// or _p.selection_*
	samplerate = field.samplerate();
	thing_display_scale = field.signal_display_scale;

	set_pattern_da_width( full_sample / field.spp());

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
		snprintf_buf( "%s/.patterns/%s", _p._p.ED->session_dir().c_str(), label);
	} else {
		string j_dir = _p._p.ED->subject_dir( _p.csubject());
		snprintf_buf( "%s/.patterns/%s", j_dir.c_str(), label);
	}

	FILE *fd = fopen( __buf__, "r");
	if ( fd ) {
		size_t	full_sample;
		if ( fscanf( fd,
			     (sizeof(TFloat) == sizeof(float))
			     ?
			     "%u  %u %lg %lg  %lg %lg %u "
			     " %g %g %g %g\n"
			     "%zu %zu %zu %zu\n"
			     "--DATA--\n"
			     :
			     "%u  %u %lg %lg  %lg %lg %u "
			     " %lg %lg %lg %lg\n"
			     "%zu %zu %zu %zu\n"
			     "--DATA--\n"
			     ,
			     &Pp.env_tightness,
			     &Pp.bwf_order, &Pp.bwf_ffrom, &Pp.bwf_fupto,
			     &Pp.dzcdf_step, &Pp.dzcdf_sigma, &Pp.dzcdf_smooth,
			     &get<0>(criteria), &get<1>(criteria), &get<2>(criteria), &get<3>(criteria),
			     &samplerate, &full_sample, &context_before, &context_after) == 14 ) {

			thing.resize( full_sample);
			for ( size_t i = 0; i < full_sample; ++i ) {
				double d;
				if ( fscanf( fd, "%la", &d) != 1 ) {
					fprintf( stderr, "load_pattern(): short read at sample %zu from %s; "
						 "Removing file\n", i, __buf__);
					thing.resize( 0);
					fclose( fd);
					unlink( __buf__);
					enumerate_patterns_to_combo();
					return;
				} else
					thing[i] = d;
			}

			if ( samplerate != field_channel->samplerate() ) {
				printf( "Loaded pattern has samplerate different from the current samplerate (%zu vs %zu); it will be resampled now.",
					samplerate, field_channel->samplerate());
				double fac = (double)field_channel->samplerate() / samplerate;
				thing =
					sigproc::resample( thing, 0, full_sample,
							   fac * full_sample);
				context_before *= fac;
				context_after  *= fac;
			}

			thing_display_scale = field_channel->signal_display_scale;
			W_V.up();

			set_pattern_da_width( full_sample / field_channel->spp());

		} else {
			thing.resize( 0);
			fprintf( stderr, "load_pattern(): corrupted %s; "
				 "Removing file\n", __buf__);
			thing.resize( 0);
			unlink( __buf__);
			enumerate_patterns_to_combo();
		}

		fclose( fd);

	} else {
		fprintf( stderr, "load_pattern(): failed to open %s; "
			 "Removing file\n", __buf__);
		unlink( __buf__);
	}
}




void
aghui::SScoringFacility::SFindDialog::
save_pattern( const char *label, bool do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns", _p._p.ED->session_dir().c_str());
		if ( mkdir( __buf__, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
			fprintf( stderr, "SScoringFacility::SFindDialog::save_pattern(): mkdir('%s') failed\n", __buf__);
		snprintf_buf( "%s/.patterns/%s", _p._p.ED->session_dir().c_str(), label);
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
			 "%u  %u %g %g  %g %g %u  %g %g %g %g\n"
			 "%zu  %zu %zu %zu\n"
			 "--DATA--\n",
			 Pp.env_tightness, Pp.bwf_order, Pp.bwf_ffrom, Pp.bwf_fupto,
			 Pp.dzcdf_step, Pp.dzcdf_sigma, Pp.dzcdf_smooth,
			 get<0>(criteria), get<1>(criteria), get<2>(criteria), get<3>(criteria),
			 samplerate, thing.size(), context_before, context_after);
		for ( size_t i = 0; i < thing.size(); ++i )
			fprintf( fd, "%a\n", (double)thing[i]);
		fclose( fd);
	}
}



void
aghui::SScoringFacility::SFindDialog::
discard_pattern( const char *label, bool do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns/%s", _p._p.ED->session_dir().c_str(), label);
	} else {
		string j_dir = _p._p.ED->subject_dir( _p.csubject());
		snprintf_buf( "%s/.patterns/%s", j_dir.c_str(), label);
	}
	unlink( __buf__);
	enumerate_patterns_to_combo();
}





void
aghui::SScoringFacility::SFindDialog::
enable_controls( bool indeed)
{
	gtk_widget_set_sensitive( (GtkWidget*)_p.bPatternGotoNext, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)_p.bPatternGotoPrevious, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)_p.bPatternSave, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)_p.bPatternDiscard, (gboolean)indeed);
}






inline namespace {
int
scandir_filter( const struct dirent *e)
{
	return strcmp( e->d_name, ".") && strcmp( e->d_name, "..");
}
const char
	*globally_marker = "[global] ";
}


void
aghui::SScoringFacility::SFindDialog::
enumerate_patterns_to_combo()
{
	g_signal_handler_block( _p.ePatternList, _p.ePatternList_changed_cb_handler_id);
	gtk_list_store_clear( _p.mPatterns);

	GtkTreeIter iter;

	struct dirent **eps;
	int n;
	snprintf_buf( "%s/.patterns", _p._p.ED->session_dir().c_str());
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
