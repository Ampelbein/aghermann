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

#include "ui/misc.hh"
#include "patterns.hh"

using namespace std;

aghui::SScoringFacility::SPatternsDialog&
aghui::SScoringFacility::
patterns_d()
{
	if ( not _patterns_d )
		_patterns_d = new SPatternsDialog(*this);
	return *_patterns_d;
}

aghui::SScoringFacility::SPatternsDialog::
SPatternsDialog (SScoringFacility& parent)
      : SPatternsDialogWidgets (parent),
	Pp2 {.25,  0., 1.5, 1,  .1, .5, 3},
	cpattern (nullptr),
	increment (.05),
	field_profile_type (metrics::TType::mc),
	draw_details (true),
	_p (parent)
{
	suppress_w_v = true;
	W_V.reg( eSFFDEnvTightness, 	&Pp2.env_scope);
	W_V.reg( eSFFDBandPassOrder, &Pp2.bwf_order);
	W_V.reg( eSFFDBandPassFrom, 	&Pp2.bwf_ffrom);
	W_V.reg( eSFFDBandPassUpto, 	&Pp2.bwf_fupto);
	W_V.reg( eSFFDDZCDFStep, 	&Pp2.dzcdf_step);
	W_V.reg( eSFFDDZCDFSigma, 	&Pp2.dzcdf_sigma);
	W_V.reg( eSFFDDZCDFSmooth, 	&Pp2.dzcdf_smooth);

	W_V.reg( eSFFDParameterA, 	&get<0>(criteria));
	W_V.reg( eSFFDParameterB, 	&get<1>(criteria));
	W_V.reg( eSFFDParameterC, 	&get<2>(criteria));
	W_V.reg( eSFFDParameterD, 	&get<3>(criteria));

	W_V.up();
	suppress_w_v = false;

	load_patterns();
}

aghui::SScoringFacility::SPatternsDialog::
~SPatternsDialog ()
{
	save_patterns();

	assert ( cpattern == nullptr );

	// g_object_unref( mPatterns);
	gtk_widget_destroy( (GtkWidget*)wSFFDPatternSave);
	gtk_widget_destroy( (GtkWidget*)wSFFD);
}



list<pattern::SPattern<TFloat>>::iterator
aghui::SScoringFacility::SPatternsDialog::
pattern_by_idx( size_t idx)
{
	size_t i = 0;
	for ( auto I = patterns.begin(); I != patterns.end(); ++I )
		if ( i == idx )
			return I;
		else
			++i;
	throw invalid_argument ("Current pattern index invalid");
}




void
aghui::SScoringFacility::SPatternsDialog::
search()
{
	if ( unlikely
	     (not field_channel or current_pattern == patterns.end()) )
		return;

	if ( field_channel != field_channel_saved )
		field_channel_saved = field_channel;

	cpattern = new pattern::CPatternTool<TFloat>
		({current_pattern->thing, current_pattern->samplerate},
		 current_pattern->context_before, current_pattern->context_after,
		 Pp2); // use this for the case when modiified current_pattern changes have not been committed
	diff_line =
		(cpattern->do_search(
			field_channel->signal_envelope( Pp2.env_scope).first,
			field_channel->signal_envelope( Pp2.env_scope).second,
			field_channel->signal_bandpass( Pp2.bwf_ffrom, Pp2.bwf_fupto, Pp2.bwf_order),
			field_channel->signal_dzcdf( Pp2.dzcdf_step, Pp2.dzcdf_sigma, Pp2.dzcdf_smooth),
			increment * current_pattern->samplerate),
		 cpattern->diff);

	delete cpattern;
	cpattern = nullptr; // don't really care though
}


size_t
aghui::SScoringFacility::SPatternsDialog::
find_occurrences()
{
	if ( unlikely (current_pattern == patterns.end()) )
		return 0;

	occurrences.resize(0);
	size_t inc = max((int)(increment * current_pattern->samplerate), 1);
	for ( size_t i = 0; i < diff_line.size(); i += inc )
		if ( diff_line[i].good_enough( criteria) ) {
			occurrences.push_back(i);
			i +=  // avoid overlapping occurrences *and* ensure we hit the stride
				current_pattern->pattern_size_essential()/inc * inc;
		}

	restore_annotations();
	occurrences_to_annotations();

	return occurrences.size();
}


void
aghui::SScoringFacility::SPatternsDialog::
occurrences_to_annotations()
{
	for ( size_t o = 0; o < occurrences.size(); ++o )
		sigfile::mark_annotation(
			field_channel->annotations,
			occurrences[o], occurrences[o] + current_pattern->pattern_size_essential(),
			(snprintf_buf("%s (%zu)", current_pattern->name.c_str(), o), __buf__));
	_p._p.populate_mGlobalAnnotations();
}

