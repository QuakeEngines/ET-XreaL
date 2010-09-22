#include "EventManager.h"

#include "imodule.h"
#include "iregistry.h"
#include "iradiant.h"
#include "itextstream.h"
#include "iselection.h"
#include <iostream>
#include <typeinfo>

#include "gdk/gdkevents.h"
#include "gdk/gdkkeysyms.h"
#include "gtk/gtkwindow.h"
#include "gtk/gtkaccelgroup.h"
#include "gtk/gtkeditable.h"
#include "gtk/gtktextview.h"

#include "xmlutil/Node.h"

#include "Statement.h"
#include "Toggle.h"
#include "WidgetToggle.h"
#include "RegistryToggle.h"
#include "KeyEvent.h"
#include "SaveEventVisitor.h"

#include "debugging/debugging.h"
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>

	namespace {
		const std::string RKEY_DEBUG = "debug/ui/debugEventManager";
	}

// RegisterableModule implementation
const std::string& EventManager::getName() const {
	static std::string _name(MODULE_EVENTMANAGER);
	return _name;
}

const StringSet& EventManager::getDependencies() const {
	static StringSet _dependencies;

	if (_dependencies.empty()) {
		_dependencies.insert(MODULE_XMLREGISTRY);
	}

	return _dependencies;
}

void EventManager::initialiseModule(const ApplicationContext& ctx) {
	globalOutputStream() << "EventManager::initialiseModule called.\n";
	
	_modifiers.loadModifierDefinitions();
	_mouseEvents.initialise();
	
	_debugMode = (GlobalRegistry().get(RKEY_DEBUG) == "1");
	
	// Deactivate the empty event, so it's safe to return it as NullEvent
	_emptyEvent->setEnabled(false);
	
	// Create an empty GClosure
	_accelGroup = gtk_accel_group_new();
	
	if (_debugMode) {
		globalOutputStream() << "EventManager intitialised in debug mode.\n";
	}
	else {
		globalOutputStream() << "EventManager successfully initialised.\n";
	}
}

void EventManager::shutdownModule() {
	globalOutputStream() << "EventManager: shutting down.\n";
	saveEventListToRegistry();

	_handlers.clear();
	_dialogWindows.clear();
	_accelerators.clear();
	_events.clear();
}

// Constructor
EventManager::EventManager() :
	_emptyEvent(new Event()),
	_emptyAccelerator(0, 0, _emptyEvent),
	_modifiers(),
	_mouseEvents(_modifiers),
	_debugMode(false)
{}

// Destructor, un-reference the GTK accelerator group
EventManager::~EventManager() {
	g_object_unref(_accelGroup);
	globalOutputStream() << "EventManager successfully shut down.\n";
}

void EventManager::connectSelectionSystem(SelectionSystem* selectionSystem) {
	_mouseEvents.connectSelectionSystem(selectionSystem);
}

// Returns a reference to the mouse event mapper
IMouseEvents& EventManager::MouseEvents() {
	return _mouseEvents;
}

IAccelerator& EventManager::addAccelerator(const std::string& key, const std::string& modifierStr) {
	guint keyVal = getGDKCode(key);
	unsigned int modifierFlags = _modifiers.getModifierFlags(modifierStr); 
	
	Accelerator accel(keyVal, modifierFlags, _emptyEvent);
	
	// Add a new Accelerator to the list
	_accelerators.push_back(accel);
	
	// return the reference to the last accelerator in the list
	AcceleratorList::reverse_iterator i = _accelerators.rbegin();
	
	return (*i);
}

IAccelerator& EventManager::addAccelerator(GdkEventKey* event) {
	// Create a new accelerator with the given arguments
	Accelerator accel(event->keyval, _modifiers.getKeyboardFlags(event->state), _emptyEvent);
	
	// Add a new Accelerator to the list
	_accelerators.push_back(accel);
	
	// return the reference to the last accelerator in the list
	AcceleratorList::reverse_iterator i = _accelerators.rbegin();
	
	return (*i);
}

IEventPtr EventManager::findEvent(const std::string& name) {
	// Try to lookup the command
	EventMap::iterator i = _events.find(name);
	
	if (i != _events.end()) {
		// Return the pointer to the command
		return i->second;
	}
	else {
		// Nothing found, return the NullEvent
		return _emptyEvent;
	}
}

