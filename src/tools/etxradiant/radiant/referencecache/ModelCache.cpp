#include "ModelCache.h"

#include "i18n.h"
#include "ifilesystem.h"
#include "imodel.h"
#include "ifiletypes.h"
#include "iselection.h"
#include "ieventmanager.h"

#include <iostream>
#include <set>
#include "os/path.h"
#include "os/file.h"

#include "modulesystem/StaticModule.h"
#include "ui/modelselector/ModelSelector.h"
#include "ui/mainframe/ScreenUpdateBlocker.h"
#include "NullModelLoader.h"
#include <boost/bind.hpp>

namespace model {

namespace {

	class ModelRefreshWalker :
		public scene::NodeVisitor
	{
	public:
		bool pre(const scene::INodePtr& node) {
			IEntityNodePtr entity = boost::dynamic_pointer_cast<IEntityNode>(node);

			if (entity != NULL) {
				entity->refreshModel();
				return false;
			}

			return true;
		}
	};

	class ModelFinder :
		public SelectionSystem::Visitor,
		public scene::NodeVisitor
	{
	public:
		typedef std::set<IEntityNodePtr> Entities;
		typedef std::set<std::string> ModelPaths;

	private:
		// All the model path of the selected modelnodes
		mutable ModelPaths _modelNames;
		// All the selected entities with modelnodes as child
		mutable Entities _entities;

	public:
		bool pre(const scene::INodePtr& node)
		{
			ModelNodePtr model = Node_getModel(node);

			if (model != NULL)
			{
				_modelNames.insert(model->getIModel().getModelPath());

				IEntityNodePtr ent = 
					boost::dynamic_pointer_cast<IEntityNode>(node->getParent());

				if (ent != NULL)
				{
					_entities.insert(ent);
				}

				return false;
			}

			return true;
		}

		void visit(const scene::INodePtr& node) const
		{
			Node_traverseSubgraph(node, *const_cast<ModelFinder*>(this));
		}

		const Entities& getEntities() const
		{
			return _entities;
		}

		const ModelPaths& getModelPaths() const
		{
			return _modelNames;
		}
	};

} // namespace

ModelCache::ModelCache() :
	_enabled(true)
{}

ModelLoaderPtr ModelCache::getModelLoaderForType(const std::string& type) {
	// Get the module name from the Filetype registry
	std::string moduleName = GlobalFiletypes().findModuleName("model", type);
	  
	if (!moduleName.empty()) {
		ModelLoaderPtr modelLoader = boost::static_pointer_cast<ModelLoader>(
			module::GlobalModuleRegistry().getModule(moduleName)
		);

		if (modelLoader != NULL) {
			return modelLoader;
		}
		else {
			globalErrorStream()	<< "ERROR: Model type incorrectly registered: \""
				<< moduleName << "\"" << std::endl;
		}
	}

	return NullModelLoader::InstancePtr();
}

scene::INodePtr ModelCache::getModelNode(const std::string& modelPath) {
	// Check if we have a reference to a modeldef
	IModelDefPtr modelDef = GlobalEntityClassManager().findModel(modelPath);

	// The actual model path (is usually the same as the incoming modelPath)
	std::string actualModelPath(modelPath);

	if (modelDef != NULL) {
		// We have a valid modelDef, override the model path
		actualModelPath = modelDef->mesh;
	}

	// Get the extension of this model
	std::string type = actualModelPath.substr(actualModelPath.rfind(".") + 1); 

	// Get a suitable model loader
	ModelLoaderPtr modelLoader = getModelLoaderForType(type);

	// Try to construct a model node using the suitable loader
	scene::INodePtr modelNode = modelLoader->loadModel(actualModelPath);

	if (modelNode != NULL) {
		// Model load was successful
		return modelNode;
	}

	// The model load failed, let's return the NullModel
	// This call should never fail, i.e. the returned model is non-NULL
	return NullModelLoader::InstancePtr()->loadModel(actualModelPath);
}

IModelPtr ModelCache::getModel(const std::string& modelPath) {
	// Try to lookup the existing model
	ModelMap::iterator found = _modelMap.find(modelPath);

	if (_enabled && found != _modelMap.end()) {
		// Try to lock the weak pointer
		IModelPtr model = found->second.lock();

		if (model != NULL) {
			// Model is cached and weak pointer could be locked, return
			return model;
		}

		// Weak pointer could not be locked, remove from the map
		_modelMap.erase(found);
	}

	// The model is not cached, the weak pointer could not be locked
	// or the cache is disabled, load afresh
	
	// Get the extension of this model
	std::string type = modelPath.substr(modelPath.rfind(".") + 1); 

	// Find a suitable model loader
	ModelLoaderPtr modelLoader = getModelLoaderForType(type);

	IModelPtr model = modelLoader->loadModelFromPath(modelPath);

	if (model != NULL) {
		// Model successfully loaded, insert a weak reference into the map
		_modelMap.insert(
			ModelMap::value_type(modelPath, IModelWeakPtr(model))
		);
	}
		
	return model;
}

void ModelCache::clear() {
	// greebo: Disable the modelcache. During map::clear(), the nodes
	// get cleared, which might trigger a loopback to insert().
	_enabled = false;
	
	_modelMap.clear();
	
	// Allow usage of the modelnodemap again.
	_enabled = true;
}

void ModelCache::refreshModels(const cmd::ArgumentList& args)
{
	// Disable screen updates for the scope of this function
	ui::ScreenUpdateBlocker blocker(_("Processing..."), _("Reloading Models"));
	
	// Clear the model cache
	clear();

	// Update all model nodes
	ModelRefreshWalker walker;
	Node_traverseSubgraph(GlobalSceneGraph().root(), walker);
		
	// greebo: Reload the modelselector too
	ui::ModelSelector::refresh();
}

void ModelCache::refreshSelectedModels(const cmd::ArgumentList& args)
{
	// Disable screen updates for the scope of this function
	ui::ScreenUpdateBlocker blocker(_("Processing..."), _("Reloading Models"));
	
	// Find all models in the current selection
	ModelFinder walker;
	GlobalSelectionSystem().foreachSelected(walker);

	// Remove the selected models from the cache
	ModelFinder::ModelPaths models = walker.getModelPaths();

	for (ModelFinder::ModelPaths::const_iterator i = models.begin();
		 i != models.end(); ++i)
	{
		ModelMap::iterator found = _modelMap.find(*i);

		if (found != _modelMap.end())
		{
			_modelMap.erase(found);
		}
	}

	// Traverse the entities and submit a refresh call
	ModelFinder::Entities entities = walker.getEntities();

	for (ModelFinder::Entities::const_iterator i = entities.begin();
		 i != entities.end(); ++i)
	{
		(*i)->refreshModel();
	}
}

// RegisterableModule implementation
const std::string& ModelCache::getName() const {
	static std::string _name("ModelCache");
	return _name;
}

const StringSet& ModelCache::getDependencies() const {
	static StringSet _dependencies;

	if (_dependencies.empty()) {
		_dependencies.insert(MODULE_MODELLOADER + "ASE");
		_dependencies.insert(MODULE_MODELLOADER + "LWO");
		_dependencies.insert(MODULE_MODELLOADER + "MD5MESH");
		_dependencies.insert(MODULE_COMMANDSYSTEM);
		_dependencies.insert(MODULE_SELECTIONSYSTEM);
	}

	return _dependencies;
}

void ModelCache::initialiseModule(const ApplicationContext& ctx) {
	globalOutputStream() << "ModelCache::initialiseModule called.\n";

	GlobalCommandSystem().addCommand(
		"RefreshModels", 
		boost::bind(&ModelCache::refreshModels, this, _1)
	);
	GlobalCommandSystem().addCommand(
		"RefreshSelectedModels", 
		boost::bind(&ModelCache::refreshSelectedModels, this, _1)
	);
	GlobalEventManager().addCommand("RefreshModels", "RefreshModels");
	GlobalEventManager().addCommand("RefreshSelectedModels", "RefreshSelectedModels");
}

void ModelCache::shutdownModule() {
	clear();
}

// The static module
module::StaticModule<ModelCache> modelCacheModule; 

} // namespace model
