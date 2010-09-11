#include "GroupDialog.h"

#include <gtk/gtk.h>
#include "iregistry.h"
#include "imainframe.h"
#include "iuimanager.h"
#include "ieventmanager.h"
#include "gtkutil/window/PersistentTransientWindow.h"
#include <iostream>
#include <vector>

namespace ui {
	
	namespace {
		const std::string RKEY_ROOT = "user/ui/groupDialog/";
		const std::string RKEY_WINDOW_STATE = RKEY_ROOT + "window";
		
		const std::string WINDOW_TITLE = "Entity";
	}

GroupDialog::GroupDialog() : 
	gtkutil::PersistentTransientWindow(WINDOW_TITLE, GlobalMainFrame().getTopLevelWindow(), true),
	_currentPage(0)
{
	// Create all the widgets and pack them into the window
	populateWindow();
	
	// Register this dialog to the EventManager, so that shortcuts can propagate to the main window
	
	// greebo: Disabled this, because the EntityInspector was propagating keystrokes back to the main
	//         main window, even when the cursor was focused on entry fields.
	// greebo: Enabled this again, it seems to annoy users (issue #458)
	GlobalEventManager().connectDialogWindow(GTK_WINDOW(getWindow()));
	
	// Connect the window position tracker
	_windowPosition.loadFromPath(RKEY_WINDOW_STATE);
		
	_windowPosition.connect(GTK_WINDOW(getWindow()));
	_windowPosition.applyPosition();
}

GtkWidget* GroupDialog::getDialogWindow() {
	return getWindow();
}

// Public static method to construct the instance
void GroupDialog::construct() {
	InstancePtr() = GroupDialogPtr(new GroupDialog);
	GlobalRadiant().addEventListener(InstancePtr());
}

void GroupDialog::reparentNotebook(GtkWidget* newParent) {
	// greebo: Use the reparent method, the commented code below
	// triggers an unrealise signal.
	gtk_widget_reparent(_notebook, newParent);
	return;

	/*// Find the current parent
	GtkWidget* oldParent = gtk_widget_get_parent(_notebook);

	// Add a reference to the notebook
	gtk_widget_ref(_notebook);
	gtk_container_remove(GTK_CONTAINER(oldParent), _notebook);
	gtk_container_add(GTK_CONTAINER(newParent), _notebook);
	gtk_widget_unref(_notebook);*/
}

void GroupDialog::populateWindow() {
	_notebook = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(getWindow()), _notebook);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(_notebook), GTK_POS_TOP);

	g_signal_connect(G_OBJECT(_notebook), "switch-page", G_CALLBACK(onPageSwitch), this);
}

GtkWidget* GroupDialog::getPage() {
	return gtk_notebook_get_nth_page(
		GTK_NOTEBOOK(_notebook), 
		gtk_notebook_get_current_page(GTK_NOTEBOOK(_notebook))
	);
}

std::string GroupDialog::getPageName() {
	// Get the widget
	GtkWidget* curPage = gtk_notebook_get_nth_page(
		GTK_NOTEBOOK(_notebook), 
		gtk_notebook_get_current_page(GTK_NOTEBOOK(_notebook))
	);

	// Now cycle through the list of pages and find the matching one
	for (std::size_t i = 0; i < _pages.size(); i++) {
		if (_pages[i].page == curPage) {

			// Found page. Set it to active if it is not already active.
			return _pages[i].name;
		}
	}

	// not found
	return "";
}

// Display the named page
void GroupDialog::setPage(const std::string& name) {
	for (std::size_t i = 0; i < _pages.size(); i++) {
		if (_pages[i].name == name) {

			// Found page. Set it to active if it is not already active.
			if (getPage() != _pages[i].page) {
				setPage(_pages[i].page);
			}

			// Show the window if the notebook is hosted here
			if (gtk_widget_get_parent(_notebook) == getWindow()) {
				show();
			}
			
			// Don't continue the loop, we've found the page
			break;
		}
	}
}

void GroupDialog::setPage(GtkWidget* page) {
	_currentPage = gtk_notebook_page_num(GTK_NOTEBOOK(_notebook), page);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(_notebook), gint(_currentPage));
}

void GroupDialog::togglePage(const std::string& name) {
	// We still own the notebook in this dialog
	if (getPageName() != name || !GTK_WIDGET_VISIBLE(getWindow())) {
		// page not yet visible, show it
		setPage(name);
	}
	else {
		// page is already active, hide the dialog
		hideDialogWindow();
	}
}

GroupDialogPtr& GroupDialog::InstancePtr() {
	static GroupDialogPtr _instancePtr;
	return _instancePtr;
}

