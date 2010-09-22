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

#include "brushmanip.h"

#include "i18n.h"
#include "imainframe.h"
#include "iclipper.h"
#include "icommandsystem.h"
#include "ieventmanager.h"

#include "gtkutil/dialog.h"
#include "gtkutil/widget.h"
#include "brush/BrushNode.h"
#include "ui/texturebrowser/TextureBrowser.h"
#include "ui/brush/QuerySidesDialog.h"
#include "shaderlib.h"

#include "brush/BrushVisit.h"
#include "brush/BrushModule.h"
#include "brush/FaceInstance.h"

#include "selection/algorithm/Shader.h"
#include "selection/algorithm/Primitives.h"
#include "xyview/GlobalXYWnd.h"

#include <list>

FaceInstanceSet g_SelectedFaceInstances;

void Brush_ConstructCuboid(Brush& brush, const AABB& bounds, const std::string& shader, const TextureProjection& projection)
{
  const unsigned char box[3][2] = { { 0, 1 }, { 2, 0 }, { 1, 2 } };
  Vector3 mins(bounds.origin - bounds.extents);
  Vector3 maxs(bounds.origin + bounds.extents);

  brush.clear();
  brush.reserve(6);

  {
    for(int i=0; i < 3; ++i)
    {
      Vector3 planepts1(maxs);
      Vector3 planepts2(maxs);
      planepts2[box[i][0]] = mins[box[i][0]];
      planepts1[box[i][1]] = mins[box[i][1]];

      brush.addPlane(maxs, planepts1, planepts2, shader, projection);
    }
  }
  {
    for(int i=0; i < 3; ++i)
    {
      Vector3 planepts1(mins);
      Vector3 planepts2(mins);
      planepts1[box[i][0]] = maxs[box[i][0]];
      planepts2[box[i][1]] = maxs[box[i][1]];

      brush.addPlane(mins, planepts1, planepts2, shader, projection);
    }
  }
}

inline float max_extent(const Vector3& extents)
{
  return std::max(std::max(extents[0], extents[1]), extents[2]);
}

inline float max_extent_2d(const Vector3& extents, int axis)
{
  switch(axis)
  {
  case 0:
    return std::max(extents[1], extents[2]);
  case 1:
    return std::max(extents[0], extents[2]);
  default:
    return std::max(extents[0], extents[1]);
  }
}

const std::size_t c_brushPrism_minSides = 3;
const std::size_t c_brushPrism_maxSides = c_brush_maxFaces - 2;
const char* const c_brushPrism_name = "brushPrism";

