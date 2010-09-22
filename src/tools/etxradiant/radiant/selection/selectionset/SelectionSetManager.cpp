#include "SelectionSetManager.h"

#include "itextstream.h"
#include "iradiant.h"
#include "iselection.h"
#include "ieventmanager.h"
#include "imainframe.h"
#include "modulesystem/StaticModule.h"

#include <gtk/gtktoolbar.h>
#include <gtk/gtkseparatortoolitem.h>

#include <boost/bind.hpp>

namespace selection
{

const std::string& SelectionSetManager::getName() const
{
	static std::string _name("SelectionSetManager");
	return _name;
}

const StringSet& SelectionSetManager::getDependencies() const
{
	static StringSet _dependencies;
	
	if (_dependencies.empty())
	{
		_dependencies.insert(MODULE_SELECTIONSYSTEM);
		_dependencies.insert(MODULE_EVENTMANAGER);
		_dependencies.insert(MODULE_COMMANDSYSTEM);
		_dependencies.insert(MODULE_RADIANT);
	}
	
	return _dependencies;
}

void SelectionSetManager::initialiseModule(const ApplicationContext& ctx)
{
	globalOutputStream() << getName() << "::initialiseModule called." << std::endl;
	
	// Register for the startup event
	GlobalRadiant().addEventListener(shared_from_this());

	GlobalCommandSystem().addCommand("DeleteAllSelectionSets", 
		boost::bind(&SelectionSetManager::deleteAllSelectionSets, this, _1));

	GlobalEventManager().addCommand("DeleteAllSelectionSets", "DeleteAllSelectionSets");
}

void SelectionSetManager::shutdownModule()
{
	_selectionSets.clear();
}

void SelectionSetManager::onRadiantStartup()
{
	// Get the horizontal toolbar and add a custom widget
	GtkToolbar* toolbar = GlobalMainFrame().getToolbar(IMainFrame::TOOLBAR_HORIZONTAL);

	// Insert a separator at the end of the toolbar
	GtkToolItem* item = GTK_TOOL_ITEM(gtk_separator_tool_item_new());
	gtk_toolbar_insert(toolbar, item, -1);

	gtk_widget_show(GTK_WIDGET(item));

	// Construct a new tool menu object
	_toolmenu = SelectionSetToolmenuPtr(new SelectionSetToolmenu);

	gtk_toolbar_insert(toolbar, _toolmenu->getToolItem(), -1);	
}

void SelectionSetManager::addObserver(Observer& observer)
{
	_observers.insert(&observer);
}

void SelectionSetManager::removeObserver(Observer& observer)
{
	_observers.erase(&observer);
}

void SelectionSetManager::notifyObservers()
{
	for (Observers::iterator i = _observers.begin(); i != _observers.end(); )
	{
		(*i++)->onSelectionSetsChanged();
	}
}

void SelectionSetManager::foreachSelectionSet(Visitor& visitor)
{
	for (SelectionSets::const_iterator i = _selectionSets.begin(); i != _selectionSets.end(); )
	{
		visitor.visit((i++)->second);
	}
}

ISelectionSetPtr SelectionSetManager::createSelectionSet(const std::string& name)
{
	SelectionSets::iterator i = _selectionSets.find(name);

	if (i == _selectionSets.end())
	{
		// Create new set
		std::pair<SelectionSets::iterator, bool> result = _selectionSets.insert(
			SelectionSets::value_type(name, SelectionSetPtr(new SelectionSet(name))));

		i = result.first;

		notifyObservers();
	}

	return i->second;
}

void SelectionSetManager::deleteSelectionSet(const std::string& name)
{
	SelectionSets::iterator i = _selectionSets.find(name);

	if (i != _selectionSets.end())
	{
		_selectionSets.erase(i);

		notifyObservers();
	}
}

void SelectionSetManager::deleteAllSelectionSets()
{
	_selectionSets.clear();
	notifyObservers();
}

void SelectionSetManager::deleteAllSelectionSets(const cmd::ArgumentList& args)
{
	deleteAllSelectionSets();
}

ISelectionSetPtr SelectionSetManager::findSelectionSet(const std::string& name)
{
	SelectionSets::iterator i = _selectionSets.find(name);

	return (i != _selectionSets.end()) ? i->second : ISelectionSetPtr();
}

// Define the static SelectionSetManager module
module::StaticModule<SelectionSetManager> selectionSetManager;

} // namespace
