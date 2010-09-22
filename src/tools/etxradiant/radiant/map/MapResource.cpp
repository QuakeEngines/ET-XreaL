#include "MapResource.h"

#include "i18n.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include "ifiletypes.h"
#include "ifilesystem.h"
#include "imainframe.h"
#include "iregistry.h"
#include "map/Map.h"
#include "map/RootNode.h"
#include "mapfile.h"
#include "gtkutil/dialog.h"
#include "referencecache/NullModelLoader.h"
#include "debugging/debugging.h"
#include "os/path.h"
#include "os/file.h"
#include "MapImportInfo.h"
#include "map/algorithm/Traverse.h"
#include "stream/textfilestream.h"
#include "referencecache/NullModelNode.h"
#include "MapExportInfo.h"
#include <boost/bind.hpp>
#include <boost/format.hpp>

namespace map {

namespace {
	// name may be absolute or relative
	inline std::string rootPath(const std::string& name) {
		return GlobalFileSystem().findRoot(
			path_is_absolute(name.c_str()) ? name : GlobalFileSystem().findFile(name)
		);
	}
}

// Constructor
MapResource::MapResource(const std::string& name) :
	_mapRoot(model::NullModelNode::InstancePtr()),
	_originalName(name),
	_type(name.substr(name.rfind(".") + 1)), 
	_modified(0),
	_realised(false)
{
	// Initialise the paths, this is all needed for realisation
    _path = rootPath(_originalName);
	_name = os::getRelativePath(_originalName, _path);
}
	
MapResource::~MapResource() {
    if (realised()) {
		unrealise();
	}
}

void MapResource::rename(const std::string& fullPath) {
	// Save the paths locally and split them into parts
	_originalName = fullPath;
	_type = fullPath.substr(fullPath.rfind(".") + 1);
	_path = rootPath(_originalName);
	_name = os::getRelativePath(_originalName, _path);

	// Rename the map root as well
	if (_mapRoot != NULL && boost::dynamic_pointer_cast<RootNode>(_mapRoot) != NULL) {
		boost::static_pointer_cast<RootNode>(_mapRoot)->setName(_name);
	}
}

bool MapResource::load() {
	ASSERT_MESSAGE(realised(), "resource not realised");
	if (_mapRoot == model::NullModelNode::InstancePtr()) {
		// Map not loaded yet, acquire map root node from loader
		_mapRoot = loadMapNode();
		
		connectMap();
		mapSave();
	}

	return _mapRoot != model::NullModelNode::InstancePtr();
}
  
/**
 * Save this resource (only for map resources).
 * 
 * @returns
 * true if the resource was saved, false otherwise.
 */
bool MapResource::save() {
	std::string moduleName = GlobalFiletypes().findModuleName("map", _type);
								
	if (!moduleName.empty()) {
		MapFormatPtr format = boost::dynamic_pointer_cast<MapFormat>(
			module::GlobalModuleRegistry().getModule(moduleName)
		);
		
		if (format == NULL) {
			globalErrorStream() << "Could not locate map loader module." << std::endl;
			return false;
		}
	
		std::string fullpath = _path + _name;

		// Save a backup of the existing file (rename it to .bak) if it exists in the first place
		if (file_exists(fullpath.c_str())) {
			if (!saveBackup()) {
				// angua: if backup creation is not possible, still save the map
				// but create message in the console
				globalErrorStream() << "Could not create backup (Map is possibly open in Doom3)" << std::endl;
				// return false;
			}
		}
		
		bool success = false;
		
		if (path_is_absolute(fullpath.c_str()))
		{
			// Save the actual file
			success = saveFile(*format, _mapRoot, map::traverse, fullpath);
		}
		else {
			globalErrorStream() << "Map path is not absolute: " << fullpath << std::endl;
			success = false;
		}
		
		if (success) {
  			mapSave();
  			return true;
		}
	}
	
	return false;
}

bool MapResource::saveBackup() {
	std::string fullpath = _path + _name;
	
	if (path_is_absolute(fullpath.c_str())) {
		// Save a backup if possible. This is done by renaming the original,
		// which won't work if the existing map is currently open by Doom 3
		// in the background.
		if (!file_exists(fullpath.c_str())) {
			return false;
		}
		
		if (file_writeable(fullpath.c_str())) {
			std::string pathWithoutExtension = fullpath.substr(0, fullpath.rfind('.'));
			std::string backup = pathWithoutExtension + ".bak";
			
			return (!file_exists(backup.c_str()) || file_remove(backup.c_str())) // remove backup
				&& file_move(fullpath.c_str(), backup.c_str()); // rename current to backup
		}
		else {
			globalErrorStream() << "map path is not writeable: " << fullpath << std::endl;
			// File is write-protected
			gtkutil::errorDialog(
				(boost::format(_("File is write-protected: %s")) % fullpath).str(), 
				GlobalMainFrame().getTopLevelWindow());
			return false;
		}
	}
	return false;
}

scene::INodePtr MapResource::getNode() {
	return _mapRoot;
}

void MapResource::setNode(scene::INodePtr node) {
	_mapRoot = node;
	connectMap();
}
	
void MapResource::addObserver(Observer& observer) {
	if (realised()) {
		observer.onResourceRealise();
	}
	_observers.insert(&observer);
}
	
void MapResource::removeObserver(Observer& observer) {
	if (realised()) {
		observer.onResourceUnrealise();
	}
	_observers.erase(&observer);
}
		
bool MapResource::realised() {
	return _realised;
}
  
// Realise this MapResource
void MapResource::realise() {
	if (_realised) {
		return; // nothing to do
	}
	
	_realised = true;

	// Realise the observers
	for (ResourceObserverList::iterator i = _observers.begin();
		 i != _observers.end(); i++)
	{
		(*i)->onResourceRealise();
	}
}
	
void MapResource::unrealise() {
	if (!_realised) {
		return; // nothing to do
	}
	
	_realised = false;
	
	// Realise the observers
	for (ResourceObserverList::iterator i = _observers.begin(); 
		 i != _observers.end(); i++)
	{
		(*i)->onResourceUnrealise();
	}

	//globalOutputStream() << "MapResource::unrealise: " << _path.c_str() << _name.c_str() << "\n";
	_mapRoot = model::NullModelNode::InstancePtr();
}

void MapResource::onMapChanged() {
	GlobalMap().setModified(true);
}

void MapResource::connectMap() {
    MapFilePtr map = Node_getMapFile(_mapRoot);
    if (map != NULL) {
    	// Reroute the changed callback to the onMapChanged() call.
		map->setChangedCallback(boost::bind(&MapResource::onMapChanged, this));
    }
}

std::time_t MapResource::modified() const {
	std::string fullpath = _path + _name;
	return file_modified(fullpath.c_str());
}

void MapResource::mapSave() {
	_modified = modified();
	MapFilePtr map = Node_getMapFile(_mapRoot);
	if (map != NULL) {
		map->save();
	}
}

bool MapResource::isModified() const {
	// had or has an absolute path // AND disk timestamp changed
	return (!_path.empty() && _modified != modified()) 
			|| !path_equal(rootPath(_originalName).c_str(), _path.c_str()); // OR absolute vfs-root changed
}

void MapResource::reload() {
    unrealise();
	realise();
}

MapFormatPtr MapResource::getMapFormat()
{
	// Get a loader module name for this type, if possible. If none is 
	// found, try again with the "map" type, since we might be loading a 
	// map with a different extension
    std::string moduleName = GlobalFiletypes().findModuleName("map", _type);
    
	// If empty, try again with "map" type
	if (moduleName.empty()) {
		moduleName = GlobalFiletypes().findModuleName("map", "map"); 
	}

	// If we have a module, use it to load the map if possible, otherwise 
	// return an error
    if (!moduleName.empty()) {
		MapFormatPtr format = boost::dynamic_pointer_cast<MapFormat>(
			module::GlobalModuleRegistry().getModule(moduleName)
		);

		if (format != NULL) {
			// valid MapFormat, return
			return format;
		} 
		else {
			globalErrorStream() << "ERROR: Map type incorrectly registered: \""
				<< moduleName << "\"" << std::endl;
			return MapFormatPtr();
		}
	} 
    else
	{
		globalErrorStream() << "Map loader module not found." << std::endl;

		if (!_type.empty())
		{
			globalErrorStream() << "Type is not supported: \""
				<< _name << "\"" << std::endl;
		}
		return MapFormatPtr();
	}
}

scene::INodePtr MapResource::loadMapNode() {
	// greebo: Check if we have valid settings
	// The _path might be empty if we're loading from a folder outside the mod
	if (_name.empty() && _type.empty()) {
		return model::NullModelNode::InstancePtr();
	}
	
	// Get the mapformat
	MapFormatPtr format = getMapFormat();
	
	if (format == NULL) {
		return model::NullModelNode::InstancePtr(); 
		// error message already printed in getMapFormat();
	}
	
	// At this point, we have a valid mapformat
	// Create a new map root node
	scene::INodePtr root(NewMapRoot(_name));

  	std::string fullpath = _path + _name;

	if (path_is_absolute(fullpath.c_str())) {

		if (loadFile(*format, root, fullpath))
		{
			return root;
		}
		else
		{
			gtkutil::errorDialog(
				(boost::format(_("Failure reading read map file:\n%s")) % fullpath).str(),
				GlobalMainFrame().getTopLevelWindow());
		}
	}
	else {
		globalErrorStream() << "map path is not fully qualified: " << fullpath << std::endl;
	}

	// Return the NULL node on failure
	return model::NullModelNode::InstancePtr();
}

bool MapResource::loadFile(const MapFormat& format, const scene::INodePtr& root, const std::string& filename)
{
	globalOutputStream() << "Open file " << filename << " for read...";

	TextFileInputStream file(filename);
	std::istream mapStream(&file);

	std::string infoFilename(filename.substr(0, filename.rfind('.')));
	infoFilename += GlobalRegistry().get(RKEY_INFO_FILE_EXTENSION);

	std::ifstream infoFileStream(infoFilename.c_str());

	if (infoFileStream.is_open()) {
		globalOutputStream() << " found information file... ";
	}

	if (!file.failed()) {
		globalOutputStream() << "success" << std::endl;

		// Create an import information structure
		if (infoFileStream.is_open()) {
			// Infostream is open, call the MapFormat
			MapImportInfo importInfo(mapStream, infoFileStream);
			importInfo.root = root;

			return format.readGraph(importInfo);
		}
		else {
			// No active infostream, pass a dummy stream
			std::istringstream emptyStream;
			MapImportInfo importInfo(mapStream, emptyStream);
			importInfo.root = root;

			return format.readGraph(importInfo);
		}
	}
	else
	{
		globalErrorStream() << "failure" << std::endl;
		return false;
	}
}

bool MapResource::saveFile(const MapFormat& format, const scene::INodePtr& root, 
						   GraphTraversalFunc traverse, const std::string& filename)
{
	globalOutputStream() << "Open file " << filename << " ";
	
	if (file_exists(filename.c_str()) && !file_writeable(filename.c_str()))
	{
		// File is write-protected
		globalErrorStream() << "failure, file is write-protected." << std::endl;
		gtkutil::errorDialog(
			(boost::format(_("File is write-protected: %s")) % filename).str(), 
			GlobalMainFrame().getTopLevelWindow());
		return false;
	}

	// Open the stream to the output file
	std::ofstream outfile(filename.c_str());

	// Open the auxiliary file too
	std::string auxFilename(filename);
	auxFilename = auxFilename.substr(0, auxFilename.rfind('.'));
	auxFilename += GlobalRegistry().get(map::RKEY_INFO_FILE_EXTENSION);

	globalOutputStream() << "and auxiliary file " << auxFilename << " for write...";

	if (file_exists(auxFilename.c_str()) && !file_writeable(auxFilename.c_str())) {
		// File is write-protected
		globalErrorStream() << "failure, file is write-protected." << std::endl;
		gtkutil::errorDialog(
			(boost::format(_("File is write-protected: %s")) % auxFilename).str(),
			GlobalMainFrame().getTopLevelWindow());
		return false;
	}

	std::ofstream auxfile(auxFilename.c_str());

	if (outfile.is_open() && auxfile.is_open()) {
		globalOutputStream() << "success" << std::endl;

		map::MapExportInfo exportInfo(outfile, auxfile);
		exportInfo.traverse = traverse;
		exportInfo.root = root;
	    
		// Let the map exporter module do its job
	    format.writeGraph(exportInfo);
	    
	    outfile.close();
		auxfile.close();
	    return true;
	}
	else {
		globalErrorStream() << "failure" << std::endl;
		return false;
	}
}

} // namespace map
