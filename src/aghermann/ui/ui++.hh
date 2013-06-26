/*
 *       File name:  aghermann/ui/ui++.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-12
 *
 *         Purpose: more complex GTK+ things we do with C++
 *
 *         License:  GPL
 */


#ifndef AGH_AGHERMANN_UI_UI_PLUSPLUS_H_
#define AGH_AGHERMANN_UI_UI_PLUSPLUS_H_

#include <cstring>
#include <string>
#include <list>
#include <gtk/gtk.h>

#include "common/lang.hh"
#include "ui.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;


namespace aghui {

class SBusyBlock {
	DELETE_DEFAULT_METHODS (SBusyBlock);
    public:
	SBusyBlock (GtkWidget* w_)
	      : w (w_)
		{
			lock();
		}
	// poor ubuntu people
	// SBusyBlock (GtkWindow* w)
	//       : SBusyBlock ((GtkWidget*)w)
	// 	{}
	// SBusyBlock (GtkDialog* w)
	//       : SBusyBlock ((GtkWidget*)w)
	// 	{}
	SBusyBlock (GtkWindow* w_)
	      : w ((GtkWidget*)w_)
		{
			lock();
		}
	SBusyBlock (GtkDialog* w_)
	      : w ((GtkWidget*)w_)
		{
			lock();
		}

       ~SBusyBlock ()
		{
			set_cursor_busy( false, w);
			gtk_widget_set_sensitive( w, TRUE);
			gtk_flush();
		}
    private:
	GtkWidget *w;
	void lock()
		{
			gtk_widget_set_sensitive( w, FALSE);
			set_cursor_busy( true, w);
			gtk_flush();
		}
};



class SUIVar_base {
    public:
	virtual ~SUIVar_base ()
		{}
	virtual void down() const = 0;
	virtual void up() const = 0;
};


template <typename Tw, typename Tv>
class SUIVar_ : public SUIVar_base {
	DELETE_DEFAULT_METHODS (SUIVar_);

	Tw	*w;
	Tv	*v;

    public:
	SUIVar_ (Tw* w_, Tv* v_)
	      : w (w_), v (v_)
		{}
	virtual ~SUIVar_ ()
		{}

	virtual void down() const;
	virtual void up() const;
};


template <> inline void
SUIVar_<GtkSpinButton, double>::up()	const { gtk_spin_button_set_value( w, *v); }
template <> inline void
SUIVar_<GtkSpinButton, double>::down()	const { *v = gtk_spin_button_get_value( w); }

template <> inline void
SUIVar_<GtkSpinButton, float>::up()	const { gtk_spin_button_set_value( w, *v); }
template <> inline void
SUIVar_<GtkSpinButton, float>::down()	const { *v = gtk_spin_button_get_value( w); }

template <> inline void
SUIVar_<GtkSpinButton, int>::up()	const { gtk_spin_button_set_value( w, (double)*v); }
template <> inline void
SUIVar_<GtkSpinButton, int>::down()	const { *v = (int)round(gtk_spin_button_get_value( w)); }

template <> inline void
SUIVar_<GtkSpinButton, size_t>::up()	const { gtk_spin_button_set_value( w, (double)*v); }
template <> inline void
SUIVar_<GtkSpinButton, size_t>::down()	const
{
	auto t = (long int)round(gtk_spin_button_get_value( w));
	if ( t < 0 ) {
		fprintf( stderr, "SUIVar_<size_t> got a negative value (%ld) from your spinbutton; value has been clamped at 0\n", t);
		*v = 0;
	} else
		*v = t;
}

template <> inline void
SUIVar_<GtkComboBox, int>::up()		const { gtk_combo_box_set_active( w, *v); }
template <> inline void
SUIVar_<GtkComboBox, int>::down()	const { *v = gtk_combo_box_get_active( w); }

template <> inline void
SUIVar_<GtkCheckButton, bool>::up()	const { gtk_toggle_button_set_active( (GtkToggleButton*)w, *v); }
template <> inline void
SUIVar_<GtkCheckButton, bool>::down()	const { *v = gtk_toggle_button_get_active( (GtkToggleButton*)w); }

template <> inline void
SUIVar_<GtkRadioButton, bool>::up()	const { gtk_toggle_button_set_active( (GtkToggleButton*)w, *v); }
template <> inline void
SUIVar_<GtkRadioButton, bool>::down()	const { *v = gtk_toggle_button_get_active( (GtkToggleButton*)w); }

template <> inline void
SUIVar_<GtkEntry, string>::up()		const { gtk_entry_set_text( w, v->c_str()); }
template <> inline void
SUIVar_<GtkEntry, string>::down()	const { v->assign( gtk_entry_get_text( w)); }



class SUIVarCollection {
    public:
       ~SUIVarCollection ()
		{
			for ( auto& A : c )
				delete A;
		}

	void reg( GtkSpinButton *w, float* v)
		{
			c.push_back( new SUIVar_<GtkSpinButton, float> (w, v));
		}
	void reg( GtkSpinButton *w, double* v)
		{
			c.push_back( new SUIVar_<GtkSpinButton, double> (w, v));
		}
	void reg( GtkSpinButton *w, int* v)
		{
			c.push_back( new SUIVar_<GtkSpinButton, int> (w, v));
		}
	void reg( GtkSpinButton *w, size_t* v)
		{
			c.push_back( new SUIVar_<GtkSpinButton, size_t> (w, v));
		}
	void reg( GtkComboBox *w, int* v)
		{
			c.push_back( new SUIVar_<GtkComboBox, int> (w, v));
		}
	void reg( GtkCheckButton *w, bool* v)
		{
			c.push_back( new SUIVar_<GtkCheckButton, bool> (w, v));
		}
	void reg( GtkRadioButton *w, bool* v)
		{
			c.push_back( new SUIVar_<GtkRadioButton, bool> (w, v));
		}
	void reg( GtkEntry *w, string* v)
		{
			c.push_back( new SUIVar_<GtkEntry, string> (w, v));
		}
	// odd one out
	void reg( GtkListStore *m, list<string> *l)
		{
			c.push_back( new SUIVar_<GtkListStore, list<string>> (m, l));
		}

	void up() const
		{
			for ( auto& A : c )
				A->up();
		}
	void down() const
		{
			for ( auto& A : c )
				A->down();
		}

    private:
	list<SUIVar_base*> c;
};


} // namespace aghui

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