void Brush_ConstructPrism(Brush& brush, const AABB& bounds, std::size_t sides, int axis, const std::string& shader, const TextureProjection& projection)
{
  if(sides < c_brushPrism_minSides)
  {
    globalErrorStream() << c_brushPrism_name << ": sides " << sides << ": too few sides, minimum is " << c_brushPrism_minSides << "\n";
    return;
  }
  if(sides > c_brushPrism_maxSides)
  {
    globalErrorStream() << c_brushPrism_name << ": sides " << sides << ": too many sides, maximum is " << c_brushPrism_maxSides << "\n";
    return;
  }

  brush.clear();
  brush.reserve(sides+2);

  Vector3 mins(bounds.origin - bounds.extents);
  Vector3 maxs(bounds.origin + bounds.extents);

  float radius = max_extent_2d(bounds.extents, axis);
  const Vector3& mid = bounds.origin;
  Vector3 planepts[3];

  planepts[2][(axis+1)%3] = mins[(axis+1)%3];
  planepts[2][(axis+2)%3] = mins[(axis+2)%3];
  planepts[2][axis] = maxs[axis];
  planepts[1][(axis+1)%3] = maxs[(axis+1)%3];
  planepts[1][(axis+2)%3] = mins[(axis+2)%3];
  planepts[1][axis] = maxs[axis];
  planepts[0][(axis+1)%3] = maxs[(axis+1)%3];
  planepts[0][(axis+2)%3] = maxs[(axis+2)%3];
  planepts[0][axis] = maxs[axis];

  brush.addPlane(planepts[0], planepts[1], planepts[2], shader, projection);

  planepts[0][(axis+1)%3] = mins[(axis+1)%3];
  planepts[0][(axis+2)%3] = mins[(axis+2)%3];
  planepts[0][axis] = mins[axis];
  planepts[1][(axis+1)%3] = maxs[(axis+1)%3];
  planepts[1][(axis+2)%3] = mins[(axis+2)%3];
  planepts[1][axis] = mins[axis];
  planepts[2][(axis+1)%3] = maxs[(axis+1)%3];
  planepts[2][(axis+2)%3] = maxs[(axis+2)%3];
  planepts[2][axis] = mins[axis];

  brush.addPlane(planepts[0], planepts[1], planepts[2], shader, projection);
 
  for (std::size_t i=0 ; i<sides ; ++i)
  {
    double sv = sin (i*3.14159265*2/sides);
    double cv = cos (i*3.14159265*2/sides);

    planepts[0][(axis+1)%3] = static_cast<float>(floor(mid[(axis+1)%3]+radius*cv+0.5));
    planepts[0][(axis+2)%3] = static_cast<float>(floor(mid[(axis+2)%3]+radius*sv+0.5));
    planepts[0][axis] = mins[axis];

    planepts[1][(axis+1)%3] = planepts[0][(axis+1)%3];
    planepts[1][(axis+2)%3] = planepts[0][(axis+2)%3];
    planepts[1][axis] = maxs[axis];

    planepts[2][(axis+1)%3] = static_cast<float>(floor(planepts[0][(axis+1)%3] - radius*sv + 0.5));
    planepts[2][(axis+2)%3] = static_cast<float>(floor(planepts[0][(axis+2)%3] + radius*cv + 0.5));
    planepts[2][axis] = maxs[axis];

    brush.addPlane(planepts[0], planepts[1], planepts[2], shader, projection);
  }
}

const std::size_t c_brushCone_minSides = 3;
const std::size_t c_brushCone_maxSides = 32;
const char* const c_brushCone_name = "brushCone";

void Brush_ConstructCone(Brush& brush, const AABB& bounds, std::size_t sides, const std::string& shader, const TextureProjection& projection)
{
  if(sides < c_brushCone_minSides)
  {
    globalErrorStream() << c_brushCone_name << ": sides " << sides << ": too few sides, minimum is " << c_brushCone_minSides << "\n";
    return;
  }
  if(sides > c_brushCone_maxSides)
  {
    globalErrorStream() << c_brushCone_name << ": sides " << sides << ": too many sides, maximum is " << c_brushCone_maxSides << "\n";
    return;
  }

  brush.clear();
  brush.reserve(sides+1);

  Vector3 mins(bounds.origin - bounds.extents);
  Vector3 maxs(bounds.origin + bounds.extents);

  float radius = max_extent(bounds.extents);
  const Vector3& mid = bounds.origin;
  Vector3 planepts[3];

  planepts[0][0] = mins[0];planepts[0][1] = mins[1];planepts[0][2] = mins[2];
  planepts[1][0] = maxs[0];planepts[1][1] = mins[1];planepts[1][2] = mins[2];
  planepts[2][0] = maxs[0];planepts[2][1] = maxs[1];planepts[2][2] = mins[2];

  brush.addPlane(planepts[0], planepts[1], planepts[2], shader, projection);

  for (std::size_t i=0 ; i<sides ; ++i)
  {
    double sv = sin (i*3.14159265*2/sides);
    double cv = cos (i*3.14159265*2/sides);

    planepts[0][0] = static_cast<float>(floor(mid[0]+radius*cv+0.5));
    planepts[0][1] = static_cast<float>(floor(mid[1]+radius*sv+0.5));
    planepts[0][2] = mins[2];

    planepts[1][0] = mid[0];
    planepts[1][1] = mid[1];
    planepts[1][2] = maxs[2];

    planepts[2][0] = static_cast<float>(floor(planepts[0][0] - radius * sv + 0.5));
    planepts[2][1] = static_cast<float>(floor(planepts[0][1] + radius * cv + 0.5));
    planepts[2][2] = maxs[2];

    brush.addPlane(planepts[0], planepts[1], planepts[2], shader, projection);
  }
}