void
aghui::SScoringFacility::SPatternsDialog::
save_annotations()
{
	saved_annotations = field_channel->annotations;
	_p._p.populate_mGlobalAnnotations();
}

void
aghui::SScoringFacility::SPatternsDialog::
restore_annotations()
{
	field_channel->annotations = saved_annotations;
	saved_annotations.clear();
	_p._p.populate_mGlobalAnnotations();
}




void
aghui::SScoringFacility::SPatternsDialog::
setup_controls_for_find()
{
	bool	have_any = current_pattern != patterns.end();

	gtk_widget_set_visible( (GtkWidget*)cSFFDSearchButton, have_any and TRUE);
	gtk_widget_set_visible( (GtkWidget*)cSFFDSearching, FALSE);
	gtk_widget_set_visible( (GtkWidget*)cSFFDAgainButton, FALSE);

	gtk_widget_set_visible( (GtkWidget*)cSFFDParameters, have_any and TRUE);

	gtk_widget_set_visible( (GtkWidget*)swSFFDField, FALSE);
	gtk_widget_set_visible( (GtkWidget*)cSFFDCriteria, FALSE);

	gtk_label_set_markup( lSFFDFoundInfo, "");
}

void
aghui::SScoringFacility::SPatternsDialog::
setup_controls_for_wait()
{
	gtk_widget_set_visible( (GtkWidget*)cSFFDSearchButton, FALSE);
	gtk_widget_set_visible( (GtkWidget*)cSFFDSearching, TRUE);
	gtk_widget_set_visible( (GtkWidget*)cSFFDAgainButton, FALSE);

	gtk_widget_set_visible( (GtkWidget*)cSFFDParameters, TRUE);

	gtk_widget_set_visible( (GtkWidget*)swSFFDField, FALSE);
	gtk_widget_set_visible( (GtkWidget*)cSFFDCriteria, FALSE);
}

void
aghui::SScoringFacility::SPatternsDialog::
setup_controls_for_tune()
{
	gtk_widget_set_visible( (GtkWidget*)cSFFDSearchButton, FALSE);
	gtk_widget_set_visible( (GtkWidget*)cSFFDSearching, FALSE);
	gtk_widget_set_visible( (GtkWidget*)cSFFDAgainButton, TRUE);

	gtk_widget_set_visible( (GtkWidget*)cSFFDParameters, FALSE);

	gtk_widget_set_visible( (GtkWidget*)swSFFDField, TRUE);
	gtk_widget_set_visible( (GtkWidget*)cSFFDCriteria, TRUE);
}



void
aghui::SScoringFacility::SPatternsDialog::
set_profile_manage_buttons_visibility()
{
	bool	have_any = current_pattern != patterns.end(),
		is_transient = have_any && current_pattern->origin == pattern::TOrigin::transient,
		is_modified  = have_any && not (current_pattern->Pp == Pp2);
	gtk_widget_set_visible( (GtkWidget*)bSFFDProfileSave, have_any and is_transient);
	gtk_widget_set_visible( (GtkWidget*)bSFFDProfileRevert, have_any and not is_transient and is_modified);
	gtk_widget_set_visible( (GtkWidget*)bSFFDProfileDiscard, have_any and not is_transient);
}


void
aghui::SScoringFacility::SPatternsDialog::
preselect_channel( const char *ch)
{
	if ( ch == NULL ) {
		gtk_combo_box_set_active_iter( eSFFDChannel, NULL);
		return;
	}

	GtkTreeModel *model = gtk_combo_box_get_model( eSFFDChannel);
	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( model, &iter);
	while ( valid ) {
		DEF_UNIQUE_CHARP (entry);
		gtk_tree_model_get( model, &iter,
				    0, &entry,
				    -1);
		if ( strcmp( entry, ch) == 0 ) {
			gtk_combo_box_set_active_iter( eSFFDChannel, &iter);
			return;
		}
		valid = gtk_tree_model_iter_next( model, &iter);
	}
}



size_t
aghui::SScoringFacility::SPatternsDialog::
nearest_occurrence( double x) const
{
	double shortest = INFINITY;
	size_t found_at = -1;
	for ( size_t o = 0; o < occurrences.size(); ++o ) {
		double d = fabs((double)occurrences[o]/diff_line.size() - x/da_field_wd);
		if ( d < shortest ) {
			shortest = d;
			found_at = o;
		}
	}
	return found_at;
}


// eof
