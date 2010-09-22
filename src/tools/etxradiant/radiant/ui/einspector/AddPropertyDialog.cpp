#include "AddPropertyDialog.h"
#include "PropertyEditorFactory.h"

#include "gtkutil/RightAlignment.h"
#include "gtkutil/TreeModel.h"
#include "gtkutil/ScrolledFrame.h"
#include "gtkutil/MultiMonitor.h"
#include "gtkutil/IconTextColumn.h"

#include "i18n.h"
#include "imainframe.h"
#include "iuimanager.h"
#include "igroupdialog.h"
#include "ieclass.h"
#include "igame.h"
#include "ientity.h"

#include <gtk/gtk.h>
#include <boost/algorithm/string/predicate.hpp>

#include <map>

namespace ui
{
	
namespace {
	
	// Tree columns
	enum {
		DISPLAY_NAME_COLUMN,
		PROPERTY_NAME_COLUMN,
		ICON_COLUMN,
		DESCRIPTION_COLUMN,
		N_COLUMNS
	};
	
	// CONSTANTS
	const char* ADDPROPERTY_TITLE = N_("Add property");
	const char* PROPERTIES_XPATH = "/entityInspector//property";
	const char* FOLDER_ICON = "folder16.png";
	
	const char* CUSTOM_PROPERTY_TEXT = N_("Custom properties defined for this "
	"entity class, if any");
	
}

// Constructor creates GTK widgets

AddPropertyDialog::AddPropertyDialog(Entity* entity)
: _widget(gtk_window_new(GTK_WINDOW_TOPLEVEL)),
  _entity(entity)
{
	// Window properties
	GtkWindow* parent = GTK_WINDOW(GlobalGroupDialog().getDialogWindow());

	if (!GTK_IS_WINDOW(parent) || !GTK_WIDGET_VISIBLE(parent))
	{
		parent = GlobalMainFrame().getTopLevelWindow();
	}
	
	gtk_window_set_transient_for(GTK_WINDOW(_widget), parent);
	gtk_window_set_modal(GTK_WINDOW(_widget), TRUE);
	gtk_window_set_title(GTK_WINDOW(_widget), _(ADDPROPERTY_TITLE));
    gtk_window_set_position(GTK_WINDOW(_widget), GTK_WIN_POS_CENTER_ON_PARENT);
    
    // Set size of dialog
	GdkRectangle rect = gtkutil::MultiMonitor::getMonitorForWindow(parent);
    gtk_window_set_default_size(GTK_WINDOW(_widget), static_cast<gint>(rect.width/2), static_cast<gint>(rect.height*2/3));
    
    // Signals
    g_signal_connect(G_OBJECT(_widget), "delete-event", 
    				 G_CALLBACK(_onDelete), this);
    
    // Create components
    GtkWidget* vbx = gtk_vbox_new(FALSE, 6);
    gtk_box_pack_start(GTK_BOX(vbx), createTreeView(), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbx), createUsagePanel(), FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbx), createButtonsPanel(), FALSE, FALSE, 0);
    
    // Pack into window
    gtk_container_set_border_width(GTK_CONTAINER(_widget), 6);
    gtk_container_add(GTK_CONTAINER(_widget), vbx);
    
    // Populate the tree view with properties
    populateTreeView();

	updateUsagePanel();
}

// Construct the tree view

GtkWidget* AddPropertyDialog::createTreeView() {
	// Set up the tree store
	_treeStore = gtk_tree_store_new(N_COLUMNS,
									G_TYPE_STRING, // display name
									G_TYPE_STRING, // property name
									GDK_TYPE_PIXBUF, // icon
									G_TYPE_STRING); // description
	// Create tree view
	_treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(_treeStore));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(_treeView), FALSE);

	// Connect up selection changed callback
	_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(_treeView));
	g_signal_connect(G_OBJECT(_selection), "changed", 
					 G_CALLBACK(_onSelectionChanged), this);

	// Allow multiple selections
	gtk_tree_selection_set_mode(_selection, GTK_SELECTION_MULTIPLE);
	
	// Display name column with icon
    gtk_tree_view_append_column(
    	GTK_TREE_VIEW(_treeView), 
    	gtkutil::IconTextColumn("", DISPLAY_NAME_COLUMN, ICON_COLUMN, true));                                                                        

	// Model owned by view
	g_object_unref(G_OBJECT(_treeStore));

	// Use the TreeModel's full string search function
	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(_treeView), 
		gtkutil::TreeModel::equalFuncStringContains, NULL, NULL);
	
	// Pack into scrolled window and frame, and return
	return gtkutil::ScrolledFrame(_treeView);
}

