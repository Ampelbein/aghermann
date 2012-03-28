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
#include "draw-signal-generic.hh"
#include "scoring-facility.hh"

using namespace std;


aghui::SScoringFacility::SFindDialog::SFindDialog( SScoringFacility& parent)
      : bwf_order (2),
	bwf_cutoff (1.5),
	bwf_scale (true),
	dzcdf_step (.1),
	dzcdf_sigma (.5),
	dzcdf_smooth (3),
	env_tightness (2),
	tolerance_a (.1),
	tolerance_b (.1),
	tolerance_c (.1),
	cpattern (NULL),
	last_find ((size_t)-1),
	increment (3),
	draw_details (true),
	_p (parent)
{
}

aghui::SScoringFacility::SFindDialog::~SFindDialog()
{
	g_object_unref( mPatterns);
	gtk_widget_destroy( (GtkWidget*)wPattern);
}




int
aghui::SScoringFacility::SFindDialog::construct_widgets()
{
	mPatterns =
		gtk_list_store_new( 1, G_TYPE_STRING);

	 GtkCellRenderer *renderer;

	if ( !AGH_GBGETOBJ3 (_p.builder, GtkDialog,		wPattern) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkDrawingArea,	daPatternSelection) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkScrolledWindow,	vpPatternSelection) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternFindPrevious) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternFindNext) ||
//	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternDismiss) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternSave) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkButton,		bPatternDiscard) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternEnvTightness) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternFilterOrder) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternFilterCutoff) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternDZCDFStep) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternDZCDFSigma) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternDZCDFSmooth) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternParameterA) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternParameterB) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkSpinButton,		ePatternParameterC) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkHBox,		cPatternLabelBox) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkLabel,		lPatternSimilarity) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkComboBox,		ePatternList) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkComboBox,		ePatternChannel) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkDialog,		wPatternName) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkEntry,		ePatternNameName) ||
	     !AGH_GBGETOBJ3 (_p.builder, GtkCheckButton,	ePatternNameSaveGlobally) )
		return -1;

	gtk_combo_box_set_model( ePatternList,
				 (GtkTreeModel*)mPatterns);
	gtk_combo_box_set_id_column( ePatternList, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)ePatternList, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)ePatternList, renderer,
					"text", 0,
					NULL);
	ePatternList_changed_cb_handler_id =
		g_signal_connect( ePatternList, "changed",
				  G_CALLBACK (ePatternList_changed_cb),
				  this);

	gtk_combo_box_set_model( ePatternChannel,
				 (GtkTreeModel*)_p._p.mAllChannels);
	gtk_combo_box_set_id_column( ePatternChannel, 0);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)ePatternChannel, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)ePatternChannel, renderer,
					"text", 0,
					NULL);

	ePatternChannel_changed_cb_handler_id =
		g_signal_connect( ePatternChannel, "changed",
				  G_CALLBACK (ePatternChannel_changed_cb),
				  this);

	g_signal_connect( daPatternSelection, "draw",
			  G_CALLBACK (daPatternSelection_draw_cb),
			  this);
	g_signal_connect( daPatternSelection, "scroll-event",
			  G_CALLBACK (daPatternSelection_scroll_event_cb),
			  this);
	g_signal_connect( bPatternFindNext, "clicked",
			  G_CALLBACK (bPatternFind_clicked_cb),
			  this);
	g_signal_connect( bPatternFindPrevious, "clicked",
			  G_CALLBACK (bPatternFind_clicked_cb),
			  this);
	g_signal_connect( bPatternSave, "clicked",
			  G_CALLBACK (bPatternSave_clicked_cb),
			  this);
	g_signal_connect( bPatternDiscard, "clicked",
			  G_CALLBACK (bPatternDiscard_clicked_cb),
			  this);

	g_signal_connect( ePatternEnvTightness, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternFilterCutoff, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternFilterOrder, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternDZCDFStep, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternDZCDFSigma, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternDZCDFSmooth, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternParameterA, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternParameterB, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);
	g_signal_connect( ePatternParameterC, "value-changed",
			  G_CALLBACK (ePattern_any_value_changed_cb),
			  this);

	g_signal_connect( wPattern, "show",
			  G_CALLBACK (wPattern_show_cb),
			  this);
	g_signal_connect( wPattern, "hide",
			  G_CALLBACK (wPattern_hide_cb),
			  this);
	return 0;
}