const std::size_t c_brushSphere_minSides = 3;
const std::size_t c_brushSphere_maxSides = 7;
const char* const c_brushSphere_name = "brushSphere";

void Brush_ConstructSphere(Brush& brush, const AABB& bounds, std::size_t sides, const std::string& shader, const TextureProjection& projection)
{
  if(sides < c_brushSphere_minSides)
  {
    globalErrorStream() << c_brushSphere_name << ": sides " << sides << ": too few sides, minimum is " << c_brushSphere_minSides << "\n";
    return;
  }
  if(sides > c_brushSphere_maxSides)
  {
    globalErrorStream() << c_brushSphere_name << ": sides " << sides << ": too many sides, maximum is " << c_brushSphere_maxSides << "\n";
    return;
  }

  brush.clear();
  brush.reserve(sides*sides);

  float radius = max_extent(bounds.extents);
  const Vector3& mid = bounds.origin;
  Vector3 planepts[3];

  double dt = 2 * c_pi / sides;
  double dp = c_pi / sides;
  for(std::size_t i=0; i < sides; i++)
  {
    for(std::size_t j=0;j < sides-1; j++)
    {
      double t = i * dt;
      double p = float(j * dp - c_pi / 2);

      planepts[0] = mid + vector3_for_spherical(t, p)*radius;
      planepts[1] = mid + vector3_for_spherical(t, p + dp)*radius;
      planepts[2] = mid + vector3_for_spherical(t + dt, p + dp)*radius;

      brush.addPlane(planepts[0], planepts[1], planepts[2], shader, projection);
    }
  }

  {
    double p = (sides - 1) * dp - c_pi / 2;
    for(std::size_t i = 0; i < sides; i++)
    {
      double t = i * dt;

      planepts[0] = mid + vector3_for_spherical(t, p)*radius;
      planepts[1] = mid + vector3_for_spherical(t + dt, p + dp)*radius;
      planepts[2] = mid + vector3_for_spherical(t + dt, p)*radius;

      brush.addPlane(planepts[0], planepts[1], planepts[2], shader, projection);
    }
  }
}

int GetViewAxis()
{
  switch(GlobalXYWnd().getActiveViewType())
  {
    case XY:
      return 2;
    case XZ:
      return 1;
    case YZ:
      return 0;
  }
  return 2;
}

void Brush_ConstructPrefab(Brush& brush, EBrushPrefab type, const AABB& bounds, std::size_t sides, const std::string& shader, const TextureProjection& projection)
{
  switch(type)
  {
  case eBrushCuboid:
    {
      UndoableCommand undo("brushCuboid");

      Brush_ConstructCuboid(brush, bounds, shader, projection);
    }
    break;
  case eBrushPrism:
    {
      int axis = GetViewAxis();
	  std::ostringstream command;
      command << c_brushPrism_name << " -sides " << sides << " -axis " << axis;
      UndoableCommand undo(command.str());

      Brush_ConstructPrism(brush, bounds, sides, axis, shader, projection);
    }
    break;
  case eBrushCone:
    {
      std::ostringstream command;
      command << c_brushCone_name << " -sides " << sides;
      UndoableCommand undo(command.str());

      Brush_ConstructCone(brush, bounds, sides, shader, projection);
    }
    break;
  case eBrushSphere:
    {
      std::ostringstream command;
      command << c_brushSphere_name << " -sides " << sides;
      UndoableCommand undo(command.str());

      Brush_ConstructSphere(brush, bounds, sides, shader, projection);
    }
    break;

    default:
        break;
  }
}


