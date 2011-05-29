// ;-*-C++-*- *  Time-stamp: "2011-05-29 20:59:00 hmmr"
/*
 *       File name:  ui/measurements.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-13
 *
 *         Purpose:  measurements overview view
 *
 *         License:  GPL
 */

#include <list>
#include <map>

#include <gtk/gtk.h>
#include <cairo/cairo.h>

#include "../structures.hh"
#include "settings.hh"
#include "libagh/primaries.hh"


#ifndef _MEASUREMENTS_H
#define _MEASUREMENTS_H

using namespace std;

namespace aghui {
namespace msmtview {

	extern float
		PPuV2;

	void	populate();
	int	construct_once();
	void	destruct();
	namespace dnd {
		int	construct_once();
		void	destruct();
		extern GtkTargetEntry
			target_list[];
		extern size_t
			n_targets;
	}

	extern gulong
		eMsmtSession_changed_cb_handler_id,
		eMsmtChannel_changed_cb_handler_id;


      // ui structures
	struct SSubjectPresentation {
		agh::CSubject&  // can't have it declared const due to CMSessionSet operator[] not permitting
			csubject;
	      // this is a little overkill, but whatever
		agh::CSCourse
			*cscourse;
		time_t	tl_start;

		typedef list<agh::CSubject::SEpisode>::iterator TEpisodeIter;
		TEpisodeIter
			episode_focused;
		GtkWidget
			*da;
		bool	is_focused;

		bool get_episode_from_timeline_click( unsigned along);  // possibly sets episode_focused

		void draw_timeline_to_widget( GtkWidget *wid) const
			{
				cairo_t *cr = gdk_cairo_create( gtk_widget_get_window( wid));
				draw_timeline( cr);
				cairo_destroy( cr);
			}
		void draw_timeline_to_file( const char *fname) const;

	      // ctor
		SSubjectPresentation( agh::CSubject& _j)
		      : csubject (_j),
			episode_focused (csubject.measurements[*_AghDi].episodes.end()),
			da (NULL),
			is_focused (false)
			{
				try {
					cscourse = new agh::CSCourse ( csubject, *_AghDi, *_AghTi,
								       settings::OperatingRangeFrom, settings::OperatingRangeUpto,
								       0., 0, false, false);
					tl_start = csubject.measurements[*_AghDi].episodes.front().start_rel;
				} catch (...) {  // can be invalid_argument (no recording in such session/channel) or some TSimPrepError
					cscourse = NULL;
				}
			}
	       ~SSubjectPresentation()
			{
				if ( cscourse )
					delete cscourse;
			}

	    private:
		void draw_timeline( cairo_t*) const;
	};

	struct SGroupPresentation : public list<SSubjectPresentation> {
		map<string, agh::CJGroup>::iterator _group;
		bool	visible;
		GtkExpander
			*expander,
			*vbox;
		const char* name() const
			{
				return _group->first.c_str();
			}
		agh::CJGroup& group()
			{
				return _group->second;
			}
		SGroupPresentation( map<string, agh::CJGroup>::iterator& _g)
		      : _group (_g)
			{}
	};

}  // namespace msmt

      // forward declarations of callbacks
	extern "C" {
		void eMsmtSession_changed_cb( GtkComboBox*, gpointer);
		void eMsmtChannel_changed_cb( GtkComboBox*, gpointer);
		void eMsmtPSDFreqFrom_value_changed_cb( GtkSpinButton*, gpointer);
		void eMsmtPSDFreqWidth_value_changed_cb( GtkSpinButton*, gpointer);

		gboolean daSubjectTimeline_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
		gboolean daSubjectTimeline_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
		gboolean daSubjectTimeline_draw_cb( GtkWidget*, cairo_t*, gpointer);
//		gboolean daSubjectTimeline_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
		gboolean daSubjectTimeline_enter_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
		gboolean daSubjectTimeline_leave_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
		gboolean daSubjectTimeline_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);

		gboolean check_gtk_entry_nonempty( GtkWidget*, GdkEventKey*, gpointer);
		gboolean cMeasurements_drag_data_received_cb( GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, gpointer);
		gboolean cMeasurements_drag_drop_cb( GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);
	}

} // namespace aghui

#endif

// eof
