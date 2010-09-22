#include "Brush.h"

#include "math/frustum.h"
#include "signal/signal.h"
#include "irenderable.h"
#include "itextstream.h"
#include "iuimanager.h"
#include "shaderlib.h"

#include "BrushModule.h"
#include "BrushNode.h"
#include "Face.h"
#include "FixedWinding.h"
#include "ui/surfaceinspector/SurfaceInspector.h"

namespace {
	/// \brief Returns true if edge (\p x, \p y) is smaller than the epsilon used to classify winding points against a plane.
	inline bool Edge_isDegenerate(const Vector3& x, const Vector3& y) {
		return (y - x).getLengthSquared() < (ON_EPSILON * ON_EPSILON);
	}
}

Brush::Brush(BrushNode& owner, const Callback& evaluateTransform, const Callback& boundsChanged) :
	_owner(owner),
	m_undoable_observer(0),
	m_map(0),
	_faceCentroidPoints(GL_POINTS),
	_uniqueVertexPoints(GL_POINTS),
	_uniqueEdgePoints(GL_POINTS),
	m_evaluateTransform(evaluateTransform),
	m_boundsChanged(boundsChanged),
	m_planeChanged(false),
	m_transformChanged(false)
{
	planeChanged();
}

Brush::Brush(BrushNode& owner, const Brush& other, const Callback& evaluateTransform, const Callback& boundsChanged) :
	_owner(owner),
	m_undoable_observer(0),
	m_map(0),
	_faceCentroidPoints(GL_POINTS),
	_uniqueVertexPoints(GL_POINTS),
	_uniqueEdgePoints(GL_POINTS),
	m_evaluateTransform(evaluateTransform),
	m_boundsChanged(boundsChanged),
	m_planeChanged(false),
	m_transformChanged(false)
{
	copy(other);
}

Brush::~Brush() {
	ASSERT_MESSAGE(m_observers.empty(), "Brush::~Brush: observers still attached");
}

BrushNode& Brush::getBrushNode()
{
	return _owner;
}

IFace& Brush::getFace(std::size_t index)
{
	assert(index < m_faces.size());
	return *m_faces[index];
}

IFace& Brush::addFace(const Plane3& plane)
{
	// Allocate a new Face
	undoSave();
	push_back(FacePtr(new Face(*this, plane, this)));

	return *m_faces.back();
}

IFace& Brush::addFace(const Plane3& plane, const Matrix4& texDef, const std::string& shader) 
{
	// Allocate a new Face
	undoSave();
	push_back(FacePtr(new Face(*this, plane, texDef, shader, this)));

	return *m_faces.back();
}

void Brush::translateDoom3Brush(const Vector3& translation) {
	transform(Matrix4::getTranslation(translation));
	freezeTransform();
}

void Brush::attach(BrushObserver& observer) {
	for (Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i) {
		observer.push_back(*(*i));
	}

	for(SelectableEdges::iterator i = m_select_edges.begin(); i !=m_select_edges.end(); ++i) {
		observer.edge_push_back(*i);
	}

	for(SelectableVertices::iterator i = m_select_vertices.begin(); i != m_select_vertices.end(); ++i) {
		observer.vertex_push_back(*i);
	}

	m_observers.insert(&observer);
}

void Brush::detach(BrushObserver& observer)
{
	m_observers.erase(&observer);
}

void Brush::forEachFace(const BrushVisitor& visitor) const {
	for(Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i) {
		visitor.visit(*(*i));
	}
}

void Brush::forEachFace_instanceAttach(MapFile* map) const {
	for(Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i)	{
		(*i)->instanceAttach(map);
	}
}

void Brush::forEachFace_instanceDetach(MapFile* map) const {
	for(Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i)
	{
		(*i)->instanceDetach(map);
	}
}

void Brush::instanceAttach(MapFile* map)
{
	if(++m_instanceCounter.m_count == 1)
	{
		m_map = map;
		m_undoable_observer = GlobalUndoSystem().observer(this);
		forEachFace_instanceAttach(m_map);
	}
}

