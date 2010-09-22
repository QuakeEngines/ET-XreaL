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

//
// Main Window for Q3Radiant
//
// Leonardo Zide (leo@lokigames.com)
//

#include "mainframe_old.h"

#include "i18n.h"
#include "imainframe.h"
#include "debugging/debugging.h"
#include "version.h"
#include "settings/PreferenceSystem.h"

#include "map/FindMapElements.h"
#include "ui/about/AboutDialog.h"
#include "ui/surfaceinspector/SurfaceInspector.h"
#include "ui/prefdialog/PrefDialog.h"
#include "ui/patch/PatchInspector.h"
#include "textool/TexTool.h"
#include "ui/lightinspector/LightInspector.h"
#include "ui/mediabrowser/MediaBrowser.h"
#include "ui/menu/FiltersMenu.h"
#include "ui/transform/TransformDialog.h"
#include "ui/mainframe/ScreenUpdateBlocker.h"
#include "ui/overlay/Overlay.h"
#include "ui/overlay/OverlayDialog.h"
#include "ui/layers/LayerControlDialog.h"
#include "ui/filterdialog/FilterDialog.h"
#include "selection/algorithm/Shader.h"
#include "selection/algorithm/General.h"
#include "selection/algorithm/Group.h"
#include "selection/algorithm/GroupCycle.h"
#include "selection/algorithm/Primitives.h"
#include "selection/algorithm/Transformation.h"
#include "selection/algorithm/Curves.h"
#include "selection/shaderclipboard/ShaderClipboard.h"
#include "iclipper.h"
#include "ifilesystem.h"
#include "iundo.h"
#include "igrid.h"
#include "ifilter.h"
#include "itoolbar.h"
#include "editable.h"
#include "ientity.h"
#include "iuimanager.h"
#include "igroupdialog.h"
#include "ieventmanager.h"
#include "ieclass.h"
#include "iregistry.h"
#include "irender.h"
#include "ishaders.h"
#include "igl.h"
#include "moduleobserver.h"
#include "os/dir.h"

#include <ctime>
#include <iostream>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkhpaned.h>
#include <gtk/gtkvpaned.h>
#include <gtk/gtktoolbar.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtkimage.h>
#include <gtk/gtktable.h>

#include "scenelib.h"
#include "os/path.h"
#include "os/file.h"
#include "moduleobservers.h"

#include "gtkutil/clipboard.h"
#include "gtkutil/glfont.h"
#include "gtkutil/GLWidget.h"
#include "gtkutil/Paned.h"
#include "gtkutil/MultiMonitor.h"
#include "gtkutil/widget.h"
#include "gtkutil/FramedWidget.h"
#include "gtkutil/dialog.h"

#include "map/AutoSaver.h"
#include "brushmanip.h"
#include "brush/BrushModule.h"
#include "brush/csg/CSG.h"
#include "log/Console.h"
#include "entity.h"
#include "patchmanip.h"
#include "select.h"
#include "ui/texturebrowser/TextureBrowser.h"
#include "camera/GlobalCamera.h"
#include "camera/CameraSettings.h"
#include "xyview/GlobalXYWnd.h"
#include "ui/mru/MRU.h"
#include "ui/commandlist/CommandList.h"
#include "ui/findshader/FindShader.h"
#include "ui/mapinfo/MapInfoDialog.h"
#include "ui/splash/Splash.h"
#include "brush/FaceInstance.h"
#include "settings/GameManager.h"
#include "modulesystem/ModuleRegistry.h"
#include "RadiantModule.h"

extern FaceInstanceSet g_SelectedFaceInstances;

	namespace {
		const std::string RKEY_WINDOW_LAYOUT = "user/ui/mainFrame/windowLayout";
		const std::string RKEY_WINDOW_STATE = "user/ui/mainFrame/window";
		const std::string RKEY_MULTIMON_START_PRIMARY = "user/ui/multiMonitor/startOnPrimaryMonitor";
	}

// This is called from main() to start up the Radiant stuff.
void Radiant_Initialise() 
{
	// Create the empty Settings node and set the title to empty.
	ui::PrefDialog::Instance().createOrFindPage(_("Game"));
	ui::PrefPagePtr settingsPage = ui::PrefDialog::Instance().createOrFindPage(_("Settings"));
	settingsPage->setTitle("");
	
	ui::Splash::Instance().setProgressAndText(_("Constructing Menu"), 0.89f);
	
	// Construct the MRU commands and menu structure
	GlobalMRU().constructMenu();
	
	// Initialise the most recently used files list
	GlobalMRU().loadRecentFiles();

	gtkutil::MultiMonitor::printMonitorInfo();
}

