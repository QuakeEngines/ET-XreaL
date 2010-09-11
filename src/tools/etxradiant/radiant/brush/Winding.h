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

#if !defined(INCLUDED_WINDING_H)
#define INCLUDED_WINDING_H

#include "debugging/debugging.h"

#include <vector>

#include "iclipper.h"
#include "irender.h"
#include "iselectable.h"
#include "ibrush.h"

#include "math/Vector2.h"
#include "math/Vector3.h"

const double ON_EPSILON	= 1.0 / (1 << 8);

class SelectionIntersection;

// The Winding structure extends the abstract IWinding class
// by a few methods for rendering and selection tests.
class Winding :
	public IWinding
{
public:
	/** greebo: Calculates the AABB of this winding
	 */
	AABB aabb() const;
	
	void testSelect(SelectionTest& test, SelectionIntersection& best);
	
	// greebo: Updates the array containing the normal vectors of this winding
	// The normal is the same for each vertex, so this just copies the values 
	void updateNormals(const Vector3& normal);
	
	// Submits this winding to OpenGL
	void render(const RenderInfo& info) const;

	// Submits the wireframe render commands to OpenGL
	void drawWireframe() const;
	
	// Wraps the given index around if it's larger than the size of this winding
	inline std::size_t wrap(std::size_t i) const
	{
		assert(!empty());
		return i % size();
	}
	
	// Returns the next winding index (wraps around)
	inline std::size_t next(std::size_t i) const
	{
		return wrap(++i);
	}
	
	std::size_t findAdjacent(std::size_t face) const;
	std::size_t opposite(const std::size_t index, const std::size_t other) const;
	std::size_t opposite(std::size_t index) const;
	
	// Returns the classification for the given plane
	BrushSplitType classifyPlane(const Plane3& plane) const;
	
	static PlaneClassification classifyDistance(const double distance, const double epsilon);
	
	/// \brief Returns true if
	/// !flipped && winding is completely BACK or ON
	/// or flipped && winding is completely FRONT or ON
	bool testPlane(const Plane3& plane, bool flipped) const;
	
	// Return the centroid of the polygon defined by this winding lying on the given plane.
	Vector3 centroid(const Plane3& plane) const;
	
	// For debugging purposes: prints the vertices and their adjacents to the console
	void printConnectivity();
	
	/// \brief Returns true if any point in \p w1 is in front of plane2, or any point in \p w2 is in front of plane1
	static bool planesConcave(const Winding& w1, const Winding& w2, const Plane3& plane1, const Plane3& plane2);
};

#endif