void ConstructRegionBrushes(scene::INodePtr brushes[6], const Vector3& region_mins, const Vector3& region_maxs)
{
	const float THICKNESS = 10;
  {
    // set mins
    Vector3 mins(region_mins[0]-THICKNESS, region_mins[1]-THICKNESS, region_mins[2]-THICKNESS);

    // vary maxs
    for(std::size_t i=0; i<3; i++)
    {
      Vector3 maxs(region_maxs[0]+THICKNESS, region_maxs[1]+THICKNESS, region_maxs[2]+THICKNESS);
      maxs[i] = region_mins[i];
      Brush_ConstructCuboid(*Node_getBrush(brushes[i]), 
      						AABB::createFromMinMax(mins, maxs),
      						texdef_name_default(), 
      						TextureProjection());
    }
  }

  {
    // set maxs
    Vector3 maxs(region_maxs[0]+THICKNESS, region_maxs[1]+THICKNESS, region_maxs[2]+THICKNESS);

    // vary mins
    for(std::size_t i=0; i<3; i++)
    {
      Vector3 mins(region_mins[0]-THICKNESS, region_mins[1]-THICKNESS, region_mins[2]-THICKNESS);
      mins[i] = region_maxs[i];
      Brush_ConstructCuboid(*Node_getBrush(brushes[i+3]), 
      						AABB::createFromMinMax(mins, maxs),
      						texdef_name_default(),
      						TextureProjection());
    }
  }
}

class FaceSetShader
{
  const std::string& m_name;
public:
  FaceSetShader(const std::string& name) : m_name(name) {}
  
  void operator()(Face& face) const {
    face.setShader(m_name);
  }
};

void Scene_BrushSetShader_Selected(scene::Graph& graph, const std::string& name)
{
  Scene_ForEachSelectedBrush_ForEachFace(graph, FaceSetShader(name));
  SceneChangeNotify();
}

void Scene_BrushSetShader_Component_Selected(scene::Graph& graph, const std::string& name)
{
  Scene_ForEachSelectedBrushFace(graph, FaceSetShader(name));
  SceneChangeNotify();
}

TextureProjection g_defaultTextureProjection;
const TextureProjection& TextureTransform_getDefault()
{
  g_defaultTextureProjection.constructDefault();
  return g_defaultTextureProjection;
}

void Scene_BrushConstructPrefab(scene::Graph& graph, EBrushPrefab type, std::size_t sides, const std::string& shader)
{
  if(GlobalSelectionSystem().countSelected() != 0)
  {
    const scene::INodePtr& node = GlobalSelectionSystem().ultimateSelected();

    Brush* brush = Node_getBrush(node);
    if(brush != 0)
    {
      AABB bounds = brush->localAABB(); // copy bounds because the brush will be modified
      Brush_ConstructPrefab(*brush, type, bounds, sides, shader, TextureTransform_getDefault());
      SceneChangeNotify();
    }
  }
}

void Scene_BrushResize_Selected(scene::Graph& graph, const AABB& bounds, const std::string& shader)
{
  if(GlobalSelectionSystem().countSelected() != 0)
  {
    const scene::INodePtr& node = GlobalSelectionSystem().ultimateSelected();

    Brush* brush = Node_getBrush(node);
    if(brush != 0)
    {
      Brush_ConstructCuboid(*brush, bounds, shader, TextureTransform_getDefault());
      SceneChangeNotify();
    }
  }
}

void Scene_BrushResize(Brush& brush, const AABB& bounds, const std::string& shader)
{
      Brush_ConstructCuboid(brush, bounds, shader, TextureTransform_getDefault());
      SceneChangeNotify();
}

/**
 * Selects all visible brushes which carry the shader name as passed
 * to the constructor.
 */
class BrushSelectByShaderWalker : 
	public scene::NodeVisitor
{
	std::string _name;
public:
	BrushSelectByShaderWalker(const std::string& name) :
		_name(name)
	{}

	bool pre(const scene::INodePtr& node) {
		if (node->visible()) {
			Brush* brush = Node_getBrush(node);

			if (brush != NULL && brush->hasShader(_name)) {
				Node_setSelected(node, true);
				return false; // don't traverse brushes
			}

			// not a suitable brush, traverse further
			return true;
		}
		else {
			return false; // don't traverse invisible nodes
		}
	}
};