void Exit(const cmd::ArgumentList& args) {
	if (GlobalMap().askForSave(_("Exit Radiant"))) {
		gtk_main_quit();
	}
}

void Map_ExportSelected(std::ostream& ostream) {
	GlobalMap().exportSelected(ostream);
}

void Map_ImportSelected(TextInputStream& istream) {
	GlobalMap().importSelected(istream);
}

void Selection_Copy()
{
  clipboard_copy(Map_ExportSelected);
}

void Selection_Paste()
{
  clipboard_paste(Map_ImportSelected);
}

void Copy(const cmd::ArgumentList& args) {
	if (g_SelectedFaceInstances.empty()) {
		Selection_Copy();
	}
	else {
		selection::algorithm::pickShaderFromSelection(args);
	}
}

void Paste(const cmd::ArgumentList& args) {
	if (g_SelectedFaceInstances.empty()) {
		UndoableCommand undo("paste");

		GlobalSelectionSystem().setSelectedAll(false);
		Selection_Paste();
	}
	else {
		selection::algorithm::pasteShaderToSelection(args);
	}
}

void PasteToCamera(const cmd::ArgumentList& args)
{
	CamWndPtr camWnd = GlobalCamera().getActiveCamWnd();
	if (camWnd == NULL) return;

  GlobalSelectionSystem().setSelectedAll(false);
  
  UndoableCommand undo("pasteToCamera");
  
  Selection_Paste();
  
  // Work out the delta
  Vector3 mid = selection::algorithm::getCurrentSelectionCenter();
  Vector3 delta = vector3_snapped(camWnd->getCameraOrigin(), GlobalGrid().getGridSize()) - mid;
  
  // Move to camera
  GlobalSelectionSystem().translateSelected(delta);
}

void updateTextureBrowser() {
	GlobalTextureBrowser().queueDraw();
}

void SetClipMode(bool enable);
void ModeChangeNotify();

void DragMode(bool);

typedef void(*ToolMode)(bool);
ToolMode g_currentToolMode = DragMode;
bool g_currentToolModeSupportsComponentEditing = false;
ToolMode g_defaultToolMode = DragMode;

void SelectionSystem_DefaultMode()
{
  GlobalSelectionSystem().SetMode(SelectionSystem::ePrimitive);
  GlobalSelectionSystem().SetComponentMode(SelectionSystem::eDefault);
  ModeChangeNotify();
}


bool EdgeMode() {
	return GlobalSelectionSystem().Mode() == SelectionSystem::eComponent
	       && GlobalSelectionSystem().ComponentMode() == SelectionSystem::eEdge;
}

bool VertexMode() {
	return GlobalSelectionSystem().Mode() == SelectionSystem::eComponent
	       && GlobalSelectionSystem().ComponentMode() == SelectionSystem::eVertex;
}

bool FaceMode() {
	return GlobalSelectionSystem().Mode() == SelectionSystem::eComponent
	       && GlobalSelectionSystem().ComponentMode() == SelectionSystem::eFace;
}

void ComponentModeChanged()
{
	GlobalEventManager().setToggled("DragVertices", VertexMode());
	GlobalEventManager().setToggled("DragEdges", EdgeMode());
	GlobalEventManager().setToggled("DragFaces", FaceMode());
}

void ComponentMode_SelectionChanged(const Selectable& selectable) {
	if (GlobalSelectionSystem().Mode() == SelectionSystem::eComponent
	        && GlobalSelectionSystem().countSelected() == 0) {
		SelectionSystem_DefaultMode();
		ComponentModeChanged();
	}
}

void ModeChanged()
{
	GlobalEventManager().setToggled("DragEntities", GlobalSelectionSystem().Mode() == SelectionSystem::eEntity);
	GlobalEventManager().setToggled("SelectionModeGroupPart", GlobalSelectionSystem().Mode() == SelectionSystem::eGroupPart);
}

void ToggleGroupPartMode(bool newState)
{
	if (GlobalSelectionSystem().Mode() == SelectionSystem::eGroupPart)
	{
		SelectionSystem_DefaultMode();
	}
	else
	{
		// De-select everything when switching to group part mode
		GlobalSelectionSystem().setSelectedAll(false);
		GlobalSelectionSystem().setSelectedAllComponents(false);

		GlobalSelectionSystem().SetMode(SelectionSystem::eGroupPart);
		GlobalSelectionSystem().SetComponentMode(SelectionSystem::eDefault);
	}

	ModeChanged();
	ComponentModeChanged();

	ModeChangeNotify();
}

