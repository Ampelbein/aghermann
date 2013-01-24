
	struct SFiltersDialog {
		DELETE_DEFAULT_METHODS (SFiltersDialog);

		SFiltersDialog (SScoringFacility& parent)
		      : _p (parent)
			{}
	      ~SFiltersDialog ();

		SUIVarCollection
			W_V;

		SScoringFacility&
			_p;
	};



	// filters dialog
	GtkDialog
		*wSFFilters;
	GtkLabel
		*lSFFilterCaption;
	GtkSpinButton
		*eSFFilterLowPassCutoff, *eSFFilterHighPassCutoff,
		*eSFFilterLowPassOrder, *eSFFilterHighPassOrder;
	GtkComboBox
		*eSFFilterNotchFilter;
	GtkListStore
		*mSFFilterNotchFilter;
	GtkButton
		*bSFFilterOK;