void Brush::instanceDetach(MapFile* map)
{
	if(--m_instanceCounter.m_count == 0)
	{
		forEachFace_instanceDetach(m_map);
		m_map = NULL;
		m_undoable_observer = 0;
		GlobalUndoSystem().release(this);
	}
}

// observer
void Brush::planeChanged() {
	m_planeChanged = true;
	aabbChanged();
	_owner.lightsChanged();
}

void Brush::shaderChanged()
{
	planeChanged();

	// Queue an UI update of the texture tools
	ui::SurfaceInspector::Instance().queueUpdate();
}

void Brush::setShader(const std::string& newShader) {
	undoSave();

	for (Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i) {
		(*i)->setShader(newShader);
	}
}

bool Brush::hasShader(const std::string& name) {
	// Traverse the faces
	for (Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i) {
		if (shader_equal((*i)->getShader(), name)) {
			return true;
		}
	}

	// not found
	return false;
}

bool Brush::hasVisibleMaterial() const
{
	// Traverse the faces
	for (Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i)
	{
		if ((*i)->getFaceShader().getGLShader()->getMaterial()->isVisible())
		{
			return true; // return true on first visible material
		}
	}

	// no visible material
	return false;
}

void Brush::evaluateBRep() const {
	if(m_planeChanged) {
		m_planeChanged = false;
		const_cast<Brush*>(this)->buildBRep();
	}
}

void Brush::transformChanged() {
	m_transformChanged = true;
	planeChanged();
}

void Brush::evaluateTransform() {
	if(m_transformChanged) {
		m_transformChanged = false;
		revertTransform();
		m_evaluateTransform();
	}
}

void Brush::aabbChanged() {
	m_boundsChanged();
}

const AABB& Brush::localAABB() const {
	evaluateBRep();
	return m_aabb_local;
}

void Brush::renderComponents(SelectionSystem::EComponentMode mode, RenderableCollector& collector, const VolumeTest& volume, const Matrix4& localToWorld) const {
	switch (mode) {
		case SelectionSystem::eVertex:
			collector.addRenderable(_uniqueVertexPoints, localToWorld);
			break;
		case SelectionSystem::eEdge:
			collector.addRenderable(_uniqueEdgePoints, localToWorld);
			break;
		case SelectionSystem::eFace:
			collector.addRenderable(_faceCentroidPoints, localToWorld);
			break;
		default:
			break;
	}
}

void Brush::transform(const Matrix4& matrix) {
	bool mirror = matrix4_handedness(matrix) == MATRIX4_LEFTHANDED;

	for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i) {
		(*i)->transform(matrix, mirror);
	}
}

void Brush::snapto(float snap) {
    for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i) {
		(*i)->snapto(snap);
	}
}

void Brush::revertTransform() {
	for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i) {
		(*i)->revertTransform();
	}
}

void Brush::freezeTransform() {
	for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i) {
		(*i)->freezeTransform();
	}
}

/// \brief Returns the absolute index of the \p faceVertex.
std::size_t Brush::absoluteIndex(FaceVertexId faceVertex) {
	std::size_t index = 0;
	for(std::size_t i = 0; i < faceVertex.getFace(); ++i) {
		index += m_faces[i]->getWinding().size();
	}
	return index + faceVertex.getVertex();
}

void Brush::appendFaces(const Faces& other) {
	clear();
	for(Faces::const_iterator i = other.begin(); i != other.end(); ++i) {
		push_back(*i);
	}
}

void Brush::undoSave() {
	if (m_map != 0) {
		m_map->changed();
	}
	
	if (m_undoable_observer != 0) {
		m_undoable_observer->save(this);
	}
}

UndoMemento* Brush::exportState() const {
	return new BrushUndoMemento(m_faces);
}

void Brush::importState(const UndoMemento* state) {
	undoSave();
	appendFaces(static_cast<const BrushUndoMemento*>(state)->m_faces);
	planeChanged();

	for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->DEBUG_verify();
	}
}

/// \brief Appends a copy of \p face to the end of the face list.
FacePtr Brush::addFace(const Face& face) {
	if (m_faces.size() == c_brush_maxFaces) {
		return FacePtr();
	}
	undoSave();
	push_back(FacePtr(new Face(*this, face, this)));
	planeChanged();
	return m_faces.back();
}

