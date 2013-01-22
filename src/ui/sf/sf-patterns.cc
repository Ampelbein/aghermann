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
#include "sf.hh"

using namespace std;


aghui::SScoringFacility::SFindDialog::
SFindDialog (SScoringFacility& parent)
      : Q (nullptr),
	Pp2 {.25,  0., 1.5, 1,  .1, .5, 3},
	cpattern (nullptr),
	increment (.05),
	field_profile_type (metrics::TType::mc),
	draw_details (true),
	_p (parent)
{
	suppress_w_v = true;
	W_V.reg( _p.eSFFDEnvTightness, 	&Pp2.env_scope);
	W_V.reg( _p.eSFFDBandPassOrder, &Pp2.bwf_order);
	W_V.reg( _p.eSFFDBandPassFrom, 	&Pp2.bwf_ffrom);
	W_V.reg( _p.eSFFDBandPassUpto, 	&Pp2.bwf_fupto);
	W_V.reg( _p.eSFFDDZCDFStep, 	&Pp2.dzcdf_step);
	W_V.reg( _p.eSFFDDZCDFSigma, 	&Pp2.dzcdf_sigma);
	W_V.reg( _p.eSFFDDZCDFSmooth, 	&Pp2.dzcdf_smooth);

	W_V.reg( _p.eSFFDParameterA, 	&get<0>(criteria));
	W_V.reg( _p.eSFFDParameterB, 	&get<1>(criteria));
	W_V.reg( _p.eSFFDParameterC, 	&get<2>(criteria));
	W_V.reg( _p.eSFFDParameterD, 	&get<3>(criteria));

	W_V.up();
	suppress_w_v = false;

	load_patterns();
}

aghui::SScoringFacility::SFindDialog::
~SFindDialog ()
{
	save_patterns();

	assert ( cpattern );

	// g_object_unref( mPatterns);
	gtk_widget_destroy( (GtkWidget*)_p.wSFFDPatternName);
	gtk_widget_destroy( (GtkWidget*)_p.wSFFD);
}



list<pattern::SPattern<TFloat>>::iterator
aghui::SScoringFacility::SFindDialog::
pattern_by_idx( size_t idx)
{
	int i = 0;
	for ( auto I : patterns )
		if ( i == idx )
			return *I;
		else
			++i;
	throw invalid_argument ("Current pattern index invalid");	
}




void
aghui::SScoringFacility::SFindDialog::
search()
{
	if ( unlikely
	     (not field_channel or not Q or Q->thing.size() == 0) )
		return;

	if ( field_channel != field_channel_saved )
		field_channel_saved = field_channel;

	cpattern = new pattern::CPatternTool<TFloat>
		({Q->thing, Q->samplerate},
		 Q->context_before, Q->context_after,
		 Q->Pp);
	diff_line =
		(cpattern->do_search(
			field_channel->signal_envelope( Pp2.env_scope).first,
			field_channel->signal_envelope( Pp2.env_scope).second,
			field_channel->signal_bandpass( Pp2.bwf_ffrom, Pp2.bwf_fupto, Pp2.bwf_order),
			field_channel->signal_dzcdf( Pp2.dzcdf_step, Pp2.dzcdf_sigma, Pp2.dzcdf_smooth),
			increment * Q->samplerate),
		 cpattern->diff);

	delete cpattern;
	cpattern = nullptr;
}


size_t
aghui::SScoringFacility::SFindDialog::
find_occurrences()
{
	assert (Q); // that button must be hidden
	occurrences.resize(0);
	size_t inc = max((int)(increment * Q->samplerate), 1);
	for ( size_t i = 0; i < diff_line.size(); i += inc )
		if ( diff_line[i].good_enough( criteria) ) {
			occurrences.push_back(i);
			i += Q->pattern_size_essential()/inc * inc; // avoid overlapping occurrences *and* ensure we hit the stride
		}

	restore_annotations();
	occurrences_to_annotations();

	return occurrences.size();
}


void
aghui::SScoringFacility::SFindDialog::
occurrences_to_annotations()
{
	for ( size_t o = 0; o < occurrences.size(); ++o )
		sigfile::mark_annotation(
			field_channel->annotations,
			occurrences[o], occurrences[o] + Q->pattern_size_essential(),
			(snprintf_buf("%s (%zu)", Q->name.c_str(), o), __buf__));
}

void
aghui::SScoringFacility::SFindDialog::
save_annotations()
{
	saved_annotations = field_channel->annotations;
}

void
aghui::SScoringFacility::SFindDialog::
restore_annotations()
{
	field_channel->annotations = saved_annotations;
	saved_annotations.clear();
}




void
aghui::SScoringFacility::SFindDialog::
setup_controls_for_find()
{
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDSearchButton, TRUE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDSearching, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDAgainButton, FALSE);

	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDParameters, TRUE);

	gtk_widget_set_visible( (GtkWidget*)_p.swSFFDField, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDCriteria, FALSE);

	gtk_label_set_markup( _p.lSFFDFoundInfo, "");
}

void
aghui::SScoringFacility::SFindDialog::
setup_controls_for_wait()
{
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDSearchButton, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDSearching, TRUE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDAgainButton, FALSE);

	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDParameters, TRUE);

	gtk_widget_set_visible( (GtkWidget*)_p.swSFFDField, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDCriteria, FALSE);
}

void
aghui::SScoringFacility::SFindDialog::
setup_controls_for_tune()
{
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDSearchButton, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDSearching, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDAgainButton, TRUE);

	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDParameters, FALSE);

	gtk_widget_set_visible( (GtkWidget*)_p.swSFFDField, TRUE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDCriteria, TRUE);
}




void
aghui::SScoringFacility::SFindDialog::
preselect_channel( const char *ch)
{
	if ( ch == NULL ) {
		gtk_combo_box_set_active_iter( _p.eSFFDChannel, NULL);
		return;
	}

	GtkTreeModel *model = gtk_combo_box_get_model( _p.eSFFDChannel);
	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( model, &iter);
	while ( valid ) {
		DEF_UNIQUE_CHARP (entry);
		gtk_tree_model_get( model, &iter,
				    0, &entry,
				    -1);
		if ( strcmp( entry, ch) == 0 ) {
			gtk_combo_box_set_active_iter( _p.eSFFDChannel, &iter);
			return;
		}
		valid = gtk_tree_model_iter_next( model, &iter);
	}
}



size_t
aghui::SScoringFacility::SFindDialog::
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
