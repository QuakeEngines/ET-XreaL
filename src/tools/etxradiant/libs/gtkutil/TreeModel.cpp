#include "TreeModel.h"

#include <boost/algorithm/string/find.hpp>

namespace gtkutil
{

// Extract a string from a TreeModel
std::string TreeModel::getString(GtkTreeModel* model, 
								 GtkTreeIter* iter, 
								 gint colNo) 
{
	// Get a GValue
	GValue val = {0, {{0}}};
	gtk_tree_model_get_value(model, iter, colNo, &val);

	// Create and return the string, and free the GValue
	const gchar* c = g_value_get_string(&val);

	// greebo: g_value_get_string can return NULL, catch this
	std::string retVal = (c != NULL) ? c : "";

	g_value_unset(&val);
	return retVal;
}

// Extract a boolean from a TreeModel
bool TreeModel::getBoolean(GtkTreeModel* model, GtkTreeIter* iter, gint colNo) {
	// Get a GValue
	GValue val = {0, {{0}}};
	gtk_tree_model_get_value(model, iter, colNo, &val);

	// Create and return the string, and free the GValue
	bool retVal = g_value_get_boolean(&val) ? true: false;
	g_value_unset(&val);
	return retVal;
}

// Extract a boolean from a TreeModel
int TreeModel::getInt(GtkTreeModel* model, GtkTreeIter* iter, gint colNo) {
	// Get a GValue
	GValue val = {0, {{0}}};
	gtk_tree_model_get_value(model, iter, colNo, &val);

	// Create and return the string, and free the GValue
	int retVal = g_value_get_int(&val);
	g_value_unset(&val);
	return retVal;
}

// Extract a boolean from a TreeModel
gpointer TreeModel::getPointer(GtkTreeModel* model, GtkTreeIter* iter, gint colNo) {
	// Get a GValue
	GValue val = {0, {{0}}};
	gtk_tree_model_get_value(model, iter, colNo, &val);

	// Create and return the string, and free the GValue
	gpointer retVal = g_value_get_pointer(&val);
	g_value_unset(&val);
	return retVal;
}

// Extract a selected string
std::string TreeModel::getSelectedString(GtkTreeSelection* sel, gint colNo)
{
	GtkTreeIter iter;
	GtkTreeModel* model;
	
	// Get the selected value by querying the selection object
	if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
		return getString(model,	&iter, colNo);
	}
	else {
		// Nothing selected, return empty string
		return "";
	}
}

bool TreeModel::getSelectedBoolean(GtkTreeSelection* selection, gint colNo)
{
	GtkTreeIter iter;
	GtkTreeModel* model;
	
	// Get the selected value by querying the selection object
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		return getBoolean(model, &iter, colNo);
	}
	else {
		// Nothing selected, return false
		return false;
	}
}

gboolean TreeModel::equalFuncStringContains(GtkTreeModel* model, 
										gint column,
										const gchar* key,
										GtkTreeIter* iter,
										gpointer search_data)
{
	// Retrieve the eclass string from the model
	std::string str = getString(model, iter, column);

	// Use a case-insensitive search
	boost::iterator_range<std::string::iterator> range = boost::algorithm::ifind_first(str, key);

	// Returning FALSE means "match".
	return (!range.empty()) ? FALSE: TRUE;
}

TreeModel::SelectionFinder::SelectionFinder(const std::string& selection, int column) : 
	_selection(selection),
	_needle(0),
	_path(NULL),
	_model(NULL),
	_column(column),
	_searchForInt(false)
{}

TreeModel::SelectionFinder::SelectionFinder(int needle, int column) : 
	_selection(""),
	_needle(needle),
	_path(NULL),
	_model(NULL),
	_column(column),
	_searchForInt(true)
{}

GtkTreePath* TreeModel::SelectionFinder::getPath() {
	return _path;
}

GtkTreeIter TreeModel::SelectionFinder::getIter() {
	// The TreeIter structure to be returned
	GtkTreeIter iter;
	
	// Convert the retrieved path into a GtkTreeIter
	if (_path != NULL) {
		gtk_tree_model_get_iter_from_string(
			GTK_TREE_MODEL(_model),
			&iter,
			gtk_tree_path_to_string(_path)
		);
	}
	
	return iter;
}

// Static callback for GTK
gboolean TreeModel::SelectionFinder::forEach(
	GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, gpointer vpSelf) 
{
	// Get the self instance from the void pointer
	TreeModel::SelectionFinder* self = 
		reinterpret_cast<TreeModel::SelectionFinder*>(vpSelf);

	// Store the TreeModel pointer for later reference
	self->_model = model;
		
	// If the visited row matches the texture to find, set the _path
	// variable and finish, otherwise continue to search
	
	if (self->_searchForInt) {
		if (TreeModel::getInt(model, iter, self->_column) == self->_needle) {
			self->_path = gtk_tree_path_copy(path);
			return TRUE; // finish the walk
		}
		else {
			return FALSE;
		}
	}
	else {
		// Search for string
		if (TreeModel::getString(model, iter, self->_column) == self->_selection) {
			self->_path = gtk_tree_path_copy(path);
			return TRUE; // finish the walk
		}
		else {
			return FALSE;
		}
	}
}

}
