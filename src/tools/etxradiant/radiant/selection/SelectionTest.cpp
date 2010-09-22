#include "SelectionTest.h"

#include "igroupnode.h"
#include "itextstream.h"
#include "entitylib.h"
#include "renderer.h"
#include "imodel.h"

inline SelectionIntersection select_point_from_clipped(Vector4& clipped) {
  return SelectionIntersection(clipped[2] / clipped[3], static_cast<float>(Vector3(clipped[0] / clipped[3], clipped[1] / clipped[3], 0).getLengthSquared()));
}

void SelectionVolume::BeginMesh(const Matrix4& localToWorld, bool twoSided) {
    _local2view = matrix4_multiplied_by_matrix4(_view.GetViewMatrix(), localToWorld);

    // Cull back-facing polygons based on winding being clockwise or counter-clockwise.
    // Don't cull if the view is wireframe and the polygons are two-sided.
    _cull = twoSided && !_view.fill() ? eClipCullNone : (matrix4_handedness(localToWorld) == MATRIX4_RIGHTHANDED) ? eClipCullCW : eClipCullCCW;

    {
      Matrix4 screen2world(matrix4_full_inverse(_local2view));

      _near = matrix4_transformed_vector4( screen2world, Vector4(0, 0, -1, 1) ).getProjected();
      

      _far = matrix4_transformed_vector4( screen2world, Vector4(0, 0, 1, 1) ).getProjected();
    }
}

void SelectionVolume::TestPoint(const Vector3& point, SelectionIntersection& best) {
    Vector4 clipped;
    if(matrix4_clip_point(_local2view, point, clipped) == c_CLIP_PASS)
    {
      best = select_point_from_clipped(clipped);
    }
}

void SelectionVolume::TestPolygon(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best) {
    Vector4 clipped[9];
    for(std::size_t i=0; i+2<count; ++i)
    {
      BestPoint(
        matrix4_clip_triangle(
          _local2view,
          vertices[0],
          vertices[i+1],
          vertices[i+2],
          clipped
        ),
        clipped,
        best,
        _cull
      );
    }
}

void SelectionVolume::TestLineLoop(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best) {
    if(count == 0)
      return;
    Vector4 clipped[9];
    for(VertexPointer::iterator i = vertices.begin(), end = i + count, prev = i + (count-1); i != end; prev = i, ++i)
    {
      BestPoint(
        matrix4_clip_line(
          _local2view,
          *prev,
          *i,
          clipped
        ),
        clipped,
        best,
        _cull
      );
    }
}

void SelectionVolume::TestLineStrip(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best) {
    if(count == 0)
      return;
    Vector4 clipped[9];
    for(VertexPointer::iterator i = vertices.begin(), end = i + count, next = i + 1; next != end; i = next, ++next)
    {
      BestPoint(
        matrix4_clip_line(
          _local2view,
          *i,
          *next,
          clipped
        ),
        clipped,
        best,
        _cull
      );
    }
}

void SelectionVolume::TestLines(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best) {
    if(count == 0)
      return;
    Vector4 clipped[9];
    for(VertexPointer::iterator i = vertices.begin(), end = i + count; i != end; i += 2)
    {
      BestPoint(
        matrix4_clip_line(
          _local2view,
          *i,
          *(i+1),
          clipped
        ),
        clipped,
        best,
        _cull
      );
    }
}

void SelectionVolume::TestTriangles(const VertexPointer& vertices, const IndexPointer& indices, SelectionIntersection& best) {
    Vector4 clipped[9];
    for(IndexPointer::iterator i(indices.begin()); i != indices.end(); i += 3)
    {
      BestPoint(
        matrix4_clip_triangle(
          _local2view,
          vertices[*i],
          vertices[*(i+1)],
          vertices[*(i+2)],
          clipped
        ),
        clipped,
        best,
        _cull
      );
    }
}

void SelectionVolume::TestQuads(const VertexPointer& vertices, const IndexPointer& indices, SelectionIntersection& best) {
    Vector4 clipped[9];
    for(IndexPointer::iterator i(indices.begin()); i != indices.end(); i += 4)
    {
      BestPoint(
        matrix4_clip_triangle(
          _local2view,
          vertices[*i],
          vertices[*(i+1)],
          vertices[*(i+3)],
          clipped
        ),
        clipped,
        best,
        _cull
      );
	    BestPoint(
        matrix4_clip_triangle(
          _local2view,
          vertices[*(i+1)],
          vertices[*(i+2)],
          vertices[*(i+3)],
          clipped
        ),
        clipped,
        best,
        _cull
      );
    }
}

void SelectionVolume::TestQuadStrip(const VertexPointer& vertices, const IndexPointer& indices, SelectionIntersection& best) {
    Vector4 clipped[9];
    for(IndexPointer::iterator i(indices.begin()); i+2 != indices.end(); i += 2)
    {
      BestPoint(
        matrix4_clip_triangle(
          _local2view,
          vertices[*i],
          vertices[*(i+1)],
          vertices[*(i+2)],
          clipped
        ),
        clipped,
        best,
        _cull
      );
      BestPoint(
        matrix4_clip_triangle(
          _local2view,
          vertices[*(i+2)],
          vertices[*(i+1)],
          vertices[*(i+3)],
          clipped
        ),
        clipped,
        best,
        _cull
      );
    }
}

// ==================================================================================

void SelectionTestWalker::printNodeName(const scene::INodePtr& node)
{
	globalOutputStream() << "Node: " << nodetype_get_name(node_get_nodetype(node)) << " ";

	if (node_get_nodetype(node) == eNodeEntity)
	{
		globalOutputStream() << " - " << Node_getEntity(node)->getKeyValue("name"); 
	}

	globalOutputStream() << std::endl;
}

