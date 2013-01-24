/*
 *       File name:  ui/sf/dialogs/artifacts.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-05
 *
 *         Purpose:  scoring facility: artifact detection dialog
 *
 *         License:  GPL
 */

#include "sf.hh"
#include "sf-widgets.hh"

using namespace std;


aghui::SScoringFacility::SArtifactDetectionDialog::
SArtifactDetectionDialog (aghui::SScoringFacility& p_)
      : P (),
	_p (p_)
{
	W_V.reg( _p.eSFADScope,		&P.scope);
	W_V.reg( _p.eSFADUpperThr,	&P.upper_thr);
	W_V.reg( _p.eSFADLowerThr,	&P.lower_thr);
	W_V.reg( _p.eSFADF0,		&P.f0);
	W_V.reg( _p.eSFADFc,		&P.fc);
	W_V.reg( _p.eSFADBandwidth,	&P.bandwidth);
	W_V.reg( _p.eSFADMCGain,	&P.mc_gain);
	W_V.reg( _p.eSFADBackpolate,	&P.iir_backpolate);
	W_V.reg( _p.eSFADEstimateE,	&P.estimate_E);
	W_V.reg( _p.eSFADEValue,	&P.E);
	W_V.reg( _p.eSFADHistRangeMin,	&P.dmin);
	W_V.reg( _p.eSFADHistRangeMax,	&P.dmax);
	W_V.reg( _p.eSFADHistBins,	(int*)&P.sssu_hist_size);
	W_V.reg( _p.eSFADSmoothSide,	(int*)&P.smooth_side);
	W_V.reg( _p.eSFADUseThisRange,	&P.use_range);
}


aghui::SScoringFacility::SArtifactDetectionDialog::
~SArtifactDetectionDialog ()
{
	gtk_widget_destroy( (GtkWidget*)_p.wSFAD);
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
