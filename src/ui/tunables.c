// ;-*-C++-*- *  Time-stamp: "2010-11-27 02:09:56 hmmr"
/*
 *       File name:  tunables.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  tunables wilderness
 *
 *         License:  GPL
 */



#include <glade/glade.h>
#include "../libagh/iface.h"
#include "ui.h"



#define _val_  0
#define _min_  1
#define _max_  2
#define _step_ 3
#define _req_  4
static GtkWidget
	*eTunable[_agh_basic_tunables_][5];


gint
agh_ui_construct_SimulationParams( GladeXML *xml)
{
      // ------------- eTunable_*
	if ( !(eTunable[_rs_][_val_]		= glade_xml_get_widget( xml, "eTunable_rs")) ||
	     !(eTunable[_rs_][_min_]		= glade_xml_get_widget( xml, "eTunable_rs_min")) ||
	     !(eTunable[_rs_][_max_]		= glade_xml_get_widget( xml, "eTunable_rs_max")) ||
	     !(eTunable[_rs_][_step_]		= glade_xml_get_widget( xml, "eTunable_rs_step")) ||
	     !(eTunable[_rs_][_req_]		= glade_xml_get_widget( xml, "eTunable_rs_req")) ||

	     !(eTunable[_rc_][_val_]		= glade_xml_get_widget( xml, "eTunable_rc")) ||
	     !(eTunable[_rc_][_min_]		= glade_xml_get_widget( xml, "eTunable_rc_min")) ||
	     !(eTunable[_rc_][_max_]		= glade_xml_get_widget( xml, "eTunable_rc_max")) ||
	     !(eTunable[_rc_][_step_]		= glade_xml_get_widget( xml, "eTunable_rc_step")) ||
	     !(eTunable[_rc_][_req_]		= glade_xml_get_widget( xml, "eTunable_rc_req")) ||

	     !(eTunable[_fcR_][_val_]		= glade_xml_get_widget( xml, "eTunable_fcR")) ||
	     !(eTunable[_fcR_][_min_]		= glade_xml_get_widget( xml, "eTunable_fcR_min")) ||
	     !(eTunable[_fcR_][_max_]		= glade_xml_get_widget( xml, "eTunable_fcR_max")) ||
	     !(eTunable[_fcR_][_step_]		= glade_xml_get_widget( xml, "eTunable_fcR_step")) ||
	     !(eTunable[_fcR_][_req_]		= glade_xml_get_widget( xml, "eTunable_fcR_req")) ||

	     !(eTunable[_fcW_][_val_]		= glade_xml_get_widget( xml, "eTunable_fcW")) ||
	     !(eTunable[_fcW_][_min_]		= glade_xml_get_widget( xml, "eTunable_fcW_min")) ||
	     !(eTunable[_fcW_][_max_]		= glade_xml_get_widget( xml, "eTunable_fcW_max")) ||
	     !(eTunable[_fcW_][_step_]		= glade_xml_get_widget( xml, "eTunable_fcW_step")) ||
	     !(eTunable[_fcW_][_req_]		= glade_xml_get_widget( xml, "eTunable_fcW_req")) ||

	     !(eTunable[_S0_][_val_]		= glade_xml_get_widget( xml, "eTunable_S0")) ||
	     !(eTunable[_S0_][_min_]		= glade_xml_get_widget( xml, "eTunable_S0_min")) ||
	     !(eTunable[_S0_][_max_]		= glade_xml_get_widget( xml, "eTunable_S0_max")) ||
	     !(eTunable[_S0_][_step_]		= glade_xml_get_widget( xml, "eTunable_S0_step")) ||
	     !(eTunable[_S0_][_req_]		= glade_xml_get_widget( xml, "eTunable_S0_req")) ||

	     !(eTunable[_SU_][_val_]		= glade_xml_get_widget( xml, "eTunable_SU")) ||
	     !(eTunable[_SU_][_min_]		= glade_xml_get_widget( xml, "eTunable_SU_min")) ||
	     !(eTunable[_SU_][_max_]		= glade_xml_get_widget( xml, "eTunable_SU_max")) ||
	     !(eTunable[_SU_][_step_]		= glade_xml_get_widget( xml, "eTunable_SU_step")) ||
	     !(eTunable[_SU_][_req_]		= glade_xml_get_widget( xml, "eTunable_SU_req")) ||

	     !(eTunable[_ta_][_val_]		= glade_xml_get_widget( xml, "eTunable_ta")) ||
	     !(eTunable[_ta_][_min_]		= glade_xml_get_widget( xml, "eTunable_ta_min")) ||
	     !(eTunable[_ta_][_max_]		= glade_xml_get_widget( xml, "eTunable_ta_max")) ||
	     !(eTunable[_ta_][_step_]		= glade_xml_get_widget( xml, "eTunable_ta_step")) ||
	     !(eTunable[_ta_][_req_]		= glade_xml_get_widget( xml, "eTunable_ta_req")) ||

	     !(eTunable[_tp_][_val_]		= glade_xml_get_widget( xml, "eTunable_tp")) ||
	     !(eTunable[_tp_][_min_]		= glade_xml_get_widget( xml, "eTunable_tp_min")) ||
	     !(eTunable[_tp_][_max_]		= glade_xml_get_widget( xml, "eTunable_tp_max")) ||
	     !(eTunable[_tp_][_step_]		= glade_xml_get_widget( xml, "eTunable_tp_step")) ||
	     !(eTunable[_tp_][_req_]		= glade_xml_get_widget( xml, "eTunable_tp_req")) ||

	     !(eTunable[_gc_][_val_]		= glade_xml_get_widget( xml, "eTunable_gc")) ||
	     !(eTunable[_gc_][_min_]		= glade_xml_get_widget( xml, "eTunable_gc_min")) ||
	     !(eTunable[_gc_][_max_]		= glade_xml_get_widget( xml, "eTunable_gc_max")) ||
	     !(eTunable[_gc_][_step_]		= glade_xml_get_widget( xml, "eTunable_gc_step")) ||
	     !(eTunable[_gc_][_req_]		= glade_xml_get_widget( xml, "eTunable_gc_req")) )
		return -1;

	return 0;
}