void Scene_BrushSelectByShader(scene::Graph& graph, const std::string& name) {
	BrushSelectByShaderWalker walker(name);
	Node_traverseSubgraph(graph.root(), walker);
}

class FaceSelectByShader
{
  std::string m_name;
public:
  FaceSelectByShader(const std::string& name)
    : m_name(name)
  {
  }
  void operator()(FaceInstance& face) const
  {
    if(shader_equal(face.getFace().getShader(), m_name))
    {
      face.setSelected(SelectionSystem::eFace, true);
    }
  }
};

void Scene_BrushSelectByShader_Component(scene::Graph& graph, const std::string& name)
{
  Scene_ForEachSelectedBrush_ForEachFaceInstance(graph, FaceSelectByShader(name));
}

class FaceTextureFlipper
{
	unsigned int _flipAxis;
public:
	FaceTextureFlipper(unsigned int flipAxis) : 
		_flipAxis(flipAxis) 
	{}
	
	void operator()(Face& face) const {
		face.flipTexture(_flipAxis);
	}
};

void Scene_BrushFlipTexture_Selected(unsigned int flipAxis) {
	Scene_ForEachSelectedBrush_ForEachFace(GlobalSceneGraph(), FaceTextureFlipper(flipAxis));
}

void Scene_BrushFlipTexture_Component_Selected(unsigned int flipAxis) {
	Scene_ForEachSelectedBrushFace(GlobalSceneGraph(), FaceTextureFlipper(flipAxis));
}


#if 0

void normalquantisation_draw()
{
  glPointSize(1);
  glBegin(GL_POINTS);
  for(std::size_t i = 0; i <= c_quantise_normal; ++i)
  {
    for(std::size_t j = 0; j <= c_quantise_normal; ++j)
    {
      Normal3f vertex(normal3f_normalised(Normal3f(
        static_cast<float>(c_quantise_normal - j - i),
        static_cast<float>(i),
        static_cast<float>(j)
        )));
      VectorScale(normal3f_to_array(vertex), 64.f, normal3f_to_array(vertex));
      glVertex3fv(normal3f_to_array(vertex));
      vertex.x = -vertex.x;
      glVertex3fv(normal3f_to_array(vertex));
    }
  }
  glEnd();
}

class RenderableNormalQuantisation : public OpenGLRenderable
{
public:
  void render(const RenderInfo& info) const
  {
    normalquantisation_draw();
  }
};

const float g_test_quantise_normal = 1.f / static_cast<float>(1 << 3);

class TestNormalQuantisation
{
  void check_normal(const Normal3f& normal, const Normal3f& other)
  {
    spherical_t spherical = spherical_from_normal3f(normal);
    double longditude = RAD2DEG(spherical.longditude);
    double latitude = RAD2DEG(spherical.latitude);
    double x = cos(spherical.longditude) * sin(spherical.latitude);
    double y = sin(spherical.longditude) * sin(spherical.latitude);
    double z = cos(spherical.latitude);

    ASSERT_MESSAGE(normal3f_dot(normal, other) > 0.99, "bleh");
  }

