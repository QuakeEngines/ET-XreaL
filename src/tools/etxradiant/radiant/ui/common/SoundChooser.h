#ifndef SOUNDCHOOSER_H_
#define SOUNDCHOOSER_H_

typedef struct _GtkTreeStore GtkTreeStore;
typedef struct _GtkTreeView GtkTreeView;
typedef struct _GtkTreeSelection GtkTreeSelection;
#include "gtkutil/window/BlockingTransientWindow.h"

#include "ui/common/SoundShaderPreview.h"
#include <string>

namespace ui
{

/**
 * Dialog for listing and selection of sound shaders.
 */
class SoundChooser :
	public gtkutil::BlockingTransientWindow
{
private:
	// Tree store for shaders, and the tree selection
	GtkTreeStore* _treeStore;
	GtkTreeView* _treeView;
	GtkTreeSelection* _treeSelection;
	
	// The preview widget group
	SoundShaderPreview _preview;
	
	// Last selected shader
	std::string _selectedSound;

private:

	// Widget construction
	GtkWidget* createTreeView();
	GtkWidget* createButtons();

	/* GTK CALLBACKS */
	static void _onOK(GtkWidget*, SoundChooser*);
	static void _onCancel(GtkWidget*, SoundChooser*);
	static void _onSelectionChange(GtkTreeSelection*, SoundChooser*);
	
	// Implement custom action on window delete
	void _onDeleteEvent();

public:
	
	/**
	 * Constructor creates widgets.
	 */
	SoundChooser();

	/**
	 * Display the dialog and return the selection.
	 */
	std::string chooseSound();

};

}

#endif /*SOUNDCHOOSER_H_*/