gboolean
fSimParamTunables_expose_event_cb()
{
	struct SConsumerTunableSetFull tset;
	agh_tunables0_get( &tset);
	const struct STunableDescription *td;
	for ( gushort t = 0; t < _agh_basic_tunables_; ++t ) {
		td = agh_tunable_get_description(t);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][_val_]),	td->display_scale_factor * tset.tunables[t]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][_min_]),	td->display_scale_factor * tset.lower_bounds[t]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][_max_]),	td->display_scale_factor * tset.upper_bounds[t]);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eTunable[t][_step_]),	td->display_scale_factor * tset.steps[t]);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (eTunable[t][_req_]),	tset.states[t] & T_REQUIRED_C);
	}

	return FALSE;
}





#define ENTRY_TO_TUNABLE_VAL(t) \
	struct SConsumerTunableSetFull tset;	\
	agh_tunables0_get(&tset);		\
	const struct STunableDescription *td = agh_tunable_get_description(t);		\
	tset.tunables[t] = gtk_spin_button_get_value(e) / td->display_scale_factor;	\
	agh_tunables0_put(&tset);

#define ENTRY_TO_TUNABLE_MIN(t) \
	struct SConsumerTunableSetFull tset;	\
	agh_tunables0_get(&tset);		\
	const struct STunableDescription *td = agh_tunable_get_description(t);		\
	tset.lower_bounds[t] = gtk_spin_button_get_value(e) / td->display_scale_factor;	\
	agh_tunables0_put(&tset);

#define ENTRY_TO_TUNABLE_MAX(t) \
	struct SConsumerTunableSetFull tset;	\
	agh_tunables0_get(&tset);		\
	const struct STunableDescription *td = agh_tunable_get_description(t);		\
	tset.upper_bounds[t] = gtk_spin_button_get_value(e) / td->display_scale_factor;	\
	agh_tunables0_put(&tset);

#define ENTRY_TO_TUNABLE_STEP(t) \
	struct SConsumerTunableSetFull tset;	\
	agh_tunables0_get(&tset);		\
	const struct STunableDescription *td = agh_tunable_get_description(t);		\
	tset.steps[t] = gtk_spin_button_get_value(e) / td->display_scale_factor;	\
	agh_tunables0_put(&tset);

#define ENTRY_TO_TUNABLE_REQ(t) \
	struct SConsumerTunableSetFull tset;	\
	agh_tunables0_get(&tset);		\
	tset.states[t] = gtk_toggle_button_get_active(e);			\
	agh_tunables0_put(&tset);

void eTunable_S0_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_S0_);  }
void eTunable_S0_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_S0_);  }
void eTunable_S0_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_S0_);  }
void eTunable_S0_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_S0_); }
void eTunable_S0_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_S0_);  }

void eTunable_SU_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_SU_);  }
void eTunable_SU_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_SU_);  }
void eTunable_SU_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_SU_);  }
void eTunable_SU_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_SU_); }
void eTunable_SU_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_SU_);  }

void eTunable_fcR_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_fcR_);  }
void eTunable_fcR_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_fcR_);  }
void eTunable_fcR_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_fcR_);  }
void eTunable_fcR_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_fcR_); }
void eTunable_fcR_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_fcR_);  }

void eTunable_fcW_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_fcW_);  }
void eTunable_fcW_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_fcW_);  }
void eTunable_fcW_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_fcW_);  }
void eTunable_fcW_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_fcW_); }
void eTunable_fcW_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_fcW_);  }

void eTunable_gc_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_gc_);  }
void eTunable_gc_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_gc_);  }
void eTunable_gc_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_gc_);  }
void eTunable_gc_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_gc_); }
void eTunable_gc_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_gc_);  }

void eTunable_rc_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_rc_);  }
void eTunable_rc_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_rc_);  }
void eTunable_rc_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_rc_);  }
void eTunable_rc_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_rc_); }
void eTunable_rc_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_rc_);  }

void eTunable_rs_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_rs_);  }
void eTunable_rs_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_rs_);  }
void eTunable_rs_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_rs_);  }
void eTunable_rs_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_rs_); }
void eTunable_rs_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_rs_);  }

void eTunable_ta_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_ta_);  }
void eTunable_ta_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_ta_);  }
void eTunable_ta_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_ta_);  }
void eTunable_ta_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_ta_); }
void eTunable_ta_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_ta_);  }

void eTunable_tp_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_VAL(_tp_);  }
void eTunable_tp_min_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MIN(_tp_);  }
void eTunable_tp_max_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_MAX(_tp_);  }
void eTunable_tp_step_value_changed_cb( GtkSpinButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_STEP(_tp_); }
void eTunable_tp_req_toggled_cb( GtkToggleButton *e, gpointer u)	{ ENTRY_TO_TUNABLE_REQ(_tp_);  }



void
bSimParamRevertTunables_clicked_cb()
{
	agh_tunables0_stock_defaults();
	fSimParamTunables_expose_event_cb();
}


// EOF