IEventPtr EventManager::findEvent(GdkEventKey* event) {
	// Retrieve the accelerators for this eventkey 
	AcceleratorList accelList = findAccelerator(event);
	
	// Did we find any matching accelerators?
	if (accelList.size() > 0) {
		// Take the first found accelerator
		Accelerator& accel = *accelList.begin();
		
		return accel.getEvent();
	}
	else {
		// No accelerators found
		return _emptyEvent;
	}
}

std::string EventManager::getEventName(const IEventPtr& event)
{
	// Try to lookup the given eventptr
	for (EventMap::iterator i = _events.begin(); i != _events.end(); i++) {
		if (i->second == event) {
			return i->first;
		}
	}
	
	return "";
}

std::string EventManager::getAcceleratorStr(const IEventPtr& event, bool forMenu) {
	std::string returnValue = "";
	
	IAccelerator& accelerator = findAccelerator(event);
	
	unsigned int keyVal = accelerator.getKey();
	const std::string keyStr = (keyVal != 0) ? gdk_keyval_name(keyVal) : "";
	
	if (keyStr != "") {
		// Return a modifier string for a menu
		const std::string modifierStr = getModifierStr(accelerator.getModifiers(), forMenu);
		
		const std::string connector = (forMenu) ? "-" : "+";
		
		returnValue = modifierStr;
		returnValue += (modifierStr != "") ? connector : "";
		returnValue += keyStr;
	}
	
	return returnValue;
}

// Checks if the eventName is already registered and writes to globalOutputStream, if so 
bool EventManager::alreadyRegistered(const std::string& eventName) {
	// Try to find the command and see if it's already registered
	IEventPtr foundEvent = findEvent(eventName);
	
	if (foundEvent->empty()) {
		return false;
	}
	else {
		globalWarningStream() << "EventManager: Event " << eventName 
			<< " already registered!" << std::endl;
		return true;
	}
}

// Add the given command to the internal list
IEventPtr EventManager::addCommand(const std::string& name, const std::string& statement, bool reactOnKeyUp) {
	
	if (!alreadyRegistered(name)) {
		// Add the command to the list
		_events[name] = IEventPtr(new Statement(statement, reactOnKeyUp));
		
		// Return the pointer to the newly created event
		return _events[name];
	}
	
	return _emptyEvent;
}

IEventPtr EventManager::addKeyEvent(const std::string& name, const ui::KeyStateChangeCallback& keyStateChangeCallback)
{
	if (!alreadyRegistered(name))
	{
		// Add the new keyevent to the list (implicitly cast onto Event&)  
		_events[name] = IEventPtr(new KeyEvent(keyStateChangeCallback));
		
		// Return the pointer to the newly created event
		return _events[name];
	}
	
	return _emptyEvent;
}

IEventPtr EventManager::addWidgetToggle(const std::string& name) {
	
	if (!alreadyRegistered(name)) {
		// Add the command to the list (implicitly cast the pointer on Event&)  
		_events[name] = IEventPtr(new WidgetToggle());
		
		// Return the pointer to the newly created event
		return _events[name];
	}
	
	return _emptyEvent;
}

IEventPtr EventManager::addRegistryToggle(const std::string& name, const std::string& registryKey)
{
	if (!alreadyRegistered(name)) {
		// Add the command to the list (implicitly cast the pointer on Event&)  
		_events[name] = IEventPtr(new RegistryToggle(registryKey));
		
		// Return the pointer to the newly created event
		return _events[name];
	}
	
	return _emptyEvent;
}

IEventPtr EventManager::addToggle(const std::string& name, const ToggleCallback& onToggled)
{
	if (!alreadyRegistered(name)) {
		// Add the command to the list (implicitly cast the pointer on Event&)  
		_events[name] = IEventPtr(new Toggle(onToggled));
		
		// Return the pointer to the newly created event
		return _events[name];
	}
	
	return _emptyEvent;
}

void EventManager::setToggled(const std::string& name, const bool toggled)
{
	// Check could be placed here by boost::shared_ptr's dynamic_pointer_cast 
	if (!findEvent(name)->setToggled(toggled)) {
		globalWarningStream() << "EventManager: Event " << name 
			<< " is not a Toggle." << std::endl;
	}
}

