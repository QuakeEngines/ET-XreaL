#ifndef ENTITYINSPECTOR_H_
#define ENTITYINSPECTOR_H_

#include "PropertyEditor.h"

#include "ientityinspector.h"
#include "iradiant.h"
#include "icommandsystem.h"
#include "iselection.h"
#include "ientity.h"
#include "iundo.h"

#include "gtkutil/menu/PopupMenu.h"
#include "gtkutil/Paned.h"
#include "gtkutil/PanedPosition.h"

#include <gtk/gtkliststore.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtktreeviewcolumn.h>
#include <map>
#include <boost/enable_shared_from_this.hpp>
#include <boost/algorithm/string/predicate.hpp>

/* FORWARD DECLS */

class Entity;
class Selectable;

namespace ui {

class EntityInspector;
typedef boost::shared_ptr<EntityInspector> EntityInspectorPtr;

/* The EntityInspector class represents the GTK dialog for editing properties
 * on the selected game entity. The class is implemented as a singleton and
 * contains a method to return the current instance.
 */
class EntityInspector :
	public IEntityInspector,
 	public SelectionSystem::Observer,
	public RadiantEventListener,
    public Entity::Observer,
	public UndoSystem::Observer,
	public boost::enable_shared_from_this<EntityInspector>
{
private:
	class StringCompareFunctor : 
		public std::binary_function<std::string, std::string, bool>
	{
	public:
		bool operator()(const std::string& lhs, const std::string& rhs) const
		{
			return boost::algorithm::ilexicographical_compare(lhs, rhs);
		}
	};

	// Currently selected entity, this pointer is only non-NULL if the
	// current entity selection includes exactly 1 entity.
	Entity* _selectedEntity;

	// Main EntityInspector widget
    GtkWidget* _widget;

	// Frame to contain the Property Editor
    GtkWidget* _editorFrame;

	// The checkbox for showing the eclass properties
	GtkWidget* _showInheritedCheckbox;
	GtkWidget* _showHelpColumnCheckbox;

    // View and model for the keyvalue list
    GtkListStore* _kvStore;
    GtkWidget* _keyValueTreeView;

	GtkTreeViewColumn* _helpColumn;

    // Cache of GtkTreeIters pointing to keyvalue rows, so we can quickly find
    // existing keys to change their values
    typedef std::map<std::string, GtkTreeIter, StringCompareFunctor> TreeIterMap;
    TreeIterMap _keyValueIterMap;

	// Key and value edit boxes. These remain available even for multiple entity
    // selections.
	GtkWidget* _keyEntry;
	GtkWidget* _valEntry;

	// The pane dividing the treeview and the property editors
	gtkutil::PanedPtr _paned;

	// An object tracking the divider position of the paned view
	gtkutil::PanedPosition _panedPosition;

	// Context menu
	gtkutil::PopupMenuPtr _contextMenu;

	// Currently displayed PropertyEditor
	IPropertyEditorPtr _currentPropertyEditor;

	// The clipboard for spawnargs
	struct ClipBoard
	{
		std::string key;
		std::string value;

		bool empty() const {
			return key.empty();
		}
	} _clipBoard;

	// Data structure to store the type (vector3, text etc) and the options
	// string for a single property.
	struct PropertyParms
	{
		std::string type;
		std::string options;
	};

	// Map of property names to PropertyParms, mapped like this: regex => parms
	typedef std::map<std::string, PropertyParms> PropertyParmMap;
	PropertyParmMap _propertyTypes;

private:

    // Utility functions to construct the Gtk components
	void construct();

    GtkWidget* createPropertyEditorPane(); // bottom widget pane
    GtkWidget* createTreeViewPane(); // tree view for selecting attributes
    void createContextMenu();

	// Utility function to retrieve the string selection from the given column in the
	// list store
	std::string getListSelection(int col);

	/* gtkutil::PopupMenu callbacks */
	void _onAddKey();
	void _onDeleteKey();
	void _onCopyKey();
	void _onCutKey();
	void _onPasteKey();

	bool _testDeleteKey();
	bool _testCopyKey();
	bool _testCutKey();
	bool _testPasteKey();

    /* GTK CALLBACKS */
    static void callbackTreeSelectionChanged(GtkWidget* widget, EntityInspector* self);
	static void _onEntryActivate(GtkWidget*, EntityInspector*);
	static void _onSetProperty(GtkWidget*, EntityInspector*);
	static void _onToggleShowInherited(GtkToggleButton*, EntityInspector*);
	static void _onToggleShowHelpIcons(GtkToggleButton*, EntityInspector*);

	static gboolean _onQueryTooltip(GtkWidget* widget,
									 gint x, gint y, gboolean keyboard_mode,
									 GtkTooltip* tooltip, EntityInspector* self);

    static std::string  cleanInputString( const std::string& );

    // Add and remove inherited properties from the entity class
    void addClassProperties();
    void removeClassProperties();

	// Update the GTK components when a new selection is made in the tree view
    void treeSelectionChanged();

    // Update our selected entity pointer from the selection system
    void getEntityFromSelectionSystem();

    // Change the selected entity pointer, setting up the observer
    void changeSelectedEntity(Entity* newEntity);

    // Set the keyval on all selected entities from the key and value textboxes
	void setPropertyFromEntries();

    // Applies the given key/value pair to the selection (works with multiple
    // selected entities)
    void applyKeyValueToSelection(const std::string& key,
                                  const std::string& value);

	// Initialise the property lookup tables
	void loadPropertyMap();

	// Returns property type and option for the given entity key
	PropertyParms getPropertyParmsForKey(const std::string& key);

    // Update tree view contents and property editor
    void updateGUIElements();

protected:
	// Get the Gtk Widget for display in the main application
    GtkWidget* _getWidget() const;

public:
	// Constructor
    EntityInspector();

	/** greebo: Gets called by the RadiantSelectionSystem upon selection change.
	 */
	void selectionChanged(const scene::INodePtr& node, bool isComponent);

	void registerPropertyEditor(const std::string& key, const IPropertyEditorPtr& editor);
	void unregisterPropertyEditor(const std::string& key);

	// RadiantEventListener implementation, gets called right before shutdown
	void onRadiantShutdown();

	// Gets called after an undo operation
	void postUndo();
	// Gets called after a redo operation
	void postRedo();

    /* Entity::Observer implementation */
    void onKeyInsert(const std::string& key, EntityKeyValue& value);
    void onKeyChange(const std::string& key, const std::string& value);
    void onKeyErase(const std::string& key, EntityKeyValue& value);

	// greebo: Tells the inspector to reload the window settings from the registry.
	void restoreSettings();

	/**
	 * greebo: Static command target for toggling the Entity Inspector in the GroupDialog.
	 */
	static void toggle(const cmd::ArgumentList& args);

	// RegisterableModule implementation
	virtual const std::string& getName() const;
	virtual const StringSet& getDependencies() const;
	virtual void initialiseModule(const ApplicationContext& ctx);
};

} // namespace ui

#endif /*ENTITYINSPECTOR_H_*/
