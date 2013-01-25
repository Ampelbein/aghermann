/*
 *       File name:  ui/sf/d/artifacts.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-05
 *
 *         Purpose:  scoring facility: artifact detection dialog
 *
 *         License:  GPL
 */

#include "artifacts.hh"

using namespace std;

aghui::SScoringFacility::SArtifactsDialog&
aghui::SScoringFacility::
artifacts_d()
{
	if ( not _artifacts_d )
		_artifacts_d = new SArtifactsDialog(*this);
	return *_artifacts_d;
}


aghui::SScoringFacility::SArtifactsDialog::
SArtifactsDialog (aghui::SScoringFacility& p_)
      : _p (p_)
{
	W_V.reg( eSFADScope,		&P.scope);
	W_V.reg( eSFADUpperThr,		&P.upper_thr);
	W_V.reg( eSFADLowerThr,		&P.lower_thr);
	W_V.reg( eSFADF0,		&P.f0);
	W_V.reg( eSFADFc,		&P.fc);
	W_V.reg( eSFADBandwidth,	&P.bandwidth);
	W_V.reg( eSFADMCGain,		&P.mc_gain);
	W_V.reg( eSFADBackpolate,	&P.iir_backpolate);
	W_V.reg( eSFADEstimateE,	&P.estimate_E);
	W_V.reg( eSFADEValue,		&P.E);
	W_V.reg( eSFADHistRangeMin,	&P.dmin);
	W_V.reg( eSFADHistRangeMax,	&P.dmax);
	W_V.reg( eSFADHistBins,		(int*)&P.sssu_hist_size);
	W_V.reg( eSFADSmoothSide,	(int*)&P.smooth_side);
	W_V.reg( eSFADUseThisRange,	&P.use_range);
}


aghui::SScoringFacility::SArtifactDetectionDialog::
~SArtifactDetectionDialog ()
{
	gtk_widget_destroy( (GtkWidget*)_p.wSFAD);
	g_object_unref( (GObject*)builder);
}


// maybe it's not needed in GTK+ 3?
void
aghui::SScoringFacility::
populate_mSFADProfiles()
{
	g_signal_handler_block( eSFADProfiles, eSFADProfiles_changed_cb_handler_id);
	gtk_list_store_clear( mSFADProfiles);
	for ( auto &P : _p.global_artifact_detection_profiles ) {
		GtkTreeIter iter;
		gtk_list_store_append( mSFADProfiles, &iter);
		gtk_list_store_set( mSFADProfiles, &iter,
				    0, P.first.c_str(),
				    -1);
	}
	gtk_combo_box_set_model( eSFADProfiles, (GtkTreeModel*)mSFADProfiles);
	g_signal_handler_unblock( eSFADProfiles, eSFADProfiles_changed_cb_handler_id);
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
