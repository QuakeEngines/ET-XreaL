#include "SoundChooser.h"

#include "i18n.h"
#include "iuimanager.h"
#include "isound.h"
#include "imainframe.h"
#include "gtkutil/IconTextColumn.h"
#include "gtkutil/ScrolledFrame.h"
#include "gtkutil/RightAlignment.h"
#include "gtkutil/TreeModel.h"
#include "gtkutil/MultiMonitor.h"
#include "gtkutil/VFSTreePopulator.h"

#include <gtk/gtkbutton.h>
#include <gtk/gtktreestore.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkstock.h>

namespace ui
{

namespace {
	
	// Treestore enum
	enum {
		DISPLAYNAME_COLUMN,
		SOUNDNAME_COLUMN,
		ICON_COLUMN,
		N_COLUMNS
	};


	const char* SOUND_ICON = "sr_icon_sound.png";
	const char* FOLDER_ICON = "folder16.png";
}

// Constructor
SoundChooser::SoundChooser() :
	BlockingTransientWindow(_("Choose sound"), GlobalMainFrame().getTopLevelWindow()),
	_treeStore(gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF)),
	_treeView(NULL),
	_treeSelection(NULL)
{
	gtk_container_set_border_width(GTK_CONTAINER(getWindow()), 12);
	gtk_window_set_type_hint(GTK_WINDOW(getWindow()), GDK_WINDOW_TYPE_HINT_DIALOG);
    
	// Set the default size of the window
	GdkRectangle rect = gtkutil::MultiMonitor::getMonitorForWindow(GTK_WINDOW(getWindow()));
	gtk_window_set_default_size(GTK_WINDOW(getWindow()), rect.width / 2, rect.height / 2);

	// Main vbox
    GtkWidget* vbx = gtk_vbox_new(FALSE, 12);
    gtk_box_pack_start(GTK_BOX(vbx), createTreeView(), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbx), _preview, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbx), createButtons(), FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(getWindow()), vbx);
}

namespace {

/**
 * Visitor class to enumerate sound files and add them to the tree store.
 */
class SoundFileFinder :
	public SoundFileVisitor,
	public gtkutil::VFSTreePopulator,
	public gtkutil::VFSTreePopulator::Visitor
{
	gtkutil::VFSTreePopulator &_pop;

public:

	// Constructor
	SoundFileFinder(GtkTreeStore* treeStore):
		gtkutil::VFSTreePopulator(treeStore)
	{ }

	// Functor operator
	void visit(const ISoundFilePtr& sound) {
		addPath(sound->getName());
	}

	// Required visit function
	void visit(GtkTreeStore* store, GtkTreeIter* iter, 
			   const std::string& path, bool isExplicit)
	{
		// Get the display path, everything after rightmost slash
		std::string displayPath = path.substr(path.rfind("/") + 1);

		// Get the icon, either folder or skin
		GdkPixbuf* pixBuf = isExplicit
							? GlobalUIManager().getLocalPixbuf(SOUND_ICON)
							: GlobalUIManager().getLocalPixbuf(FOLDER_ICON);

		//gtk_tree_store_append(store, it, NULL);
		gtk_tree_store_set(store, iter,
						   DISPLAYNAME_COLUMN, displayPath.c_str(),
						   SOUNDNAME_COLUMN, path.c_str(),
						   ICON_COLUMN, pixBuf,
						   -1);
	}
};


} // namespace

// Create the tree view
GtkWidget* SoundChooser::createTreeView()
{
	// Tree view with single text icon column
	_treeView = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(_treeStore)));
	gtk_tree_view_append_column(
		_treeView,
		gtkutil::IconTextColumn(_("Sound"), DISPLAYNAME_COLUMN, ICON_COLUMN, false)
	);

	_treeSelection = gtk_tree_view_get_selection(_treeView);
	g_signal_connect(G_OBJECT(_treeSelection), "changed", 
					 G_CALLBACK(_onSelectionChange), this);

	// Populate the tree store with sounds


	// Add "All sounds" toplevel node
	GtkTreeIter allSounds;
	gtk_tree_store_append(_treeStore, &allSounds, NULL);
	gtk_tree_store_set(_treeStore, &allSounds,
					   DISPLAYNAME_COLUMN, "All sounds",
					   SOUNDNAME_COLUMN, "",
					   ICON_COLUMN, GlobalUIManager().getLocalPixbuf(FOLDER_ICON),
					   -1);

	// Create a TreePopulator for the tree store and pass in each of the sound names.
	gtkutil::VFSTreePopulator pop(_treeStore, &allSounds);
	SoundFileFinder finder(pop);
	GlobalSoundManager().forEachSoundFile(finder);

	// Visit the tree populator in order to fill in the column data
	SoundTreeVisitor visitor;
	pop.forEachNode(visitor);

	// Pack treeview into a ScrolledFrame and return
	return gtkutil::ScrolledFrame(_treeView);
}

// Create buttons panel
GtkWidget* SoundChooser::createButtons() {
	GtkWidget* hbx = gtk_hbox_new(TRUE, 6);

	GtkWidget* okButton = gtk_button_new_from_stock(GTK_STOCK_OK);
	GtkWidget* cancelButton = gtk_button_new_from_stock(GTK_STOCK_CANCEL);

	g_signal_connect(G_OBJECT(okButton), "clicked",
					 G_CALLBACK(_onOK), this);
	g_signal_connect(G_OBJECT(cancelButton), "clicked",
					 G_CALLBACK(_onCancel), this);

	gtk_box_pack_end(GTK_BOX(hbx), okButton, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(hbx), cancelButton, TRUE, TRUE, 0);

	return gtkutil::RightAlignment(hbx);
}

// Show and block
std::string SoundChooser::chooseSound() {
	gtk_widget_show_all(_widget);
	gtk_main();
	return _selectedSound;
}

/* GTK CALLBACKS */

// Delete dialog
gboolean SoundChooser::_onDelete(GtkWidget* w, GdkEvent* e, SoundChooser* self) {
	self->_selectedSound = "";
	gtk_main_quit();
	return false; // propagate event
}

// OK button
void SoundChooser::_onOK(GtkWidget* w, SoundChooser* self)
{
	self->destroy();
}

// Cancel button
void SoundChooser::_onCancel(GtkWidget* w, SoundChooser* self)
{
	self->_selectedShader.clear();
	self->destroy();
}

// Tree Selection Change
void SoundChooser::_onSelectionChange(GtkTreeSelection* selection, SoundChooser* self)
{
	bool isFolder = gtkutil::TreeModel::getSelectedBoolean(self->_treeSelection, IS_FOLDER_COLUMN);

	if (isFolder) 
	{
		self->_selectedSound.clear();
	}
	else
	{
		self->_selectedShader = gtkutil::TreeModel::getSelectedString(
			self->_treeSelection,
			SOUNDNAME_COLUMN
		);
	}
	
	// Notify the preview widget about the change 
	self->_preview.setSound(self->_selectedSound);
}

} // namespace
