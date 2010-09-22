#include "Entity.h"

#include "i18n.h"
#include "selectionlib.h"
#include "imainframe.h"
#include "iregistry.h"
#include "itextstream.h"
#include "entitylib.h"
#include "gtkutil/dialog.h"

#include "../../entity.h"

namespace selection {
	namespace algorithm {

const std::string RKEY_BIND_KEY("game/defaults/bindKey");

/**
 * greebo: This walker traverses a subgraph and changes the classname
 *         of all selected entities to the one passed to the constructor.
 */
class EntitySetClassnameSelected : 
	public SelectionSystem::Visitor
{
	std::string _classname;

	// Entites are getting accumulated in this list and processed at destruction time
	mutable std::set<scene::INodePtr> _entities;
public:
	EntitySetClassnameSelected(const std::string& classname) :
		_classname(classname)
	{}

	~EntitySetClassnameSelected() {
		for (std::set<scene::INodePtr>::iterator i = _entities.begin();
			 i != _entities.end(); ++i)
		{
			// "Rename" the entity, this deletes the old node and creates a new one
			scene::INodePtr newNode = changeEntityClassname(*i, _classname);

			// Select the new entity node
			Node_setSelected(newNode, true);
		}
	}

	virtual void visit(const scene::INodePtr& node) const {
		// Check if we have an entity
		Entity* entity = Node_getEntity(node);

		if (entity != NULL && Node_isSelected(node)) {
			if (entity->getKeyValue("classname") != "worldspawn") {
				_entities.insert(node);
			}
			else {
				gtkutil::errorDialog(
					_("Cannot change classname of worldspawn entity."),
					GlobalMainFrame().getTopLevelWindow());
			}
		}
	}
};

void setEntityClassname(const std::string& classname) {

	if (classname.empty()) {
		globalErrorStream() << "Cannot set classname to an empty string!" << std::endl;
	}

	// greebo: instantiate a walker and traverse the current selection
	EntitySetClassnameSelected classnameSetter(classname);
	GlobalSelectionSystem().foreachSelected(classnameSetter);

	// The destructor of the classNameSetter will rename the entities
}

void bindEntities(const cmd::ArgumentList& args) {
	const SelectionInfo& info = GlobalSelectionSystem().getSelectionInfo();

	if (info.totalCount == 2 && info.entityCount == 2) {
		UndoableCommand command("bindEntities");

		Entity* first = Node_getEntity(GlobalSelectionSystem().ultimateSelected());
		Entity* second = Node_getEntity(GlobalSelectionSystem().penultimateSelected());

		if (first != NULL && second != NULL) {
			// Get the bind key
			std::string bindKey = GlobalRegistry().get(RKEY_BIND_KEY);

			if (bindKey.empty()) {
				// Fall back to a safe default
				bindKey = "bind";
			}

			// Set the spawnarg
			second->setKeyValue(bindKey, first->getKeyValue("name"));
		}
		else {
			gtkutil::errorDialog(
				_("Critical: Cannot find selected entities."),
				GlobalMainFrame().getTopLevelWindow());
		}
	}
	else {
		gtkutil::errorDialog(
			_("Exactly two entities must be selected for this operation."),
			GlobalMainFrame().getTopLevelWindow());
	}
}

void connectSelectedEntities(const cmd::ArgumentList& args)
{
	if (GlobalSelectionSystem().countSelected() == 2)
	{
		GlobalEntityCreator().connectEntities(
			GlobalSelectionSystem().penultimateSelected(),	// source
			GlobalSelectionSystem().ultimateSelected()		// target
		);
	}
	else
	{
		gtkutil::errorDialog(
			_("Exactly two entities must be selected for this operation."),
			GlobalMainFrame().getTopLevelWindow());
	}
}

	} // namespace algorithm
} // namespace selection