/// \brief Appends a new face constructed from the parameters to the end of the face list.
FacePtr Brush::addPlane(const Vector3& p0, const Vector3& p1, const Vector3& p2, const std::string& shader, const TextureProjection& projection) {
	if(m_faces.size() == c_brush_maxFaces) {
		return FacePtr();
	}
	undoSave();
	push_back(FacePtr(new Face(*this, p0, p1, p2, shader, projection, this)));
	planeChanged();
	return m_faces.back();
}

void Brush::constructStatic() {
	Face::m_quantise = quantiseFloating;

	m_state_point = GlobalRenderSystem().capture("$POINT");
}

void Brush::destroyStatic() {

}

std::size_t Brush::DEBUG_size() {
	return m_faces.size();
}

Brush::const_iterator Brush::begin() const {
	return m_faces.begin();
}
Brush::const_iterator Brush::end() const {
	return m_faces.end();
}

FacePtr Brush::back() {
	return m_faces.back();
}
const FacePtr Brush::back() const {
	return m_faces.back();
}

void Brush::reserve(std::size_t count) {
	m_faces.reserve(count);
	for (Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->reserve(count);
	}
}

void Brush::push_back(Faces::value_type face) {
	m_faces.push_back(face);
	
	if (m_instanceCounter.m_count != 0) {
		m_faces.back()->instanceAttach(m_map);
	}
	
	for (Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->push_back(*face);
		(*i)->DEBUG_verify();
	}
}

void Brush::pop_back() {
	if (m_instanceCounter.m_count != 0) {
		m_faces.back()->instanceDetach(m_map);
	}
	
	m_faces.pop_back();
	for (Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->pop_back();
		(*i)->DEBUG_verify();
	}
}

void Brush::erase(std::size_t index) {
	if (m_instanceCounter.m_count != 0) {
		m_faces[index]->instanceDetach(m_map);
	}
	
	m_faces.erase(m_faces.begin() + index);
	for (Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->erase(index);
		(*i)->DEBUG_verify();
	}
}

void Brush::connectivityChanged() {
	for (Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->connectivityChanged();
	}
}

void Brush::clear() {
	undoSave();
	if (m_instanceCounter.m_count != 0) {
		forEachFace_instanceDetach(m_map);
	}
	
	m_faces.clear();
	
	for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->clear();
		(*i)->DEBUG_verify();
	}
}

std::size_t Brush::getNumFaces() const
{
	return m_faces.size();
}

bool Brush::empty() const {
	return m_faces.empty();
}

/// \brief Returns true if any face of the brush contributes to the final B-Rep.
bool Brush::hasContributingFaces() const {
	for (const_iterator i = begin(); i != end(); ++i) {
		if ((*i)->contributes()) {
			return true;
		}
	}
	return false;
}

/// \brief Removes faces that do not contribute to the brush. This is useful for cleaning up after CSG operations on the brush.
/// Note: removal of empty faces is not performed during direct brush manipulations, because it would make a manipulation irreversible if it created an empty face.
void Brush::removeEmptyFaces() {
	evaluateBRep();

	std::size_t i = 0;
	while (i < m_faces.size()) {
		if (!m_faces[i]->contributes()) {
			erase(i);
			planeChanged();
		}
		else {
			++i;
		}
	}
}

/// \brief Constructs \p winding from the intersection of \p plane with the other planes of the brush.
void Brush::windingForClipPlane(Winding& winding, const Plane3& plane) const {
	FixedWinding buffer[2];
	bool swap = false;

	// get a poly that covers an effectively infinite area
	buffer[swap].createInfinite(plane, m_maxWorldCoord + 1);

	// chop the poly by all of the other faces
	{
		for (std::size_t i = 0;  i < m_faces.size(); ++i) {
			const Face& clip = *m_faces[i];

			if (clip.plane3() == plane 
				|| !clip.plane3().isValid() || !plane_unique(i)
				|| plane == -clip.plane3())
			{
				continue;
			}

			buffer[!swap].clear();

			{
				// flip the plane, because we want to keep the back side
				Plane3 clipPlane(-clip.plane3().normal(), -clip.plane3().dist());
				buffer[swap].clip(plane, clipPlane, i, buffer[!swap]);
			}

			swap = !swap;
		}
	}

	buffer[swap].writeToWinding(winding);
}