void ToggleEntityMode(bool newState)
{
	if (GlobalSelectionSystem().Mode() == SelectionSystem::eEntity)
	{
		SelectionSystem_DefaultMode();
	}
	else {
		GlobalSelectionSystem().SetMode(SelectionSystem::eEntity);
		GlobalSelectionSystem().SetComponentMode(SelectionSystem::eDefault);
	}

	ModeChanged();
	ComponentModeChanged();

	ModeChangeNotify();
}

void ToggleEdgeMode(bool newState) {
	if (EdgeMode()) {
		// De-select all the selected edges before switching back
		GlobalSelectionSystem().setSelectedAllComponents(false);
		SelectionSystem_DefaultMode();
	}
	else if (GlobalSelectionSystem().countSelected() != 0) {
		if (!g_currentToolModeSupportsComponentEditing) {
			g_defaultToolMode(true);
		}

		GlobalSelectionSystem().SetMode(SelectionSystem::eComponent);
		GlobalSelectionSystem().SetComponentMode(SelectionSystem::eEdge);
	}

	ComponentModeChanged();

	ModeChangeNotify();
}

void ToggleVertexMode(bool newState) {
	if (VertexMode()) {
		// De-select all the selected vertices before switching back
		GlobalSelectionSystem().setSelectedAllComponents(false);
		SelectionSystem_DefaultMode();
	}
	else if(GlobalSelectionSystem().countSelected() != 0) {
		if (!g_currentToolModeSupportsComponentEditing) {
			g_defaultToolMode(true);
		}

		GlobalSelectionSystem().SetMode(SelectionSystem::eComponent);
		GlobalSelectionSystem().SetComponentMode(SelectionSystem::eVertex);
	}

	ComponentModeChanged();

	ModeChangeNotify();
}

void ToggleFaceMode(bool newState) {
	if (FaceMode()) {
		// De-select all the selected faces before switching back
		GlobalSelectionSystem().setSelectedAllComponents(false);
		SelectionSystem_DefaultMode();
	}
	else if (GlobalSelectionSystem().countSelected() != 0) {
		if (!g_currentToolModeSupportsComponentEditing) {
			g_defaultToolMode(true);
		}

		GlobalSelectionSystem().SetMode(SelectionSystem::eComponent);
		GlobalSelectionSystem().SetComponentMode(SelectionSystem::eFace);
	}

	ComponentModeChanged();

	ModeChangeNotify();
}

// called when the escape key is used (either on the main window or on an inspector)
void Selection_Deselect(const cmd::ArgumentList& args)
{
  if(GlobalSelectionSystem().Mode() == SelectionSystem::eComponent)
  {
    if(GlobalSelectionSystem().countSelectedComponents() != 0)
    {
      GlobalSelectionSystem().setSelectedAllComponents(false);
    }
    else
    {
      SelectionSystem_DefaultMode();
      ComponentModeChanged();
    }
  }
  else
  {
    if(GlobalSelectionSystem().countSelectedComponents() != 0)
    {
      GlobalSelectionSystem().setSelectedAllComponents(false);
    }
    else
    {
      GlobalSelectionSystem().setSelectedAll(false);
    }
  }
}

void ToolChanged() {  
	GlobalEventManager().setToggled("ToggleClipper", GlobalClipper().clipMode());
	GlobalEventManager().setToggled("MouseTranslate", GlobalSelectionSystem().ManipulatorMode() == SelectionSystem::eTranslate);
	GlobalEventManager().setToggled("MouseRotate", GlobalSelectionSystem().ManipulatorMode() == SelectionSystem::eRotate);
	//GlobalEventManager().setToggled("MouseScale", GlobalSelectionSystem().ManipulatorMode() == SelectionSystem::eScale);
	GlobalEventManager().setToggled("MouseDrag", GlobalSelectionSystem().ManipulatorMode() == SelectionSystem::eDrag);
}

void DragMode(bool newState)
{
  if(g_currentToolMode == DragMode && g_defaultToolMode != DragMode)
  {
    g_defaultToolMode(true);
  }
  else
  {
    g_currentToolMode = DragMode;
    g_currentToolModeSupportsComponentEditing = true;

    GlobalClipper().onClipMode(false);

    GlobalSelectionSystem().SetManipulatorMode(SelectionSystem::eDrag);
    ToolChanged();
    ModeChangeNotify();
  }
}

void TranslateMode(bool newState)
{
  if(g_currentToolMode == TranslateMode && g_defaultToolMode != TranslateMode)
  {
    g_defaultToolMode(true);
  }
  else
  {
    g_currentToolMode = TranslateMode;
    g_currentToolModeSupportsComponentEditing = true;

    GlobalClipper().onClipMode(false);

    GlobalSelectionSystem().SetManipulatorMode(SelectionSystem::eTranslate);
    ToolChanged();
    ModeChangeNotify();
  }
}

