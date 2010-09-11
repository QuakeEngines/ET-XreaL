#ifndef _GTKUTIL_MULTIMON_H_
#define _GTKUTIL_MULTIMON_H_

#include <gtk/gtkwindow.h>
#include <gdk/gdkscreen.h>
#include <gdk/gdkdisplay.h>
#include "itextstream.h"

namespace gtkutil {

/** 
 * greebo: This class acts as container for several
 * multi-monitor-related functions. Use the getMonitor() method
 * to acquire the screen dimensions of the given screen.
 */
class MultiMonitor 
{
public:
	/**
	 * Returns the number of monitors of the default screen.
	 */
	static int getNumMonitors() {
		// Acquire the default screen reference
		GdkScreen* screen = gdk_display_get_default_screen(gdk_display_get_default());
		
		// Get and return the number of monitors
		return gdk_screen_get_n_monitors(screen);
	}

	/** 
	 * Returns the screen rectangle of the screen with the given index.
	 * The first screen is always present and has the index 0.
	 */
	static GdkRectangle getMonitor(int monitorNum) {
		GdkScreen* screen = gdk_display_get_default_screen(gdk_display_get_default());

		GdkRectangle geom;
		gdk_screen_get_monitor_geometry(screen, monitorNum, &geom);

		return geom;
	}

	/** 
	 * greebo: Returns the rectangle (width/height) for the monitor 
	 * which the given window is displayed on.
	 */
	static GdkRectangle getMonitorForWindow(GtkWindow* window) {
		// Retrieve the screen
		GdkScreen* scr = gtk_window_get_screen(GTK_WINDOW(window));

		// Get the monitor which the GtkWindow is displayed on
		gint monitorNum = gdk_screen_get_monitor_at_window(scr, GTK_WIDGET(window)->window);

		return getMonitor(monitorNum);
	}

	static void printMonitorInfo() {
		globalOutputStream() << "Default screen has " << getNumMonitors() << " monitors.\n";

		// detect multiple monitors
		for (int j = 0; j < getNumMonitors(); j++) {
			GdkRectangle geom = getMonitor(j);

			globalOutputStream() << "Monitor " << j << " geometry: " 
								 << geom.width << "x" << geom.height << " at "
								 << geom.x << ", " << geom.y << "\n";
		}
	}
};

} // namespace gtkutil

#endif /* _GTKUTIL_MULTIMON_H_ */
