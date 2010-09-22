/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

#include "entity.h"

#include "i18n.h"
#include "ieventmanager.h"
#include "icommandsystem.h"
#include "ientity.h"
#include "iregistry.h"
#include "ieclass.h"
#include "iselection.h"
#include "imodel.h"
#include "ifilesystem.h"
#include "iundo.h"
#include "editable.h"

#include "scenelib.h"
#include "os/path.h"
#include "os/file.h"

#include "gtkutil/FileChooser.h"
#include "ui/mainframe/ScreenUpdateBlocker.h"
#include "selectionlib.h"
#include "select.h"
#include "map/Map.h"

#include "xyview/GlobalXYWnd.h"
#include "selection/algorithm/Shader.h"
#include "selection/algorithm/General.h"
#include "selection/algorithm/Group.h"
#include "selection/algorithm/Entity.h"
#include "ui/modelselector/ModelSelector.h"
#include <boost/format.hpp>

#include <iostream>

	namespace {
		const std::string RKEY_FREE_MODEL_ROTATION = "user/ui/freeModelRotation";
		const std::string RKEY_DEFAULT_CURVE_ENTITY = "game/defaults/defaultCurveEntity";
		const std::string RKEY_CURVE_NURBS_KEY = "game/defaults/curveNurbsKey";
		const std::string RKEY_CURVE_CATMULLROM_KEY = "game/defaults/curveCatmullRomKey";
	}

class RefreshSkinWalker :
	public scene::NodeVisitor
{
public:
	bool pre(const scene::INodePtr& node) {
		// Check if we have a skinnable model
		SkinnedModelPtr skinned = boost::dynamic_pointer_cast<SkinnedModel>(node);

		if (skinned != NULL) {
			// Let the skinned model reload its current skin.
			skinned->skinChanged(skinned->getSkin());
		}

		return true; // traverse children
	}
};

void ReloadSkins(const cmd::ArgumentList& args) {
	GlobalModelSkinCache().refresh();
	RefreshSkinWalker walker;
	Node_traverseSubgraph(GlobalSceneGraph().root(), walker);
	
	// Refresh the ModelSelector too
	ui::ModelSelector::refresh();
}

// This takes care of relading the entityDefs and refreshing the scenegraph
void ReloadDefs(const cmd::ArgumentList& args)
{
	// Disable screen updates for the scope of this function
	ui::ScreenUpdateBlocker blocker(_("Processing..."), _("Reloading Defs"));

	GlobalEntityClassManager().reloadDefs();
}

// Function to return an AABB based on the current workzone AABB (retrieved
// from the currently selected brushes), or to use the default light radius
// if the workzone AABB is not valid or none is available.

AABB Doom3Light_getBounds(AABB aabb)
{
	// If the extents are 0 or invalid (-1), replace with the default radius
    for (int i = 0; i < 3; i++) {
		if (aabb.extents[i] <= 0)
	    	aabb.extents[i] = DEFAULT_LIGHT_RADIUS;
    }
	return aabb;
}