void RotateMode(bool newState)
{
  if(g_currentToolMode == RotateMode && g_defaultToolMode != RotateMode)
  {
    g_defaultToolMode(true);
  }
  else
  {
    g_currentToolMode = RotateMode;
    g_currentToolModeSupportsComponentEditing = true;

    GlobalClipper().onClipMode(false);

    GlobalSelectionSystem().SetManipulatorMode(SelectionSystem::eRotate);
    ToolChanged();
    ModeChangeNotify();
  }
}

void ScaleMode(bool newState)
{
  /*if(g_currentToolMode == ScaleMode && g_defaultToolMode != ScaleMode)
  {
    g_defaultToolMode();
  }
  else
  {
    g_currentToolMode = ScaleMode;
    g_currentToolModeSupportsComponentEditing = true;

    GlobalClipper().onClipMode(false);

    GlobalSelectionSystem().SetManipulatorMode(SelectionSystem::eScale);
    ToolChanged();
    ModeChangeNotify();
  }*/
}


void ClipperMode(bool newState) {
	if (g_currentToolMode == ClipperMode && g_defaultToolMode != ClipperMode) {
		g_defaultToolMode(true);
	}
	else {
		g_currentToolMode = ClipperMode;
		g_currentToolModeSupportsComponentEditing = false;

		SelectionSystem_DefaultMode();

		GlobalClipper().onClipMode(true);

		GlobalSelectionSystem().SetManipulatorMode(SelectionSystem::eClip);
		ToolChanged();
		ModeChangeNotify();
	}
}

class SnappableSnapToGridSelected : 
	public SelectionSystem::Visitor
{
	float _snap;
public:
	SnappableSnapToGridSelected(float snap) : 
		_snap(snap)
	{}

	void visit(const scene::INodePtr& node) const {
		// Don't do anything with hidden nodes
		if (!node->visible()) return;

		SnappablePtr snappable = Node_getSnappable(node);

		if (snappable != NULL) {
			snappable->snapto(_snap);
		}
	}
};

/* greebo: This is the visitor class to snap all components of a selected instance to the grid
 * While visiting all the instances, it checks if the instance derives from ComponentSnappable 
 */
class ComponentSnappableSnapToGridSelected : 
	public SelectionSystem::Visitor
{
	float _snap;
public:
	// Constructor: expects the grid size that should be snapped to
	ComponentSnappableSnapToGridSelected(float snap) : 
		_snap(snap)
	{}
  
	void visit(const scene::INodePtr& node) const {
		// Don't do anything with hidden nodes
	    if (!node->visible()) return;

    	// Check if the visited instance is componentSnappable
		ComponentSnappablePtr componentSnappable = Node_getComponentSnappable(node);
		
		if (componentSnappable != NULL) {
			componentSnappable->snapComponents(_snap);
		}
	}
}; // ComponentSnappableSnapToGridSelected

void Selection_SnapToGrid(const cmd::ArgumentList& args)
{
	std::ostringstream command;
	command << "snapSelected -grid " << GlobalGrid().getGridSize();
	UndoableCommand undo(command.str());

	if (GlobalSelectionSystem().Mode() == SelectionSystem::eComponent) {
		// Component mode
		ComponentSnappableSnapToGridSelected walker(GlobalGrid().getGridSize());
		GlobalSelectionSystem().foreachSelectedComponent(walker);
	}
	else {
		// Non-component mode
		SnappableSnapToGridSelected walker(GlobalGrid().getGridSize());
		GlobalSelectionSystem().foreachSelected(walker);
	}
}

void ModeChangeNotify()
{
	SceneChangeNotify();
}

void ClipperChangeNotify() {
	GlobalMainFrame().updateAllWindows();
}

// The "Flush & Reload Shaders" command target 
void RefreshShaders(const cmd::ArgumentList& args) {
	// Disable screen updates for the scope of this function
	ui::ScreenUpdateBlocker blocker(_("Processing..."), _("Loading Shaders"));
	
	// Destroy all the OpenGLShader objects
	GlobalRenderSystem().unrealise();
	
	// Reload the Shadersystem
	GlobalMaterialManager().refresh();
	
	// Now realise the OpenGLShader objects again
	GlobalRenderSystem().realise();
	
	ui::MediaBrowser::getInstance().reloadMedia();
	GlobalMainFrame().updateAllWindows();
}

#if 0
#include "debugging/ScopedDebugTimer.h"

