#include "MapPositionManager.h"

#include "ieventmanager.h"
#include "iregistry.h"
#include "itextstream.h"
#include "icommandsystem.h"
#include "map/Map.h"
#include "entitylib.h"
#include "string/string.h"
#include <boost/bind.hpp>

namespace map {
	
	namespace {
		const std::string SAVE_COMMAND_ROOT = "SavePosition";
		const std::string LOAD_COMMAND_ROOT = "LoadPosition";
		
		unsigned int MAX_POSITIONS = 10;
	}

// Constructor
MapPositionManager::MapPositionManager()
{}

void MapPositionManager::initialise() {
	// Create the MapPosition objects and add the commands to the eventmanager
	for (unsigned int i = 1; i <= MAX_POSITIONS; i++) {
		// Allocate a new MapPosition object and store the shared_ptr
		_positions[i] = MapPositionPtr(new MapPosition(i));
					
		// Add the load/save commands to the eventmanager and point it to the member
		GlobalCommandSystem().addCommand(
			SAVE_COMMAND_ROOT + intToStr(i),
			boost::bind(&MapPosition::store, _positions[i].get(), _1)
		);
		GlobalCommandSystem().addCommand(
			LOAD_COMMAND_ROOT + intToStr(i),
			boost::bind(&MapPosition::recall, _positions[i].get(), _1)
		);

		GlobalEventManager().addCommand(
			SAVE_COMMAND_ROOT + intToStr(i),
			SAVE_COMMAND_ROOT + intToStr(i)
		);
		
		GlobalEventManager().addCommand(
			LOAD_COMMAND_ROOT + intToStr(i),
			LOAD_COMMAND_ROOT + intToStr(i)
		);
	}

}

void MapPositionManager::loadPositions() {
	// Find the worldspawn node
	Entity* worldspawn = Scene_FindEntityByClass("worldspawn");
	
	if (worldspawn != NULL) {
		for (unsigned int i = 1; i <= MAX_POSITIONS; i++) {
			if (_positions[i] != NULL) {
				_positions[i]->load(worldspawn);
			}
		}
	}
	else {
		globalErrorStream() << "MapPositionManager: Could not locate worldspawn entity.\n";
	}
}

void MapPositionManager::savePositions() {
	// Find the worldspawn node
	Entity* worldspawn = Scene_FindEntityByClass("worldspawn");
	
	for (unsigned int i = 1; i <= MAX_POSITIONS; i++) {
		if (_positions[i] != NULL) {
			_positions[i]->save(worldspawn);
		}
	}
}

void MapPositionManager::removePositions() {
	// Find the worldspawn node
	Entity* worldspawn = Scene_FindEntityByClass("worldspawn");
	
	for (unsigned int i = 1; i <= MAX_POSITIONS; i++) {
		if (_positions[i] != NULL) {
			_positions[i]->remove(worldspawn);
		}
	}
}

// The home of the static instance
MapPositionManager& GlobalMapPosition() {
	static MapPositionManager _instance;
	return _instance;
}

} // namespace map