// Connects the given accelerator to the given command (identified by the string)
void EventManager::connectAccelerator(IAccelerator& accelerator, const std::string& command) {
	IEventPtr event = findEvent(command);
		
	if (!event->empty()) {
		// Command found, connect it to the accelerator by passing its pointer			
		accelerator.connectEvent(event);
	}
	else {
		// Command NOT found
		globalWarningStream() << "EventManager: Unable to connect command: " << command << std::endl;
	}
}

void EventManager::disconnectAccelerator(const std::string& command) {
	IEventPtr event = findEvent(command);
		
	if (!event->empty()) {
		// Cycle through the accelerators and check for matches
		for (AcceleratorList::iterator i = _accelerators.begin(); i != _accelerators.end(); i++) {
			if (i->match(event)) {
				// Connect the accelerator to the empty event (disable the accelerator)
				i->connectEvent(_emptyEvent);
				i->setKey(0);
				i->setModifiers(0);
			}
		}
	}
	else {
		// Command NOT found
		globalWarningStream() << "EventManager: Unable to disconnect command: " << command << std::endl;
	}
}

void EventManager::disableEvent(const std::string& eventName) {
	findEvent(eventName)->setEnabled(false);
}

void EventManager::enableEvent(const std::string& eventName) {
	findEvent(eventName)->setEnabled(true);
}

void EventManager::removeEvent(const std::string& eventName) {
	// Try to lookup the command
	EventMap::iterator i = _events.find(eventName);
	
	if (i != _events.end()) {
		// Remove all accelerators beforehand
		disconnectAccelerator(eventName);

		// Remove the event from the list
		_events.erase(i);
	}
}

// Catches the key/mouse press/release events from the given GtkObject
void EventManager::connect(GtkObject* object)	{
	// Create and store the handler into the map
	gulong handlerId = g_signal_connect(G_OBJECT(object), "key-press-event", 
										G_CALLBACK(onKeyPress), this);
	_handlers[handlerId] = object;
	
	handlerId = g_signal_connect(G_OBJECT(object), "key-release-event", 
								 G_CALLBACK(onKeyRelease), this);
	_handlers[handlerId] = object;
}

void EventManager::disconnect(GtkObject* object) {
	for (HandlerMap::iterator i = _handlers.begin(); i != _handlers.end(); ) {
		if (i->second == object) {
			g_signal_handler_disconnect(G_OBJECT(i->second), i->first);
			// Be sure to increment the iterator with a postfix ++, so that the "old" iterator is passed
			_handlers.erase(i++);
		}
		else {
			i++;
		}
	}
}

/* greebo: This connects an dialog window to the event handler. This means the following:
 * 
 * An incoming key-press event reaches the static method onDialogKeyPress which
 * passes the key event to the connect dialog FIRST, before the key event has a
 * chance to be processed by the standard shortcut processor. IF the dialog window
 * standard handler returns TRUE, that is. If the gtk_window_propagate_key_event()
 * function returns FALSE, the window couldn't find a use for this specific key event
 * and the event can be passed safely to the onKeyPress() method. 
 * 
 * This way it is ensured that the dialog window can handle, say, text entries without
 * firing global shortcuts all the time.   
 */
void EventManager::connectDialogWindow(GtkWindow* window) {
	gulong handlerId = g_signal_connect(G_OBJECT(window), "key-press-event", 
										G_CALLBACK(onDialogKeyPress), this);

	_dialogWindows[handlerId] = GTK_OBJECT(window);		

	handlerId = g_signal_connect(G_OBJECT(window), "key-release-event", 
								 G_CALLBACK(onDialogKeyRelease), this);

	_dialogWindows[handlerId] = GTK_OBJECT(window);		
}

void EventManager::disconnectDialogWindow(GtkWindow* window) {
	GtkObject* object = GTK_OBJECT(window);
	
	for (HandlerMap::iterator i = _dialogWindows.begin(); i != _dialogWindows.end(); ) {
		// If the object pointer matches the one stored in the list, remove the handler id
		if (i->second == object) {
			g_signal_handler_disconnect(G_OBJECT(i->second), i->first);
			// Be sure to increment the iterator with a postfix ++, so that the "old" iterator is passed
			_dialogWindows.erase(i++);
		}
		else {
			i++;
		}
	}
}

void EventManager::connectAccelGroup(GtkWindow* window) {
	gtk_window_add_accel_group(window, _accelGroup); 
}

