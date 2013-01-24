	// aghui::SScoringFacility::SFiltersDialog::

      // ------- wSFFilter
	if ( !AGH_GBGETOBJ (GtkDialog,		wSFFilters) ||
	     !AGH_GBGETOBJ (GtkLabel,		lSFFilterCaption) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFilterLowPassCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFilterLowPassOrder) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFilterHighPassCutoff) ||
	     !AGH_GBGETOBJ (GtkSpinButton,	eSFFilterHighPassOrder) ||
	     !AGH_GBGETOBJ (GtkComboBox,	eSFFilterNotchFilter) ||
	     !AGH_GBGETOBJ (GtkListStore,	mSFFilterNotchFilter) ||
	     !AGH_GBGETOBJ (GtkButton,		bSFFilterOK) )
		throw runtime_error ("Failed to construct SF widgets (10)");

	gtk_combo_box_set_model_properly(
		eSFFilterNotchFilter, mSFFilterNotchFilter); // can't reuse _p.mNotchFilter

	G_CONNECT_2 (eSFFilterHighPassCutoff, value, changed);
	G_CONNECT_2 (eSFFilterLowPassCutoff, value, changed);

