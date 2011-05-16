// ;-*-C++-*- *  Time-stamp: "2011-05-16 01:41:33 hmmr"
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {
namespace sf {


SScoringFacility::SFindDialog::SFindDialog( SScoringFacility& parent)
      : bwf_order (2),
	bwf_cutoff (1.5),
	bwf_scale (true),
	dzcdf_step (.1),
	dzcdf_sigma (.5),
	dzcdf_smooth (3),
	env_tightness (2),
	a (.1), b (.1), c (.1),
	cpattern (NULL),
	last_find ((size_t)-1),
	increment (3),
	field_channel (&parent.channels.front()),
	draw_details (false),
	_parent (parent)
{
	if ( construct_widgets() )
		throw runtime_error( "SScoringFacility::SFindDialog(): Failed to construct own wisgets");;

	gtk_spin_button_set_value( ePatternEnvTightness, env_tightness);
	gtk_spin_button_set_value( ePatternFilterCutoff, bwf_cutoff);
	gtk_spin_button_set_value( ePatternFilterOrder,  bwf_order);
	gtk_spin_button_set_value( ePatternDZCDFStep,    dzcdf_step);
	gtk_spin_button_set_value( ePatternDZCDFSigma,   dzcdf_sigma);
	gtk_spin_button_set_value( ePatternDZCDFSmooth,  dzcdf_smooth);

	gtk_spin_button_set_value( ePatternParameterA,	a);
	gtk_spin_button_set_value( ePatternParameterB,	b);
	gtk_spin_button_set_value( ePatternParameterC,	c);
}

SScoringFacility::SFindDialog::~SFindDialog()
{
	gtk_widget_destroy( (GtkWidget*)wPattern);
}




int
SScoringFacility::SFindDialog::construct_widgets()
{
	 GtkCellRenderer *renderer;

	if ( !AGH_GBGETOBJ (GtkDialog, wPattern) ||
	     !AGH_GBGETOBJ (GtkDrawingArea, daPatternSelection) ||
	     !AGH_GBGETOBJ (GtkButton, bPatternFindPrevious) ||
	     !AGH_GBGETOBJ (GtkButton, bPatternFindNext) ||
	     !AGH_GBGETOBJ (GtkButton, bPatternSave) ||
	     !AGH_GBGETOBJ (GtkButton, bPatternDiscard) ||
	     !AGH_GBGETOBJ (GtkSpinButton, ePatternEnvTightness) ||
	     !AGH_GBGETOBJ (GtkSpinButton, ePatternFilterOrder) ||
	     !AGH_GBGETOBJ (GtkSpinButton, ePatternFilterCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton, ePatternDZCDFStep) ||
	     !AGH_GBGETOBJ (GtkSpinButton, ePatternDZCDFSigma) ||
	     !AGH_GBGETOBJ (GtkSpinButton, ePatternDZCDFSmooth) ||
	     !AGH_GBGETOBJ (GtkSpinButton, ePatternParameterA) ||
	     !AGH_GBGETOBJ (GtkSpinButton, ePatternParameterB) ||
	     !AGH_GBGETOBJ (GtkSpinButton, ePatternParameterC) ||
	     !AGH_GBGETOBJ (GtkHBox, cPatternLabelBox) ||
	     !AGH_GBGETOBJ (GtkLabel, lPatternSimilarity) ||
	     !AGH_GBGETOBJ (GtkComboBox, ePatternList) ||
	     !AGH_GBGETOBJ (GtkComboBox, ePatternChannel) ||
	     !AGH_GBGETOBJ (GtkDialog, wPatternName) ||
	     !AGH_GBGETOBJ (GtkEntry, ePatternNameName) ||
	     !AGH_GBGETOBJ (GtkCheckButton, ePatternNameSaveGlobally) )
		return -1;

	gtk_combo_box_set_model( ePatternList,
				 (GtkTreeModel*)patterns::mPatterns);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)ePatternList, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)ePatternList, renderer,
					"text", 0,
					NULL);
	ePatternList_changed_cb_handler_id =
		g_signal_connect_after( ePatternList, "changed",
					G_CALLBACK (ePatternList_changed_cb),
					(gpointer)this);

	gtk_combo_box_set_model( ePatternChannel,
				 (GtkTreeModel*)mAllChannels);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)ePatternChannel, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)ePatternChannel, renderer,
					"text", 0,
					NULL);

	ePatternChannel_changed_cb_handler_id =
		g_signal_connect_after( ePatternChannel, "changed",
					G_CALLBACK (ePatternChannel_changed_cb),
					(gpointer)this);

	g_signal_connect_after( daPatternSelection, "expose-event",
				G_CALLBACK (daPatternSelection_expose_event_cb),
				(gpointer)this);
	g_signal_connect_after( daPatternSelection, "scroll-event",
				G_CALLBACK (daPatternSelection_scroll_event_cb),
				(gpointer)this);
	g_signal_connect_after( bPatternFindNext, "clicked",
				G_CALLBACK (bPatternFind_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bPatternFindPrevious, "clicked",
				G_CALLBACK (bPatternFind_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bPatternSave, "clicked",
				G_CALLBACK (bPatternSave_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( bPatternDiscard, "clicked",
				G_CALLBACK (bPatternDiscard_clicked_cb),
				(gpointer)this);
	g_signal_connect_after( wPattern, "show",
				G_CALLBACK (wPattern_show_cb),
				(gpointer)this);
	g_signal_connect_after( wPattern, "hide",
				G_CALLBACK (wPattern_hide_cb),
				(gpointer)this);
	return 0;
}






static int
scandir_filter( const struct dirent *e)
{
	return strcmp( e->d_name, ".") && strcmp( e->d_name, "..");
}

#define GLOBALLY_MARKER "[global] "

void
SScoringFacility::SFindDialog::enumerate_patterns_to_combo()
{
	g_signal_handler_block( ePatternList, ePatternList_changed_cb_handler_id);
	gtk_list_store_clear( patterns::mPatterns);

	GtkTreeIter iter;

	struct dirent **eps;
	int n;
	snprintf_buf( "%s/.patterns", AghCC->session_dir());
	n = scandir( __buf__, &eps, scandir_filter, alphasort);
//	printf( "n = %d in %s\n", n, __buf__);
	if ( n >= 0 ) {
		for ( int cnt = 0; cnt < n; ++cnt ) {
			snprintf_buf( "%s%s", GLOBALLY_MARKER, eps[cnt]->d_name);
			gtk_list_store_append( patterns::mPatterns, &iter);
			gtk_list_store_set( patterns::mPatterns, &iter,
					    0, __buf__,
					    -1);
			free( eps[cnt]);
		}
		free( (void*)eps);
	}
	string j_dir = AghCC->subject_dir( _parent.csubject());
	snprintf_buf( "%s/.patterns", j_dir.c_str());
	n = scandir( __buf__, &eps, scandir_filter, alphasort);
//	printf( "n = %d in %s\n", n, __buf__);
	if ( n >= 0 ) {
		for ( int cnt = 0; cnt < n; ++cnt ) {
			gtk_list_store_append( patterns::mPatterns, &iter);
			gtk_list_store_set( patterns::mPatterns, &iter,
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
SScoringFacility::SFindDialog::preselect_entry( const char *label, bool do_globally)
{
	if ( label == NULL ) {
		gtk_combo_box_set_active_iter( ePatternList, NULL);
		return;
	}

	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( (GtkTreeModel*)patterns::mPatterns, &iter);
	while ( valid ) {
		char *entry;
		gtk_tree_model_get( (GtkTreeModel*)patterns::mPatterns, &iter,
				    0, &entry,
				    -1);
		if ( (!do_globally && strcmp( entry, label) == 0) ||
		     (do_globally && (strlen( entry) > strlen( GLOBALLY_MARKER) && strcmp( entry+strlen(GLOBALLY_MARKER), label) == 0)) ) {
			gtk_combo_box_set_active_iter( ePatternList, &iter);
			free( entry);
			return;
		}
		free( entry);
		valid = gtk_tree_model_iter_next( (GtkTreeModel*)patterns::mPatterns, &iter);
	}
}

void
SScoringFacility::SFindDialog::preselect_channel( const char *ch)
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
		char *entry;
		gtk_tree_model_get( model, &iter,
				    0, &entry,
				    -1);
		if ( strcmp( entry, ch) == 0 ) {
			gtk_combo_box_set_active_iter( ePatternChannel, &iter);
			free( entry);
			return;
		}
		free( entry);
		valid = gtk_tree_model_iter_next( model, &iter);
	}
}





void
SScoringFacility::SFindDialog::load_pattern( SScoringFacility::SChannel& field)
{
	// double check, possibly redundant after due check in callback
	size_t	run = _parent.selection_size();
	if ( run == 0 )
		return;
	size_t	full_sample = context_before + run + context_after;

	field_channel = &field;
	context_before = (_parent.selection_start < context_pad)
		? context_pad - _parent.selection_start
		: context_pad;
	context_after  = (_parent.selection_end + context_pad > field.n_samples())
		? field.n_samples() - _parent.selection_end
		: context_pad;
	pattern = field.signal_filtered[ slice (field.sf.selection_start - context_before,
						full_sample, 1) ];
				// or _parent.selection_*
	samplerate = field.samplerate();
	display_scale = field.signal_display_scale;

      // prepare the 3 feature tracks at channel
	field.compute_lowpass( bwf_cutoff, bwf_order);
	field.compute_tightness( env_tightness);
	field.compute_dzcdf( dzcdf_step, dzcdf_sigma, dzcdf_smooth);

	preselect_channel( field.name);

	g_object_set( (GObject*)daPatternSelection,
		      "width-request", (int)full_sample,
		      NULL);

	last_find = (size_t)-1;

	preselect_entry( NULL, 0);
	gtk_label_set_markup( lPatternSimilarity, "");

	gtk_widget_show_all( (GtkWidget*)wPattern);
}




void
SScoringFacility::SFindDialog::load_pattern( const char *label, bool do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns/%s", AghCC->session_dir(), label);
	} else {
		string j_dir = AghCC->subject_dir( _parent.csubject());
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
			     &a, &b, &c,
			     &samplerate, &full_sample, &context_before, &context_after) == 13 ) {
			if ( samplerate != field_channel->samplerate() ) {
				snprintf_buf( "Loaded pattern has samplerate different from the current samplerate (%zu vs %zu)",
					      samplerate, field_channel->samplerate());
				pop_ok_message( (GtkWindow*)wPattern, __buf__);
			}
			field_channel->compute_lowpass( bwf_cutoff, bwf_order);
			field_channel->compute_tightness( env_tightness);
			field_channel->compute_dzcdf( dzcdf_step, dzcdf_sigma, dzcdf_smooth);
			pattern.resize( full_sample);
			for ( size_t i = 0; i < full_sample; ++i )
				if ( fscanf( fd, "%a", &pattern[i]) != 1 ) {
					fprintf( stderr, "SScoringFacility::SFindDialog::load_pattern(): short read at sample %zu from %s\n", i, __buf__);
					pattern.resize( 0);
					break;
				}
			update_displayed_parameters();
		} else
			pattern.resize( 0);

		fclose( fd);
	}
}




void
SScoringFacility::SFindDialog::save_pattern( const char *label, bool do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns", AghCC->session_dir());
		if ( mkdir( __buf__, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
			;
		snprintf_buf( "%s/.patterns/%s", AghCC->session_dir(), label);
	} else {
		string j_dir = AghCC->subject_dir( _parent.csubject());
		snprintf_buf( "%s/.patterns", j_dir.c_str());
		if ( mkdir( __buf__, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
			;
		snprintf_buf( "%s/.patterns/%s", j_dir.c_str(), label);
	}
	FILE *fd = fopen( __buf__, "w");
	if ( fd ) {
		acquire_parameters();
		fprintf( fd,
			 "%u  %u %g  %g %g %u  %g %g %g\n"
			 "%zu %zu %zu %zu\n",
			 env_tightness, bwf_order, bwf_cutoff,
			 dzcdf_step, dzcdf_sigma, dzcdf_smooth, a, b, c,
			 samplerate, pattern.size(), context_before, context_after);
		for ( size_t i = 0; i < pattern.size(); ++i )
			fprintf( fd, "%a\n", pattern[i]);
		fclose( fd);
	}
}



void
SScoringFacility::SFindDialog::discard_pattern( const char *label, bool do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns/%s", AghCC->session_dir(), label);
	} else {
		string j_dir = AghCC->subject_dir( _parent.csubject());
		snprintf_buf( "%s/.patterns/%s", j_dir.c_str(), label);
	}
	unlink( __buf__);
	enumerate_patterns_to_combo();
}



bool
SScoringFacility::SFindDialog::search( ssize_t from)
{
	if ( field_channel && pattern.size() > 0 ) {
		cpattern = new sigproc::CPattern<float>
			(pattern, context_before, context_after,
			 field_channel->samplerate(),
			 bwf_order, bwf_cutoff, bwf_scale,
			 env_tightness,
			 dzcdf_step, dzcdf_sigma, dzcdf_smooth,
			 a, b, c);
		last_find = cpattern->find(
			field_channel->signal_lowpass.data,
			valarray<float>
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
SScoringFacility::SFindDialog::acquire_parameters()
{
	env_tightness = gtk_spin_button_get_value( ePatternEnvTightness);
	bwf_order  = gtk_spin_button_get_value( ePatternFilterOrder);
	bwf_cutoff = gtk_spin_button_get_value( ePatternFilterCutoff);
	dzcdf_step   = gtk_spin_button_get_value( ePatternDZCDFStep);
	dzcdf_sigma  = gtk_spin_button_get_value( ePatternDZCDFSigma);
	dzcdf_smooth = gtk_spin_button_get_value( ePatternDZCDFSmooth);
	a = gtk_spin_button_get_value( ePatternParameterA);
	b = gtk_spin_button_get_value( ePatternParameterB);
	c = gtk_spin_button_get_value( ePatternParameterC);

	// field_channel is set immediately in the ePatternChannel_changed_cb()
}

void
SScoringFacility::SFindDialog::update_displayed_parameters()
{
	gtk_spin_button_set_value( ePatternEnvTightness, env_tightness);
	gtk_spin_button_set_value( ePatternFilterCutoff, bwf_cutoff   );
	gtk_spin_button_set_value( ePatternFilterOrder,  bwf_order    );
	gtk_spin_button_set_value( ePatternDZCDFStep,    dzcdf_step   );
	gtk_spin_button_set_value( ePatternDZCDFSigma,   dzcdf_sigma  );
	gtk_spin_button_set_value( ePatternDZCDFSmooth,  dzcdf_smooth );
}

void
SScoringFacility::SFindDialog::enable_controls( bool indeed)
{
	gtk_widget_set_sensitive( (GtkWidget*)bPatternFindNext, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)bPatternFindPrevious, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)bPatternSave, (gboolean)indeed);
	gtk_widget_set_sensitive( (GtkWidget*)bPatternDiscard, (gboolean)indeed);
}

namespace patterns {

GtkListStore
	*mPatterns;

int
construct_once()
{
	mPatterns =
		gtk_list_store_new( 1, G_TYPE_STRING);

	return 0;
}



inline namespace {

}



} // namespace patterns
} // namespace sf



// callbacks


using namespace aghui;
using namespace aghui::sf;
using namespace aghui::patterns;


extern "C" {



	gboolean
	daPatternSelection_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		GdkWindow *window = gtk_widget_get_window( wid);
		FD.acquire_parameters();

		if ( FD.pattern.size() > 0 ) {
			cairo_t *cr = gdk_cairo_create( window);
			cairo_set_source_rgba( cr, 1., 1., 1., .2);
			cairo_set_font_size( cr, 18);
			cairo_show_text( cr, "(no pattern)");
			cairo_stroke( cr);
			cairo_destroy( cr);

			FD.enable_controls( false);
			return FALSE;
		} else {
			FD.enable_controls( true);
		}

		gint	ht = gdk_window_get_height( window),
			wd = gdk_window_get_width( window);

		size_t	run = FD.pattern_size_essential();

		cairo_t *cr = gdk_cairo_create( window);
		cairo_set_source_rgb( cr, 1., 1., 1.);
		cairo_paint( cr);
		cairo_stroke( cr);

		// ticks
		{
			cairo_set_font_size( cr, 9);
			CwB[TColour::ticks_sf].set_source_rgb( cr);
			float	seconds = (float)run / FD.samplerate;
			for ( size_t i8 = 0; (float)i8 / 8 < seconds; ++i8 ) {
				guint x = (float)i8/8 / seconds * wd;
				cairo_set_line_width( cr, (i8%8 == 0) ? 1. : (i8%4 == 0) ? .6 : .3);
				cairo_move_to( cr, x, 0);
				cairo_line_to( cr, x, ht);
				cairo_stroke( cr);

				if ( i8%8 == 0 ) {
					cairo_move_to( cr, x + 5, ht-2);
					snprintf_buf( "%g", (float)i8/8);
					cairo_show_text( cr, __buf__);
					cairo_stroke( cr);
				}
			}
		}

		// snippet
		cairo_set_source_rgb( cr, 0.1, 0.1, 0.1);
		cairo_set_line_width( cr, .8);
		::draw_signal( FD.pattern, FD.context_before, FD.context_before + FD.pattern.size(),
			       FD.display_scale,
			       wd, ht/3, cr, false);
		if ( FD.last_find != (size_t)-1 )
			::draw_signal( FD.field_channel->signal_filtered,
				       FD.last_find, run,
				       FD.display_scale,
				       wd, ht*2/3, cr, false);

		cairo_stroke( cr);

		if ( FD.draw_details ) {
			// envelope
			valarray<float>
				env_u, env_l,
				course,
				dzcdf;

			{
				if ( sigproc::envelope( FD.pattern, FD.env_tightness, FD.samplerate,
							1./FD.samplerate,
							env_l, env_u) == 0 ) {
					cairo_set_source_rgba( cr, 0., 0., 0., .6);
					cairo_set_font_size( cr, 15);
					cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
					cairo_text_extents_t extents;
					snprintf_buf( "Selection is too short");
					cairo_text_extents( cr, __buf__, &extents);
					cairo_move_to( cr, wd/2 - extents.width/2, 18);
					cairo_show_text( cr, __buf__);
					cairo_stroke( cr);
					FD.enable_controls( false);
					goto out;
				} else {
					FD.enable_controls( true);
				}

				cairo_set_source_rgba( cr, 0.3, 0.2, 0.8, .5);
				cairo_set_line_width( cr, .5);
				::draw_signal( env_u, FD.context_before, run,
					       FD.display_scale,
					       wd, ht/3, cr, false);
				::draw_signal( env_l, FD.context_before, run,
					       FD.display_scale,
					       wd, ht/3, cr, false);
				cairo_stroke( cr);
			}

			// low-pass filter
			{
				course = exstrom::low_pass( FD.pattern, FD.samplerate,
							    FD.bwf_cutoff, FD.bwf_order, true);

				cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
				cairo_set_line_width( cr, 3.);
				::draw_signal( course, FD.context_before, run,
					       FD.display_scale,
					       wd, ht/3, cr, false);
				cairo_stroke( cr);
			}

			// dzcdf
			{
				dzcdf = sigproc::dzcdf( FD.pattern, FD.samplerate,
							FD.dzcdf_step, FD.dzcdf_sigma, FD.dzcdf_smooth);
				float	dzcdf_display_scale = ht/8.
					/ valarray<float> (FD.pattern[ slice (FD.context_before, run, 1) ]).sum() / run;

				cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
				cairo_set_line_width( cr, 1.);
				::draw_signal( dzcdf, FD.context_before, run,
					       dzcdf_display_scale,
					       wd, ht/2-5, cr, false);
				cairo_stroke( cr);
				cairo_set_line_width( cr, .5);
				cairo_rectangle( cr, 0, ht/2-5, wd, ht/2-4);
				cairo_stroke( cr);
			}
		}

	out:
		cairo_destroy( cr);

		return TRUE;
	}




	gboolean
	daPatternSelection_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;

		switch ( event->direction ) {
		case GDK_SCROLL_UP:
			FD.display_scale *= 1.1;
			break;
		case GDK_SCROLL_DOWN:
			FD.display_scale /= 1.1;
		default:
			break;
		}

		gtk_widget_queue_draw( wid);

		return TRUE;
	}




	void
	bPatternFind_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		gboolean
			go_forward = strcmp( gtk_widget_get_name( (GtkWidget*)button), "bPatternFindNext") == 0;

		size_t from;
		if ( FD.last_find == (size_t)-1 )
			from = go_forward
				? FD.context_before
				: FD.field_channel->n_samples() - FD.pattern.size();
		else
			from = FD.last_find + (go_forward ? 10 : -10);

		set_cursor_busy( true, (GtkWidget*)FD.wPattern);
		FD.search( from);
		if ( FD.last_find == (size_t)-1 )
			pop_ok_message( (GtkWindow*)FD.wPattern, "Not found");
		else { // reach up and out
			FD.field_channel->sf.set_cur_vpage(
				FD.last_find / FD.samplerate / FD.field_channel->sf.vpagesize());
			auto& SF = FD.field_channel->sf;
			size_t	lpp = FD.samplerate * SF.vpagesize();
			SF.marking_in_widget = FD.field_channel->da_page;
			SF.marquee_start = (float)(FD.last_find % lpp) / lpp * FD.field_channel->da_page_wd;
			SF.marquee_virtual_end = SF.marquee_start + (float)FD.pattern.size() / lpp * FD.field_channel->da_page_wd;
			SF.queue_redraw_all();
			SF.marking_in_widget = NULL;

			snprintf_buf( "at p. %zu (a = %4.2f, b = %4.2f, c = %4.2f)\n",
				      SF.cur_vpage()+1, FD.match_a, FD.match_b, FD.match_c);
			gtk_label_set_markup( FD.lPatternSimilarity, __buf__);

			gtk_widget_queue_draw( (GtkWidget*)FD.lPatternSimilarity);
			gtk_widget_queue_draw( (GtkWidget*)FD.daPatternSelection);
		}

		set_cursor_busy( false, (GtkWidget*)FD.wPattern);
	}





	void
	bPatternSave_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		if ( gtk_dialog_run( FD.wPatternName) == GTK_RESPONSE_OK ) {
			const char *label = gtk_entry_get_text( FD.ePatternNameName);
			gboolean do_globally = gtk_toggle_button_get_active( (GtkToggleButton*)FD.ePatternNameSaveGlobally);
			FD.save_pattern( label, do_globally);

			// add to dropdown list & select the newly added entry
			FD.enumerate_patterns_to_combo();
			g_signal_handler_block( FD.ePatternList, FD.ePatternList_changed_cb_handler_id);
			FD.preselect_entry( label, do_globally);
			g_signal_handler_unblock( FD.ePatternList, FD.ePatternList_changed_cb_handler_id);
		}
	}


	void
	bPatternDiscard_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		GtkTreeIter iter;
		if ( gtk_combo_box_get_active_iter( FD.ePatternList, &iter) == FALSE )
			return;
		char *label;
		gtk_tree_model_get( (GtkTreeModel*)mPatterns, &iter,
				    0, &label,
				    -1);
		gboolean do_globally = strncmp( label, GLOBALLY_MARKER, strlen( GLOBALLY_MARKER)) == 0;
		char *fname = do_globally
			? label + strlen(GLOBALLY_MARKER)
			: label;
		FD.discard_pattern( fname, do_globally);
		free( label);
		g_signal_handler_block( FD.ePatternList, FD.ePatternList_changed_cb_handler_id);
		FD.preselect_entry( NULL, do_globally);
		g_signal_handler_unblock( FD.ePatternList, FD.ePatternList_changed_cb_handler_id);
	}


	void
	ePatternList_changed_cb( GtkComboBox *combo, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		GtkTreeIter iter;
		if ( gtk_combo_box_get_active_iter( combo, &iter) == FALSE )
			return;
		char *label;
		gtk_tree_model_get( (GtkTreeModel*)mPatterns, &iter,
				    0, &label,
				    -1);
		gboolean do_globally = strncmp( label, GLOBALLY_MARKER, strlen( GLOBALLY_MARKER)) == 0;
		char *fname = do_globally
			? label + strlen(GLOBALLY_MARKER)
			: label;
		FD.load_pattern( fname, do_globally);
		free( label);

		gtk_label_set_markup( FD.lPatternSimilarity, "");

		gtk_widget_queue_draw( (GtkWidget*)FD.daPatternSelection);
	}


	void
	ePatternChannel_changed_cb( GtkComboBox *combo, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		GtkTreeIter iter;
		if ( gtk_combo_box_get_active_iter( combo, &iter) == FALSE )
			return;

		char *label;
		gtk_tree_model_get( gtk_combo_box_get_model( combo), &iter,
				    0, &label,
				    -1);
		auto& SF = FD.field_channel->sf;
		for ( auto H = SF.channels.begin(); H != SF.channels.end(); ++H ) {
			if ( strcmp( H->name, label) == 0 ) {
				FD.field_channel = SF.using_channel = &*H;
				break;
			}
		}
		free( label);
	}



	void
	wPattern_show_cb( GtkWidget *widget, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		FD.enumerate_patterns_to_combo();
		auto& SF = FD.field_channel->sf;
		if ( SF.using_channel == NULL ) // not invoked for a preselected signal via a menu
			SF.using_channel = &SF.channels.front();
		FD.preselect_channel( SF.using_channel->name);
	}

	void
	wPattern_hide_cb( GtkWidget *widget, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		gtk_toggle_button_set_active( (GtkToggleButton*)FD.field_channel->sf.bScoringFacShowFindDialog, FALSE);
	}


} // extern "C"

} // namespace aghui

// EOF