void
aghui::SScoringFacility::SFindDialog::set_pattern_da_width( int width)
{
	g_object_set( (GObject*)daPatternSelection,
		      "width-request", da_wd = width,
		      "height-request", da_ht,
		      NULL);
	g_object_set( (GObject*)vpPatternSelection,
		      "width-request", min( width, 600),
		      "height-request", da_ht + 20,
		      NULL);
}


inline namespace {
	void
	center_message( const char *msg, cairo_t *cr, int wd, int ht)
	{
		cairo_set_font_size( cr, 18);
		cairo_set_source_rgba( cr, 1., 1., 1., .4);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_text_extents_t extents;
		cairo_text_extents( cr, msg, &extents);
		cairo_move_to( cr, (wd - extents.width)/2, (ht - extents.height)/2);
		cairo_show_text( cr, msg);
		cairo_stroke( cr);
	}
}

void
aghui::SScoringFacility::SFindDialog::draw( cairo_t *cr)
{
	if ( pattern.size() == 0 ) {
		set_pattern_da_width( 200);
		center_message( "(no selection)", cr, da_wd, da_ht);
		enable_controls( false);
		return;
	} else {
		enable_controls( true);
	}

      // ticks
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size( cr, 9);
	float	seconds = (float)pattern.size() / samplerate;
	for ( size_t i8 = 0; (float)i8 / 8 < seconds; ++i8 ) {
		_p._p.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgba( cr, .4);
		cairo_set_line_width( cr, (i8%8 == 0) ? 1. : (i8%4 == 0) ? .6 : .3);
		guint x = (float)i8/8 / seconds * da_wd;
		cairo_move_to( cr, x, 0);
		cairo_rel_line_to( cr, 0, da_ht);
		cairo_stroke( cr);

		if ( i8 % 8 == 0 ) {
			_p._p.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgba( cr, .9);
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
	::draw_signal( pattern, 0, pattern.size(),
		       da_wd, da_ht/3, display_scale, cr, false);
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
		::draw_signal( field_channel->signal_filtered,
			       last_find - context_before, last_find + run + context_after,
			       da_wd, da_ht*2/3, display_scale, cr, false);
		cairo_stroke( cr);
	}

	if ( draw_details ) {
	      // envelope
		valarray<TFloat>
			env_u, env_l,
			course,
			dzcdf;

		{
			if ( sigproc::envelope( pattern, env_tightness, samplerate,
						1./samplerate,
						env_l, env_u) == 0 ) {
				center_message( "Selection is too short", cr, da_wd, da_ht);
				enable_controls( false);
				goto out;
			} else {
				enable_controls( true);
			}

			cairo_set_source_rgba( cr, 0.3, 0.2, 0.8, .5);
			cairo_set_line_width( cr, .5);
			::draw_signal( env_u, 0, env_u.size(),
				       da_wd, da_ht/3, display_scale, cr, false);
			::draw_signal( env_l, 0, env_l.size(),
				       da_wd, da_ht/3, display_scale, cr, false);
			cairo_stroke( cr);
		}

	      // low-pass filter
		{
			course = exstrom::low_pass( pattern, samplerate,
						    bwf_cutoff, bwf_order, true);

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
			cairo_set_line_width( cr, 3.);
			::draw_signal( course, 0, course.size(),
				       da_wd, da_ht/3, display_scale, cr, false);
			cairo_stroke( cr);
		}

	      // dzcdf
		{
			if ( dzcdf_step * 6 > pattern_length() ) {
				center_message( "Selection is too short", cr, da_wd, da_ht);
				enable_controls( false);
				goto out;
			}
			dzcdf = sigproc::dzcdf( pattern, samplerate,
						dzcdf_step, dzcdf_sigma, dzcdf_smooth);
			float	dzcdf_display_scale = da_ht/4. / dzcdf.max();

			cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
			cairo_set_line_width( cr, 1.);
			::draw_signal( dzcdf, 0, dzcdf.size(),
				       da_wd, da_ht/2-5, dzcdf_display_scale, cr, false);
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
aghui::SScoringFacility::SFindDialog::load_pattern( SScoringFacility::SChannel& field)
{
	// double check, possibly redundant after due check in callback
	size_t	run = field.selection_size();
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
	printf( "bwf %f %u; tightness %d\n", bwf_cutoff, bwf_order, env_tightness);

	// printf( "%s (%zu): selection_start, end, padbefore, padafter: %zu, %zu, %zu, %zu\nfull %zu\n",
	// 	field.name, samplerate, field.selection_start, field.selection_end, context_before, context_after, full_sample);
	set_pattern_da_width( full_sample / field.spp());

	last_find = (size_t)-1;

	preselect_channel( field.name);
	preselect_entry( NULL, 0);
	gtk_label_set_markup( lPatternSimilarity, "");

	gtk_widget_queue_draw( (GtkWidget*)daPatternSelection);
}




void
aghui::SScoringFacility::SFindDialog::load_pattern( const char *label, bool do_globally)
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
			     "%u  %u %g  %g %g %u  %g %g %g\n"
			     "%zu %zu %zu %zu\n",
			     &env_tightness,
			     &bwf_order, &bwf_cutoff,
			     &dzcdf_step, &dzcdf_sigma, &dzcdf_smooth,
			     &tolerance_a, &tolerance_b, &tolerance_c,
			     &samplerate, &full_sample, &context_before, &context_after) == 13 ) {
			if ( samplerate != field_channel->samplerate() ) {
				snprintf_buf( "Loaded pattern has samplerate different from the current samplerate (%zu vs %zu)",
					      samplerate, field_channel->samplerate());
				pop_ok_message( (GtkWindow*)wPattern, __buf__);
			}
			pattern.resize( full_sample);
//			printf( "full_sample %zu; display_scale %f\n", full_sample, display_scale);
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
			update_displayed_parameters();

			set_pattern_da_width( full_sample / field_channel->spp());
		} else
			pattern.resize( 0);

		fclose( fd);
	}
}