  void test_normal(const Normal3f& normal)
  {
    Normal3f test = normal3f_from_spherical(spherical_from_normal3f(normal));
    check_normal(normal, test);

    EOctant octant = normal3f_classify_octant(normal);
    Normal3f folded = normal3f_fold_octant(normal, octant);
    ESextant sextant = normal3f_classify_sextant(folded);
    folded = normal3f_fold_sextant(folded, sextant);

    double scale = static_cast<float>(c_quantise_normal) / (folded.x + folded.y + folded.z);

    double zbits = folded.z * scale;
    double ybits = folded.y * scale;

    std::size_t zbits_q = static_cast<std::size_t>(zbits);
    std::size_t ybits_q = static_cast<std::size_t>(ybits);

    ASSERT_MESSAGE(zbits_q <= (c_quantise_normal / 8) * 3, "bleh");
    ASSERT_MESSAGE(ybits_q <= (c_quantise_normal / 2), "bleh");
    ASSERT_MESSAGE(zbits_q + ((c_quantise_normal / 2) - ybits_q) <= (c_quantise_normal / 2), "bleh");

    std::size_t y_t = (zbits_q < (c_quantise_normal / 4)) ? ybits_q : (c_quantise_normal / 2) - ybits_q;
    std::size_t z_t = (zbits_q < (c_quantise_normal / 4)) ? zbits_q : (c_quantise_normal / 2) - zbits_q;
    std::size_t index = (c_quantise_normal / 4) * y_t + z_t;
    ASSERT_MESSAGE(index <= (c_quantise_normal / 4)*(c_quantise_normal / 2), "bleh");

    Normal3f tmp(c_quantise_normal - zbits_q - ybits_q, ybits_q, zbits_q);
    tmp = normal3f_normalised(tmp);

    Normal3f unfolded = normal3f_unfold_octant(normal3f_unfold_sextant(tmp, sextant), octant);

    check_normal(normal, unfolded);

    double dot = normal3f_dot(normal, unfolded);
    float length = VectorLength(normal3f_to_array(unfolded));
    float inv_length = 1 / length;

    Normal3f quantised = normal3f_quantised(normal);
    check_normal(normal, quantised);
  }
  void test2(const Normal3f& normal, const Normal3f& other)
  {
    if(normal3f_quantised(normal) != normal3f_quantised(other))
    {
      int bleh = 0;
    }
  }

  static Normal3f normalise(float x, float y, float z)
  {
    return normal3f_normalised(Normal3f(x, y, z));
  }

  float vec_rand()
  {
    return static_cast<float>(rand() - (RAND_MAX/2));
  }

  Normal3f normal3f_rand()
  {
    return normalise(vec_rand(), vec_rand(), vec_rand());
  }

public:
  TestNormalQuantisation()
  {
    for(int i = 4096; i > 0; --i)
      test_normal(normal3f_rand());

    test_normal(normalise(1, 0, 0));
    test_normal(normalise(0, 1, 0));
    test_normal(normalise(0, 0, 1));
    test_normal(normalise(1, 1, 0));
    test_normal(normalise(1, 0, 1));
    test_normal(normalise(0, 1, 1));
    
    test_normal(normalise(10000, 10000, 10000));
    test_normal(normalise(10000, 10000, 10001));
    test_normal(normalise(10000, 10000, 10002));
    test_normal(normalise(10000, 10000, 10010));
    test_normal(normalise(10000, 10000, 10020));
    test_normal(normalise(10000, 10000, 10030));
    test_normal(normalise(10000, 10000, 10100));
    test_normal(normalise(10000, 10000, 10101));
    test_normal(normalise(10000, 10000, 10102));
    test_normal(normalise(10000, 10000, 10200));
    test_normal(normalise(10000, 10000, 10201));
    test_normal(normalise(10000, 10000, 10202));
    test_normal(normalise(10000, 10000, 10203));
    test_normal(normalise(10000, 10000, 10300));


    test2(normalise(10000, 10000, 10000), normalise(10000, 10000, 10001));
    test2(normalise(10000, 10000, 10001), normalise(10000, 10001, 10000));
  }
};

TestNormalQuantisation g_testNormalQuantisation;


#endif

void brushMakeSided(const cmd::ArgumentList& args) {
	if (args.size() != 1) {
		return;
	}

	// First argument contains the number of sides
	int input = args[0].getInt();

	if (input < 0) {
		globalErrorStream() << "BrushMakeSide: invalid number of sides: " << input << std::endl;
		return;
	}

	std::size_t numSides = static_cast<std::size_t>(input);
	Scene_BrushConstructPrefab(GlobalSceneGraph(), eBrushPrism, numSides, GlobalTextureBrowser().getSelectedShader());
}

