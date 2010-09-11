#ifndef SCROLLEDFRAME_H_
#define SCROLLEDFRAME_H_

#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkframe.h>

namespace gtkutil
{

/** 
 * Container widget which packs its child into a GtkScrolledWindow.
 */

class ScrolledFrame
{
	// Main widget
	GtkWidget* _widget;
	
public:

	/** 
	 * Construct a ScrolledFrame around the provided child widget.
	 * @useViewPort: use this to add "non-scrollable" widgets to this container.
	 */
	ScrolledFrame(GtkWidget* child, bool useViewPort = false)
	{
		// Create the GtkScrolledWindow
		_widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(_widget),
									   GTK_POLICY_AUTOMATIC,
									   GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(
            GTK_SCROLLED_WINDOW(_widget), GTK_SHADOW_ETCHED_IN
        );

		if (useViewPort)
		{
			gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(_widget), child);
		}
		else
		{
			gtk_container_add(GTK_CONTAINER(_widget), child);
		}
	}
	
	/** Operator cast to GtkWidget*.
	 */
	operator GtkWidget* () {
		return _widget;	
	}
};

}

#endif /*SCROLLEDFRAME_H_*/
