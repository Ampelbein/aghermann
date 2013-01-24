// ;-*-C++-*-
/*
 *       File name:  ui/sf/sf-patterns-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-24
 *
 *         Purpose:  scoring facility Patterns widget construction
 *
 *         License:  GPL
 */

#include <stdexcept>


	mSFFDPatterns =
		gtk_list_store_new( 1, G_TYPE_STRING);

	if ( !AGH_GBGETOBJ (GtkDialog,		wSFFD) ||
	     !AGH_GBGETOBJ (GtkDrawingArea,	daSFFDThing) ||
	     !AGH_GBGETOBJ (GtkScrolledWindow,	swSFFDThing) ||
	     !AGH_GBGETOBJ (GtkDrawingArea,	daSFFDField) ||
	     !AGH_GBGETOBJ (GtkMenu,		iiSFFDField) ||
	     !AGH_GBGETOBJ (GtkScrolledWindow,	swSFFDField) ||
	     !AGH_GBGETOBJ (GtkTable,		cSFFDSearchButton) ||
	     !AGH_GBGETOBJ (GtkTable,		cSFFDAgainButton) ||
	     !AGH_GBGETOBJ (GtkBox,		cSFFDSearching) ||
	     !AGH_GBGETOBJ (GtkTable,		cSFFDParameters) ||
	     !AGH_GBGETOBJ (GtkTable,		cSFFDCriteria) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFFDSearch) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFFDAgain) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFFDProfileSave) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFFDProfileDiscard) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFFDProfileRevert) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDEnvTightness) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDBandPassOrder) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDBandPassFrom) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDBandPassUpto) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDDZCDFStep) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDDZCDFSigma) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDDZCDFSmooth) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDParameterA) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDParameterB) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDParameterC) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFDParameterD) ||
	     !AGH_GBGETOBJ (GtkHBox,		cSFFDLabelBox) ||
	     !AGH_GBGETOBJ (GtkLabel,		lSFFDParametersBrief) ||
	     !AGH_GBGETOBJ (GtkLabel,		lSFFDFoundInfo) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eSFFDPatternList) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eSFFDChannel) ||
	     !AGH_GBGETOBJ (GtkDialog,		wSFFDPatternSave) ||
	     !AGH_GBGETOBJ (GtkEntry,		eSFFDPatternSaveName) ||
	     !AGH_GBGETOBJ (GtkToggleButton,	eSFFDPatternSaveOriginSubject) ||
	     !AGH_GBGETOBJ (GtkToggleButton,	eSFFDPatternSaveOriginExperiment) ||
	     !AGH_GBGETOBJ (GtkToggleButton,	eSFFDPatternSaveOriginUser) )
		throw runtime_error ("Failed to construct SF widgets (9)");

	gtk_combo_box_set_model_properly( eSFFDPatternList, mSFFDPatterns);
	eSFFDPatternList_changed_cb_handler_id =
		G_CONNECT_1 (eSFFDPatternList, changed);

	gtk_combo_box_set_model_properly( eSFFDChannel, _p.mAllChannels);
	eSFFDChannel_changed_cb_handler_id =
		G_CONNECT_1 (eSFFDChannel, changed);

	G_CONNECT_2 (wSFFD, configure, event);
	G_CONNECT_1 (daSFFDThing, draw);
	G_CONNECT_2 (daSFFDThing, scroll, event);
	G_CONNECT_1 (daSFFDField, draw);
	G_CONNECT_2 (daSFFDField, scroll, event);
	G_CONNECT_3 (daSFFDField, motion, notify, event);
	G_CONNECT_3 (daSFFDField, button, press, event);
	G_CONNECT_1 (bSFFDProfileSave, clicked);
	G_CONNECT_1 (bSFFDProfileDiscard, clicked);
	G_CONNECT_1 (bSFFDProfileRevert, clicked);
	G_CONNECT_1 (bSFFDSearch, clicked);
	G_CONNECT_1 (bSFFDAgain, clicked);

	for ( auto& W : {eSFFDEnvTightness,
			 eSFFDBandPassFrom, eSFFDBandPassUpto, eSFFDBandPassOrder,
			 eSFFDDZCDFStep, eSFFDDZCDFSigma, eSFFDDZCDFSmooth} )
		g_signal_connect( W, "value-changed",
				  (GCallback)eSFFD_any_pattern_value_changed_cb,
				  this);
	for ( auto& W : {eSFFDParameterA, eSFFDParameterB, eSFFDParameterC, eSFFDParameterD} )
		g_signal_connect( W, "value-changed",
				  (GCallback)eSFFD_any_criteria_value_changed_cb,
				  this);

	G_CONNECT_1 (wSFFD, show);
	G_CONNECT_1 (wSFFD, hide);

}