// Loads the default shortcuts from the registry
void EventManager::loadAccelerators() {
	if (_debugMode) {
		std::cout << "EventManager: Loading accelerators...\n";
	}
	
	xml::NodeList shortcutSets = GlobalRegistry().findXPath("user/ui/input//shortcuts");
	
	if (_debugMode) {
		std::cout << "Found " << shortcutSets.size() << " sets.\n";
	}
	
	// If we have two sets of shortcuts, delete the default ones
	if (shortcutSets.size() > 1) {
		GlobalRegistry().deleteXPath("user/ui/input//shortcuts[@name='default']");
	}
	
	// Find all accelerators
	xml::NodeList shortcutList = GlobalRegistry().findXPath("user/ui/input/shortcuts//shortcut");
	
	if (shortcutList.size() > 0) {
		globalOutputStream() << "EventManager: Shortcuts found in Registry: " << 
			static_cast<int>(shortcutList.size()) << std::endl;
		for (unsigned int i = 0; i < shortcutList.size(); i++) {
			const std::string key = shortcutList[i].getAttributeValue("key");
			
			if (_debugMode) {
				std::cout << "Looking up command: " << shortcutList[i].getAttributeValue("command") << "\n";
				std::cout << "Key is: >> " << key << " << \n";
			} 
			
			// Try to lookup the command
			IEventPtr event = findEvent(shortcutList[i].getAttributeValue("command"));
			
			// Check for a non-empty key string
			if (key != "") {
				 // Check for valid command definitions were found
				if (!event->empty()) {
					// Get the modifier string (e.g. "SHIFT+ALT")
					const std::string modifierStr = shortcutList[i].getAttributeValue("modifiers");
					
					if (!duplicateAccelerator(key, modifierStr, event)) {
						// Create the accelerator object
						IAccelerator& accelerator = addAccelerator(key, modifierStr);
					
						// Connect the newly created accelerator to the command
						accelerator.connectEvent(event);
					}
				} 
				else {
					globalWarningStream() << "EventManager: Cannot load shortcut definition (command invalid)." 
						<< std::endl;
				}
			}
		}
	}
	else {
		// No accelerator definitions found!
		globalWarningStream() << "EventManager: No shortcut definitions found..." << std::endl;
	}
}

void EventManager::foreachEvent(IEventVisitor& eventVisitor) {
	// Cycle through the event and pass them to the visitor class
	for (EventMap::iterator i = _events.begin(); i != _events.end(); i++) {
		const std::string eventName = i->first;
		IEventPtr event = i->second;
		
		eventVisitor.visit(eventName, event);
	}
}

// Tries to locate an accelerator, that is connected to the given command
IAccelerator& EventManager::findAccelerator(const IEventPtr& event) {
	// Cycle through the accelerators and check for matches
	for (AcceleratorList::iterator i = _accelerators.begin(); i != _accelerators.end(); i++) {
		if (i->match(event)) {
			// Return the reference to the found accelerator
			return (*i);
		}
	}

	// Return an empty accelerator if nothing is found
	return _emptyAccelerator;
}

// Returns the string representation of the given modifier flags 
std::string EventManager::getModifierStr(const unsigned int modifierFlags, bool forMenu) {
	// Pass the call to the modifiers helper class
	return _modifiers.getModifierStr(modifierFlags, forMenu);
}

unsigned int EventManager::getModifierState() {
	return _modifiers.getState();
}

void EventManager::saveEventListToRegistry() {
	const std::string rootKey = "user/ui/input";
	
	// The visitor class to save each event definition into the registry
	// Note: the SaveEventVisitor automatically wipes all the existing shortcuts from the registry
	SaveEventVisitor visitor(rootKey, this);
	 
	foreachEvent(visitor);
}

EventManager::AcceleratorList EventManager::findAccelerator(
	const std::string& key, const std::string& modifierStr)
{
	guint keyVal = getGDKCode(key);
	unsigned int modifierFlags = _modifiers.getModifierFlags(modifierStr); 
	
	return findAccelerator(keyVal, modifierFlags);
}

bool EventManager::duplicateAccelerator(const std::string& key, 
										const std::string& modifiers, 
										const IEventPtr& event)
{
	AcceleratorList accelList = findAccelerator(key, modifiers);
	
	for (AcceleratorList::iterator i = accelList.begin(); i != accelList.end(); i++) {
		// If one of the accelerators in the list matches the event, return true
		if (i->match(event)) {
			return true;
		}
	}
	
	return false;
}