void Brush::update_wireframe(RenderableWireframe& wire, const bool* faces_visible) const
{
	wire.m_faceVertex.resize(_edgeIndices.size());
	wire.m_vertices = _uniqueVertexPoints.size() > 0 ? &_uniqueVertexPoints.front() : NULL;
	wire.m_size = 0;

	for (std::size_t i = 0; i < _edgeFaces.size(); ++i)
	{
		if (faces_visible[_edgeFaces[i].first] || faces_visible[_edgeFaces[i].second])
		{
			wire.m_faceVertex[wire.m_size++] = _edgeIndices[i];
		}
	}
}

void Brush::update_faces_wireframe(RenderablePointVector& wire, 
								   const std::size_t* visibleFaceIndices, 
								   std::size_t numVisibleFaces) const
{
	assert(numVisibleFaces <= _faceCentroidPoints.size());

	// Assure that the pointvector can carry as many faces as are visible
	wire.resize(numVisibleFaces);

	const std::size_t* visibleFaceIter = visibleFaceIndices;
	
	// Pick all the visible face centroids from the vector
	for (std::size_t i = 0; i < numVisibleFaces; ++i)
	{
		wire[i] = _faceCentroidPoints[*visibleFaceIter++];
	}
}

/// \brief Makes this brush a deep-copy of the \p other.
void Brush::copy(const Brush& other) {
	for (Faces::const_iterator i = other.m_faces.begin(); i != other.m_faces.end(); ++i) {
		addFace(*(*i));
	}
	planeChanged();
}

void Brush::edge_push_back(FaceVertexId faceVertex) {
	m_select_edges.push_back(SelectableEdge(m_faces, faceVertex));
	for (Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->edge_push_back(m_select_edges.back());
	}
}

void Brush::edge_clear() {
	m_select_edges.clear();
	for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->edge_clear();
	}
}

void Brush::vertex_push_back(FaceVertexId faceVertex) {
	m_select_vertices.push_back(SelectableVertex(m_faces, faceVertex));
	for (Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->vertex_push_back(m_select_vertices.back());
	}
}

void Brush::vertex_clear() {
	m_select_vertices.clear();
	for (Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
		(*i)->vertex_clear();
	}
}

/// \brief Returns true if the face identified by \p index is preceded by another plane that takes priority over it.
bool Brush::plane_unique(std::size_t index) const {
	// duplicate plane
	for (std::size_t i = 0; i < m_faces.size(); ++i) {
		if (index != i && !plane3_inside(m_faces[index]->plane3(), m_faces[i]->plane3())) {
			return false;
		}
	}
	return true;
}

/// \brief Removes edges that are smaller than the tolerance used when generating brush windings.
void Brush::removeDegenerateEdges() {
	for (std::size_t i = 0;  i < m_faces.size(); ++i) {
		Winding& winding = m_faces[i]->getWinding();
		
		for (std::size_t index = 0; index < winding.size();) {
			//std::size_t index = std::distance(winding.begin(), j);
			std::size_t next = winding.next(index);
			
			if (Edge_isDegenerate(winding[index].vertex, winding[next].vertex)) {
				Winding& other = m_faces[winding[index].adjacent]->getWinding();
				std::size_t adjacent = other.findAdjacent(i);
				if (adjacent != c_brush_maxFaces) {
					other.erase(other.begin() + adjacent);
				}
				
				// Delete and leave index where it is
				winding.erase(winding.begin() + index);
			}
			else {
				++index;
			}
		}
	}
}

