#ifndef XMLREGISTRY_H_
#define XMLREGISTRY_H_

/*	This is the implementation of the XMLRegistry structure providing easy methods to store 
 * 	all kinds of information like ui state, toolbar structures and anything that fits into an XML file. 
 * 
 * 	This is the actual implementation of the abstract base class defined in iregistry.h.
 * 
 *  Note: You need to include iregistry.h in order to use the Registry like in the examples below.
 *  
 *  Example: store a global variable:
 *  	GlobalRegistry().set("user/ui/showAllLightRadii", "1");
 * 
 *  Example: retrieve a global variable 
 *  (this returns "" if the key is not found and an error is written to globalOutputStream):
 *  	std::string value = GlobalRegistry().get("user/ui/showalllightradii");
 * 
 *  Example: import an XML file into the registry (note: imported keys overwrite previous ones!) 
 * 		GlobalRegistry().importFromFile(absolute_path_to_file[, where_to_import]);
 * 
 *  Example: export a path/key to a file:
 *  	GlobalRegistry().exportToFile(node_to_export, absolute_path_to_file);
 */

#include "iregistry.h"		// The Abstract Base Class
#include <map>

#include "imodule.h"
#include "RegistryTree.h"

class XMLRegistry : 
	public Registry
{
	// The map of RegistryKeyObservers. The same observer can observe several keys, and
	// the same key can be observed by several observers, hence the multimap. 
	typedef std::multimap<const std::string, RegistryKeyObserver*> KeyObserverMap;

private:
	// The default import node and toplevel node
	std::string _topLevelNode;

	// The map with all the keyobservers that are currently connected
	KeyObserverMap _keyObservers;
	
	// The "install" tree, is basically treated as read-only
	RegistryTree _standardTree;
	
	// The "user" tree, this is where all the run-time changes go
	// Note: this tree is queried first for a given key
	RegistryTree _userTree;
	
	// The query counter for some statistics :)
	unsigned int _queryCounter;

public:
	/* Constructor: 
	 * Creates two empty RegistryTrees in the memory with the default toplevel node 
	 */
	XMLRegistry();

	// The destructor exports all user settings to .xml files
	virtual ~XMLRegistry();

	xml::NodeList findXPath(const std::string& path);

	/*	Checks whether a key exists in the XMLRegistry by querying the XPath
	 */
	bool keyExists(const std::string& key);

	/* Deletes this key and all its children, 
	 * this includes multiple instances nodes matching this key 
	 */ 
	void deleteXPath(const std::string& path);
	
	//	Adds a key <key> as child to <path> to the XMLRegistry (with the name attribute set to <name>)
	xml::Node createKeyWithName(const std::string& path, const std::string& key, const std::string& name);
	
	/*	Adds a key to the XMLRegistry (without value, just the node)
	 *  All required parent nodes are created automatically, if they don't exist */
	xml::Node createKey(const std::string& key);

	// Set the value of the given attribute at the specified <path>.
	void setAttribute(const std::string& path, 
					  const std::string& attrName, 
					  const std::string& attrValue);

	// Loads the string value of the given attribute of the node at <path>.
	std::string getAttribute(const std::string& path, const std::string& attrName);
	
	// Gets a key from the registry, user tree overrides default tree
	std::string get(const std::string& key);
	
	/* Gets a key containing a float from the registry, basically loads the string and
	 * converts it into a float via boost libraries */
	float getFloat(const std::string& key);
	
	/* Sets a registry key value to the given float. The floating point variable
	 * is converted via boost libraries first. */
	void setFloat(const std::string& key, const double value);
	
	/* Gets a key containing an integer from the registry, basically loads the string and
	 * converts it into an int via boost libraries */
	int getInt(const std::string& key);
	
	// Sets a registry key value to the given integer. The value is converted via boost libraries first.
	void setInt(const std::string& key, const int value);
	
	// Sets the value of a key from the registry, 
	void set(const std::string& key, const std::string& value);

	// Get the boolean interpretation of the given keyvalue
	bool getBool(const std::string& key);

	// Set the value as boolean (true will be "1")
	void setBool(const std::string& key, const bool value);
	
	/* Appends a whole (external) XML file to the XMLRegistry. The toplevel nodes of this file
	 * are appended to _topLevelNode (e.g. <darkradiant>) if parentKey is set to the empty string "", 
	 * otherwise they are imported as a child of the specified parentKey. Choose the target tree by
	 * passing the correct enum value (e.g. treeUser for the user tree)
	 */
	void import(const std::string& importFilePath, const std::string& parentKey, Tree tree);
	
	// Dumps the current registry to std::out, for debugging purposes
	void dump() const;
	
	/* Saves a specified path from the user tree to the file <filename>. 
	 * Use "-" as <filename> if you want to write to std::out.
	 */
	void exportToFile(const std::string& key, const std::string& filename);
	
	// Add an observer watching the <observedKey> to the internal list of observers. 
	void addKeyObserver(RegistryKeyObserver* observer, const std::string& observedKey);
	
	// Removes an observer watching the <observedKey> from the internal list of observers. 
	void removeKeyObserver(RegistryKeyObserver* observer);

	// RegisterableModule implementation
	virtual const std::string& getName() const;
	virtual const StringSet& getDependencies() const;
	virtual void initialiseModule(const ApplicationContext& ctx);

private:
	// Cycles through the key observers and notifies the ones that observe the given <changedKey>
	void notifyKeyObservers(const std::string& changedKey, const std::string& newVal);
};
typedef boost::shared_ptr<XMLRegistry> XMLRegistryPtr;

#endif /* XMLREGISTRY_H_ */