EventManager::AcceleratorList EventManager::findAccelerator(guint keyVal, 
															const unsigned int modifierFlags)
{
	AcceleratorList returnList;
	
	// Cycle through the accelerators and check for matches
	for (AcceleratorList::iterator i = _accelerators.begin(); i != _accelerators.end(); i++) {
		
		if (i->match(keyVal, modifierFlags)) {
			// Add the pointer to the found accelerators
			returnList.push_back((*i));
		}
	}

	return returnList;
}

// Returns the pointer to the accelerator for the given GdkEvent, but convert the key to uppercase before passing it
EventManager::AcceleratorList EventManager::findAccelerator(GdkEventKey* event) {
	unsigned int keyval = gdk_keyval_to_upper(event->keyval);
	
	// greebo: I saw this in the original GTKRadiant code, maybe this is necessary to catch GTK_ISO_Left_Tab...
	if (keyval == GDK_ISO_Left_Tab) {
		keyval = GDK_Tab;
	}
	
	return findAccelerator(keyval, _modifiers.getKeyboardFlags(event->state));
}

// The GTK keypress callback
gboolean EventManager::onDialogKeyPress(GtkWindow* window, GdkEventKey* event, EventManager* self) {
	// Pass the key event to the connected dialog window and see if it can process it (returns TRUE)
	gboolean keyProcessed = gtk_window_propagate_key_event(window, event);

	// Get the focus widget, is it an editable widget?
	GtkWidget* focus = gtk_window_get_focus(window);
	bool isEditableWidget = GTK_IS_EDITABLE(focus) || GTK_IS_TEXT_VIEW(focus);

	// never pass onKeyPress event to the accelerator manager if an editable widget is focused
	// the only exception is the ESC key
	if (isEditableWidget && event->keyval != GDK_Escape) {
		return keyProcessed;
	}
	
	if (!keyProcessed) {
		// The dialog window returned FALSE, pass the key on to the default onKeyPress handler
		self->onKeyPress(GTK_WIDGET(window), event, self);
	}
	
	// If we return true here, the dialog window could process the key, and the GTK callback chain is stopped 
	return keyProcessed;
}

// The GTK keyrelease callback
gboolean EventManager::onDialogKeyRelease(GtkWindow* window, GdkEventKey* event, EventManager* self) {
	// Pass the key event to the connected dialog window and see if it can process it (returns TRUE)
	gboolean keyProcessed = gtk_window_propagate_key_event(window, event);
	
	// Get the focus widget, is it an editable widget?
	GtkWidget* focus = gtk_window_get_focus(window);
	bool isEditableWidget = GTK_IS_EDITABLE(focus) || GTK_IS_TEXT_VIEW(focus);

	if (isEditableWidget && event->keyval != GDK_Escape) {
		// never pass onKeyPress event to the accelerator manager if an editable widget is focused
		return keyProcessed;
	}

	if (!keyProcessed) {
		// The dialog window returned FALSE, pass the key on to the default onKeyPress handler
		self->onKeyRelease(GTK_WIDGET(window), event, self);
	}
	
	// If we return true here, the dialog window could process the key, and the GTK callback chain is stopped 
	return keyProcessed;
}

void EventManager::updateStatusText(GdkEventKey* event, bool keyPress) {
	// Make a copy of the given event key
	GdkEventKey eventKey = *event;
	
	// Sometimes the ALT modifier is not set, so this is a workaround for this
	if (eventKey.keyval == GDK_Alt_L || eventKey.keyval == GDK_Alt_R) {
		if (keyPress) {
			eventKey.state |= GDK_MOD1_MASK;
		}
		else {
			eventKey.state &= ~GDK_MOD1_MASK;
		}
	}
	
	_mouseEvents.updateStatusText(&eventKey);
}