// Public method to retrieve the instance
GroupDialog& GroupDialog::Instance() {
	if (InstancePtr() == NULL) {
		construct();
	}
	return *InstancePtr();
}

void GroupDialog::showDialogWindow() {
	show();
}

void GroupDialog::hideDialogWindow() {
	hide();
}

// Public static method to toggle the window visibility
void GroupDialog::toggle() {
	if (Instance().isVisible())
		Instance().hide();
	else
		Instance().show();
}

// Pre-hide callback from TransientWindow
void GroupDialog::_preHide() {
	if (isVisible()) {
		// Save the window position, to make sure
		_windowPosition.readPosition();
	}
	
	// Tell the position tracker to save the information
	_windowPosition.saveToPath(RKEY_WINDOW_STATE);
}

// Pre-show callback from TransientWindow
void GroupDialog::_preShow() {
	// Restore the position
	_windowPosition.applyPosition();
}

// Post-show callback from TransientWindow
void GroupDialog::_postShow() {
	// Unset the focus widget for this window to avoid the cursor 
	// from jumping into any entry fields
	gtk_window_set_focus(GTK_WINDOW(getWindow()), NULL);
}

void GroupDialog::onRadiantShutdown() {
	hide();
	
	GlobalEventManager().disconnectDialogWindow(GTK_WINDOW(getWindow()));

	// Call the PersistentTransientWindow::destroy chain
	destroy();
}

GtkWidget* GroupDialog::addPage(const std::string& name,
								const std::string& tabLabel, 
								const std::string& tabIcon, 
								GtkWidget* page, 
								const std::string& windowLabel,
								const std::string& insertBefore) 
{
	// Make sure the notebook is visible before adding pages
	gtk_widget_show(_notebook);
	
	// Create the icon GtkImage and tab label
	GtkWidget* icon = gtk_image_new_from_pixbuf(GlobalUIManager().getLocalPixbuf(tabIcon));
	GtkWidget* label = gtk_label_new(tabLabel.c_str());

	// Pack into an hbox to create the title widget	
	GtkWidget* titleWidget = gtk_hbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(titleWidget), icon, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(titleWidget), label, FALSE, FALSE, 0);
	gtk_widget_show_all(titleWidget);
	
	// Show the child page before adding it to the notebook (GTK recommendation)
	gtk_widget_show(page);
	
	// Create the notebook page
	gint position = -1;
	Pages::iterator insertIter = _pages.end();

	if (!insertBefore.empty())
	{
		// Find the page with that name
		for (Pages::iterator i = _pages.begin(); i != _pages.end(); ++i) 
        {
			// Skip the wrong ones
			if (i->name != insertBefore) continue;

			// Found, extract the tab position and break the loop
			position = gtk_notebook_page_num(GTK_NOTEBOOK(_notebook), i->page);
			insertIter = i;
			break;
		}
	}

	GtkWidget* notebookPage = gtk_notebook_get_nth_page(
		GTK_NOTEBOOK(_notebook), 
		gtk_notebook_insert_page(GTK_NOTEBOOK(_notebook), page, titleWidget, position)
	);

	// Add this page to the local list
	Page newPage;
	newPage.name = name;
	newPage.page = notebookPage;
	newPage.title = windowLabel;
	
	_pages.insert(insertIter, newPage);

	return notebookPage;
}

void GroupDialog::removePage(const std::string& name) {
	// Find the page with that name
	for (Pages::iterator i = _pages.begin(); i != _pages.end(); ++i) {
		// Skip the wrong ones
		if (i->name != name) continue;

		// Remove the page from the notebook
		gtk_notebook_remove_page(
			GTK_NOTEBOOK(_notebook), 
			gtk_notebook_page_num(GTK_NOTEBOOK(_notebook), i->page)
		);

		// Remove the page and break the loop, iterators are invalid
		_pages.erase(i);
		break;
	}
}

void GroupDialog::updatePageTitle(unsigned int pageNumber) {
	if (pageNumber < _pages.size()) {
		gtk_window_set_title(GTK_WINDOW(getWindow()), _pages[pageNumber].title.c_str());
	}
}

gboolean GroupDialog::onDelete(GtkWidget* widget, GdkEvent* event, GroupDialog* self) {
	// Toggle the visibility of the inspector window
	self->toggle();
	
	// Don't propagate the delete event
	return TRUE;
}

gboolean GroupDialog::onPageSwitch(GtkWidget* notebook, GtkWidget* page, guint pageNumber, GroupDialog* self) {
	self->updatePageTitle(pageNumber);
	return FALSE;
}

} // namespace ui
