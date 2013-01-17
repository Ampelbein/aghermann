// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-patterns-enumerate.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-16
 *
 *         Purpose:  scoring facility patterns (enumerating & io)
 *
 *         License:  GPL
 */

#include <tuple>
#include <dirent.h>
#include <sys/stat.h>

#include "ui/misc.hh"
#include "sf.hh"

using namespace std;


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

	set_thing_da_width( full_sample / field.spp());

	preselect_channel( field.name);
	preselect_entry( NULL, 0);
	gtk_label_set_markup( _p.lSFFDSimilarity, "");

	gtk_widget_queue_draw( (GtkWidget*)_p.daSFFDThing);
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

			set_thing_da_width( full_sample / field_channel->spp());

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
	g_signal_handler_block( _p.eSFFDPatternList, _p.eSFFDPatternList_changed_cb_handler_id);
	gtk_list_store_clear( _p.mSFFDPatterns);

	GtkTreeIter iter;

	struct dirent **eps;
	int n;
	snprintf_buf( "%s/.patterns", _p._p.ED->session_dir().c_str());
	n = scandir( __buf__, &eps, scandir_filter, alphasort);
//	printf( "n = %d in %s\n", n, __buf__);
	if ( n >= 0 ) {
		for ( int cnt = 0; cnt < n; ++cnt ) {
			snprintf_buf( "%s%s", globally_marker, eps[cnt]->d_name);
			gtk_list_store_append( _p.mSFFDPatterns, &iter);
			gtk_list_store_set( _p.mSFFDPatterns, &iter,
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
			gtk_list_store_append( _p.mSFFDPatterns, &iter);
			gtk_list_store_set( _p.mSFFDPatterns, &iter,
					    0, eps[cnt]->d_name,
					    -1);
			free( eps[cnt]);
		}
		free( (void*)eps);
	}
	gtk_combo_box_set_active_iter( _p.eSFFDPatternList, NULL);
	g_signal_handler_unblock( _p.eSFFDPatternList, _p.eSFFDPatternList_changed_cb_handler_id);
}





void
aghui::SScoringFacility::SFindDialog::
preselect_entry( const char *label, bool do_globally)
{
	if ( label == NULL ) {
		gtk_combo_box_set_active_iter( _p.eSFFDPatternList, NULL);
		return;
	}

	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( (GtkTreeModel*)_p.mSFFDPatterns, &iter);
	while ( valid ) {
		char *entry;
		gtk_tree_model_get( (GtkTreeModel*)_p.mSFFDPatterns, &iter,
				    0, &entry,
				    -1);
		if ( (!do_globally && strcmp( entry, label) == 0) ||
		     (do_globally && (strlen( entry) > strlen( globally_marker) && strcmp( entry+strlen(globally_marker), label) == 0)) ) {
			gtk_combo_box_set_active_iter( _p.eSFFDPatternList, &iter);
			free( entry);
			return;
		}
		free( entry);
		valid = gtk_tree_model_iter_next( (GtkTreeModel*)_p.mSFFDPatterns, &iter);
	}
}


// eof
