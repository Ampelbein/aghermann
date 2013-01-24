	struct SPhasediffDialog {
		DELETE_DEFAULT_METHODS (SPhasediffDialog);

		SPhasediffDialog (SScoringFacility&);
	       ~SPhasediffDialog ();

		const SChannel
			*channel1,
			*channel2;
		bool	use_original_signal;
		float	from,
			upto;

		unsigned
			bwf_order,
			scope;
		float	display_scale;

		valarray<TFloat>
			course;
		size_t	smooth_side;
		void update_course();

		const SChannel* channel_from_cbox( GtkComboBox *cbox);
		void preselect_channel( GtkComboBox *cbox, const char *ch);

		void draw( cairo_t* cr, int wd, int ht);

		bool suspend_draw;

		SScoringFacility&
			_p;
	};


	// phasediff dialog
	GtkDialog
		*wSFPD;
	GtkComboBox
		*eSFPDChannelA, *eSFPDChannelB;
	GtkDrawingArea
		*daSFPD;
	GtkSpinButton
		*eSFPDFreqFrom,
		*eSFPDBandwidth;
	GtkScaleButton
		*eSFPDSmooth;
	gulong
		eSFPDChannelA_changed_cb_handler_id,
		eSFPDChannelB_changed_cb_handler_id;

