#include "Curve.h"

#include "itextstream.h"
#include "string/string.h"
#include "parser/Tokeniser.h"

namespace entity {

	namespace {
		
		inline void PointVertexArray_testSelect(PointVertex* first, std::size_t count, 
			SelectionTest& test, SelectionIntersection& best) 
		{
			test.TestLineStrip(
			    VertexPointer(&first->vertex, sizeof(PointVertex)),
			    IndexPointer::index_type(count),
			    best
			);
		}
		
	} // namespace

Curve::Curve(const Callback& boundsChanged) :
	_boundsChanged(boundsChanged)
{}

std::string Curve::getEntityKeyValue() {
	std::string value;
	
	if (!_controlPointsTransformed.empty()) {
		value = sizetToStr(_controlPointsTransformed.size()) + " (";
		for (ControlPoints::const_iterator i = _controlPointsTransformed.begin(); 
			 i != _controlPointsTransformed.end(); 
			 ++i)
		{
			value += " " + doubleToStr(i->x()) + " " + 
					 doubleToStr(i->y()) + " " + doubleToStr(i->z()) + " ";
		}
		value += ")";
	}
	
	return value;
}

std::size_t Curve::connect(const CurveChangedCallback& curveChanged)
{
	curveChanged();
	return _curveChanged.connect(curveChanged);
}

void Curve::disconnect(std::size_t id)
{
	_curveChanged.disconnect(id);
}

void Curve::testSelect(Selector& selector, SelectionTest& test, SelectionIntersection& best) {
	if (_renderCurve.m_vertices.size() > 0) {
		PointVertexArray_testSelect(
			&_renderCurve.m_vertices[0], 
			_renderCurve.m_vertices.size(), 
			test, 
			best
		);
	}
}

void Curve::revertTransform() {
	_controlPointsTransformed = _controlPoints;
}

void Curve::freezeTransform() {
	_controlPoints = _controlPointsTransformed;
}

ControlPoints& Curve::getTransformedControlPoints() {
	return _controlPointsTransformed;
}

ControlPoints& Curve::getControlPoints() {
	return _controlPoints;
}

void Curve::renderSolid(RenderableCollector& collector, const VolumeTest& volume, 
	const Matrix4& localToWorld) const
{
	collector.addRenderable(_renderCurve, localToWorld);
}

const AABB& Curve::getBounds() const {
	return _bounds;
}

bool Curve::isEmpty() const {
	return _renderCurve.m_vertices.size() == 0;
}

bool Curve::parseCurve(const std::string& value) {
	parser::BasicStringTokeniser tokeniser(value, " ");
	
	try {
		// First token is the number of control points
		std::size_t size = strToInt(tokeniser.nextToken());
		if (size < 3) {
			throw parser::ParseException("Curve size < 3.");
		}
		_controlPoints.resize(size);

		tokeniser.assertNextToken("(");
		
		for (ControlPoints::iterator i = _controlPoints.begin(); 
			 i != _controlPoints.end(); 
			 ++i)
		{
			i->x() = strToFloat(tokeniser.nextToken());
			i->y() = strToFloat(tokeniser.nextToken());
			i->z() = strToFloat(tokeniser.nextToken());
		}
		
		tokeniser.assertNextToken(")");
	}
	catch (parser::ParseException p) {
		globalErrorStream() << "Curve::parseCurve: " << p.what() << "\n";
		return false;
	}
	
	return true;
}

void Curve::curveChanged() {
	// Recalculate the tesselation
	tesselate();

	// Recalculate bounds
    _bounds = AABB();
    for (ControlPoints::iterator i = _controlPointsTransformed.begin(); 
    	 i != _controlPointsTransformed.end(); 
    	 ++i)
	{
		_bounds.includePoint(*i);
	}

	// Notify the bounds changed observer
	_boundsChanged();
	
	// Emit the "curve changed" signal
	_curveChanged();
}

void Curve::onKeyValueChanged(const std::string& value) {
	// Try to parse and check for validity
	if (value.empty() || !parseCurve(value)) {
		clearCurve();
	}
	
	// Assimilate the working set 
	_controlPointsTransformed = _controlPoints;
	
	// Do the tesselation and emit the signals
	curveChanged();
}

void Curve::appendControlPoints(unsigned int numPoints) {
	std::size_t size = _controlPoints.size();
	
	if (size < 1) {
		return;
	}
	
	// The coordinates of the penultimate point (can theoretically be 0,0,0)
	Vector3 penultimate = (size > 1) ? _controlPoints[size - 2] : Vector3(0,0,0);
	Vector3 ultimate = _controlPoints[size - 1];
	
	// Calculate the extrapolation vector
	Vector3 extrapolation = ultimate - penultimate;
	
	// Don't use a 0,0,0 extrapolation vector, this is impractical
	if (extrapolation.getLengthSquared() == 0) {
		extrapolation = Vector3(10,10,0);
	}
	
	// Add as many points as requested to the end of the list
	for (unsigned int i = 1; i <= numPoints; i++) {
		_controlPoints.push_back(ultimate + extrapolation);
	}
	
	// Update the transformation working set
	_controlPointsTransformed = _controlPoints;
}

void Curve::removeControlPoints(IteratorList iterators) {
	ControlPoints newSet;
	
	// Copy all the control points from the existing set into the
	// new set, skipping all points that are marked as deleted.
	for (ControlPoints::iterator i = _controlPointsTransformed.begin();
		 i != _controlPointsTransformed.end();
		 i++)
	{
		// Try to lookup the iterator in the given list
		if (std::find(iterators.begin(), iterators.end(), i) == iterators.end()) {
			// This point is not to be deleted, save it into the new set
			newSet.push_back(*i);
		}
	}
	
	_controlPoints = newSet;
	_controlPointsTransformed = _controlPoints;
}

void Curve::insertControlPointsAt(IteratorList iterators) {
	ControlPoints newSet;
	
	// Copy all the control points from the existing set into the
	// new set, inserting new points at the given locations
	for (ControlPoints::iterator i = _controlPointsTransformed.begin();
		 i != _controlPointsTransformed.end();
		 i++)
	{
		IteratorList::iterator found = std::find(iterators.begin(), iterators.end(), i);
		// Try to lookup the iterator in the given list
		if (found != iterators.end()) {
			// This point is an insert point, add a new control vertex
			
			// Check if this is the first vertex, this would be illegal
			if (i != _controlPointsTransformed.begin()) {
				// Iterator is valid, now add the point in the 
				// middle of the previous point and the current one
				Vector3 intermediate = (*(i-1) + *i) * 0.5;
				newSet.push_back(intermediate);
			}
		}
		// Add the original point to the target list as well.
		newSet.push_back(*i);
	}
	
	_controlPoints = newSet;
	_controlPointsTransformed = _controlPoints;
}

} // namespace entity
