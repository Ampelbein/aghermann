      // ------- wPhaseDiff
	if ( !(AGH_GBGETOBJ (GtkDialog,		wSFPD)) ||
	     !(AGH_GBGETOBJ (GtkDrawingArea,	daSFPD)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eSFPDChannelA)) ||
	     !(AGH_GBGETOBJ (GtkComboBox,	eSFPDChannelB)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFPDFreqFrom)) ||
	     !(AGH_GBGETOBJ (GtkSpinButton,	eSFPDBandwidth)) ||
	     !(AGH_GBGETOBJ (GtkScaleButton,	eSFPDSmooth)) )
		throw runtime_error ("Failed to construct SF widgets (11)");

	gtk_combo_box_set_model_properly(
		eSFPDChannelA, _p.mEEGChannels);
	eSFPDChannelA_changed_cb_handler_id =
		G_CONNECT_1 (eSFPDChannelA, changed);

	gtk_combo_box_set_model_properly( eSFPDChannelB, _p.mEEGChannels);
	eSFPDChannelB_changed_cb_handler_id =
		G_CONNECT_1 (eSFPDChannelB, changed);

	G_CONNECT_1 (daSFPD, draw);
	G_CONNECT_2 (daSFPD, scroll, event);
	G_CONNECT_1 (eSFPDChannelA, changed);
	G_CONNECT_1 (eSFPDChannelB, changed);
	G_CONNECT_2 (eSFPDFreqFrom, value, changed);
	G_CONNECT_2 (eSFPDBandwidth, value, changed);
	G_CONNECT_2 (eSFPDSmooth, value, changed);
	G_CONNECT_1 (wSFPD, show);
	G_CONNECT_1 (wSFPD, hide);