scene::INodePtr SelectionTestWalker::getEntityNode(const scene::INodePtr& node)
{
	return (Node_isEntity(node)) ? node : scene::INodePtr();
}

scene::INodePtr SelectionTestWalker::getParentGroupEntity(const scene::INodePtr& node)
{
	scene::INodePtr parent = node->getParent();

	return (Node_getGroupNode(parent) != NULL) ? parent : scene::INodePtr();
}

bool SelectionTestWalker::entityIsWorldspawn(const scene::INodePtr& node)
{
	return node_is_worldspawn(node);
}

bool EntitySelector::visit(const scene::INodePtr& node)
{
	// Check directly for an entity
	scene::INodePtr entity = getEntityNode(node);

	if (entity == NULL)
	{
		// Skip any models, the parent entity is taking care of the selection test
		if (Node_isModel(node))
		{
			return true;
		}

		// Second chance check: is the parent a group node?
		entity = getParentGroupEntity(node);
	}

	// Skip worldspawn in any case
	if (entity == NULL || entityIsWorldspawn(entity)) return true;

	// Comment out to hide debugging output
	//printNodeName(node);

	// The entity is the selectable, but the actual node will be tested for selection
	SelectablePtr selectable = Node_getSelectable(entity);

    if (selectable == NULL)
	{
    	return true; // skip
    }

	_selector.pushSelectable(*selectable);

	// Test the entity for selection, this will add an intersection to the selector
    SelectionTestablePtr selectionTestable = Node_getSelectionTestable(node);

    if (selectionTestable)
	{
		selectionTestable->testSelect(_selector, _test);
    }

	_selector.popSelectable();

	return true;
}

bool PrimitiveSelector::visit(const scene::INodePtr& node)
{
	// Skip all entities
	if (Node_isEntity(node)) return true;

	// Node is not an entity, check parent
	scene::INodePtr parent = getParentGroupEntity(node);

	if (parent != NULL && !entityIsWorldspawn(parent))
	{
		// Don't select primitives of non-worldspawn entities,
		// the EntitySelector is taking care of that case
		return true;
	}

	SelectablePtr selectable = Node_getSelectable(node);

    if (selectable == NULL) return true; // skip non-selectables

	_selector.pushSelectable(*selectable);

	// Test the entity for selection, this will add an intersection to the selector
    SelectionTestablePtr selectionTestable = Node_getSelectionTestable(node);

    if (selectionTestable)
	{
		selectionTestable->testSelect(_selector, _test);
    }

	_selector.popSelectable();

	return true;
}

bool GroupChildPrimitiveSelector::visit(const scene::INodePtr& node)
{
	// Skip all entities
	if (Node_isEntity(node)) return true;

	// Node is not an entity, check parent
	scene::INodePtr parent = getParentGroupEntity(node);

	if (parent != NULL && !entityIsWorldspawn(parent))
	{
		// We have a candidate
		SelectablePtr selectable = Node_getSelectable(node);

		if (selectable == NULL) return true; // skip non-selectables

		_selector.pushSelectable(*selectable);

		// Test the entity for selection, this will add an intersection to the selector
		SelectionTestablePtr selectionTestable = Node_getSelectionTestable(node);

		if (selectionTestable)
		{
			selectionTestable->testSelect(_selector, _test);
		}

		_selector.popSelectable();

		return true;
	}

	return true;
}

bool AnySelector::visit(const scene::INodePtr& node)
{
	scene::INodePtr entity = getEntityNode(node);

	scene::INodePtr candidate;

	if (entity != NULL)
	{
		// skip worldspawn
		if (entityIsWorldspawn(entity)) return true;

		// Use this entity as selectable
		candidate = entity;
	}
	else if (Node_isPrimitive(node))
	{
		// Primitives are ok, check for func_static children
		scene::INodePtr parentEntity = getParentGroupEntity(node);

		if (parentEntity != NULL)
		{
			// If this node is a child of worldspawn, it can be directly selected
			// Otherwise this node is a child primitve of a non-worldspawn entity, 
			// in which case we want to select the parent entity
			candidate = (entityIsWorldspawn(parentEntity)) ? node : parentEntity;
		}
		else
		{
			// A primitive without parent group entity? Error?
			return true; // skip
		}
	}

	// The entity is the selectable, but the actual node will be tested for selection
	SelectablePtr selectable = Node_getSelectable(candidate);

    if (selectable == NULL) return true; // skip unselectable nodes

	_selector.pushSelectable(*selectable);

	// Test the entity for selection, this will add an intersection to the selector
    SelectionTestablePtr selectionTestable = Node_getSelectionTestable(node);

    if (selectionTestable)
	{
		selectionTestable->testSelect(_selector, _test);
    }

	_selector.popSelectable();

	return true;
}

// scene::Graph::Walker
bool ComponentSelector::visit(const scene::INodePtr& node)
{
	ComponentSelectionTestablePtr testable = Node_getComponentSelectionTestable(node);

	if (testable != NULL)
	{
		testable->testSelectComponents(_selector, _test, _mode);
    }

	return true;
}

// SelectionSystem::Visitor
void ComponentSelector::visit(const scene::INodePtr& node) const
{
	ComponentSelectionTestablePtr testable = Node_getComponentSelectionTestable(node);

	if (testable != NULL)
	{
		testable->testSelectComponents(_selector, _test, _mode);
    }
}