/// \brief Invalidates faces that have only two vertices in their winding, while preserving edge-connectivity information.
void Brush::removeDegenerateFaces() {
	// save adjacency info for degenerate faces
	for (std::size_t i = 0;  i < m_faces.size(); ++i) {
		Winding& degen = m_faces[i]->getWinding();
  
		if (degen.size() == 2) {
			/*std::cout << "Removed degenerate face: " << Vector3(m_faces[i]->getPlane().plane3().normal()) 
								<< " - " << float(m_faces[i]->getPlane().plane3().dist()) << "\n";*/
			
			// this is an "edge" face, where the plane touches the edge of the brush
			{
				Winding& winding = m_faces[degen[0].adjacent]->getWinding();
				std::size_t index = winding.findAdjacent(i);
				if (index != c_brush_maxFaces) {
					winding[index].adjacent = degen[1].adjacent;
				}
			}

			{
				Winding& winding = m_faces[degen[1].adjacent]->getWinding();
				std::size_t index = winding.findAdjacent(i);
				
				if (index != c_brush_maxFaces) {
					winding[index].adjacent = degen[0].adjacent;
				}
			}

			degen.resize(0);
		}
	}
}

/// \brief Removes edges that have the same adjacent-face as their immediate neighbour.
void Brush::removeDuplicateEdges() {
	// verify face connectivity graph
	for(std::size_t i = 0; i < m_faces.size(); ++i) {
		//if(m_faces[i]->contributes())
			{
				Winding& winding = m_faces[i]->getWinding();
				for (std::size_t j = 0; j != winding.size();) {
					std::size_t next = winding.next(j);
					if (winding[j].adjacent == winding[next].adjacent) {
						winding.erase(winding.begin() + next);
					}
					else {
						++j;
					}
				}
			}
		}
	}

/// \brief Removes edges that do not have a matching pair in their adjacent-face.
void Brush::verifyConnectivityGraph() {
	// verify face connectivity graph
	for (std::size_t i = 0; i < m_faces.size(); ++i) {
		//if(m_faces[i]->contributes())
		{
			Winding& winding = m_faces[i]->getWinding();

			for (std::size_t j = 0; j < winding.size();) {
				WindingVertex& vertex = winding[j];

				// remove unidirectional graph edges
				if (vertex.adjacent == c_brush_maxFaces
					|| m_faces[vertex.adjacent]->getWinding().findAdjacent(i) == c_brush_maxFaces)
				{
					// Delete the offending vertex and leave the index j where it is
					winding.erase(winding.begin() + j);
				}
				else {
					++j;
				}
			}

			/*for (Winding::iterator j = winding.begin(); j != winding.end();) {
				// remove unidirectional graph edges
				if (j->adjacent == c_brush_maxFaces
					|| m_faces[j->adjacent]->getWinding().findAdjacent(i) == c_brush_maxFaces)
				{
					// Delete and return the new iterator
					j = winding.erase(j);
				}
				else {
					++j;
				}
			}*/
		}
	}
}

/// \brief Returns true if the brush is a finite volume. A brush without a finite volume extends past the maximum world bounds and is not valid.
bool Brush::isBounded() {
	for (const_iterator i = begin(); i != end(); ++i) {
		if (!(*i)->is_bounded()) {
			return false;
		}
	}
	return true;
}

/// \brief Constructs the polygon windings for each face of the brush. Also updates the brush bounding-box and face texture-coordinates.
bool Brush::buildWindings() {
	{
		m_aabb_local = AABB();

		for (std::size_t i = 0;  i < m_faces.size(); ++i) {
			Face& f = *m_faces[i];

			if (!f.plane3().isValid() || !plane_unique(i)) {
				f.getWinding().resize(0);
			}
			else {
				windingForClipPlane(f.getWinding(), f.plane3());

				// update brush bounds
				const Winding& winding = f.getWinding();
				
				for (Winding::const_iterator i = winding.begin(); i != winding.end(); ++i) {
					m_aabb_local.includePoint(i->vertex);
				}

				// update texture coordinates
				f.EmitTextureCoordinates();
			}
			
			// greebo: Update the winding, now that it's constructed
			f.updateWinding();
		}
	}

	bool degenerate = !isBounded();

	if (!degenerate) {
		// clean up connectivity information.
		// these cleanups must be applied in a specific order.
		removeDegenerateEdges();
		removeDegenerateFaces();
		removeDuplicateEdges();
		verifyConnectivityGraph();
	}

	return degenerate;
}

