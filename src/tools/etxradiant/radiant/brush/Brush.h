#ifndef BRUSH_BRUSH_H_
#define BRUSH_BRUSH_H_

#include "scenelib.h"
#include "editable.h"

#include "Face.h"
#include "SelectableComponents.h"
#include "RenderableWireFrame.h"

#include <boost/noncopyable.hpp>

class RenderableCollector;

const std::size_t c_brush_maxFaces = 1024;

inline double quantiseFloating(double f) {
	return float_snapped(f, 1.f / (1 << 16));
}

/// \brief Returns true if 'self' takes priority when building brush b-rep.
inline bool plane3_inside(const Plane3& self, const Plane3& other) {
	if (vector3_equal_epsilon(self.normal(), other.normal(), 0.001)) {
		return self.dist() < other.dist();
	}
	return true;
}

/// \brief Returns the unique-id of the edge adjacent to \p faceVertex in the edge-pair for the set of \p faces.
inline FaceVertexId next_edge(const Faces& faces, FaceVertexId faceVertex) {
	std::size_t adjacent_face = faces[faceVertex.getFace()]->getWinding()[faceVertex.getVertex()].adjacent;
	std::size_t adjacent_vertex = faces[adjacent_face]->getWinding().findAdjacent(faceVertex.getFace());

	ASSERT_MESSAGE(adjacent_vertex != c_brush_maxFaces, "connectivity data invalid");
	
	if (adjacent_vertex == c_brush_maxFaces) {
		return faceVertex;
	}

	return FaceVertexId(adjacent_face, adjacent_vertex);
}

/// \brief Returns the unique-id of the vertex adjacent to \p faceVertex in the vertex-ring for the set of \p faces.
inline FaceVertexId next_vertex(const Faces& faces, FaceVertexId faceVertex) {
	FaceVertexId nextEdge = next_edge(faces, faceVertex);
	return FaceVertexId(nextEdge.getFace(), faces[nextEdge.getFace()]->getWinding().next(nextEdge.getVertex()));
}

// greebo: A structure associating a brush edge to two faces
struct EdgeFaces {
	std::size_t first;
	std::size_t second;

	EdgeFaces()
		: first(c_brush_maxFaces), second(c_brush_maxFaces)
	{}

	EdgeFaces(const std::size_t _first, const std::size_t _second)
		: first(_first), second(_second)
	{}
};

class BrushObserver {
public:
    virtual ~BrushObserver() {}
	virtual void reserve(std::size_t size) = 0;
	virtual void clear() = 0;
	virtual void push_back(Face& face) = 0;
	virtual void pop_back() = 0;
	virtual void erase(std::size_t index) = 0;
	virtual void connectivityChanged() = 0;
	
	virtual void edge_clear() = 0;
	virtual void edge_push_back(SelectableEdge& edge) = 0;
	
	virtual void vertex_clear() = 0;
	virtual void vertex_push_back(SelectableVertex& vertex) = 0;
	
	virtual void DEBUG_verify() = 0;
};

class BrushVisitor {
public:
    virtual ~BrushVisitor() {}
	virtual void visit(Face& face) const = 0;
};

class BrushNode;

