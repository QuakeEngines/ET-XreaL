/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined (INCLUDED_ISCENEGRAPH_H)
#define INCLUDED_ISCENEGRAPH_H

#include <cstddef>
#include "imodule.h"
#include "inode.h"
#include "ipath.h"
#include <boost/function/function_fwd.hpp>

/**
 * \defgroup scenegraph Scenegraph
 * 
 * \namespace scene
 * \ingroup scenegraph
 * Interfaces and types relating to the scene-graph.
 */

// String identifier for the registry module
const std::string MODULE_SCENEGRAPH("SceneGraph");

class VolumeTest;

namespace scene
{

class ISpacePartitionSystem;
typedef boost::shared_ptr<ISpacePartitionSystem> ISpacePartitionSystemPtr;

/** 
* A scene-graph - a Directed Acyclic Graph (DAG).
*
* Each node may refer to zero or more 'child' nodes (directed).
* A node may never have itself as one of its ancestors (acyclic).
*/
class Graph :
	public RegisterableModule
{
public:
	typedef boost::function<void ()> BoundsChangedFunc;

	/* greebo: Derive from this class to get notified on scene changes 
	 */
	class Observer 
	{
	public:
		// destructor
		virtual ~Observer() {}

		// Gets called when anything in the scenegraph changes
		virtual void onSceneGraphChange() {}
		
		// Gets called when a new <node> is inserted into the scenegraph
		virtual void onSceneNodeInsert(const scene::INodePtr& node) {}
		
		// Gets called when <node> is removed from the scenegraph
		virtual void onSceneNodeErase(const scene::INodePtr& node) {} 
	};

	// Returns the root-node of the graph.
	virtual const INodePtr& root() const = 0;

	// Sets the root-node of the graph to be 'newRoot'.
	virtual void setRoot(const INodePtr& newRoot) = 0;

	// greebo: Adds a node to the scenegraph
	virtual void insert(const scene::INodePtr& node) = 0;
	// Removes a node from the scenegraph
	virtual void erase(const scene::INodePtr& node) = 0;

	/// \brief Invokes all scene-changed callbacks. Called when any part of the scene changes the way it will appear when the scene is rendered.
	/// \todo Move to a separate class.
	virtual void sceneChanged() = 0;
	/// \brief Add a \p callback to be invoked when the scene changes.
	/// \todo Move to a separate class.
	virtual void addSceneObserver(Observer* observer) = 0;
	// greebo: Remove the scene observer from the list
	virtual void removeSceneObserver(Observer* observer) = 0;

	/// \brief Invokes all bounds-changed callbacks. Called when the bounds of any instance in the scene change.
	/// \todo Move to a separate class.
	virtual void boundsChanged() = 0;

	/// \brief Add a \p callback to be invoked when the bounds of any instance in the scene change.
	virtual std::size_t addBoundsChangedCallback(const BoundsChangedFunc& callback) = 0;

	/// \brief Remove a \p callback to be invoked when the bounds of any instance in the scene change.
	virtual void removeBoundsChangedCallback(std::size_t handle) = 0;

	// A specific node has changed its bounds
	virtual void nodeBoundsChanged(const scene::INodePtr& node) = 0;

	// A walker class to be used in "foreachNodeInVolume"
	class Walker
	{
	public:
		virtual ~Walker() {}

		// Called for each visited node, returns TRUE if traversal should continue
		virtual bool visit(const INodePtr& node) = 0;
	};

	// Visit each scene node in the given volume using the given walker class, even hidden ones
	virtual void foreachNodeInVolume(const VolumeTest& volume, Walker& walker) = 0;

	// Same as above, but culls any hidden nodes
	virtual void foreachVisibleNodeInVolume(const VolumeTest& volume, Walker& walker) = 0;

	// Returns the associated spacepartition
	virtual ISpacePartitionSystemPtr getSpacePartition() = 0;
};

  class Cloneable
  {
  public:
    /// \brief destructor
    virtual ~Cloneable() {}
    /// \brief Returns a copy of itself.
    virtual scene::INodePtr clone() const = 0;
  };
  typedef boost::shared_ptr<Cloneable> CloneablePtr;
}

inline scene::Graph& GlobalSceneGraph() {
	// Cache the reference locally
	static scene::Graph& _sceneGraph(
		*boost::static_pointer_cast<scene::Graph>(
			module::GlobalModuleRegistry().getModule(MODULE_SCENEGRAPH)
		)
	);
	return _sceneGraph;
}

inline void SceneChangeNotify()
{
  GlobalSceneGraph().sceneChanged();
}

#endif