// The GTK keypress callback
gboolean EventManager::onKeyPress(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	// Convert the passed pointer onto a KeyEventManager pointer
	EventManager* self = reinterpret_cast<EventManager*>(data);

	if (!GTK_IS_WIDGET(widget)) return FALSE;

	if (GTK_IS_WINDOW(widget))
	{
		// Pass the key event to the connected window and see if it can process it (returns TRUE)
		gboolean keyProcessed = gtk_window_propagate_key_event(GTK_WINDOW(widget), event);

		// Get the focus widget, is it an editable widget?
		GtkWidget* focus = gtk_window_get_focus(GTK_WINDOW(widget));
		bool isEditableWidget = GTK_IS_EDITABLE(focus) || GTK_IS_TEXT_VIEW(focus);

		// Never propagate keystrokes if editable widgets are focused
		if ((isEditableWidget && event->keyval != GDK_Escape) || keyProcessed)
		{
			return keyProcessed;
		}
	}

	// Try to find a matching accelerator
	AcceleratorList accelList = self->findAccelerator(event);
	
	if (!accelList.empty())
	{
		// Release any modifiers
		self->_modifiers.setState(0);
		
		// Fake a "non-modifier" event to the MouseEvents class 
		GdkEventKey eventKey = *event;
		eventKey.state &= ~(GDK_MOD1_MASK|GDK_SHIFT_MASK|GDK_CONTROL_MASK);
		self->_mouseEvents.updateStatusText(&eventKey);
		
		// Pass the execute() call to all found accelerators
		for (AcceleratorList::iterator i = accelList.begin(); i != accelList.end(); ++i)
		{
			i->keyDown();
		}
		
		return true;
	}
	
	self->_modifiers.updateState(event, true);
	
	self->updateStatusText(event, true);
	
	return false;
}

gboolean EventManager::onKeyRelease(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	// Convert the passed pointer onto a KeyEventManager pointer
	EventManager* self = reinterpret_cast<EventManager*>(data);
	
	if (!GTK_IS_WIDGET(widget)) return FALSE;

	if (GTK_IS_WINDOW(widget))
	{
		// Pass the key event to the connected window and see if it can process it (returns TRUE)
		gboolean keyProcessed = gtk_window_propagate_key_event(GTK_WINDOW(widget), event);

		// Get the focus widget, is it an editable widget?
		GtkWidget* focus = gtk_window_get_focus(GTK_WINDOW(widget));
		bool isEditableWidget = GTK_IS_EDITABLE(focus) || GTK_IS_TEXT_VIEW(focus);

		// Never propagate keystrokes if editable widgets are focused
		if ((isEditableWidget && event->keyval != GDK_Escape) || keyProcessed)
		{
			return keyProcessed;
		}
	}

	// Try to find a matching accelerator
	AcceleratorList accelList = self->findAccelerator(event);
	
	if (!accelList.empty())
	{
		// Pass the execute() call to all found accelerators
		for (AcceleratorList::iterator i = accelList.begin(); i != accelList.end(); ++i)
		{
			i->keyUp();
		}
		
		return true;
	}
	
	self->_modifiers.updateState(event, false);
	
	self->updateStatusText(event, false);
	
	return false;
}

guint EventManager::getGDKCode(const std::string& keyStr) {
	guint returnValue = gdk_keyval_to_upper(gdk_keyval_from_name(keyStr.c_str()));
	
	if (returnValue == GDK_VoidSymbol) {
		globalWarningStream() << "EventManager: Could not recognise key " << keyStr << std::endl;
	}

	return returnValue;
}

bool EventManager::isModifier(GdkEventKey* event) {
	return (event->keyval == GDK_Control_L || event->keyval == GDK_Control_R || 
			event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R ||
			event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R ||
			event->keyval == GDK_Meta_L || event->keyval == GDK_Meta_R);
}

std::string EventManager::getGDKEventStr(GdkEventKey* event) {
	std::string returnValue("");
	
	// Don't react on modifiers only (no actual key like A, 2, U, etc.)
	if (isModifier(event)) {
		return returnValue;
	}
	
	// Convert the GDKEvent state into modifier flags
	const unsigned int modifierFlags = _modifiers.getKeyboardFlags(event->state);
	
	// Construct the complete string
	returnValue += _modifiers.getModifierStr(modifierFlags, true);
	returnValue += (returnValue != "") ? "-" : "";
	returnValue += gdk_keyval_name(gdk_keyval_to_upper(event->keyval));
	
	return returnValue;
}

extern "C" void DARKRADIANT_DLLEXPORT RegisterModule(IModuleRegistry& registry) {
	registry.registerModule(EventManagerPtr(new EventManager));
	
	// Initialise the streams using the given application context
	module::initialiseStreams(registry.getApplicationContext());
	
	// Remember the reference to the ModuleRegistry
	module::RegistryReference::Instance().setRegistry(registry);

	// Set up the assertion handler
	GlobalErrorHandler() = registry.getApplicationContext().getErrorHandlingFunction();
}
