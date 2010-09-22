#include "ToolbarManager.h"

#include <gtk/gtk.h>
#include <stdexcept>
#include "itextstream.h"
#include "ieventmanager.h"
#include "iregistry.h"
#include "iradiant.h"
#include "i18n.h"

#include <iostream>

namespace ui {

/* Load the definitions from the XMLRegistry
 */
void ToolbarManager::initialise() {
	try {
		// Query the registry
		loadToolbars();
	}
	catch (std::runtime_error& e) {
		std::cout << "ToolbarManager: Warning: " << e.what() << std::endl;
	}
}

/*	Returns the toolbar that is named toolbarName
 */
GtkToolbar* ToolbarManager::getToolbar(const std::string& toolbarName) {
	// Check if the toolbarName exists
	if (toolbarExists(toolbarName)) {
		
		// Instantiate the toolbar with buttons
		globalOutputStream() << "ToolbarManager: Instantiating toolbar: " << toolbarName << std::endl;
						
		// Build the path into the registry, where the toolbar should be found
		std::string toolbarPath = std::string("//ui//toolbar") + "[@name='"+ toolbarName +"']";
		xml::NodeList toolbarList = GlobalRegistry().findXPath(toolbarPath);
			
		if (toolbarList.size() > 0) {
			return createToolbar(toolbarList[0]);
		}
		else {
			globalErrorStream() << "ToolbarManager: Critical: Could not instantiate " << toolbarName << "!\n";
			return NULL;
		}		
	} 
	else {
		return NULL;
	}
}

/* Checks the passed xmlNode for a recognized item (ToolButton, ToggleToolButton, Separator)
 * Returns the widget or NULL if nothing useful is found   
 */
GtkWidget* ToolbarManager::createToolItem(xml::Node& node) {
	const std::string nodeName = node.getName();
	GtkWidget* toolItem;
	
	if (nodeName == "separator") {
		toolItem = GTK_WIDGET(gtk_separator_tool_item_new());			
	}
	else if (nodeName == "toolbutton" || nodeName == "toggletoolbutton") {
		// Found a button, load the values that are shared by both types
		const std::string name 		= node.getAttributeValue("name");
		const std::string icon 		= node.getAttributeValue("icon");
		const std::string tooltip 	= _(node.getAttributeValue("tooltip").c_str());
		const std::string action 	= node.getAttributeValue("action");
		
		if (nodeName == "toolbutton") {
			// Create a new GtkToolButton and assign the right callback
			toolItem = GTK_WIDGET(gtk_tool_button_new(NULL, name.c_str()));
		}
		else {
			// Create a new GtkToggleToolButton and assign the right callback
			toolItem = GTK_WIDGET(gtk_toggle_tool_button_new());
		}
		
		IEventPtr event = GlobalEventManager().findEvent(action);
			
		if (!event->empty()) {
			event->connectWidget(GTK_WIDGET(toolItem));

			// Tell the event to update the state of this button
			event->updateWidgets();
		}
		else {
			globalErrorStream() << "ToolbarManager: Failed to lookup command " << action << std::endl; 
		}
		
		// Set the tooltip, if not empty
		if (tooltip != "") {
			gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolItem), _tooltips, tooltip.c_str(), "");
		}
		
		// Load and assign the icon, if specified
		if (icon != "") {
			GtkWidget* image = gtk_image_new_from_pixbuf(GlobalUIManager().getLocalPixbufWithMask(icon));
			gtk_widget_show(image);
			gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(toolItem), image);
		}
	}
	else {
		return NULL;
	}
			
	gtk_widget_show(toolItem);
	return toolItem;
}

/*	Creates a toolbar based on the data found in the passed xmlNode
 * 	Returns the fully populated GtkToolbar 	
 */
GtkToolbar* ToolbarManager::createToolbar(xml::Node& node) {
	// Get all action children elements 
	xml::NodeList toolItemList = node.getChildren();
	GtkWidget* toolbar;
		
	if (!toolItemList.empty()) {
		// Create a new toolbar
		toolbar = gtk_toolbar_new();
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
		
		// Try to set the alignment, if the attribute is properly set
		std::string align = node.getAttributeValue("align");
		GtkOrientation orientation = (align == "vertical") ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
		gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar), orientation);
		
		for (std::size_t i = 0; i < toolItemList.size(); i++) {
			// Create and get the toolItem with the parsing 
			GtkWidget* toolItem = createToolItem(toolItemList[i]);
			
			// It is possible that no toolItem is returned, only add it if it's safe to do so
			if (toolItem != NULL) {    				
				gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(toolItem), -1);    				
			}
		}
	}
	else {
		throw std::runtime_error("No elements in toolbar.");
	}
	
	return GTK_TOOLBAR(toolbar);
}

bool ToolbarManager::toolbarExists(const std::string& toolbarName) {
	ToolbarList::iterator it = _toolbars.find(toolbarName);
   	return (it != _toolbars.end());
}

/* Parses the XML Document for toolbars and instantiates them
 * Returns nothing, toolbars can be obtained via GetToolbar()
 */
void ToolbarManager::loadToolbars() {
	xml::NodeList toolbarList = GlobalRegistry().findXPath("//ui//toolbar");
	
	if (!toolbarList.empty()) {
		// Create a new tooltips element
		_tooltips = gtk_tooltips_new();
		gtk_tooltips_enable(_tooltips);
		
		for (std::size_t i = 0; i < toolbarList.size(); i++) {
			std::string toolbarName = toolbarList[i].getAttributeValue("name");
			
			if (toolbarExists(toolbarName)) {
				//globalOutputStream() << "This toolbar already exists: ";
				continue;
			}
			
			globalOutputStream() << "Found toolbar: " << toolbarName << std::endl;
			
			_toolbars.insert(toolbarName);
		}
	}
	else {
		throw std::runtime_error("No toolbars found.");
	}
}
	
} // namespace ui
