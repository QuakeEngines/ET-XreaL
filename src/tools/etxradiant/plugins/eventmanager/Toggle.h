#ifndef TOGGLE_H_
#define TOGGLE_H_

#include "ieventmanager.h"
#include <boost/function.hpp>

#include "gtk/gtktoggletoolbutton.h"
#include "gtk/gtktogglebutton.h"
#include "gtk/gtkcheckmenuitem.h"

#include "Event.h"

/* greebo: A Toggle object has a state (toggled = TRUE/FALSE) and a callback that
 * is invoked on toggle.
 * 
 * A Toggle can be connected to a GtkToggleToolButton/GtkCheckMenuItem via the according method.
 * 
 * Use the updateWidget() method to export the current state of the Toggle object to
 * the connected widgets. 
 */
class Toggle :
	public Event
{
private:
	// The callback to be performed on toggle()
	ToggleCallback _callback;
	
protected:
	// The list of connected widgets (associates handler IDs)
	typedef std::map<GtkWidget*, gulong> ToggleWidgetList;
	ToggleWidgetList _toggleWidgets;
	
	bool _callbackActive;
	
	// The toggled state of this object
	bool _toggled;
	
public:
	Toggle(const ToggleCallback& callback);

	virtual ~Toggle();

	// Returns usually false, because a Toggle is never empty 
	virtual bool empty() const;
	
	// Set the toggled state to true/false, according to <toggled> and update
	// any associated widgets or notify any callbacks.
	virtual bool setToggled(const bool toggled);
	
	// Update the "active" state of the connected widgets
	virtual void updateWidgets();
	
	// On key press >> toggle the internal state
	virtual void keyDown();
	
	// Returns true if the internal state is true
	bool isToggled() const;
	
	// Returns true for this and all derived classes
	virtual bool isToggle() const;
	
	// Connect a Widget (e.g. GtkToggleToolButton or GtkCheckMenuItem to this Toggle)
	virtual void connectWidget(GtkWidget* widget);
	virtual void disconnectWidget(GtkWidget* widget);
	
	// Invoke the registered callback and update/notify
	virtual void toggle();

	// The static GTK callback methods that can be connected to a ToolButton or a MenuItem
	static gboolean onToggleToolButtonClicked(GtkToggleToolButton* toolButton, Toggle* self);
	static gboolean onToggleButtonClicked(GtkToggleButton* toggleButton, Toggle* self);
	static gboolean onCheckMenuItemClicked(GtkMenuItem* menuitem, Toggle* self);

}; // class Toggle

typedef boost::shared_ptr<Toggle> TogglePtr;

#endif /*TOGGLE_H_*/
