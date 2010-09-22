#ifndef PATCHNODE_H_
#define PATCHNODE_H_

#include "irenderable.h"
#include "scenelib.h"
#include "iscenegraph.h"
#include "imap.h"
#include "Patch.h"
#include "selectionlib.h"
#include "PatchControlInstance.h"
#include "dragplanes.h"

class PatchNode :
	public scene::Node,
	public scene::Cloneable,
	public Nameable,
	public Snappable,
	public IdentityTransform,
	public IPatchNode,
	public Selectable,
	public SelectionTestable,
	public ComponentSelectionTestable,
	public ComponentEditable,
	public ComponentSnappable,
	public PlaneSelectable,
	public LightCullable,
	public Renderable,
	public Transformable
{
	DragPlanes m_dragPlanes;

	// The attached selectable
	ObservedSelectable _selectable;

	// The patch control instances
	typedef std::vector<PatchControlInstance> PatchControlInstances;
	PatchControlInstances m_ctrl_instances;

	// An array of renderable points
	mutable RenderablePointVector m_render_selected;

	const LightList* m_lightList;

	Patch m_patch;
	
	// An internal AABB variable to calculate the bounding box of the selected components (has to be mutable) 
	mutable AABB m_aabb_component;

	static ShaderPtr m_state_selpoint;

public:
	// Construct a PatchNode with no arguments
	PatchNode(bool patchDef3 = false);
  
	// Copy Constructor
	PatchNode(const PatchNode& other);

	virtual ~PatchNode();

	// Patch::Observer implementation
	void allocate(std::size_t size);
	
	// Nameable implementation
	std::string name() const;

	void lightsChanged();

	// Bounded implementation
	const AABB& localAABB() const;

	// IPatchNode implementation
	Patch& getPatchInternal();
	IPatch& getPatch();

	// Snappable implementation
	virtual void snapto(float snap);

	// Test the Patch instance for selection (SelectionTestable)
	void testSelect(Selector& selector, SelectionTest& test);

	// Check if the drag planes pass the given selection test (and select them of course and call the callback) 
	void selectPlanes(Selector& selector, SelectionTest& test, const PlaneCallback& selectedPlaneCallback);
  	void selectReversedPlanes(Selector& selector, const SelectedPlanes& selectedPlanes);

	// Returns true if any of the patch components is selected
	bool isSelectedComponents() const;
	// Set the components (control points or dragplanes) selection to <select>
	void setSelectedComponents(bool select, SelectionSystem::EComponentMode mode);
	// Tests the patch components on selection using the passed SelectionTest 
	void testSelectComponents(Selector& selector, SelectionTest& test, SelectionSystem::EComponentMode mode);

	// override scene::Inode::onRemoveFromScene to deselect the child components
	virtual void onInsertIntoScene();
	virtual void onRemoveFromScene();

	// Create the axis aligned bounding box of the selected components
	const AABB& getSelectedComponentsBounds() const;

	// Sets all Control Instances to selected = <selected>
  	void selectCtrl(bool selected);

	// Returns true if this patch can be rendered
	bool isVisible() const;

	// Returns true if the material itself is visible
	bool hasVisibleMaterial() const;

	// greebo: snaps all the _selected_ components to the grid (should be called "snapSelectedComponents")
	void snapComponents(float snap);

	// Returns true if any of the Control Vertices is selected
	bool selectedVertices();
	
	// Clones this node, allocates a new Node on the heap and passes itself to the constructor of the new node
	scene::INodePtr clone() const;

	// Set the selection status. As this is an ObservedSelectable, the onChanged callback is performed automatically.
	virtual void setSelected(bool select);
	virtual bool isSelected() const;
	virtual void invertSelected();

	// The callback function that gets called when the attached selectable gets changed
	void selectedChanged(const Selectable& selectable);

	// greebo: This gets called by the ObservedSelectable as soon as its selection state changes 
	// (see ObservedSelectable and PatchControlInstance)
	void selectedChangedComponent(const Selectable& selectable);

	// LightCullable implementation
	bool testLight(const RendererLight& light) const;

	// Initialise/release the static member variables
	static void constructStatic();
	static void destroyStatic();

	// Renderable implementation

	// Render functions, these make sure that all things get rendered properly. The calls are also passed on
	// to the contained patch <m_patch>
	void renderSolid(RenderableCollector& collector, const VolumeTest& volume) const;
	void renderWireframe(RenderableCollector& collector, const VolumeTest& volume) const;
	// Renders the components of this patch instance, makes use of the Patch::render_component() method 
	void renderComponents(RenderableCollector& collector, const VolumeTest& volume) const;

	void evaluateTransform();

protected:
	// Gets called by the Transformable implementation whenever
	// scale, rotation or translation is changed.
	void _onTransformationChanged();

	// Called by the Transformable implementation before freezing
	// or when reverting transformations.
	void _applyTransformation();

private:
	// Transforms the patch components with the given transformation matrix
	void transformComponents(const Matrix4& matrix);

	// greebo: Updates the internal render array m_render_selected, that contains all control vertices that should be
	// rendered as highlighted.
	void update_selected() const;

	// greebo: Renders the selected components. This is called by the above two render functions
	void renderComponentsSelected(RenderableCollector& collector, const VolumeTest& volume) const;
};
typedef boost::shared_ptr<PatchNode> PatchNodePtr;
typedef boost::weak_ptr<PatchNode> PatchNodeWeakPtr;

#endif /*PATCHNODE_H_*/