class Brush :
	public IBrush,
	public Bounded,
	public Snappable,
	public Undoable,
	public FaceObserver,
	public BrushDoom3,
	public boost::noncopyable
{
private:
	BrushNode& _owner;

	typedef std::set<BrushObserver*> Observers;
	Observers m_observers;
	UndoObserver* m_undoable_observer;
	MapFile* m_map;
	
	// state
	Faces m_faces;
	// ----
	
	// cached data compiled from state
	RenderablePointVector _faceCentroidPoints;
	RenderablePointVector _uniqueVertexPoints;
	RenderablePointVector _uniqueEdgePoints;

	typedef std::vector<SelectableVertex> SelectableVertices;
	SelectableVertices m_select_vertices;
	
	typedef std::vector<SelectableEdge> SelectableEdges;
	SelectableEdges m_select_edges;
	
	// A list of all edge render indices, one for each unique edge
	std::vector<EdgeRenderIndices> _edgeIndices;

	// A list of face indices, one for each unique edge
	std::vector<EdgeFaces> _edgeFaces;
	
	AABB m_aabb_local;
	// ----
	
	Callback m_evaluateTransform;
	Callback m_boundsChanged;
	
	mutable bool m_planeChanged; // b-rep evaluation required
	mutable bool m_transformChanged; // transform evaluation required
	// ----

public:  
	/// \brief The undo memento for a brush stores only the list of face references - the faces are not copied.
	class BrushUndoMemento : public UndoMemento {
	public:
		BrushUndoMemento(const Faces& faces) : m_faces(faces) {}
		virtual ~BrushUndoMemento() {}
		
		void release() {
			delete this;
		}
	
		Faces m_faces;
	};
	
	// static data
	static ShaderPtr m_state_point;
	// ----
	
	static double m_maxWorldCoord;
	
	// Constructors
	Brush(BrushNode& owner, const Callback& evaluateTransform, const Callback& boundsChanged);
	Brush(BrushNode& owner, const Brush& other, const Callback& evaluateTransform, const Callback& boundsChanged);
	
	// Destructor
	virtual ~Brush();

	// BrushNode
	BrushNode& getBrushNode();

	IFace& getFace(std::size_t index);

	IFace& addFace(const Plane3& plane);
	IFace& addFace(const Plane3& plane, const Matrix4& texDef, const std::string& shader);
	
	/** greebo: This translates the brush about the given translation vector,
	 * this is used by the Doom3Group entity to add/substract the origin from
	 * their child brushes. The translation is TextureLock-sensitive.
	 */
	void translateDoom3Brush(const Vector3& translation);
	
	void attach(BrushObserver& observer);
	void detach(BrushObserver& observer);
	
	void forEachFace(const BrushVisitor& visitor) const;
	
	void forEachFace_instanceAttach(MapFile* map) const;
	void forEachFace_instanceDetach(MapFile* map) const;
	
	InstanceCounter m_instanceCounter;
	
	void instanceAttach(MapFile* map);
	void instanceDetach(MapFile* map);
	
	// observer
	void planeChanged();
	void shaderChanged();
	
	// Sets the shader of all faces to the given name
	void setShader(const std::string& newShader);

	// Returns TRUE if any of the faces has the given shader
	bool hasShader(const std::string& name);

	// Returns TRUE if any face materials are visible
	bool hasVisibleMaterial() const;

	void evaluateBRep() const;
	
	void transformChanged();
	void evaluateTransform();
	
	void aabbChanged();
	
	const AABB& localAABB() const;
	
	void renderComponents(SelectionSystem::EComponentMode mode, RenderableCollector& collector, const VolumeTest& volume, const Matrix4& localToWorld) const;
	
	void transform(const Matrix4& matrix);
	
	void snapto(float snap);
	
	void revertTransform();
	void freezeTransform();
	
	/// \brief Returns the absolute index of the \p faceVertex.
	std::size_t absoluteIndex(FaceVertexId faceVertex);
	
	void appendFaces(const Faces& other);

	void undoSave();
	UndoMemento* exportState() const;
	void importState(const UndoMemento* state);
	
	/// \brief Appends a copy of \p face to the end of the face list.
	FacePtr addFace(const Face& face);

	/// \brief Appends a new face constructed from the parameters to the end of the face list.
	FacePtr addPlane(const Vector3& p0, const Vector3& p1, const Vector3& p2, const std::string& shader, const TextureProjection& projection);
	
	static void constructStatic();
	static void destroyStatic();

	std::size_t DEBUG_size();

	typedef Faces::const_iterator const_iterator;

	const_iterator begin() const;
	const_iterator end() const;

	FacePtr back();
	const FacePtr back() const;
	
	void reserve(std::size_t count);
	
	void push_back(Faces::value_type face);
	
	void pop_back();
	void erase(std::size_t index);
	
	void connectivityChanged();

	void clear();
	
	std::size_t getNumFaces() const;
	
	bool empty() const;

	/// \brief Returns true if any face of the brush contributes to the final B-Rep.
	bool hasContributingFaces() const;

	/// \brief Removes faces that do not contribute to the brush. This is useful for cleaning up after CSG operations on the brush.
	/// Note: removal of empty faces is not performed during direct brush manipulations, because it would make a manipulation irreversible if it created an empty face.
	void removeEmptyFaces();

	/// \brief Constructs \p winding from the intersection of \p plane with the other planes of the brush.
	void windingForClipPlane(Winding& winding, const Plane3& plane) const;

	void update_wireframe(RenderableWireframe& wire, const bool* faces_visible) const;

	void update_faces_wireframe(RenderablePointVector& wire, 
								const std::size_t* visibleFaceIndices, 
								std::size_t numVisibleFaces) const;

	/// \brief Makes this brush a deep-copy of the \p other.
	void copy(const Brush& other);

private:
	void edge_push_back(FaceVertexId faceVertex);
	
	void edge_clear();
	
	void vertex_push_back(FaceVertexId faceVertex);
	
	void vertex_clear();

	/// \brief Returns true if the face identified by \p index is preceded by another plane that takes priority over it.
	bool plane_unique(std::size_t index) const;

	/// \brief Removes edges that are smaller than the tolerance used when generating brush windings.
	void removeDegenerateEdges();

	/// \brief Invalidates faces that have only two vertices in their winding, while preserving edge-connectivity information.
	void removeDegenerateFaces();

	/// \brief Removes edges that have the same adjacent-face as their immediate neighbour.
	void removeDuplicateEdges();

	/// \brief Removes edges that do not have a matching pair in their adjacent-face.
	void verifyConnectivityGraph();

	/// \brief Returns true if the brush is a finite volume. A brush without a finite volume extends past the maximum world bounds and is not valid.
	bool isBounded();

	/// \brief Constructs the polygon windings for each face of the brush. Also updates the brush bounding-box and face texture-coordinates.
	bool buildWindings();

	/// \brief Constructs the face windings and updates anything that depends on them.
	void buildBRep();
}; // class Brush

typedef std::vector<Brush*> BrushVector;

/**
 * Stream insertion for Brush objects.
 */
inline std::ostream& operator<< (std::ostream& os, const Brush& b) {
	os << "Brush { size = " << b.getNumFaces() << ", localAABB = " << b.localAABB()
       << " }";
    return os;
}

#endif /*BRUSH_BRUSH_H_*/
