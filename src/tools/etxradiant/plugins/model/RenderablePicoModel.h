#ifndef RENDERABLEPICOMODEL_H_
#define RENDERABLEPICOMODEL_H_

#include "imodel.h"
#include "picomodel.h"
#include "math/aabb.h"

#include <boost/shared_ptr.hpp>

/* FORWARD DECLS */
namespace model { class RenderablePicoSurface; }
class RenderableCollector;
class RendererLight;
class SelectionTest;
class Selector;

namespace model
{

/**
 * List of RenderablePicoSurfaces.
 */
typedef std::vector<boost::shared_ptr<RenderablePicoSurface> > SurfaceList;

/**
 * Renderable class containing a model loaded via the picomodel library. A
 * RenderablePicoModel is made up of one or more RenderablePicoSurface objects,
 * each of which contains a number of polygons with the same texture. Rendering
 * a RenderablePicoModel involves rendering all of its surfaces, each of which
 * binds its texture(s) and submits its geometry via OpenGL calls.
 */
class RenderablePicoModel
: public IModel
{
	// Vector of renderable surfaces for this model
	SurfaceList _surfVec;
	
	// Local AABB for this model
	AABB _localAABB;

	// Vector of materials used by this model (one for each surface)
	mutable std::vector<std::string> _materialList;
	
	// The filename this model was loaded from
	std::string _filename;

	// The VFS path to this model
	std::string _modelPath;
	
private:

	// Update the list of materials by querying each surface for its current
	// material.
	void updateMaterialList() const;
	
public:

	/** 
	 * Constructor. Accepts a picoModel_t struct containing the raw model data
	 * loaded from picomodel, and a string filename extension to allow the
	 * correct handling of material paths (which differs between ASE and LWO)
	 */
	RenderablePicoModel(picoModel_t* mod, const std::string& fExt);
		
	/**
	 * Front-end render function used by the main collector.
	 * 
	 * @param rend
	 * The sorting RenderableCollector object which accepts renderable geometry.
	 * 
	 * @param localToWorld
	 * Object to world-space transform.
	 */
	void submitRenderables(RenderableCollector& rend, const Matrix4& localToWorld);		
		
	/** 
	 * Back-end render function from OpenGLRenderable. This is called from the
	 * model selector but not the main renderer, which uses the front-end render
	 * method.
	 */
	void render(const RenderInfo& info) const;

	/** 
	 * Return the number of surfaces in this model.
	 */
	int getSurfaceCount() const {
		return static_cast<int>(_surfVec.size());
	}
	
	/** 
	 * Return the number of vertices in this model, by summing the vertex
	 * counts for each surface.
	 */
	int getVertexCount() const;
	
	/** Return the polycount (tricount) of this model by summing the surface
	 * polycounts.
	 */
	 
	int getPolyCount() const;
	
	/** 
	 * Return the enclosing AABB for this model.
	 */
	const AABB& localAABB() const {
		return _localAABB;
	}
	
	/** Return the list of active materials for this model.
	 */
	const std::vector<std::string>& getActiveMaterials() const;
	
	// Sets the filename this model was loaded from
	void setFilename(const std::string& name);
	
	// Returns the filename this model was loaded from
	virtual std::string getFilename() const;

	// Returns the VFS path to the model file
	virtual std::string getModelPath() const;

	void setModelPath(const std::string& modelPath);
	
	/** Apply the given skin to this model.
	 */
	void applySkin(const ModelSkin& skin);

	/**
	 * Selection test. Test each surface against the SelectionTest object and
	 * if the surface is selected, add it to the selector.
	 * 
	 * @param selector
	 * Selector object which builds a list of selectables.
	 * 
	 * @param test
	 * The SelectionTest object defining the 3D properties of the selection.
	 * 
	 * @param localToWorld
	 * Object to world space transform.
	 */
	void testSelect(Selector& selector, 
					SelectionTest& test, 
					const Matrix4& localToWorld);
	
	/**
	 * Return the list of RenderablePicoSurface objects.
	 */
	const SurfaceList& getSurfaces() const {
		return _surfVec;	
	}	
};
typedef boost::shared_ptr<RenderablePicoModel> RenderablePicoModelPtr;

}

#endif /*RENDERABLEPICOMODEL_H_*/