// Construct the usage panel
GtkWidget* AddPropertyDialog::createUsagePanel() {
	// Create a GtkTextView
	_usageTextView = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(_usageTextView), GTK_WRAP_WORD);

	return gtkutil::ScrolledFrame(_usageTextView);	
}

// Construct the buttons panel
GtkWidget* AddPropertyDialog::createButtonsPanel() {
	GtkWidget* hbx = gtk_hbox_new(TRUE, 6);
	
	GtkWidget* okButton = gtk_button_new_from_stock(GTK_STOCK_OK);
	GtkWidget* cancelButton = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	
	g_signal_connect(G_OBJECT(okButton), "clicked", G_CALLBACK(_onOK), this);
	g_signal_connect(G_OBJECT(cancelButton), "clicked", G_CALLBACK(_onCancel), this);
	
	gtk_box_pack_end(GTK_BOX(hbx), okButton, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(hbx), cancelButton, TRUE, TRUE, 0);
	
	return gtkutil::RightAlignment(hbx);	
}

namespace {

/* EntityClassAttributeVisitor instance to obtain custom properties from an 
 * entityclass and add them into the provided GtkTreeStore under the provided 
 * parent iter.
 */
class CustomPropertyAdder
: public EntityClassAttributeVisitor
{
	// Treestore to add to
	GtkTreeStore* _store;
	
	// Parent iter
	GtkTreeIter* _parent;

	// The entity we're adding the properties to
	Entity* _entity;

public:

	// Constructor sets tree stuff
	CustomPropertyAdder(Entity* entity, GtkTreeStore* store, GtkTreeIter* par) : 
		_store(store), 
		_parent(par), 
		_entity(entity)
	{ }

	// Required visit function
	void visit(const EntityClassAttribute& attr) {
		// greebo: Only add the property if it hasn't been set directly on the entity itself.
		if (!_entity->getKeyValue(attr.name).empty() && !_entity->isInherited(attr.name)) {
			return;
		}

		// Also ignore all attributes with empty descriptions
		if (attr.description.empty()) {
			return;
		}

		// Escape any Pango markup in the attribute name (e.g. "<" or ">")
		gchar* escName = g_markup_escape_text(attr.name.c_str(), -1);
		
		GtkTreeIter tmp;
		gtk_tree_store_append(_store, &tmp, _parent);
		gtk_tree_store_set(_store, &tmp,
			DISPLAY_NAME_COLUMN, escName,
			PROPERTY_NAME_COLUMN, attr.name.c_str(),
			ICON_COLUMN, PropertyEditorFactory::getPixbufFor(attr.type),
			DESCRIPTION_COLUMN, attr.description.c_str(),
			-1);
			
		// Free the escaped string
		g_free(escName);
	}
	
};	
	
} // namespace

// Populate tree view
void AddPropertyDialog::populateTreeView() 
{
	/* DEF-DEFINED PROPERTIES */

	// First add a top-level category named after the entity class, and populate
	// it with custom keyvals defined in the DEF for that class
	std::string cName = "<b><span foreground=\"blue\">" 
						+ _entity->getEntityClass()->getName() + "</span></b>";
	GtkTreeIter cnIter;
	gtk_tree_store_append(_treeStore, &cnIter, NULL);
	gtk_tree_store_set(_treeStore, &cnIter, 
					   DISPLAY_NAME_COLUMN, cName.c_str(),
					   PROPERTY_NAME_COLUMN, "",
					   ICON_COLUMN, GlobalUIManager().getLocalPixbuf(FOLDER_ICON),
					   DESCRIPTION_COLUMN, _(CUSTOM_PROPERTY_TEXT),
					   -1);
					   
	// Use a CustomPropertyAdder class to visit the entityclass and add all
	// custom properties from it
	CustomPropertyAdder adder(_entity, _treeStore, &cnIter);
	_entity->getEntityClass()->forEachClassAttribute(adder);

	/* REGISTRY (GAME FILE) DEFINED PROPERTIES */

	// Ask the XML registry for the list of properties
    game::IGamePtr currentGame = GlobalGameManager().currentGame();
    xml::NodeList propNodes = currentGame->getLocalXPath(PROPERTIES_XPATH);
	
	// Cache of property categories to GtkTreeIters, to allow properties
	// to be parented to top-level categories
	typedef std::map<std::string, GtkTreeIter*> CategoryMap;
	CategoryMap categories;
	
	// Add each .game-specified property to the tree view
	for (xml::NodeList::const_iterator iter = propNodes.begin();
		 iter != propNodes.end();
		 ++iter)
	{
		// Skip hidden properties
		if (iter->getAttributeValue("hidden") == "1")
		{
			continue;
		}

		GtkTreeIter t;

		// If this property has a category, look up the top-level parent iter
		// or add it if necessary.
		std::string category = iter->getAttributeValue("category");
		if (!category.empty()) {
			CategoryMap::iterator mIter = categories.find(category);
			
			if (mIter == categories.end()) {
				// Not found, add to treestore
				GtkTreeIter tIter;
				gtk_tree_store_append(_treeStore, &tIter, NULL);
				gtk_tree_store_set(_treeStore, &tIter,
					DISPLAY_NAME_COLUMN, category.c_str(),
					PROPERTY_NAME_COLUMN, "",
					ICON_COLUMN, GlobalUIManager().getLocalPixbuf(FOLDER_ICON),
					DESCRIPTION_COLUMN, "",
					-1);
					
				// Add to map
				mIter = categories.insert(CategoryMap::value_type(category, gtk_tree_iter_copy(&tIter))).first;
			}
			
			// Category sorted, add this property below it
			gtk_tree_store_append(_treeStore, &t, mIter->second);
		}
		else {
			// No category, add at toplevel
			gtk_tree_store_append(_treeStore, &t, NULL);
		}
		
		// Obtain information from the XML node and add it to the treeview
		std::string name = iter->getAttributeValue("match");
		std::string type = iter->getAttributeValue("type");
		std::string description = iter->getContent();
		
		gtk_tree_store_set(_treeStore, &t,
			DISPLAY_NAME_COLUMN, name.c_str(),
			PROPERTY_NAME_COLUMN, name.c_str(),
			ICON_COLUMN, PropertyEditorFactory::getPixbufFor(type),
			DESCRIPTION_COLUMN, description.c_str(),
			-1);
	}
}

