#include "PicoModelNode.h"

#include "RenderablePicoSurface.h"
#include "ivolumetest.h"
#include "ishaders.h"
#include "ifilter.h"
#include "imodelcache.h"
#include "math/frustum.h"
#include "generic/callback.h"
#include <boost/bind.hpp>

namespace model {

PicoModelNode::PicoModelNode(const RenderablePicoModelPtr& picoModel) :  
	_picoModel(picoModel),
	_name(picoModel->getFilename()),
	_lightList(GlobalRenderSystem().attach(*this))
{
	Node::setTransformChangedCallback(boost::bind(&PicoModelNode::lightsChanged, this));

	// Update the skin
	skinChanged("");
}

PicoModelNode::~PicoModelNode() {
	GlobalRenderSystem().detach(*this);
}

const IModel& PicoModelNode::getIModel() const {
	return *_picoModel;
}

const AABB& PicoModelNode::localAABB() const {
	return _picoModel->localAABB();
}

// SelectionTestable implementation
void PicoModelNode::testSelect(Selector& selector, SelectionTest& test) {
	_picoModel->testSelect(selector, test, localToWorld());
}

std::string PicoModelNode::name() const {
  	return _picoModel->getFilename();
}
  
const RenderablePicoModelPtr& PicoModelNode::getModel() const {
	return _picoModel;
}

void PicoModelNode::setModel(const RenderablePicoModelPtr& model) {
	_picoModel = model;
}

// LightCullable test function
bool PicoModelNode::testLight(const RendererLight& light) const {
	return light.testAABB(worldAABB());
}
	
// Add a light to this model instance
void PicoModelNode::insertLight(const RendererLight& light) {
	// Calculate transform from the superclass
	const Matrix4& l2w = localToWorld();
	
	// If the light's AABB intersects the oriented AABB of this model instance,
	// add the light to our light list
	if (light.testAABB(aabb_for_oriented_aabb(_picoModel->localAABB(),
											  l2w)))
	{
		_lights.addLight(light);
	}
}
	
// Clear all lights from this model instance
void PicoModelNode::clearLights() {
	_lights.clear();
}

void PicoModelNode::renderSolid(RenderableCollector& collector, const VolumeTest& volume) const {
	_lightList.evaluateLights();

	submitRenderables(collector, volume, localToWorld());
}

void PicoModelNode::renderWireframe(RenderableCollector& collector, const VolumeTest& volume) const {
	renderSolid(collector, volume);
}

// Renderable submission
void PicoModelNode::submitRenderables(RenderableCollector& collector, 
									  const VolumeTest& volume, 
									  const Matrix4& localToWorld) const
{
	// Test the model's intersection volume, if it intersects pass on the 
	// render call
	if (volume.TestAABB(_picoModel->localAABB(), localToWorld) != VOLUME_OUTSIDE) 
    {
		// Submit the lights
		collector.setLights(_lights);
	
		// If the surface cache is populated, then use this instead of the
		// original model in order to get the skinned textures
		if (!_mappedSurfs.empty()) {
			for (MappedSurfaces::const_iterator i = _mappedSurfs.begin();
				 i != _mappedSurfs.end();
				 ++i)
			{
				// Submit the surface and shader to the collector, checking first
				// to make sure the texture is not filtered
				MaterialPtr surfaceShader = i->second->getMaterial();
				if (surfaceShader->isVisible()) { 
					collector.SetState(i->second, RenderableCollector::eFullMaterials);
					collector.addRenderable(*i->first, localToWorld);
				}
			}			
		}
		else {
			// Submit the model's geometry
			_picoModel->submitRenderables(collector, localToWorld);
		}
	}
}

// Skin changed notify
void PicoModelNode::skinChanged(const std::string& newSkinName) {

	// Clear all the surface mappings before doing anything
	_mappedSurfs.clear();

	// The new skin name is stored locally
	_skin = newSkinName;

	// greebo: Acquire the ModelSkin reference from the SkinCache
	// Note: This always returns a valid reference
	ModelSkin& skin = GlobalModelSkinCache().capture(_skin);

	// Otherwise get the list of RenderablePicoSurfaces from the model and
	// determine a texture remapping for each one
	const SurfaceList& surfs = _picoModel->getSurfaces();
	for (SurfaceList::const_iterator i = surfs.begin();
		 i != surfs.end();
		 ++i)
	{
		// Get the surface's material and test the skin for a remap
		std::string material = (*i)->getActiveMaterial();
		std::string mapped = skin.getRemap(material);
		if (mapped.empty())
			mapped = material; // use original material for remap
		
		// Add the surface and the mapped shader to our surface cache
		_mappedSurfs.push_back(
			std::make_pair(
				*i,
				GlobalRenderSystem().capture(mapped)
			)
		);
	}
	
	// Refresh the scene
	GlobalSceneGraph().sceneChanged();
}

// Returns the name of the currently active skin
std::string PicoModelNode::getSkin() const {
	return _skin;
}

} // namespace model
