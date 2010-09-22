#ifndef SELECTION_ALGORITHM_ENTITY_H_
#define SELECTION_ALGORITHM_ENTITY_H_

#include "icommandsystem.h"
#include <string>

namespace selection {
	namespace algorithm {

	/**
	 * greebo: Changes the classname of the currently selected entities.
	 */
	void setEntityClassname(const std::string& classname);

	/** 
	 * greebo: "Binds" the selected entites together by setting the "bind"
	 * spawnarg on both entities. Two entities must be highlighted for this
	 * command to function correctly.
	 */
	void bindEntities(const cmd::ArgumentList& args);

	/** 
	 * greebo: Sets up the target spawnarg of the selected entities such that
	 * the first selected entity is targetting the second.
	 */
	void connectSelectedEntities(const cmd::ArgumentList& args);

	} // namespace algorithm
} // namespace selection

#endif /* SELECTION_ALGORITHM_ENTITY_H_ */