void
aghui::SScoringFacility::SFindDialog::save_pattern( const char *label, bool do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns", _p._p.ED->session_dir());
		if ( mkdir( __buf__, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
			;
		snprintf_buf( "%s/.patterns/%s", _p._p.ED->session_dir(), label);
	} else {
		string j_dir = _p._p.ED->subject_dir( _p.csubject());
		snprintf_buf( "%s/.patterns", j_dir.c_str());
		if ( mkdir( __buf__, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
			;
		snprintf_buf( "%s/.patterns/%s", j_dir.c_str(), label);
	}
	FILE *fd = fopen( __buf__, "w");
	if ( fd ) {
		//acquire_parameters();
		fprintf( fd,
			 "%u  %u %g  %g %g %u  %g %g %g\n"
			 "%zu %zu %zu %zu\n",
			 env_tightness, bwf_order, bwf_cutoff,
			 dzcdf_step, dzcdf_sigma, dzcdf_smooth, tolerance_a, tolerance_b, tolerance_c,
			 samplerate, pattern.size(), context_before, context_after);
		for ( size_t i = 0; i < pattern.size(); ++i )
			fprintf( fd, "%a\n", (double)pattern[i]);
		fclose( fd);
	}
}



void
aghui::SScoringFacility::SFindDialog::discard_pattern( const char *label, bool do_globally)
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
aghui::SScoringFacility::SFindDialog::search( ssize_t from)
{
	if ( field_channel && pattern.size() > 0 ) {
		field_channel->compute_lowpass( bwf_cutoff, bwf_order);
		field_channel->compute_tightness( env_tightness);
		field_channel->compute_dzcdf( dzcdf_step, dzcdf_sigma, dzcdf_smooth);
		cpattern = new sigproc::CPattern<TFloat>
			(pattern, context_before, context_after,
			 field_channel->samplerate(),
			 bwf_order, bwf_cutoff, bwf_scale,
			 env_tightness,
			 dzcdf_step, dzcdf_sigma, dzcdf_smooth,
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
		cpattern = NULL;
		return last_find != (size_t)-1;
	} else
		return false;
}



void
aghui::SScoringFacility::SFindDialog::acquire_parameters()
{
	env_tightness = gtk_spin_button_get_value( ePatternEnvTightness);
	bwf_order     = gtk_spin_button_get_value( ePatternFilterOrder);
	bwf_cutoff    = gtk_spin_button_get_value( ePatternFilterCutoff);
	dzcdf_step    = gtk_spin_button_get_value( ePatternDZCDFStep);
	dzcdf_sigma   = gtk_spin_button_get_value( ePatternDZCDFSigma);
	dzcdf_smooth  = gtk_spin_button_get_value( ePatternDZCDFSmooth);
	tolerance_a   = gtk_spin_button_get_value( ePatternParameterA);
	tolerance_b   = gtk_spin_button_get_value( ePatternParameterB);
	tolerance_c   = gtk_spin_button_get_value( ePatternParameterC);

	// field_channel is set immediately in the ePatternChannel_changed_cb()
}

void
aghui::SScoringFacility::SFindDialog::update_displayed_parameters()
{
	gtk_spin_button_set_value( ePatternEnvTightness, env_tightness);
	gtk_spin_button_set_value( ePatternFilterCutoff, bwf_cutoff   );
	gtk_spin_button_set_value( ePatternFilterOrder,  bwf_order    );
	gtk_spin_button_set_value( ePatternDZCDFStep,    dzcdf_step   );
	gtk_spin_button_set_value( ePatternDZCDFSigma,   dzcdf_sigma  );
	gtk_spin_button_set_value( ePatternDZCDFSmooth,  dzcdf_smooth );
	gtk_spin_button_set_value( ePatternParameterA,	 tolerance_a  );
	gtk_spin_button_set_value( ePatternParameterB,	 tolerance_b  );
	gtk_spin_button_set_value( ePatternParameterC,	 tolerance_c  );
}

void
aghui::SScoringFacility::SFindDialog::enable_controls( bool indeed)
{
	gtk_widget_set_sensitive( (GtkWidget*)bPatternFindNext, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)bPatternFindPrevious, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)bPatternSave, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)bPatternDiscard, (gboolean)indeed);
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
aghui::SScoringFacility::SFindDialog::enumerate_patterns_to_combo()
{
	g_signal_handler_block( ePatternList, ePatternList_changed_cb_handler_id);
	gtk_list_store_clear( mPatterns);

	GtkTreeIter iter;

	struct dirent **eps;
	int n;
	snprintf_buf( "%s/.patterns", _p._p.ED->session_dir());
	n = scandir( __buf__, &eps, scandir_filter, alphasort);
//	printf( "n = %d in %s\n", n, __buf__);
	if ( n >= 0 ) {
		for ( int cnt = 0; cnt < n; ++cnt ) {
			snprintf_buf( "%s%s", globally_marker, eps[cnt]->d_name);
			gtk_list_store_append( mPatterns, &iter);
			gtk_list_store_set( mPatterns, &iter,
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
			gtk_list_store_append( mPatterns, &iter);
			gtk_list_store_set( mPatterns, &iter,
					    0, eps[cnt]->d_name,
					    -1);
			free( eps[cnt]);
		}
		free( (void*)eps);
	}
	gtk_combo_box_set_active_iter( ePatternList, NULL);
	g_signal_handler_unblock( ePatternList, ePatternList_changed_cb_handler_id);
}



void
aghui::SScoringFacility::SFindDialog::preselect_entry( const char *label, bool do_globally)
{
	if ( label == NULL ) {
		gtk_combo_box_set_active_iter( ePatternList, NULL);
		return;
	}

	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( (GtkTreeModel*)mPatterns, &iter);
	while ( valid ) {
		char *entry;
		gtk_tree_model_get( (GtkTreeModel*)mPatterns, &iter,
				    0, &entry,
				    -1);
		if ( (!do_globally && strcmp( entry, label) == 0) ||
		     (do_globally && (strlen( entry) > strlen( globally_marker) && strcmp( entry+strlen(globally_marker), label) == 0)) ) {
			gtk_combo_box_set_active_iter( ePatternList, &iter);
			free( entry);
			return;
		}
		free( entry);
		valid = gtk_tree_model_iter_next( (GtkTreeModel*)mPatterns, &iter);
	}
}

void
aghui::SScoringFacility::SFindDialog::preselect_channel( const char *ch)
{
	if ( ch == NULL ) {
		gtk_combo_box_set_active_iter( ePatternChannel, NULL);
		return;
	}

	GtkTreeModel *model = gtk_combo_box_get_model( ePatternChannel);
	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( model, &iter);
	while ( valid ) {
		DEF_UNIQUE_CHARP (entry);
		gtk_tree_model_get( model, &iter,
				    0, &entry,
				    -1);
		if ( strcmp( entry, ch) == 0 ) {
			gtk_combo_box_set_active_iter( ePatternChannel, &iter);
			return;
		}
		valid = gtk_tree_model_iter_next( model, &iter);
	}
}






// EOF