struct SListNode {
	SListNode* m_next;
};

class ProximalVertex {
public:
	const SListNode* m_vertices;

	ProximalVertex(const SListNode* next)
		: m_vertices(next)
	{}

	bool operator<(const ProximalVertex& other) const {
		if (!(operator==(other))) {
			return m_vertices < other.m_vertices;
		}
		return false;
	}
	
	bool operator==(const ProximalVertex& other) const {
		const SListNode* v = m_vertices;
		std::size_t DEBUG_LOOP = 0;
		
		do {
			if (v == other.m_vertices)
				return true;
			v = v->m_next;
			//ASSERT_MESSAGE(DEBUG_LOOP < c_brush_maxFaces, "infinite loop");
			if (!(DEBUG_LOOP < c_brush_maxFaces)) {
				break;
			}
			++DEBUG_LOOP;
		} while(v != m_vertices);
		
		return false;
	}
};

typedef std::vector<SListNode> ProximalVertexArray;
std::size_t ProximalVertexArray_index(const ProximalVertexArray& array, const ProximalVertex& vertex) {
	return vertex.m_vertices - &array.front();
}

/// \brief Constructs the face windings and updates anything that depends on them.
void Brush::buildBRep() {
  bool degenerate = buildWindings();

  Vector3 colourVertexVec = ColourSchemes().getColour("brush_vertices");
  const Colour4b colour_vertex(int(colourVertexVec[0]*255), int(colourVertexVec[1]*255), 
  							   int(colourVertexVec[2]*255), 255);

  std::size_t faces_size = 0;
  std::size_t faceVerticesCount = 0;
  for (Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i) {
    if ((*i)->contributes()) {
      ++faces_size;
    }
    faceVerticesCount += (*i)->getWinding().size();
  }

  if(degenerate || faces_size < 4 || faceVerticesCount != (faceVerticesCount>>1)<<1) // sum of vertices for each face of a valid polyhedron is always even
  {
    _uniqueVertexPoints.resize(0);

    vertex_clear();
    edge_clear();

    _edgeIndices.resize(0);
    _edgeFaces.resize(0);

    _faceCentroidPoints.resize(0);
    _uniqueEdgePoints.resize(0);

    for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      (*i)->getWinding().resize(0);
    }
  }
  else
  {
    {
      typedef std::vector<FaceVertexId> FaceVertices;
      FaceVertices faceVertices;
      faceVertices.reserve(faceVerticesCount);

      {
        for(std::size_t i = 0; i != m_faces.size(); ++i)
        {
          for(std::size_t j = 0; j < m_faces[i]->getWinding().size(); ++j)
          {
            faceVertices.push_back(FaceVertexId(i, j));
          }
        }
      }

      IndexBuffer uniqueEdgeIndices;
      typedef VertexBuffer<ProximalVertex> UniqueEdges;
      UniqueEdges uniqueEdges;

      uniqueEdgeIndices.reserve(faceVertices.size());
      uniqueEdges.reserve(faceVertices.size());

      {
        ProximalVertexArray edgePairs;
        edgePairs.resize(faceVertices.size());

        {
          for(std::size_t i=0; i<faceVertices.size(); ++i)
          {
            edgePairs[i].m_next = &edgePairs.front() + absoluteIndex(next_edge(m_faces, faceVertices[i]));
          }
        }

        {
          UniqueVertexBuffer<ProximalVertex> inserter(uniqueEdges);
          for(ProximalVertexArray::iterator i = edgePairs.begin(); i != edgePairs.end(); ++i)
          {
            uniqueEdgeIndices.insert(inserter.insert(ProximalVertex(&(*i))));
          }
        }

        {
          edge_clear();
          m_select_edges.reserve(uniqueEdges.size());
          for(UniqueEdges::iterator i = uniqueEdges.begin(); i != uniqueEdges.end(); ++i)
          {
            edge_push_back(faceVertices[ProximalVertexArray_index(edgePairs, *i)]);
          }
        }

        {
          _edgeFaces.resize(uniqueEdges.size());
          for(std::size_t i=0; i<uniqueEdges.size(); ++i)
          {
            FaceVertexId faceVertex = faceVertices[ProximalVertexArray_index(edgePairs, uniqueEdges[i])];
            _edgeFaces[i] = EdgeFaces(faceVertex.getFace(), m_faces[faceVertex.getFace()]->getWinding()[faceVertex.getVertex()].adjacent);
          }
        }

        {
          _uniqueEdgePoints.resize(uniqueEdges.size());

          for(std::size_t i=0; i<uniqueEdges.size(); ++i)
          {
            FaceVertexId faceVertex = faceVertices[ProximalVertexArray_index(edgePairs, uniqueEdges[i])];

            const Winding& w = m_faces[faceVertex.getFace()]->getWinding();
            Vector3 edge = vector3_mid(w[faceVertex.getVertex()].vertex, w[w.next(faceVertex.getVertex())].vertex);
            _uniqueEdgePoints[i] = PointVertex(edge, colour_vertex);
          }
        }

      }
    

      IndexBuffer uniqueVertexIndices;
      typedef VertexBuffer<ProximalVertex> UniqueVertices;
      UniqueVertices uniqueVertices;

      uniqueVertexIndices.reserve(faceVertices.size());
      uniqueVertices.reserve(faceVertices.size());

      {
        ProximalVertexArray vertexRings;
        vertexRings.resize(faceVertices.size());

        {
          for(std::size_t i=0; i<faceVertices.size(); ++i)
          {
            vertexRings[i].m_next = &vertexRings.front() + absoluteIndex(next_vertex(m_faces, faceVertices[i]));
          }
        }

        {
          UniqueVertexBuffer<ProximalVertex> inserter(uniqueVertices);
          for(ProximalVertexArray::iterator i = vertexRings.begin(); i != vertexRings.end(); ++i)
          {
            uniqueVertexIndices.insert(inserter.insert(ProximalVertex(&(*i))));
          }
        }

        {
          vertex_clear();
          m_select_vertices.reserve(uniqueVertices.size());  
          for(UniqueVertices::iterator i = uniqueVertices.begin(); i != uniqueVertices.end(); ++i)
          {
            vertex_push_back(faceVertices[ProximalVertexArray_index(vertexRings, (*i))]);
          }
        }

        {
          _uniqueVertexPoints.resize(uniqueVertices.size());

          for(std::size_t i=0; i<uniqueVertices.size(); ++i)
          {
            FaceVertexId faceVertex = faceVertices[ProximalVertexArray_index(vertexRings, uniqueVertices[i])];

            const Winding& winding = m_faces[faceVertex.getFace()]->getWinding();
            _uniqueVertexPoints[i] = PointVertex(winding[faceVertex.getVertex()].vertex, colour_vertex);
          }
        }
      }

      if((uniqueVertices.size() + faces_size) - uniqueEdges.size() != 2)
      {
        globalErrorStream() << "Final B-Rep: inconsistent vertex count\n";
      }

      // edge-index list for wireframe rendering
      {
        _edgeIndices.resize(uniqueEdgeIndices.size());

        for(std::size_t i=0, count=0; i<m_faces.size(); ++i)
        {
          const Winding& winding = m_faces[i]->getWinding();
          for(std::size_t j = 0; j < winding.size(); ++j)
          {
            const RenderIndex edge_index = uniqueEdgeIndices[count+j];

            _edgeIndices[edge_index].first = uniqueVertexIndices[count + j];
            _edgeIndices[edge_index].second = uniqueVertexIndices[count + winding.next(j)];
          }
          count += winding.size();
        }
      }
    }

    {
      _faceCentroidPoints.resize(m_faces.size());

      for(std::size_t i=0; i<m_faces.size(); ++i)
      {
        m_faces[i]->construct_centroid();
        _faceCentroidPoints[i] = PointVertex(m_faces[i]->centroid(), colour_vertex);
      }
    }
  }
}

// ----------------------------------------------------------------------------

double Brush::m_maxWorldCoord = 0;
ShaderPtr Brush::m_state_point;
