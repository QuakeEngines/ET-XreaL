#include "SplitPaneLayout.h"

#include "itextstream.h"
#include "ieventmanager.h"
#include "iuimanager.h"
#include "igroupdialog.h"
#include "imainframe.h"
#include "ientityinspector.h"

#include "gtkutil/FramedWidget.h"
#include "gtkutil/Paned.h"

#include "camera/GlobalCamera.h"
#include "ui/texturebrowser/TextureBrowser.h"
#include "xyview/GlobalXYWnd.h"

namespace ui {

	namespace
	{
		const std::string RKEY_SPLITPANE_ROOT = "user/ui/mainFrame/splitPane";
		const std::string RKEY_SPLITPANE_TEMP_ROOT = RKEY_SPLITPANE_ROOT + "/temp";
	}

std::string SplitPaneLayout::getName() {
	return SPLITPANE_LAYOUT_NAME;
}

void SplitPaneLayout::activate() {

	GtkWindow* parent = GlobalMainFrame().getTopLevelWindow();

	// Create a new camera window and parent it
	_camWnd = GlobalCamera().createCamWnd();
	 // greebo: The mainframe window acts as parent for the camwindow
	_camWnd->setContainer(parent);

	GtkWidget* camera = _camWnd->getWidget();

	// Allocate the three ortho views
    XYWndPtr xyWnd = GlobalXYWnd().createEmbeddedOrthoView();
    xyWnd->setViewType(XY);
    GtkWidget* xy = xyWnd->getWidget();
    
    XYWndPtr yzWnd = GlobalXYWnd().createEmbeddedOrthoView();
    yzWnd->setViewType(YZ);
    GtkWidget* yz = yzWnd->getWidget();

    XYWndPtr xzWnd = GlobalXYWnd().createEmbeddedOrthoView();
    xzWnd->setViewType(XZ);
    GtkWidget* xz = xzWnd->getWidget();

	// Cam / YZ Pane
	gtkutil::Paned vertPane1(gtkutil::Paned::Vertical);

	vertPane1.setFirstChild(gtkutil::FramedWidget(camera), true); // allow shrinking
	vertPane1.setSecondChild(gtkutil::FramedWidget(yz), true); // allow shrinking

	gtkutil::Paned vertPane2(gtkutil::Paned::Vertical);

	vertPane2.setFirstChild(gtkutil::FramedWidget(xy), true); // allow shrinking
	vertPane2.setSecondChild(gtkutil::FramedWidget(xz), true); // allow shrinking

	// Arrange the widgets into the paned views
	_splitPane.vertPane1 = vertPane1.getWidget();
	_splitPane.vertPane2 = vertPane2.getWidget();

	// The overall horizontal pane, containing the other two as children
	gtkutil::Paned horizPane(gtkutil::Paned::Horizontal);

	horizPane.setFirstChild(_splitPane.vertPane1, true);
	horizPane.setSecondChild(_splitPane.vertPane2, true);

	_splitPane.horizPane = horizPane.getWidget();

	// Retrieve the main container of the main window
	GtkWidget* mainContainer = GlobalMainFrame().getMainContainer();
	gtk_container_add(GTK_CONTAINER(mainContainer), GTK_WIDGET(_splitPane.horizPane));

	gtk_paned_set_position(GTK_PANED(_splitPane.horizPane), 200);
	gtk_paned_set_position(GTK_PANED(_splitPane.vertPane1), 200);
	gtk_paned_set_position(GTK_PANED(_splitPane.vertPane2), 400);

	_splitPane.posHPane.connect(_splitPane.horizPane);
	_splitPane.posVPane1.connect(_splitPane.vertPane1);
	_splitPane.posVPane2.connect(_splitPane.vertPane2);
	
	// Attempt to restore this layout's state
	restoreStateFromPath(RKEY_SPLITPANE_ROOT);
	
    {      
		GtkWidget* textureBrowser = gtkutil::FramedWidget(
			GlobalTextureBrowser().constructWindow(parent)
		);

		// Add the Media Browser page
		GlobalGroupDialog().addPage(
	    	"textures",	// name
	    	"Textures", // tab title
	    	"icon_texture.png", // tab icon 
	    	GTK_WIDGET(textureBrowser), // page widget
	    	"Texture Browser"
	    );
    }

	GlobalGroupDialog().showDialogWindow();

	// greebo: Now that the dialog is shown, tell the Entity Inspector to reload 
	// the position info from the Registry once again.
	GlobalEntityInspector().restoreSettings();

	GlobalGroupDialog().hideDialogWindow();

	gtk_widget_show_all(mainContainer);

	// Hide the camera toggle option for non-floating views
    GlobalUIManager().getMenuManager().setVisibility("main/view/cameraview", false);
}

void SplitPaneLayout::deactivate()
{
	if (GlobalRegistry().keyExists(RKEY_SPLITPANE_TEMP_ROOT))
	{
		// We're maximised, restore the size first
		restorePanePositions();
	}

	// Show the camera toggle option again
    GlobalUIManager().getMenuManager().setVisibility("main/view/cameraview", true);

	// Remove all previously saved pane information 
	GlobalRegistry().deleteXPath(RKEY_SPLITPANE_ROOT + "//pane");

	// Save the pane info
	saveStateToPath(RKEY_SPLITPANE_ROOT);

	// Delete all active views
	GlobalXYWnd().destroyViews();

	// Delete the CamWnd
	_camWnd = CamWndPtr();

	// Hide the group dialog
	GlobalGroupDialog().hideDialogWindow();

	// Remove the texture browser from the groupdialog
	GlobalGroupDialog().removePage("textures");
	GlobalTextureBrowser().destroyWindow();

	// Destroy the widget, so it gets removed from the main container
	gtk_widget_destroy(GTK_WIDGET(_splitPane.horizPane));
}

void SplitPaneLayout::maximiseCameraSize()
{
	// Save the current state to the registry
	saveStateToPath(RKEY_SPLITPANE_TEMP_ROOT);

	// Maximise the pane positions
	_splitPane.posHPane.applyMaxPosition();
	_splitPane.posVPane1.applyMaxPosition();
}

void SplitPaneLayout::restorePanePositions()
{
	// Restore state
	restoreStateFromPath(RKEY_SPLITPANE_TEMP_ROOT);

	// Remove all previously stored pane information
	GlobalRegistry().deleteXPath(RKEY_SPLITPANE_TEMP_ROOT);
}

void SplitPaneLayout::restoreStateFromPath(const std::string& path)
{
	if (GlobalRegistry().keyExists(path + "/pane[@name='horizontal']"))
	{
		_splitPane.posHPane.loadFromPath(path + "/pane[@name='horizontal']");
		_splitPane.posHPane.applyPosition();
	}
	
	if (GlobalRegistry().keyExists(path + "/pane[@name='vertical1']"))
	{
		_splitPane.posVPane1.loadFromPath(path + "/pane[@name='vertical1']");
		_splitPane.posVPane1.applyPosition();
	}
	
	if (GlobalRegistry().keyExists(path + "/pane[@name='vertical2']"))
	{
		_splitPane.posVPane2.loadFromPath(path + "/pane[@name='vertical2']");
		_splitPane.posVPane2.applyPosition();
	}
}

void SplitPaneLayout::saveStateToPath(const std::string& path)
{
	GlobalRegistry().createKeyWithName(path, "pane", "horizontal");
	_splitPane.posHPane.saveToPath(path + "/pane[@name='horizontal']");
	
	GlobalRegistry().createKeyWithName(path, "pane", "vertical1");
	_splitPane.posVPane1.saveToPath(path + "/pane[@name='vertical1']");

	GlobalRegistry().createKeyWithName(path, "pane", "vertical2");
	_splitPane.posVPane2.saveToPath(path + "/pane[@name='vertical2']");
}

void SplitPaneLayout::toggleFullscreenCameraView()
{
	if (GlobalRegistry().keyExists(RKEY_SPLITPANE_TEMP_ROOT))
	{
		restorePanePositions();
	}
	else
	{
		// No saved info found in registry, maximise cam
		maximiseCameraSize();
	}
}

// The creation function, needed by the mainframe layout manager
SplitPaneLayoutPtr SplitPaneLayout::CreateInstance() {
	return SplitPaneLayoutPtr(new SplitPaneLayout);
}

} // namespace ui
