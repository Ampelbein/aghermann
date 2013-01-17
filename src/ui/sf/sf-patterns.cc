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
      : Pp {2,  0., 1.5, 1,  .1, .5, 3},
	Pp2 (Pp),
	cpattern (nullptr),
	increment (.05),
	draw_details (true),
	_p (parent)
{
	W_V.reg( _p.eSFFDEnvTightness, 	&Pp.env_tightness);
	W_V.reg( _p.eSFFDBandPassOrder, 	&Pp.bwf_order);
	W_V.reg( _p.eSFFDBandPassFrom, 	&Pp.bwf_ffrom);
	W_V.reg( _p.eSFFDBandPassUpto, 	&Pp.bwf_fupto);
	W_V.reg( _p.eSFFDDZCDFStep, 		&Pp.dzcdf_step);
	W_V.reg( _p.eSFFDDZCDFSigma, 	&Pp.dzcdf_sigma);
	W_V.reg( _p.eSFFDDZCDFSmooth, 	&Pp.dzcdf_smooth);

	W_V.reg( _p.eSFFDParameterA, 	&get<0>(criteria));
	W_V.reg( _p.eSFFDParameterB, 	&get<1>(criteria));
	W_V.reg( _p.eSFFDParameterC, 	&get<2>(criteria));
	W_V.reg( _p.eSFFDParameterD, 	&get<3>(criteria));
}

aghui::SScoringFacility::SFindDialog::
~SFindDialog ()
{
	if ( cpattern )
		delete cpattern;
	// g_object_unref( mPatterns);
	gtk_widget_destroy( (GtkWidget*)_p.wSFFDPatternName);
	gtk_widget_destroy( (GtkWidget*)_p.wSFFD);
}






void
aghui::SScoringFacility::SFindDialog::
search()
{
	set_field_da_width( _p.total_pages() * 3);
	field_display_scale =
		agh::alg::calibrate_display_scale(
			field_channel->psd.course, _p.total_pages(),
			da_field_ht);

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
setup_controls_for_find()
{
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDSearchButton, TRUE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDParameters, TRUE);

	gtk_widget_set_visible( (GtkWidget*)_p.swSFFDField, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDCriteria, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDAgainButton, FALSE);
}

void
aghui::SScoringFacility::SFindDialog::
setup_controls_for_tune()
{
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDSearchButton, FALSE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDParameters, FALSE);

	gtk_widget_set_visible( (GtkWidget*)_p.swSFFDField, TRUE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDCriteria, TRUE);
	gtk_widget_set_visible( (GtkWidget*)_p.cSFFDAgainButton, TRUE);
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



// eof
