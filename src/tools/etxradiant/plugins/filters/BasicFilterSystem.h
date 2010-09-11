#ifndef BASICFILTERSYSTEM_H_
#define BASICFILTERSYSTEM_H_

#include "XMLFilter.h"
#include "imodule.h"
#include "ifilter.h"
#include "xmlutil/Node.h"

#include <map>
#include <vector>
#include <string>
#include <iostream>

namespace filters
{

/** FilterSystem implementation class.
 */

class BasicFilterSystem 
: public FilterSystem
{
	// Hashtable of available filters, indexed by name
	typedef std::map<std::string, XMLFilter> FilterTable;
	FilterTable _availableFilters;
	
	// Second table containing just the active filters
	FilterTable _activeFilters;

	// Cache of visibility flags for item names, to avoid having to
	// traverse the active filter list for each lookup
	typedef std::map<std::string, bool> StringFlagCache;
	StringFlagCache _visibilityCache;

	typedef std::set<FilterSystem::ObserverPtr> ObserverList;
	ObserverList _observers;

private:
	
	// Perform a traversal of the scenegraph, setting or clearing the filtered
	// flag on Nodes depending on their entity class
	void updateScene();

	void updateShaders();

	void updateEvents();

	void addFiltersFromXML(const xml::NodeList& nodes, bool readOnly);

	// Notifies all observers about a change
	void notifyObservers();
	
public:
    virtual ~BasicFilterSystem() {}

	void addObserver(const ObserverPtr& observer);
	void removeObserver(const ObserverPtr& observer);

	// Invoke the InstanceUpateWalker to update the filtered status.
	void update();

	// Updates the given subgraph
	void updateSubgraph(const scene::INodePtr& root);

	// Filter system visit function
	void forEachFilter(IFilterVisitor& visitor);
	
	std::string getFilterEventName(const std::string& filter);
	
	bool getFilterState(const std::string& filter) {
		return (_activeFilters.find(filter) != _activeFilters.end());
	}
	
	// Set the state of a filter
	void setFilterState(const std::string& filter, bool state);

	// Query whether an item is visible or filtered out
	bool isVisible(const std::string& item, const std::string& name);

	// Whether this filter is read-only and can't be changed
	bool filterIsReadOnly(const std::string& filter);

	// Adds a new filter to the system
	bool addFilter(const std::string& filterName, const FilterRules& ruleSet);

	// Removes the filter and returns true on success
	bool removeFilter(const std::string& filter);

	// Renames the filter and updates eventmanager
	bool renameFilter(const std::string& oldFilterName, const std::string& newFilterName);

	// Returns the ruleset of the named filter
	FilterRules getRuleSet(const std::string& filter);

	// Applies the ruleset and replaces the previous one for a given filter.
	bool setFilterRules(const std::string& filter, const FilterRules& ruleSet);
	
	// RegisterableModule implementation
	virtual const std::string& getName() const;
	virtual const StringSet& getDependencies() const;
	virtual void initialiseModule(const ApplicationContext& ctx);
	virtual void shutdownModule();
};
typedef boost::shared_ptr<BasicFilterSystem> BasicFilterSystemPtr;

} // namespace filters

#endif /*BASICFILTERSYSTEM_H_*/