namespace entity
{

/** 
 * Create an instance of the given entity at the given position, and return
 * the Node containing the new entity.
 * 
 * @returns: the scene::INodePtr referring to the new entity.
 */
scene::INodePtr createEntityFromSelection(const std::string& name, const Vector3& origin)
{
	// Obtain the structure containing the selection counts
	const SelectionInfo& info = GlobalSelectionSystem().getSelectionInfo();

    IEntityClassPtr entityClass = GlobalEntityClassManager().findOrInsert(name, true);

    // TODO: to be replaced by inheritance-based class detection
    bool isModel = (info.totalCount == 0 && name == "func_static");
    
    // Some entities are based on the size of the currently-selected primitive(s)
    bool primitivesSelected = info.brushCount > 0 || info.patchCount > 0;

    if (!(entityClass->isFixedSize() || isModel) && !primitivesSelected) {
		throw EntityCreationException(
			(boost::format(_("Unable to create entity %s, no brushes selected.")) % name).str()
		);
    }

	// Get the selection workzone bounds
	AABB workzone = GlobalSelectionSystem().getWorkZone().bounds;
    
    scene::INodePtr node(GlobalEntityCreator().createEntity(entityClass));
	Entity* entity = Node_getEntity(node);

	GlobalSceneGraph().root()->addChildNode(node);
    
    if (entityClass->isFixedSize() || (isModel && !primitivesSelected)) {
		selection::algorithm::deleteSelection();
    
        ITransformablePtr transform = Node_getTransformable(node);
    
        if (transform != 0) {
            transform->setType(TRANSFORM_PRIMITIVE);
            transform->setTranslation(origin);
            transform->freezeTransform();
        }
    
        GlobalSelectionSystem().setSelectedAll(false);

		// Move the item to the first visible layer
		node->moveToLayer(GlobalLayerSystem().getFirstVisibleLayer());
    
        Node_setSelected(node, true);
    }
    else // brush-based entity
	{ 
		// Add selected brushes as children of non-fixed entity
		entity->setKeyValue("model", entity->getKeyValue("name"));

		// Take the selection center as new origin
		Vector3 newOrigin = selection::algorithm::getCurrentSelectionCenter();
		entity->setKeyValue("origin", std::string(newOrigin));
		
        // If there is an "editor_material" class attribute, apply this shader
        // to all of the selected primitives before parenting them
        std::string material = entity->getEntityClass()->getAttribute("editor_material").value;

        if (!material.empty()) {
            selection::algorithm::applyShaderToSelection(material);
        }
                
        // Parent the selected primitives to the new node
		selection::algorithm::ParentPrimitivesToEntityWalker walker(node);
		GlobalSelectionSystem().foreachSelected(walker);
		walker.reparent();

	    // De-select the children and select the newly created parent entity
	    GlobalSelectionSystem().setSelectedAll(false);
	    Node_setSelected(node, true);
    }
	
    // Set the light radius and origin

    if (entityClass->isLight() && primitivesSelected)
	{
        AABB bounds(Doom3Light_getBounds(workzone));    
        entity->setKeyValue("origin", bounds.getOrigin());
        entity->setKeyValue("light_radius", bounds.getExtents());
    }
    
    // Flag the map as unsaved after creating the entity
    GlobalMap().setModified(true);

	// Check for auto-setting key values
	EntityClassAttributeList list = entityClass->getAttributeList("editor_setKeyValue");

	if (!list.empty())
	{
		for (EntityClassAttributeList::const_iterator i = list.begin(); i != list.end(); ++i)
		{
			// Cut off the "editor_setKeyValueN " string from the key to get the spawnarg name
			std::string key = i->name.substr(i->name.find_first_of(' ') + 1, 18);
			entity->setKeyValue(key, i->value);
		}
	}
    
	// Return the new node
	return node;
}

/** greebo: Creates a new entity with an attached curve
 * 
 * @key: The curve type: pass either "curve_CatmullRomSpline" or "curve_Nurbs".
 */
void createCurve(const std::string& key) {
	UndoableCommand undo(std::string("createCurve: ") + key);
	
	// De-select everything before we proceed
	GlobalSelectionSystem().setSelectedAll(false);
	GlobalSelectionSystem().setSelectedAllComponents(false);
	
	std::string curveEClass = GlobalRegistry().get(RKEY_DEFAULT_CURVE_ENTITY);
	// Fallback to func_static, if nothing defined in the registry
	if (curveEClass.empty()) {
		curveEClass = "func_static"; 
	}
	
	// Find the default curve entity
	IEntityClassPtr entityClass = GlobalEntityClassManager().findOrInsert(
		curveEClass, 
		true
	);
	
	// Create a new entity node deriving from this entityclass
	scene::INodePtr curve(GlobalEntityCreator().createEntity(entityClass));
    
    // Insert this new node into the scenegraph root 
    GlobalSceneGraph().root()->addChildNode(curve);
    
    // Select this new curve node
    Node_setSelected(curve, true);
    
	Entity* entity = Node_getEntity(curve);
	assert(entity); // this must be true
	
	// Set the model key to be the same as the name
	entity->setKeyValue("model", entity->getKeyValue("name"));
	
	// Initialise the curve using three pre-defined points
	entity->setKeyValue(
		key, 
		"3 ( 0 0 0  50 50 0  50 100 0 )"
	);
	
	ITransformablePtr transformable = Node_getTransformable(curve);
	if (transformable != NULL) {
		// Translate the entity to the center of the current workzone
		transformable->setTranslation(GlobalXYWnd().getActiveXY()->getOrigin());
		transformable->freezeTransform();
	}
}

void createCurveNURBS(const cmd::ArgumentList& args) {
	createCurve(GlobalRegistry().get(RKEY_CURVE_NURBS_KEY));
}

void createCurveCatmullRom(const cmd::ArgumentList& args) {
	createCurve(GlobalRegistry().get(RKEY_CURVE_CATMULLROM_KEY));
}

void registerCommands()
{
	GlobalCommandSystem().addCommand("ConnectSelection", selection::algorithm::connectSelectedEntities);
	GlobalCommandSystem().addCommand("BindSelection", selection::algorithm::bindEntities);
	GlobalCommandSystem().addCommand("CreateCurveNURBS", entity::createCurveNURBS);
	GlobalCommandSystem().addCommand("CreateCurveCatmullRom", entity::createCurveCatmullRom);

	GlobalEventManager().addCommand("ConnectSelection", "ConnectSelection");
	GlobalEventManager().addCommand("BindSelection", "BindSelection");
	GlobalEventManager().addRegistryToggle("ToggleFreeModelRotation", RKEY_FREE_MODEL_ROTATION);
	GlobalEventManager().addCommand("CreateCurveNURBS", "CreateCurveNURBS");
	GlobalEventManager().addCommand("CreateCurveCatmullRom", "CreateCurveCatmullRom");
}

} // namespace entity
