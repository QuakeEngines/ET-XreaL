#include "TargetableNode.h"

#include "TargetManager.h"
#include "RenderableTargetInstances.h"

namespace entity {

TargetableNode::TargetableNode(Doom3Entity& entity, scene::Node& node) :
	_d3entity(entity),
	_renderableLines(_targetKeys),
	_node(node)
{
	// Note: don't do anything with _d3Entity here,
	// the structure is not fully constructed yet at this point.
	// Execute initialisation code in construct()
}

void TargetableNode::construct() {
	_d3entity.attachObserver(this);
	_d3entity.attachObserver(&_targetKeys);

	RenderableTargetInstances::Instance().attach(*this);
}

// Disconnect this class from the entity
void TargetableNode::destruct() {
	RenderableTargetInstances::Instance().detach(*this);

	_d3entity.detachObserver(&_targetKeys);
	_d3entity.detachObserver(this);
}

void TargetableNode::setTargetsChanged(const Callback& targetsChanged) {
	_targetKeys.setTargetsChanged(targetsChanged);
}

void TargetableNode::targetsChanged() {
	_targetKeys.targetsChanged();
}

// Gets called as soon as the "name" keyvalue changes
void TargetableNode::targetnameChanged(const std::string& name) {
	// Check if we were registered before
	if (!_targetName.empty()) {
		// Old name is not empty
		// Tell the Manager to disassociate us from the target
		TargetManager::Instance().clearTarget(_targetName, _node);
	}
	
	// Store the new name, in any case
	_targetName = name;

	if (_targetName.empty()) {
		// New name is empty, do not associate
		return;
	}

	// Tell the TargetManager to associate the name with this scene::INode here
	TargetManager::Instance().associateTarget(_targetName, _node);
}

// Entity::Observer implementation, gets called on key insert
void TargetableNode::onKeyInsert(const std::string& key, EntityKeyValue& value) {
	if (key == "name") {
		// Subscribe to this keyvalue to get notified about "name" changes
		value.attach(TargetnameChangedCaller(*this));
	}
}

// Entity::Observer implementation, gets called on key erase
void TargetableNode::onKeyErase(const std::string& key, EntityKeyValue& value) {
	if (key == "name") {
		// Unsubscribe from this keyvalue
		value.detach(TargetnameChangedCaller(*this));
	}
}

const Vector3& TargetableNode::getWorldPosition() const {
	const AABB& bounds = _node.worldAABB();
	if (bounds.isValid()) {
		return bounds.getOrigin();
	}
	return _node.localToWorld().t().getVector3();
}

void TargetableNode::render(RenderableCollector& collector, const VolumeTest& volume) const {
	if (!_node.visible()) return;

	collector.SetState(_d3entity.getEntityClass()->getWireShader(), RenderableCollector::eWireframeOnly);
	collector.SetState(_d3entity.getEntityClass()->getWireShader(), RenderableCollector::eFullMaterials);
	_renderableLines.render(collector, volume, getWorldPosition());
}

} // namespace entity