void BenchmarkPatches(const cmd::ArgumentList& args) {
	// Disable screen updates for the scope of this function
	ui::ScreenUpdateBlocker blocker("Processing...", "Performing Patch Benchmark");
	
	scene::INodePtr node = GlobalPatchCreator(DEF2).createPatch();
	Patch* patch = Node_getPatch(node);
	GlobalMap().findOrInsertWorldspawn()->addChildNode(node);

	patch->setDims(15, 15);

	// Retrieve the boundaries 
	AABB bounds(Vector3(-512, -512, 0), Vector3(512, 512, 0));
	patch->ConstructPrefab(bounds, ePlane, GlobalXYWnd().getActiveViewType(), 15, 15);

	for (PatchControlIter i = patch->begin(); i != patch->end(); ++i)
	{
		PatchControl& control = *i;
		int randomNumber = int(250 * (float(std::rand()) / float(RAND_MAX)));
		control.vertex.set(control.vertex.x(), control.vertex.y(), control.vertex.z() + randomNumber);
	}

	patch->controlPointsChanged();

	{
		ScopedDebugTimer timer("Benchmark");
		for (std::size_t i = 0; i < 200; i++)
		{
			patch->UpdateCachedData();
		}
	}
}
#endif

void MainFrame_Construct()
{
	DragMode(true);

#if 0
	GlobalCommandSystem().addCommand("BenchmarkPatches", BenchmarkPatches);
#endif

	GlobalCommandSystem().addCommand("Exit", Exit);
	GlobalCommandSystem().addCommand("ReloadSkins", ReloadSkins);
	GlobalCommandSystem().addCommand("ReloadDefs", ReloadDefs);
	GlobalCommandSystem().addCommand("ProjectSettings", ui::PrefDialog::showProjectSettings);
	GlobalCommandSystem().addCommand("Copy", Copy);
	GlobalCommandSystem().addCommand("Paste", Paste);
	GlobalCommandSystem().addCommand("PasteToCamera", PasteToCamera);

	GlobalCommandSystem().addCommand("CloneSelection", selection::algorithm::cloneSelected); 
	GlobalCommandSystem().addCommand("DeleteSelection", selection::algorithm::deleteSelectionCmd);
	GlobalCommandSystem().addCommand("ParentSelection", selection::algorithm::parentSelection);
	GlobalCommandSystem().addCommand("ParentSelectionToWorldspawn", selection::algorithm::parentSelectionToWorldspawn);

	GlobalCommandSystem().addCommand("UnSelectSelection", Selection_Deselect);
	GlobalCommandSystem().addCommand("InvertSelection", selection::algorithm::invertSelection);
	GlobalCommandSystem().addCommand("SelectInside", selection::algorithm::selectInside);
	GlobalCommandSystem().addCommand("SelectTouching", selection::algorithm::selectTouching);
	GlobalCommandSystem().addCommand("SelectCompleteTall", selection::algorithm::selectCompleteTall);
	GlobalCommandSystem().addCommand("ExpandSelectionToEntities", selection::algorithm::expandSelectionToEntities);
	GlobalCommandSystem().addCommand("MergeSelectedEntities", selection::algorithm::mergeSelectedEntities);
	GlobalCommandSystem().addCommand("SelectChildren", selection::algorithm::selectChildren);

	GlobalCommandSystem().addCommand("Preferences", ui::PrefDialog::toggle);
	GlobalCommandSystem().addCommand("ToggleConsole", ui::Console::toggle);

	GlobalCommandSystem().addCommand("ToggleMediaBrowser", ui::MediaBrowser::toggle);
	GlobalCommandSystem().addCommand("ToggleLightInspector", ui::LightInspector::toggleInspector);
	GlobalCommandSystem().addCommand("SurfaceInspector", ui::SurfaceInspector::toggle);
	GlobalCommandSystem().addCommand("PatchInspector", ui::PatchInspector::toggle);
	GlobalCommandSystem().addCommand("OverlayDialog", ui::OverlayDialog::display);
	GlobalCommandSystem().addCommand("TransformDialog", ui::TransformDialog::toggle);

	GlobalCommandSystem().addCommand("ShowHidden", selection::algorithm::showAllHidden);
	GlobalCommandSystem().addCommand("HideSelected", selection::algorithm::hideSelected);
	GlobalCommandSystem().addCommand("HideDeselected", selection::algorithm::hideDeselected);

	GlobalCommandSystem().addCommand("MirrorSelectionX", Selection_Flipx);
	GlobalCommandSystem().addCommand("RotateSelectionX", Selection_Rotatex);
	GlobalCommandSystem().addCommand("MirrorSelectionY", Selection_Flipy);
	GlobalCommandSystem().addCommand("RotateSelectionY", Selection_Rotatey);
	GlobalCommandSystem().addCommand("MirrorSelectionZ", Selection_Flipz);
	GlobalCommandSystem().addCommand("RotateSelectionZ", Selection_Rotatez);

	GlobalCommandSystem().addCommand("FindBrush", DoFind);
	GlobalCommandSystem().addCommand("ConvertSelectedToFuncStatic", selection::algorithm::convertSelectedToFuncStatic);
	GlobalCommandSystem().addCommand("RevertToWorldspawn", selection::algorithm::revertGroupToWorldSpawn);
	GlobalCommandSystem().addCommand("MapInfo", ui::MapInfoDialog::showDialog);
	GlobalCommandSystem().addCommand("EditFiltersDialog", ui::FilterDialog::showDialog);

	GlobalCommandSystem().addCommand("CSGSubtract", brush::algorithm::subtractBrushesFromUnselected);
	GlobalCommandSystem().addCommand("CSGMerge", brush::algorithm::mergeSelectedBrushes);
	GlobalCommandSystem().addCommand("CSGHollow", brush::algorithm::hollowSelectedBrushes);
	GlobalCommandSystem().addCommand("CSGRoom", brush::algorithm::makeRoomForSelectedBrushes);

	GlobalCommandSystem().addCommand("RefreshShaders", RefreshShaders);
	
	GlobalCommandSystem().addCommand("SnapToGrid", Selection_SnapToGrid);
	
	GlobalCommandSystem().addCommand("SelectAllOfType", selection::algorithm::selectAllOfType);
	GlobalCommandSystem().addCommand("GroupCycleForward", selection::GroupCycle::cycleForward);
	GlobalCommandSystem().addCommand("GroupCycleBackward", selection::GroupCycle::cycleBackward);
	
	GlobalCommandSystem().addCommand("TexRotate", selection::algorithm::rotateTexture, cmd::ARGTYPE_INT|cmd::ARGTYPE_STRING);
	GlobalCommandSystem().addCommand("TexScale", selection::algorithm::scaleTexture, cmd::ARGTYPE_VECTOR2|cmd::ARGTYPE_STRING);
	GlobalCommandSystem().addCommand("TexShift", selection::algorithm::shiftTextureCmd, cmd::ARGTYPE_VECTOR2|cmd::ARGTYPE_STRING);

	GlobalCommandSystem().addCommand("TexAlign", selection::algorithm::alignTextureCmd, cmd::ARGTYPE_STRING);

	// Add the nudge commands (one general, four specialised ones)
	GlobalCommandSystem().addCommand("NudgeSelected", selection::algorithm::nudgeSelectedCmd, cmd::ARGTYPE_STRING);

	GlobalCommandSystem().addCommand("NormaliseTexture", selection::algorithm::normaliseTexture);

	GlobalCommandSystem().addCommand("CopyShader", selection::algorithm::pickShaderFromSelection);
	GlobalCommandSystem().addCommand("PasteShader", selection::algorithm::pasteShaderToSelection);
	GlobalCommandSystem().addCommand("PasteShaderNatural", selection::algorithm::pasteShaderNaturalToSelection);
  
	GlobalCommandSystem().addCommand("FlipTextureX", selection::algorithm::flipTextureS);
	GlobalCommandSystem().addCommand("FlipTextureY", selection::algorithm::flipTextureT);
	
	GlobalCommandSystem().addCommand("MoveSelectionDOWN", Selection_MoveDown);
	GlobalCommandSystem().addCommand("MoveSelectionUP", Selection_MoveUp);
	
	GlobalCommandSystem().addCommand("CurveAppendControlPoint", selection::algorithm::appendCurveControlPoint);
	GlobalCommandSystem().addCommand("CurveRemoveControlPoint", selection::algorithm::removeCurveControlPoints);
	GlobalCommandSystem().addCommand("CurveInsertControlPoint", selection::algorithm::insertCurveControlPoints);
	GlobalCommandSystem().addCommand("CurveConvertType", selection::algorithm::convertCurveTypes);

	GlobalCommandSystem().addCommand("BrushExportCM", selection::algorithm::createCMFromSelection);
	
	GlobalCommandSystem().addCommand("CreateDecalsForFaces", selection::algorithm::createDecalsForSelectedFaces);

	GlobalCommandSystem().addCommand("FindReplaceTextures", ui::FindAndReplaceShader::showDialog);
	GlobalCommandSystem().addCommand("ShowCommandList", ui::CommandList::showDialog);
	GlobalCommandSystem().addCommand("About", ui::AboutDialog::showDialog);

	// ----------------------- Bind Events ---------------------------------------

	GlobalEventManager().addCommand("Exit", "Exit");
	GlobalEventManager().addCommand("ReloadSkins", "ReloadSkins");
	GlobalEventManager().addCommand("ReloadDefs", "ReloadDefs");
	GlobalEventManager().addCommand("ProjectSettings", "ProjectSettings");
	
	GlobalEventManager().addCommand("Copy", "Copy");
	GlobalEventManager().addCommand("Paste", "Paste");
	GlobalEventManager().addCommand("PasteToCamera", "PasteToCamera");

	GlobalEventManager().addCommand("CloneSelection", "CloneSelection", true); // react on keyUp
	GlobalEventManager().addCommand("DeleteSelection", "DeleteSelection");
	GlobalEventManager().addCommand("ParentSelection", "ParentSelection");
	GlobalEventManager().addCommand("ParentSelectionToWorldspawn", "ParentSelectionToWorldspawn");
	GlobalEventManager().addCommand("MergeSelectedEntities", "MergeSelectedEntities");

	GlobalEventManager().addCommand("UnSelectSelection", "UnSelectSelection");
	GlobalEventManager().addCommand("InvertSelection", "InvertSelection");

	GlobalEventManager().addCommand("SelectInside", "SelectInside");
	GlobalEventManager().addCommand("SelectTouching", "SelectTouching");
	GlobalEventManager().addCommand("SelectCompleteTall", "SelectCompleteTall");
	GlobalEventManager().addCommand("ExpandSelectionToEntities", "ExpandSelectionToEntities");
	GlobalEventManager().addCommand("SelectChildren", "SelectChildren");

	GlobalEventManager().addCommand("Preferences", "Preferences");
	
	GlobalEventManager().addCommand("ToggleConsole", "ToggleConsole");
	
	GlobalEventManager().addCommand("ToggleMediaBrowser", "ToggleMediaBrowser");
	GlobalEventManager().addCommand("ToggleLightInspector",	"ToggleLightInspector");
	GlobalEventManager().addCommand("SurfaceInspector", "SurfaceInspector");
	GlobalEventManager().addCommand("PatchInspector", "PatchInspector");
	GlobalEventManager().addCommand("OverlayDialog", "OverlayDialog");
	GlobalEventManager().addCommand("TransformDialog", "TransformDialog");

	GlobalEventManager().addCommand("ShowHidden", "ShowHidden");
	GlobalEventManager().addCommand("HideSelected", "HideSelected");
	GlobalEventManager().addCommand("HideDeselected", "HideDeselected");
	
	GlobalEventManager().addToggle("DragVertices", ToggleVertexMode);
	GlobalEventManager().addToggle("DragEdges", ToggleEdgeMode);
	GlobalEventManager().addToggle("DragFaces", ToggleFaceMode);
	GlobalEventManager().addToggle("DragEntities", ToggleEntityMode);
	GlobalEventManager().addToggle("SelectionModeGroupPart", ToggleGroupPartMode);
	GlobalEventManager().setToggled("DragVertices", false);
	GlobalEventManager().setToggled("DragEdges", false);
	GlobalEventManager().setToggled("DragFaces", false); 
	GlobalEventManager().setToggled("DragEntities", false);
	GlobalEventManager().setToggled("SelectionModeGroupPart", false);
	
	GlobalEventManager().addCommand("MirrorSelectionX", "MirrorSelectionX");
	GlobalEventManager().addCommand("RotateSelectionX", "RotateSelectionX");
	GlobalEventManager().addCommand("MirrorSelectionY", "MirrorSelectionY");
	GlobalEventManager().addCommand("RotateSelectionY", "RotateSelectionY");
	GlobalEventManager().addCommand("MirrorSelectionZ", "MirrorSelectionZ");
	GlobalEventManager().addCommand("RotateSelectionZ", "RotateSelectionZ");
	
	GlobalEventManager().addCommand("FindBrush", "FindBrush");
	GlobalEventManager().addCommand("ConvertSelectedToFuncStatic", "ConvertSelectedToFuncStatic");
	GlobalEventManager().addCommand("RevertToWorldspawn", "RevertToWorldspawn");
	GlobalEventManager().addCommand("MapInfo", "MapInfo");
	GlobalEventManager().addCommand("EditFiltersDialog", "EditFiltersDialog");
	
	GlobalEventManager().addToggle("ToggleClipper", ClipperMode);
	
	GlobalEventManager().addToggle("MouseTranslate", TranslateMode);
	GlobalEventManager().addToggle("MouseRotate", RotateMode);
	//GlobalEventManager().addToggle("MouseScale", FreeCaller<ScaleMode>());
	GlobalEventManager().addToggle("MouseDrag", DragMode);
	
	GlobalEventManager().addCommand("CSGSubtract", "CSGSubtract");
	GlobalEventManager().addCommand("CSGMerge", "CSGMerge");
	GlobalEventManager().addCommand("CSGHollow", "CSGHollow");
	GlobalEventManager().addCommand("CSGRoom", "CSGRoom");
	
	GlobalEventManager().addCommand("RefreshShaders", "RefreshShaders");
	
	GlobalEventManager().addCommand("SnapToGrid", "SnapToGrid");
	
	GlobalEventManager().addCommand("SelectAllOfType", "SelectAllOfType");
	GlobalEventManager().addCommand("GroupCycleForward", "GroupCycleForward");
	GlobalEventManager().addCommand("GroupCycleBackward", "GroupCycleBackward");
	
	GlobalEventManager().addCommand("TexRotateClock", "TexRotateClock");
	GlobalEventManager().addCommand("TexRotateCounter", "TexRotateCounter");
	GlobalEventManager().addCommand("TexScaleUp", "TexScaleUp");
	GlobalEventManager().addCommand("TexScaleDown", "TexScaleDown");
	GlobalEventManager().addCommand("TexScaleLeft", "TexScaleLeft");
	GlobalEventManager().addCommand("TexScaleRight", "TexScaleRight");
	GlobalEventManager().addCommand("TexShiftUp", "TexShiftUp");
	GlobalEventManager().addCommand("TexShiftDown", "TexShiftDown");
	GlobalEventManager().addCommand("TexShiftLeft", "TexShiftLeft");
	GlobalEventManager().addCommand("TexShiftRight", "TexShiftRight");
	GlobalEventManager().addCommand("TexAlignTop", "TexAlignTop");
	GlobalEventManager().addCommand("TexAlignBottom", "TexAlignBottom");
	GlobalEventManager().addCommand("TexAlignLeft", "TexAlignLeft");
	GlobalEventManager().addCommand("TexAlignRight", "TexAlignRight");

	GlobalEventManager().addCommand("NormaliseTexture", "NormaliseTexture");

	GlobalEventManager().addCommand("CopyShader", "CopyShader");
	GlobalEventManager().addCommand("PasteShader", "PasteShader");
	GlobalEventManager().addCommand("PasteShaderNatural", "PasteShaderNatural");
  
	GlobalEventManager().addCommand("FlipTextureX", "FlipTextureX");
	GlobalEventManager().addCommand("FlipTextureY", "FlipTextureY");
	
	GlobalEventManager().addCommand("MoveSelectionDOWN", "MoveSelectionDOWN");
	GlobalEventManager().addCommand("MoveSelectionUP", "MoveSelectionUP");
	
	GlobalEventManager().addCommand("SelectNudgeLeft", "SelectNudgeLeft");
	GlobalEventManager().addCommand("SelectNudgeRight", "SelectNudgeRight");
	GlobalEventManager().addCommand("SelectNudgeUp", "SelectNudgeUp");
	GlobalEventManager().addCommand("SelectNudgeDown", "SelectNudgeDown");

	GlobalEventManager().addRegistryToggle("ToggleRotationPivot", "user/ui/rotationPivotIsOrigin");
	GlobalEventManager().addRegistryToggle("ToggleOffsetClones", selection::algorithm::RKEY_OFFSET_CLONED_OBJECTS);
	
	GlobalEventManager().addCommand("CurveAppendControlPoint", "CurveAppendControlPoint");
	GlobalEventManager().addCommand("CurveRemoveControlPoint", "CurveRemoveControlPoint");
	GlobalEventManager().addCommand("CurveInsertControlPoint", "CurveInsertControlPoint");
	GlobalEventManager().addCommand("CurveConvertType", "CurveConvertType");
	
	GlobalEventManager().addCommand("BrushExportCM", "BrushExportCM");
	
	GlobalEventManager().addCommand("CreateDecalsForFaces", "CreateDecalsForFaces");

	GlobalEventManager().addCommand("FindReplaceTextures", "FindReplaceTextures");
	GlobalEventManager().addCommand("ShowCommandList", "ShowCommandList");
	GlobalEventManager().addCommand("About", "About");

	ui::TexTool::registerCommands();

  Patch_registerCommands();

	GlobalSelectionSystem().addSelectionChangeCallback(ComponentMode_SelectionChanged);
}
