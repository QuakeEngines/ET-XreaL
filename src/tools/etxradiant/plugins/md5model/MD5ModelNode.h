#ifndef MD5MODELNODE_H_
#define MD5MODELNODE_H_

#include "scenelib.h"
#include "nameable.h"
#include "MD5Model.h"
#include "modelskin.h"
#include "VectorLightList.h"

namespace md5 {

class MD5ModelNode : 
	public scene::Node,
	public model::ModelNode,
	public Nameable,
	public SelectionTestable,
	public LightCullable,
	public Renderable,
	public SkinnedModel
{
	MD5ModelPtr _model;

	const LightList* _lightList;

	typedef std::vector<VectorLightList> SurfaceLightLists;
	SurfaceLightLists _surfaceLightLists;

	struct Remap {
		std::string name;
		ShaderPtr shader;
	};
  
	typedef std::vector<Remap> SurfaceRemaps;
	SurfaceRemaps _surfaceRemaps;

	// The name of this model's skin
	std::string _skin;
	
public:
	MD5ModelNode(const MD5ModelPtr& model);
	virtual ~MD5ModelNode();

	// ModelNode implementation
	virtual const model::IModel& getIModel() const;

	void lightsChanged();

	// returns the contained model
	void setModel(const MD5ModelPtr& model);
	const MD5ModelPtr& getModel() const;

	// Bounded implementation
	virtual const AABB& localAABB() const;

	// Nameable implementation
	virtual std::string name() const;

	// SelectionTestable implementation
	void testSelect(Selector& selector, SelectionTest& test);

	// LightCullable implementation
	bool testLight(const RendererLight& light) const;
	void insertLight(const RendererLight& light);
	void clearLights();

	// Renderable implementation
	void renderSolid(RenderableCollector& collector, const VolumeTest& volume) const;
	void renderWireframe(RenderableCollector& collector, const VolumeTest& volume) const;

	// Returns the name of the currently active skin
	virtual std::string getSkin() const;
	void skinChanged(const std::string& newSkinName);

private:
	void constructRemaps();
	void destroyRemaps();

	void render(RenderableCollector& collector, const VolumeTest& volume, const Matrix4& localToWorld) const;
};
typedef boost::shared_ptr<MD5ModelNode> MD5ModelNodePtr;

} // namespace md5

#endif /*MD5MODELNODE_H_*/