void brushMakePrefab(const cmd::ArgumentList& args)
{
	if (args.size() != 1) {
		return;
	}

	if (GlobalSelectionSystem().getSelectionInfo().brushCount != 1)
	{
		// Display a modal error dialog	
		gtkutil::errorDialog(_("Exactly one brush must be selected for this operation."), GlobalMainFrame().getTopLevelWindow());
		return;
	}

	// First argument contains the number of sides
	int input = args[0].getInt();

	if (input >= eBrushCuboid && input < eNumPrefabTypes) {
		// Boundary checks passed
		EBrushPrefab type = static_cast<EBrushPrefab>(input);

		int minSides = 3;
		int maxSides = c_brushPrism_maxSides;

		switch (type)
		{
		case eBrushCuboid:
			// Cuboids don't need to query the number of sides
			Scene_BrushConstructPrefab(GlobalSceneGraph(), type, 0, GlobalTextureBrowser().getSelectedShader());
			return;

		case eBrushPrism:
			minSides = c_brushPrism_minSides;
			maxSides = c_brushPrism_maxSides;
			break;

		case eBrushCone:
			minSides = c_brushCone_minSides;
			maxSides = c_brushCone_maxSides;
			break;

		case eBrushSphere: 
			minSides = c_brushSphere_minSides;
			maxSides = c_brushSphere_maxSides;
			break;
		default:
			maxSides = 9999;
		};

		ui::QuerySidesDialog dialog(minSides, maxSides);

		int sides = dialog.queryNumberOfSides();

		if (sides != -1)
		{
			Scene_BrushConstructPrefab(GlobalSceneGraph(), type, sides, GlobalTextureBrowser().getSelectedShader());
		}
	}
	else {
		globalErrorStream() << "BrushMakePrefab: invalid prefab type. Allowed types are: " << std::endl 
			<< eBrushCuboid << " = cuboid " << std::endl
			<< eBrushPrism  << " = prism " << std::endl
			<< eBrushCone  << " = cone " << std::endl
			<< eBrushSphere << " = sphere " << std::endl;
	}
}

void Brush_registerCommands()
{
	GlobalEventManager().addRegistryToggle("TogTexLock", RKEY_ENABLE_TEXTURE_LOCK);

	GlobalCommandSystem().addCommand("BrushMakePrefab", brushMakePrefab, cmd::ARGTYPE_INT);

	GlobalEventManager().addCommand("BrushCuboid", "BrushCuboid");
	GlobalEventManager().addCommand("BrushPrism", "BrushPrism");
	GlobalEventManager().addCommand("BrushCone", "BrushCone");
	GlobalEventManager().addCommand("BrushSphere", "BrushSphere");

	GlobalCommandSystem().addCommand("BrushMakeSided", brushMakeSided, cmd::ARGTYPE_INT);

	// Link the Events to the corresponding statements
	GlobalEventManager().addCommand("Brush3Sided", "Brush3Sided");
	GlobalEventManager().addCommand("Brush4Sided", "Brush4Sided");
	GlobalEventManager().addCommand("Brush5Sided", "Brush5Sided");
	GlobalEventManager().addCommand("Brush6Sided", "Brush6Sided");
	GlobalEventManager().addCommand("Brush7Sided", "Brush7Sided");
	GlobalEventManager().addCommand("Brush8Sided", "Brush8Sided");
	GlobalEventManager().addCommand("Brush9Sided", "Brush9Sided");

	GlobalCommandSystem().addCommand("TextureNatural", selection::algorithm::naturalTexture);
	GlobalCommandSystem().addCommand("MakeVisportal", selection::algorithm::makeVisportal);
	GlobalCommandSystem().addCommand("SurroundWithMonsterclip", selection::algorithm::surroundWithMonsterclip);
	GlobalEventManager().addCommand("TextureNatural", "TextureNatural");
	GlobalEventManager().addCommand("MakeVisportal", "MakeVisportal");
	GlobalEventManager().addCommand("SurroundWithMonsterclip", "SurroundWithMonsterclip");
}