// Static method to create and show an instance, and return the chosen
// property to calling function.
AddPropertyDialog::PropertyList AddPropertyDialog::chooseProperty(Entity* entity) {
	
	// Construct a dialog and show the main widget
	AddPropertyDialog dialog(entity);
	gtk_widget_show_all(dialog._widget);
	
	// Block for a selection
	gtk_main();
	
	// Return the last selection to calling process
	return dialog._selectedProperties;
}

void AddPropertyDialog::updateUsagePanel()
{
	GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(_usageTextView));

	if (_selectedProperties.size() != 1)
	{
		gtk_text_buffer_set_text(buf, "", -1);
		gtk_widget_set_sensitive(GTK_WIDGET(_usageTextView), FALSE);
	}
	else
	{
		// Load the description
		GList* list = gtk_tree_selection_get_selected_rows(_selection, NULL);

		if (list != NULL)
		{
			GtkTreePath* path = static_cast<GtkTreePath*>(list->data);

			GtkTreeIter iter;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(_treeStore), &iter, path))
			{
				std::string desc = gtkutil::TreeModel::getString(GTK_TREE_MODEL(_treeStore), &iter, DESCRIPTION_COLUMN);
				gtk_text_buffer_set_text(buf, desc.c_str() , -1);
			}

			// Free the path during traversal, not needed anymore
			gtk_tree_path_free(path);
		}

		g_list_free(list);

		gtk_widget_set_sensitive(GTK_WIDGET(_usageTextView), TRUE);
	}
}

/* GTK CALLBACKS */

void AddPropertyDialog::_onDelete(GtkWidget* w, GdkEvent* e, AddPropertyDialog* self)
{
	self->_selectedProperties.clear();
	gtk_widget_destroy(self->_widget);
	gtk_main_quit(); // exit recursive main loop	
}

void AddPropertyDialog::_onOK(GtkWidget* w, AddPropertyDialog* self)
{
	gtk_widget_destroy(self->_widget);
	gtk_main_quit(); // exit recursive main loop	
}

void AddPropertyDialog::_onCancel(GtkWidget* w, AddPropertyDialog* self)
{
	self->_selectedProperties.clear();
	gtk_widget_destroy(self->_widget);
	gtk_main_quit(); // exit recursive main loop	
}

void AddPropertyDialog::_onSelectionChanged(GtkWidget* w, 
											AddPropertyDialog* self) 
{
	self->_selectedProperties.clear();
	
	GList* list = gtk_tree_selection_get_selected_rows(self->_selection, NULL);

	while (list != NULL)
	{
		GtkTreePath* path = static_cast<GtkTreePath*>(list->data);

		GtkTreeIter iter;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(self->_treeStore), &iter, path))
		{
			self->_selectedProperties.push_back(
				gtkutil::TreeModel::getString(GTK_TREE_MODEL(self->_treeStore), &iter, PROPERTY_NAME_COLUMN)
			);
		}

		// Free the path during traversal, not needed anymore
		gtk_tree_path_free(path);

		list = list->next;
	}

	g_list_free(list);

	self->updateUsagePanel();
}

} // namespace ui
