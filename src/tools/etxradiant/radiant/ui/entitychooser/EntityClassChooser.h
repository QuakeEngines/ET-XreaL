#ifndef ENTITYCLASSCHOOSER_H_
#define ENTITYCLASSCHOOSER_H_

#include "iradiant.h"
#include "ieclass.h"
#include "imodelpreview.h"

#include "math/Vector3.h"

#include <gtk/gtkwidget.h>

typedef struct _GtkTreeStore GtkTreeStore;
typedef struct _GtkTreeSelection GtkTreeSelection;

namespace {

    /* CONSTANTS */
    
    const char* FOLDER_ICON = "folder16.png";
    const char* ENTITY_ICON = "cmenu_add_entity.png";
    
    // Registry XPath to lookup key that specifies the display folder
    const char* FOLDER_KEY_PATH = "game/entityChooser/displayFolderKey";

    // Tree column enum
    enum {
        NAME_COLUMN,
        ICON_COLUMN,
        DIR_FLAG_COLUMN,
        N_COLUMNS
    };

}

namespace ui
{

class EntityClassChooser;
typedef boost::shared_ptr<EntityClassChooser> EntityClassChooserPtr;

/** 
 * Dialog window displaying a tree of Entity Classes, allowing the selection
 * of a class to create at the current location. 
 */
class EntityClassChooser :
	public RadiantEventListener,
	public IEntityClassManager::Observer
{
	// Main dialog window
	GtkWidget* _widget;
	
	// Tree model holding the classnames, and the corresponding treeview
	GtkTreeStore* _treeStore;
	GtkWidget* _treeView;
	
	// GtkTreeSelection holding the currently-selected classname
	GtkTreeSelection* _selection;

	// Usage information textview
	GtkWidget* _usageTextView;

	// OK button. Needs to be a member since we enable/disable it in the
	// selectionchanged callback.
	GtkWidget* _okButton;
	
	// Last selected classname
	std::string _selectedName;

	// Model preview widget
	IModelPreviewPtr _modelPreview;

private:

	/* Widget construction helpers */
	
	GtkWidget* createTreeView();
	GtkWidget* createUsagePanel();
	GtkWidget* createButtonPanel();

	// Update the usage panel with information from the provided entityclass
	void updateUsageInfo(const std::string& eclass);

	// Updates the member variables based on the current tree selection
	void updateSelection();

	/* GTK callbacks */
	
	// Called when close button is clicked, ensure that window is hidden
	// not destroyed.
	static gboolean callbackHide(GtkWidget* widget, GdkEvent* ev, EntityClassChooser* self);

	// Button callbacks
	static void callbackCancel(GtkWidget*, EntityClassChooser*);
	static void callbackOK(GtkWidget*, EntityClassChooser*);

	// Check when the selection changes, disable the add button if there
	// is nothing selected.
	static void callbackSelectionChanged(GtkWidget*, EntityClassChooser*);

	/// Constructor. Creates the GTK widgets.
	EntityClassChooser();
	
	// Show the dialog and choose an entity class.
	std::string showAndBlock();

	// This is where the static shared_ptr of the singleton instance is held.
	static EntityClassChooserPtr& InstancePtr();

	// Loads or reloads the entity class tree
	void loadEntityClasses();
	
public:
	
	/** 
	 * Display the dialog and block awaiting the selection of an entity class,
	 * which is returned to the caller. If the dialog is cancelled or no
	 * selection is made, and empty string will be returned.
	 */
	static std::string chooseEntityClass();

	// RadiantEventListener implementation
	void onRadiantShutdown();

	// EntityClassManager::Observer impl.
	void onEClassReload();
};

} // namespace ui

#endif /*ENTITYCLASSCHOOSER_H_*/
