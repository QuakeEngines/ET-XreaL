#ifndef ECLASSTREE_H_
#define ECLASSTREE_H_

#include "iradiant.h"
#include "icommandsystem.h"
#include "gtkutil/window/BlockingTransientWindow.h"
#include <boost/shared_ptr.hpp>

typedef struct _GtkListStore GtkListStore;
typedef struct _GtkTreeView GtkTreeView;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkTreeStore GtkTreeStore;
typedef struct _GtkTreeSelection GtkTreeSelection;
typedef struct _GtkTreeModel GtkTreeModel;
typedef struct _GtkTreeIter GtkTreeIter;

namespace ui {

	namespace {
		// Tree column enum
	    enum {
	        NAME_COLUMN,
	        ICON_COLUMN,
	        N_COLUMNS
	    };
	}

class EClassTree;
typedef boost::shared_ptr<EClassTree> EClassTreePtr;

class EClassTree :
	public gtkutil::BlockingTransientWindow
{
	// The EClass treeview widget and underlying liststore
	GtkTreeView* _eclassView;
	GtkTreeStore* _eclassStore;
	GtkTreeSelection* _eclassSelection;
	
	// The treeview and liststore for the property pane
	GtkTreeView* _propertyView;
	GtkListStore* _propertyStore;
	
	GtkWidget* _dialogVBox;
	
	// Private constructor, traverses the entity classes
	EClassTree();

public:
	// Shows the singleton class (static command target)
	static void showWindow(const cmd::ArgumentList& args);
	
private:
	virtual void _preShow();

	// Constructs and adds all the dialog widgets
	void populateWindow();
	
	GtkWidget* createButtons(); 	// Dialog buttons
	GtkWidget* createEClassTreeView(); // EClass Tree
	GtkWidget* createPropertyTreeView(); // Property Tree
	
	// Loads the spawnargs into the right treeview
	void updatePropertyView(const std::string& eclassName);
	
	// Static GTK callbacks
	static void onClose(GtkWidget* button, EClassTree* self);
	static void onSelectionChanged(GtkWidget* widget, EClassTree* self);

	// The comparison function for eclasstree's type-in search
	static gboolean eClassnameEqualFunc(GtkTreeModel* model, 
										 gint column,
										 const gchar* key,
										 GtkTreeIter* iter,
										 gpointer search_data);
};

} // namespace ui

#endif /*ECLASSTREE_H_*/

